#include <vector> 
#include <algorithm>

#include "SimDC3D-Server.h"

Server::Server(unsigned int cpuGen, long int bandwidth, long int memory, int ch, int sv, map <VirtualMachine*, std::string> *vmDataFlowKeyVM)
{
	idChassi = ch;
	idServer = sv;
	clock = 0;
	howManySecondsOverEmergencyTemp = 0;
	howManyTimesDVFSChanged = 0;
	currentPerformanceStateOutof100 = 100;
	currentPowerDraw = 0.0;
	currentFanPowerDraw = 0.0;
	currentCPUPowerFactor = 1.0;
	currentPerformanceFactor = 1.0;
	timeServerOverLoaded = 0; 
	timeServerPowerOn = 0;
	averageCPU = 0.00;
	varianceCPU = 0.00;
	standardDeviationCPU = 0.00;

	rack = 0;
	totalVMsMigrate = 0;
	nodeFNSS = ' ';
	ToR = ' ';
	predictedTime = 0;
	clockPredicted = 0;



	isON = false;
	isOFF = false;
	isPOOL = false;
	isMigrating = false;
	isInitializing = false;
	isEnding = false;
	isHibernating = false;
	isSuperHeatedInTheServerPOOL = false;
	CPUpredicted = false;

	vmDestinationDataFlowKeyVM = vmDataFlowKeyVM;

	for (int i=0; i<SIZE_OF_HEAT_TIMING_BUFFER; ++i) {
		additionalHeatTimingBuffer[i] = 0.0;
		supplyTempTimingBuffer[i] = LOWEST_SUPPLY_TEMPERATURE;
	}

	cpuGeneration = cpuGen;

	switch (NUMBER_OF_CORES_IN_ONE_SERVER) {
	case 2: case 4:
		cpuTDP = 80.0; // Xeon E5502 / Xeon E3-1235
		break;
	case 8:
		cpuTDP = 105.0; // Xeon E3-2820
		break;
	case 10:
		cpuTDP = 130.0; // Xeon E7-2850
		break;
	case 16:
		cpuTDP = 210.0; // 8 cores X2
		break;
	default:
		cpuTDP = 100.0;
	}
	if (cpuTDP < 1.0)
		cout << "Error: cpu tdp is less than 1W" << endl;

	for (unsigned int i=0; i<cpuGeneration; ++i) {
		cpuTDP *= 0.8;
	}

	nonLinearEnergyConsumption[0] = 86;
	nonLinearEnergyConsumption[1] = 89.4;
	nonLinearEnergyConsumption[2] = 92.6;
	nonLinearEnergyConsumption[3] = 96;
	nonLinearEnergyConsumption[4] = 99.5;	
	nonLinearEnergyConsumption[5] = 102;
	nonLinearEnergyConsumption[6] = 106;
	nonLinearEnergyConsumption[7] = 108;
	nonLinearEnergyConsumption[8] = 112;
	nonLinearEnergyConsumption[9] = 114;
	nonLinearEnergyConsumption[10] = 117;

	coolerMaxRPM = 3000.0;
	coolerMinRPM = 500.0;
	coolerMaxPower = 15.0;
	coolerMaxDieTemperature = 70.0;
	coolerMinTemperatureGap = coolerMaxDieTemperature - EMERGENCY_TEMPERATURE;

	memServer = memory;
	bandwidthServer = bandwidth;
	errorPrediction = 0;
	hitPrediction = 0;
	RMSE = 0.00;
	numberElements=0;
	errorMean = 0.0;
	powerOnServer = 0;
	powerOffServer = 0;

	sumDownTimeVM = 0.00;
	sumLeaseTimeVM = 0;
	timeServerOverLoaded = 0; 
	timeServerPowerOn = 0;
	averagetrafficKBPS = 0;


    utilizationCPU.clear();
	temperatureServer.clear();
	temperaturePredictionServer.clear();
	timePredictionServer.clear();
    varianceErrorMean.clear();
	vRunningVMs.clear();
    vFinishedVMs.clear();
	trafficKBPS.clear();


}

Server::~Server(void)
{
    utilizationCPU.clear();
	temperatureServer.clear();
	temperaturePredictionServer.clear();
	timePredictionServer.clear();
    varianceErrorMean.clear();
	vRunningVMs.clear();
    vFinishedVMs.clear();
	trafficKBPS.clear();
}

void Server::TurnOFF()
{
	isON = false;
	isOFF = true;
	isMigrating = false;
	isPOOL = false;
	isInitializing = false;
	isEnding = false;
	isSuperHeatedInTheServerPOOL = false;
	isHibernating = false;

    pTopologyServer->disconnectLink(nodeFNSS, ToR);
	trafficKBPS.clear();
	utilizationCPU.clear();
}

void Server::TurnON(int clockSimul)
{
	isON = false;
	isOFF = false;
	isMigrating = false;
	isPOOL = false;
	isInitializing = true;
	isEnding = false;
	isHibernating = false;
	isSuperHeatedInTheServerPOOL = false;

	powerOnServer = clockSimul + POWER_ON;
	pTopologyServer->connectLink(nodeFNSS, ToR);
}

void Server::ServerRead()
{
	isON = true;
	isOFF = false;
	isMigrating = false;
	isPOOL = false;
	isInitializing = false;
	isEnding = false;
	isHibernating = false;
	isSuperHeatedInTheServerPOOL = false;
}

void Server::EnterHibernation()
{
	isON = false;
	isOFF = false;
	isMigrating = false;
	isPOOL = false;
	isInitializing = false;
	isEnding = false;
	isHibernating = true;
	isSuperHeatedInTheServerPOOL = false;

	trafficKBPS.clear();
	utilizationCPU.clear();
}

void Server::ExitHibernation(int clockSimul)
{
	isON = false;
	isOFF = false;
	isMigrating = false;
	isPOOL = false;
	isInitializing = true;
	isEnding = false;
	isHibernating = false;
	isSuperHeatedInTheServerPOOL = false;
	
	powerOnServer = clockSimul + HIBERNATING_POWER_ON;
}

void Server::FinishInitialization()
{
	isON = true;
	isInitializing = false;
}


void Server::AddPOOL(void)
{
  isON = true;
  isPOOL = true;
}

void Server::RemovePOOL(void)
{
  isPOOL = false;
  isON = true;
}

void Server::TimePowerOffServer (int clockSimul)
{
  isON = false;
  isEnding = true;
  isOFF = false;
  isPOOL = false;
  isMigrating = false;
  isInitializing = false;
  isHibernating = false;

  powerOffServer = clockSimul + POWER_OFF;

}

void Server::TimeHibernatingServer (int clockSimul)
{
  isON = false;
  isEnding = true;
  isOFF = false;
  isPOOL = false;
  isMigrating = false;
  isInitializing = false;
  isHibernating = false;

  powerOffServer = clockSimul + HIBERNATING_POWER_OFF;

}


FLOATINGPOINT Server::VMRequiresThisMuchUtilization()
{
	FLOATINGPOINT sum = 0.0;

	if ((isOFF) || (isHibernating)) {
	   return 0.0;
	}
	if (isInitializing) {
  	   return 0.30; // searching the average use of a cpu
	}
	if (isEnding) {
		return 0.10;
	}

	for (vector<VirtualMachine *>::iterator it = vRunningVMs.begin(); it != vRunningVMs.end(); ++it) {
  		sum += (*it)->HowMuchCPULoadWillThisVMRequire(); // retorna cpuLoadRatio
	}
	return sum/NUMBER_OF_CORES_IN_ONE_SERVER; // this can be more than (1.0)
}

FLOATINGPOINT Server::VMRequiresThisMuchCPUScale()
{
	if ((isOFF) || (isHibernating)) {
	   return 0.0;
	}
	if (isInitializing) {
		return 30;  // searching the average use of a cpu
	}

	if (isEnding) {
		return 10;
	}

	FLOATINGPOINT sum = 0.0;

	for (vector<VirtualMachine *>::iterator it = vRunningVMs.begin(); it != vRunningVMs.end(); ++it)
		sum += (*it)->HowMuchCPULoadWillThisVMRequire(); // retorna cpuLoadRatio

	return sum; // this can be more than (NUMBER_OF_CORES_IN_ONE_SERVER)
}

long int Server::VMRequiresThisMemory()
{
	if ((isOFF) || (isHibernating) || (isEnding)) {
	   return 0;
	}

	long int sum = 0;
	for (vector<VirtualMachine *>::iterator it = vRunningVMs.begin(); it != vRunningVMs.end(); ++it)
		sum += (*it)->GetMemUseVM();
	return sum; 
}

FLOATINGPOINT Server::GetPowerDraw()
{
	if (isOFF)  {
	   return 0.0;
	}
	if (isHibernating) {
		return POWER_CONSUMPTION_HIBERNATING;
	}
	else {
	   return currentPowerDraw;
	}
}

FLOATINGPOINT Server::GetFanPower()
{
	if ((isOFF) || (isHibernating)) {
	   return 0.0;
	}
	else {
	   return currentFanPowerDraw;
	}
}

FLOATINGPOINT Server::MaxUtilization()
{
	if ((isOFF) || (isHibernating)) {
	   return 0.0;
	}

	return (FLOATINGPOINT)currentPerformanceStateOutof100/100.0;
}

FLOATINGPOINT Server::CurrentUtilization()
{
	if ((isOFF) || (isHibernating)) {
	   return 0.0;
	}

	FLOATINGPOINT required = VMRequiresThisMuchUtilization();
	FLOATINGPOINT max = MaxUtilization();
	return ((required > max) ? max : required);
}
//*------------------------------------------------------------------------------------------------------------------
void Server::EveryASecond(int clockSimul) 
{
	if (isOFF) {
		ClockPlusPlus();
		return;
	}

	if (isHibernating) {
		CalculatePowerDraw(CurrentUtilization(), CurrentInletTemperature());
		ClockPlusPlus();
		return;
	}


	if ((isPOOL) || (isInitializing) || (isEnding)) {
		// Recalculate how much this server can perform
	    RecalculatePerformanceByTemperature();
		CalculatePowerDraw(CurrentUtilization(), CurrentInletTemperature());
		ClockPlusPlus();
		return;
	}

	// It calculates the time that the server was connected
    timeServerPowerOn += 1;

	// Recalculate how much this server can perform
	RecalculatePerformanceByTemperature();

	// run VMs
	FLOATINGPOINT sumUtil = VMRequiresThisMuchUtilization();
	FLOATINGPOINT maxUtil = MaxUtilization();

	// Save workload CPU
	InsertWorkLoadCPU(sumUtil);

	// Save traffic Server
	InsertServerTrafficKBPS(ReturnServerTrafficKBPS());


	//  Run CPU Prediction
	if (USING_PREDICTION_TO_OVERLOAD) {
	   if (!isMigrating) {
		   if (HasVMs()) {
		      if (sumUtil > 0.75) { 
				  if (clock > predictedTime) {
 			         CPUpredicted = IsHostOverUtilized(PREDICTION_ALGORITHM_OVERLOAD);
				     predictedTime = 0;
				  }
		      }
		      else {
			     CPUpredicted = false;
				 if (clock > (predictedTime)) {
					predictedTime = 0;
				 }
		      }
		   }
		   else {
   		      CPUpredicted = false;
			  if (clock > (predictedTime)) {
				 predictedTime = 0;
			  }
		   }
	   }
	   else {
		   CPUpredicted = false;
		   if (clock > (predictedTime)) {
			  predictedTime = 0;
		   }
	   }
	}

	//Calculate Standard Deviation CPU
	CalculatingStandardDeviationCPU();

	if (SCHEDULING_WITH_PREDICTION || OPTIMIZATION_WITH_PREDICTION) {
	   // Monitor utilization CPU/Temperature
	   if (clockSimul%MONITORINGTIME == 0) {
	      InsertVectorTemperature(CurrentInletTemperature());
 	   }
	}

	if (sumUtil <= maxUtil) { // run every VM for a sec
		for (vector<VirtualMachine *>::iterator it = vRunningVMs.begin(); it != vRunningVMs.end(); ++it) {
			(*it)->RunVMAndReturnActualTime(1.0, idChassi, idServer);
		}
	}
	else { // partially run every VM
		// It calculates the time that the server was overloaded
		timeServerOverLoaded += 1; 
        // partially executes each vm
		for (vector<VirtualMachine *>::iterator it = vRunningVMs.begin(); it != vRunningVMs.end(); ++it) {
			(*it)->RunVMAndReturnActualTime(maxUtil/sumUtil, idChassi, idServer);
		}
	}


	// detect finished VMs. TODO: this goto statement is correct but ugly
	BEFORE_DEFINING_ITERATOR:
	for (vector<VirtualMachine *>::iterator it = vRunningVMs.begin(); it != vRunningVMs.end(); ++it) {
		if ((*it)->IsFinished()) {
		   vFinishedVMs.push_back((*it));
		   vRunningVMs.erase(it);
		   goto BEFORE_DEFINING_ITERATOR;
		}
	}

	for (vector<VirtualMachine *>::iterator it = vRunningVMs.begin(); it != vRunningVMs.end(); ++it) {
		if (DYNAMIC_WORKLOAD_VMS) {
		   (*it)->GenerateCPUUtilization(clock);
		}
	}

	CalculatePowerDraw(CurrentUtilization(), CurrentInletTemperature());

	ClockPlusPlus();
}

void Server::ClockPlusPlus()
{
	// empty current additionalHeatTimingBuffer
	unsigned int slotIndex = clock % SIZE_OF_HEAT_TIMING_BUFFER;
	additionalHeatTimingBuffer[slotIndex] = 0.0;

	clock++;
}


VirtualMachine* Server::InsertNewVM(VirtualMachine* VM, FLOATINGPOINT timeM)
{
   
  if ((isOFF) || (isHibernating) || (isEnding) || (isPOOL) || (isInitializing)) {
     cout << "Error: Get Finished VM Vector called to a turned off or pool or initializing server !!!" << " Power OFF = " << isOFF << " Hibernating = " << isHibernating << " Ending = " << isEnding << " is POOL = " << isPOOL << " is Initializing = " << isInitializing << endl;
  }
  else {
	  isMigrating = true;
	  vRunningVMs.push_back(new VirtualMachine(*VM)); // The objects of the VMs are different because of variables isMove and timeMoveFinish
	  vRunningVMs[vRunningVMs.size()-1]->CalculateMigrationDowntime(bandwidthServer);
	  vRunningVMs[vRunningVMs.size()-1]->InsertTimeMigrationDestination(timeM);
	  vRunningVMs[vRunningVMs.size()-1]->InsertTimeOfTheLastMigration(clock);
      // When a VM migrates updating data flow information
	  if (SIMULATES_NETWORK) {
		 //cout << " idChassi " << idChassi << " idServer " << idServer << " nodeFNSS " << nodeFNSS << " VM " << vRunningVMs[vRunningVMs.size()-1] << endl;
		 vRunningVMs[vRunningVMs.size()-1]->UpdateDataFlow(idChassi, idServer, nodeFNSS);
		
		 (*vmDestinationDataFlowKeyVM).insert(pair<VirtualMachine*, string>(vRunningVMs[vRunningVMs.size()-1],vRunningVMs[vRunningVMs.size()-1]->ReturnNodeFNSSVM()) );
		 
		 //cout << "Inserindo a VM no Vetor vmDestinationFlowKeyVM " <<  vRunningVMs[vRunningVMs.size()-1] << " chassi " << vRunningVMs[vRunningVMs.size()-1]->ReturnChassi() << " server " << vRunningVMs[vRunningVMs.size()-1]->ReturnServer() << endl;

	  }

	  CalculatePowerDraw(CurrentUtilization(), CurrentInletTemperature());
  }

  return vRunningVMs[vRunningVMs.size()-1];
}

void Server::MoveVMTo(Server* (*mserver)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], int chassiDestination, int serverDestination, VirtualMachine* VMSelected)
{
   FLOATINGPOINT timeMigration = 0;
   VirtualMachine* newVirtualMachine_;

   timeMigration = clock + VMSelected->CalculateMigrationTime(bandwidthServer);
  
  //creates a replica of the target vm
    //cout << " Vm atual "  << VMSelected << " clock " << clock << " Time Migration " << VMSelected->CalculateMigrationTime(bandwidthServer) << " timeMigration " << timeMigration << endl;
    //cout << " chassiDestination " << chassiDestination << " serverDestination " << serverDestination << endl;
	newVirtualMachine_ = (*mserver)[chassiDestination][serverDestination]->InsertNewVM(VMSelected, timeMigration);

    if (CPUpredicted) {
	   predictedTime = clock + SIZE_WINDOWN_PREDICTION_CPU;
    }
    
	// initializes the migration source and set the end time of migration on the source VM
	isMigrating = true;
	for (unsigned int i=0; i < vRunningVMs.size(); i++){
	    if (vRunningVMs[i] == VMSelected) {
		   vRunningVMs[i]->InsertTimeMigrationSource(timeMigration);
		   vRunningVMs[i]->InsertNewVM(newVirtualMachine_);
		   break;
		}
	}

    totalVMsMigrate ++;
 }

vector<VirtualMachine *> Server::GetALLVMs(void)
{
  vector<VirtualMachine *> VMsTemp;
  
  if ((isOFF) || (isHibernating) || (isEnding) || (isPOOL) || (isInitializing)) {
     cout << "Error: Get Finished VM Vector called to a turned off or pool or initializing server !!!" << " Power OFF = " << isOFF << " Hibernating = " << isHibernating << " Ending = " << isEnding << " is POOL = " << isPOOL << " is Initializing = " << isInitializing << endl;
 	 return VMsTemp;
  }

  for(unsigned int i=0; i < vRunningVMs.size(); i++){
	   if ( (!vRunningVMs[i]->IsFinished()) && (!vRunningVMs[i]->ReturnIsMove()) ) {
		  VMsTemp.push_back(vRunningVMs[i]);
	  }
  }

  return VMsTemp;
}

vector<VirtualMachine *> Server::GetNVMs(unsigned int N, string Algortimo_Selection)
{
  vector<VirtualMachine *> VMTemp;
  int numSort = 0;
  unsigned int sumSort = 0;
  srand( (unsigned)time(NULL) );
  unsigned int ind = 0;
  unsigned int indVM = 0;

  if ((isOFF) || (isHibernating) || (isEnding) || (isPOOL) || (isInitializing)) {
     cout << "Error: Get Finished VM Vector called to a turned off or pool or initializing server !!!" << " Power OFF = " << isOFF << " Hibernating = " << isHibernating << " Ending = " << isEnding << " is POOL = " << isPOOL << " is Initializing = " << isInitializing << endl;
	 return VMTemp;
  }

  if (VMS_ALGORITHM_SELECTION == "HIGHER_UTILIZATION_CPU/LESS_MEMORY") {
     sort(vRunningVMs.begin(), vRunningVMs.end(), Sort_HigherCPU_LessMemory);
  }
  else {
     if (VMS_ALGORITHM_SELECTION == "HIGHER_UTILIZATION_AVERAGE_CPU/LESS_MEMORY") {
        sort(vRunningVMs.begin(), vRunningVMs.end(), Sort_HigherAverageCPU_LessMemory);
     }
     else {
		if (VMS_ALGORITHM_SELECTION == "HIGHER_UTILIZATION_AVERAGE_CPU/LESS_MEMORY/HIGHER_IO_NETWORK") {
		   sort(vRunningVMs.begin(), vRunningVMs.end(), Sort_HigherAverageCPU_LessMemory_HigherNetwork);
        }
        else {
           if (VMS_ALGORITHM_SELECTION == "HIGHER_UTILIZATION_AVERAGE_CPU") {
			  sort(vRunningVMs.begin(), vRunningVMs.end(), Sort_HigherAverageCPU);
		   }
		   else {
		      if (VMS_ALGORITHM_SELECTION == "LESS_MEMORY") {
				 sort(vRunningVMs.begin(), vRunningVMs.end(), Sort_LessMemory);
			  }
			  else {
				  if (VMS_ALGORITHM_SELECTION == "HIGHER_IO_NETWORK") {
				     sort(vRunningVMs.begin(), vRunningVMs.end(), Sort_Higher_IO_Network);
				  }
			  }
		   }		  
		}
	 }	   
  }		   
		   
  if (N <= vRunningVMs.size()) {
	  while (ind < N) {
		    if (VMS_ALGORITHM_SELECTION == "RANDOM_SELECTION") {
			   sumSort = 0;
			   while (sumSort < vRunningVMs.size()) {
				     numSort = rand() % vRunningVMs.size();
					 if ((!vRunningVMs[numSort]->IsFinished()) && (!vRunningVMs[numSort]->ReturnIsMove()) ) {	
						VMTemp.push_back(vRunningVMs[numSort]);
						break;
					 }
					 sumSort++;
			   }
               ind += 1;
		   }
		   else {
			   indVM = 0;
			   while (indVM < vRunningVMs.size()) {
				     if ( (!vRunningVMs[indVM]->IsFinished()) && (!vRunningVMs[indVM]->ReturnIsMove()) )  {
    		            VMTemp.push_back(vRunningVMs[indVM]);
						break;
					 }
					 indVM += 1;
			  }
              ind +=1;
     	  } 
	  }
  }
  return VMTemp;
}


void Server::CheckFinishMove(int clockSim)
{
  if ((isOFF) || (isHibernating) || (isEnding) || (isPOOL) || (isInitializing)) {
     cout << "Error: Get Finished VM Vector called to a turned off or pool or initializing server !!!" << " Power OFF = " << isOFF << " Hibernating = " << isHibernating << " Ending = " << isEnding << " is POOL = " << isPOOL << " is Initializing = " << isInitializing << endl;
     return; 
  }

  INI:
  for (vector<VirtualMachine *>::iterator it = vRunningVMs.begin(); it != vRunningVMs.end(); ++it) {
      if ((*it)->ReturnIsMove()) {
		 if ((*it)->ReturnTimeMigrationDestination() == clockSim) {
   	        (*it)->ChangeTimeMigrationDestination(0.0); 
			(*it)->SetIsMove(false);
	     } 	 
		 if ((*it)->ReturnTimeMigrationSource() == clockSim){
			vFinishedVMs.push_back((*it));
			vRunningVMs.erase(it);
			goto INI;
		 }
	  }
  }
  
  CalculatePowerDraw(CurrentUtilization(), CurrentInletTemperature());
}

bool Server::HasVMsToMigrate(void)
{
  if ((isOFF) || (isHibernating) || (isEnding) || (isPOOL) || (isInitializing)) {
     cout << "Error: Get Finished VM Vector called to a turned off or pool or initializing server !!!" << " Power OFF = " << isOFF << " Hibernating = " << isHibernating << " Ending = " << isEnding << " is POOL = " << isPOOL << " is Initializing = " << isInitializing << endl;
  }

  for (vector<VirtualMachine *>::iterator it = vRunningVMs.begin(); it != vRunningVMs.end(); ++it) {
      if ( (*it)->ReturnIsMove()) {
		 return true;
	  }
  }
  return false;
}

bool Server::HasVMs(void)
{
	if (vRunningVMs.size() > 0) {
	   return true;
	}
	else {
	   return false;
	}
}

unsigned int Server::HowManyVMs(void)
{
	return vRunningVMs.size();
}

void Server::AssignOneVM(VirtualMachine *vm)
{
	if ((isOFF) || (isHibernating) || (isEnding) || (isPOOL) || (isInitializing)) {
   	   cout << "Error: Get Finished VM Vector called to a turned off or pool or initializing server !!!" << " Power OFF = " << isOFF << " Hibernating = " << isHibernating << " Ending = " << isEnding << " is POOL = " << isPOOL << " is Initializing = " << isInitializing << endl;
	}

	vRunningVMs.push_back(vm);

	CalculatePowerDraw(CurrentUtilization(), CurrentInletTemperature());

	if (SIMULATES_NETWORK) {
   	   // create Flow VM
	   vRunningVMs[vRunningVMs.size()-1]->CreateFlowVM(idChassi, idServer, nodeFNSS, pTopologyServer, pTrafficMatrixSequence); // create flow intra data center
	}
}


VirtualMachine* Server::TakeAVM()
{
  if ((isOFF) || (isHibernating) || (isEnding) || (isPOOL) || (isInitializing)) {
     cout << "Error: Get Finished VM Vector called to a turned off or pool or initializing server !!!" << " Power OFF = " << isOFF << " Hibernating = " << isHibernating << " Ending = " << isEnding << " is POOL = " << isPOOL << " is Initializing = " << isInitializing << endl;
  }

  if (vRunningVMs.empty()) {
	  return NULL;
  }
  
  VirtualMachine* retVal = vRunningVMs.back();
  
  vRunningVMs.pop_back();
  
  CalculatePowerDraw(CurrentUtilization(), CurrentInletTemperature());
 
  return retVal;
}

void Server::RemoveTheLastAssignedVM()
{
	if ((isOFF) || (isHibernating) || (isEnding) || (isPOOL) || (isInitializing)) {
	   cout << "Error: Get Finished VM Vector called to a turned off or pool or initializing server !!!" << " Power OFF = " << isOFF << " Hibernating = " << isHibernating << " Ending = " << isEnding << " is POOL = " << isPOOL << " is Initializing = " << isInitializing << endl;
	}
	vRunningVMs.pop_back();
}


vector<VirtualMachine *>* Server::GetFinishedVMVector()
{
	if ((isOFF) || (isHibernating) || (isEnding) || (isPOOL) || (isInitializing)) {
	   cout << "Error: Get Finished VM Vector called to a turned off or pool or initializing server !!!" << " Power OFF = " << isOFF << " Hibernating = " << isHibernating << " Ending = " << isEnding << " is POOL = " << isPOOL << " is Initializing = " << isInitializing << endl;
	}

	return &vFinishedVMs;
}

void Server::RecalculatePerformanceByTemperature()
{
	if ( (isOFF) || (isHibernating) ) {
	   cout << "Error: SetInletTemperature called to a turned off or hibernating" << endl;
	}

	int oldPerformanceStateOutof100 = currentPerformanceStateOutof100;

	FLOATINGPOINT inletTempNow = CurrentInletTemperature();

	if (inletTempNow <= (EMERGENCY_TEMPERATURE))
		currentPerformanceStateOutof100 = 100;
	else {
		currentPerformanceStateOutof100 = 100;
		howManySecondsOverEmergencyTemp++;
		if (isPOOL) { // If the machine is in the POOL and the inlet temperature this high, find for a machine off to replace it.
		   isSuperHeatedInTheServerPOOL = true;
		}
	}

	if (TEMPERATURE_SENSING_PERFORMANCE_CAPPING)
	{
		// 1/30 CPU power down for every 1'c up
		currentCPUPowerFactor = 1.0 - (inletTempNow-(EMERGENCY_TEMPERATURE))/30;
		currentPerformanceFactor = sqrt(currentCPUPowerFactor);
		//currentPerformanceFactor = currentCPUPowerFactor;
		if (currentCPUPowerFactor > 1.0)
			currentCPUPowerFactor = 1.0;
		if (currentPerformanceFactor > 1.0)
			currentPerformanceFactor = 1.0;
		currentPerformanceStateOutof100 = (int)(currentPerformanceFactor*100);
	}

	if (currentPerformanceStateOutof100 > 100)
		currentPerformanceStateOutof100 = 100;

	if (currentPerformanceStateOutof100 != oldPerformanceStateOutof100)
		howManyTimesDVFSChanged++;
}

unsigned int Server::HowManySecondsOverEmergencyTemp()
{
	return howManySecondsOverEmergencyTemp;
}

unsigned int Server::HowManyTimesDVFSChanged()
{
	return howManyTimesDVFSChanged;
}

void Server::CalculatePowerDraw(FLOATINGPOINT utilization, FLOATINGPOINT temperature)
{
	FLOATINGPOINT idlePower = 0; // assuming half power when idle
	FLOATINGPOINT currentCPUpower = 0;
	FLOATINGPOINT temperatureGap = 0;
	FLOATINGPOINT additionalFanPower = 0;
	

	if (isOFF) {
		currentPowerDraw = currentFanPowerDraw = 0.0;
		return;
	}

	if (isHibernating) {
	   currentFanPowerDraw = 0;
	   currentPowerDraw = POWER_CONSUMPTION_HIBERNATING;
	   return;
	}

	if (MODEL_OF_ENERGY_CONSUMPTION == "LINEAR_MODEL") {
	   idlePower = cpuTDP; // assuming half power when idle
	   currentCPUpower = cpuTDP*utilization;
	   temperatureGap = coolerMaxDieTemperature - temperature;
	   
   	   if (CONSTANT_FAN_POWER) {
		  currentPowerDraw = currentCPUpower + idlePower;
		  currentFanPowerDraw = coolerMaxPower*2; // two fans
		  return;
	   }

	   // cpu cooling
	   {
		FLOATINGPOINT targetRPM = (currentCPUpower/temperatureGap)/(cpuTDP/coolerMinTemperatureGap)*coolerMaxRPM;
		if (targetRPM < coolerMinRPM)
			targetRPM = coolerMinRPM;
		if (!FAN_RPM_NO_LIMIT) {
			if (targetRPM > coolerMaxRPM)
				targetRPM = coolerMaxRPM;
		}
		FLOATINGPOINT fanFactor = (targetRPM/coolerMaxRPM)*(targetRPM/coolerMaxRPM)*(targetRPM/coolerMaxRPM); // ? porque 3x
		additionalFanPower = coolerMaxPower * fanFactor;
	   }

	   // case cooling (two fans)
	   {
		FLOATINGPOINT targetRPM = (currentCPUpower/cpuTDP*(coolerMaxRPM-coolerMinRPM) + coolerMinRPM /* -> for removing idle power */ ) * temperature/EMERGENCY_TEMPERATURE;
		if (targetRPM < coolerMinRPM)
			targetRPM = coolerMinRPM;
		if (!FAN_RPM_NO_LIMIT) {
			if (targetRPM > coolerMaxRPM)
				targetRPM = coolerMaxRPM;
		}
		FLOATINGPOINT fanFactor = (targetRPM/coolerMaxRPM)*(targetRPM/coolerMaxRPM)*(targetRPM/coolerMaxRPM);
		additionalFanPower += 2 *(coolerMaxPower * fanFactor);
	   }
	   currentPowerDraw = currentCPUpower + idlePower + additionalFanPower;
	   currentFanPowerDraw = additionalFanPower;
	}

	/**
	* The abstract class of power models created based on data from SPECpower benchmark:
	* http://www.spec.org/power_ssj2008/
	* 
	* If you are using any algorithms, policies or workload included in the power package, please cite
	* the following paper:
	* 
	* Anton Beloglazov, and Rajkumar Buyya, "Optimal Online Deterministic Algorithms and Adaptive
	* Heuristics for Energy and Performance Efficient Dynamic Consolidation of Virtual Machines in
	 * Cloud Data Centers", Concurrency and Computation: Practice and Experience (CCPE), Volume 24,
	* Issue 13, Pages: 1397-1420, John Wiley & Sons, Ltd, New York, USA, 2012
	* 
	* @author Anton Beloglazov
	* @since CloudSim Toolkit 3.0
	*/

	if (MODEL_OF_ENERGY_CONSUMPTION == "POWERMODELSPECPOWER") {

		if (utilization < 0 || utilization > 1) {
		   cout << "ERROR !!! Utilization value must be between 0 and 1" << endl;
		   return; 
		}
		if (( (int) (utilization*10) % 10) == 0) {
			currentCPUpower = nonLinearEnergyConsumption[(int) (utilization * 10)];
		}
		int utilization1 = (int) floor(utilization * 10);
		int utilization2 = (int) ceil(utilization * 10);
		double power1 = nonLinearEnergyConsumption[utilization1];
		double power2 = nonLinearEnergyConsumption[utilization2];
   		double delta = (power2 - power1) / 10;
		currentCPUpower = power1 + delta * (utilization - (double) utilization1 / 10) * 100;
	

	    if (CONSTANT_FAN_POWER) {
		   currentPowerDraw = currentCPUpower;
		   currentFanPowerDraw = coolerMaxPower*2; // two fans
		   return;
	    }

	    // cpu cooling
	    {
		 FLOATINGPOINT targetRPM = (currentCPUpower/temperatureGap)/(nonLinearEnergyConsumption[0]/coolerMinTemperatureGap)*coolerMaxRPM;
	 	 if (targetRPM < coolerMinRPM)
			targetRPM = coolerMinRPM;
		 if (!FAN_RPM_NO_LIMIT) {
			if (targetRPM > coolerMaxRPM)
				targetRPM = coolerMaxRPM;
		 }
		FLOATINGPOINT fanFactor = (targetRPM/coolerMaxRPM)*(targetRPM/coolerMaxRPM)*(targetRPM/coolerMaxRPM); // ? porque 3x
		additionalFanPower = coolerMaxPower * fanFactor;
	   }

	   // case cooling (two fans)
	   {
		FLOATINGPOINT targetRPM = (currentCPUpower/nonLinearEnergyConsumption[0]*(coolerMaxRPM-coolerMinRPM) + coolerMinRPM) * temperature/EMERGENCY_TEMPERATURE;
		if (targetRPM < coolerMinRPM)
			targetRPM = coolerMinRPM;
		if (!FAN_RPM_NO_LIMIT) {
			if (targetRPM > coolerMaxRPM)
				targetRPM = coolerMaxRPM;
		}
		FLOATINGPOINT fanFactor = (targetRPM/coolerMaxRPM)*(targetRPM/coolerMaxRPM)*(targetRPM/coolerMaxRPM);
		additionalFanPower += 2 *(coolerMaxPower * fanFactor);
	   }
	   
	   currentPowerDraw = currentCPUpower;
	   currentFanPowerDraw = additionalFanPower;
	} 
}


FLOATINGPOINT Server::EstimatePowerDraw(FLOATINGPOINT utilization, FLOATINGPOINT utilizationNewVM)
{
	if (isOFF) {
		return 0.00;
	}
	if (isHibernating) {
	   return POWER_CONSUMPTION_HIBERNATING;
	}

	FLOATINGPOINT idlePower = cpuTDP; // assuming half power when idle
	FLOATINGPOINT currentCPUpower = cpuTDP*(utilization+utilizationNewVM);
	FLOATINGPOINT currentPower = 0.00;

	currentPower = currentCPUpower + idlePower;
	
    return currentPower;
}


FLOATINGPOINT Server::EstimatePowerDrawWithTemperature(FLOATINGPOINT utilization, FLOATINGPOINT utilizationNewVM, FLOATINGPOINT temperature)
{
	if (isOFF) {
		return 0;
	}

	if (isHibernating) {
	   return POWER_CONSUMPTION_HIBERNATING;
	}

	FLOATINGPOINT idlePower = cpuTDP; // assuming half power when idle
	FLOATINGPOINT currentCPUpower = cpuTDP*(utilization+utilizationNewVM);
	FLOATINGPOINT temperatureGap = coolerMaxDieTemperature - temperature;
	FLOATINGPOINT additionalFanPower = 0;
	FLOATINGPOINT currentPower = 0;
	FLOATINGPOINT currentFanPower = 0;


	if (CONSTANT_FAN_POWER) {
		currentPower = currentCPUpower + idlePower;
		currentFanPower = coolerMaxPower*2; // two fans
		return (currentPower+currentFanPower);
	}

	// cpu cooling
	{
		FLOATINGPOINT targetRPM = (currentCPUpower/temperatureGap)/(cpuTDP/coolerMinTemperatureGap)*coolerMaxRPM;
		if (targetRPM < coolerMinRPM)
			targetRPM = coolerMinRPM;
		if (!FAN_RPM_NO_LIMIT) {
			if (targetRPM > coolerMaxRPM)
				targetRPM = coolerMaxRPM;
		}
		FLOATINGPOINT fanFactor = (targetRPM/coolerMaxRPM)*(targetRPM/coolerMaxRPM)*(targetRPM/coolerMaxRPM); // ? porque 3x
		additionalFanPower = coolerMaxPower * fanFactor;
	}

	// case cooling (two fans)
	{
		FLOATINGPOINT targetRPM = (currentCPUpower/cpuTDP*(coolerMaxRPM-coolerMinRPM) + coolerMinRPM /* -> for removing idle power */ ) * temperature/EMERGENCY_TEMPERATURE;
		if (targetRPM < coolerMinRPM)
			targetRPM = coolerMinRPM;
		if (!FAN_RPM_NO_LIMIT) {
			if (targetRPM > coolerMaxRPM)
				targetRPM = coolerMaxRPM;
		}
		FLOATINGPOINT fanFactor = (targetRPM/coolerMaxRPM)*(targetRPM/coolerMaxRPM)*(targetRPM/coolerMaxRPM);
		additionalFanPower += 2 *(coolerMaxPower * fanFactor);
	}
	currentPower = currentCPUpower + idlePower + additionalFanPower;
	return currentPower;
}

bool Server::IsFinished()
{
	if ((isOFF) || (isHibernating)) {
	    return true;
	}

	if (!vRunningVMs.empty()) {
	   return false;
	}

	return true;
}

void Server::AddHeatToTimingBuffer(FLOATINGPOINT temperature, int timing)
{
	unsigned int slotIndex = (clock + timing) % SIZE_OF_HEAT_TIMING_BUFFER;
	additionalHeatTimingBuffer[slotIndex] += temperature;
}

void Server::SetSupplyTempToTimingBuffer(FLOATINGPOINT temperature, int timing)  
{
	unsigned int slotIndex = (clock + timing) % SIZE_OF_HEAT_TIMING_BUFFER;
	supplyTempTimingBuffer[slotIndex] = temperature;
}

FLOATINGPOINT Server::ReadSupplyTempToTimingBuffer()  
{
	unsigned int slotIndex = clock % SIZE_OF_HEAT_TIMING_BUFFER;
	return supplyTempTimingBuffer[slotIndex];
}

FLOATINGPOINT Server::ReadHeatFromTimingBuffer()
{
	unsigned int slotIndex = clock % SIZE_OF_HEAT_TIMING_BUFFER;
	return additionalHeatTimingBuffer[slotIndex];
}

FLOATINGPOINT Server::CurrentInletTemperature()
{
	unsigned int slotIndex = clock % SIZE_OF_HEAT_TIMING_BUFFER;
	return (supplyTempTimingBuffer[slotIndex]+additionalHeatTimingBuffer[slotIndex]);
}

FLOATINGPOINT Server::CurrentAddedTemperature()
{
	unsigned int slotIndex = clock % SIZE_OF_HEAT_TIMING_BUFFER;
	return additionalHeatTimingBuffer[slotIndex];
}

unsigned int Server::ReturnSizeVectorTemperature(void)
{
	return temperatureServer.size();
}

void Server::InsertWorkLoadCPU(FLOATINGPOINT utilCPU)
{	
	utilizationCPU.push_back(utilCPU);
	averageCPU += utilCPU;

	if (utilizationCPU.size() > SIZE_WINDOWN_PREDICTION_CPU) {  // sliding window
	   averageCPU -= utilizationCPU[0];
       utilizationCPU.erase(utilizationCPU.begin());
	} 
}

void Server::InsertServerTrafficKBPS(FLOATINGPOINT trafficServer)
{	
	trafficKBPS.push_back(trafficServer);
	averagetrafficKBPS += trafficServer;

	if (trafficKBPS.size() > SIZE_WINDOWN_PREDICTION_CPU) {  // sliding window
	   averagetrafficKBPS -= trafficKBPS[0];
       trafficKBPS.erase(trafficKBPS.begin());
	} 
}


void Server::CalculatingStandardDeviationCPU(void)
{
 FLOATINGPOINT avg = 0; 
 FLOATINGPOINT var = 0;

 avg = AverageUsageofCPU();

 for(unsigned int j=0; j < utilizationCPU.size(); j++) {
 	 var += pow(utilizationCPU[j] - avg,2);
 }

  if (utilizationCPU.size() != 0) {
	varianceCPU = var / (utilizationCPU.size()-1);
    standardDeviationCPU = sqrt(varianceCPU);
  }

}


void Server::InsertVectorTemperature(FLOATINGPOINT tempServer)
{	
	temperatureServer.push_back(tempServer);
	if (temperatureServer.size() > SIZE_WINDOWN_PREDICTION) {  // sliding window
       temperatureServer.erase(temperatureServer.begin());
	} 
}

vector<FLOATINGPOINT> Server::ReturnVectorTemperature(void)
{
	return temperatureServer;
}

void Server::InsertTemperaturePredictionServer(FLOATINGPOINT temperature)
{	
	temperaturePredictionServer.push_back(temperature);
    clockPredicted = clock;
}

void Server::InsertTimePredictionServer(unsigned int timeprediction)
{	
	timePredictionServer.push_back(timeprediction);
}

unsigned int Server::ReturnClock (void)
{
	return clock;
}

void Server::AddErrorPrediction(void)
{
   errorPrediction += 1;
}

void Server::AddHitPrediction(void)
{
   hitPrediction += 1;
}

unsigned int Server::ReturnErrorPrediction(void)
{
	return errorPrediction;
}

unsigned int Server::ReturnHitPrediction(void)
{
	return hitPrediction;
}

unsigned int Server::ReturnFirstTimePredictionServer(void)
{
	if (!timePredictionServer.empty()) {
		return timePredictionServer.front();
	}
	else {
		return -1;
	}
}

void Server::CalculateRMSE()
{
  double diff = 0.0;
  double temperatureServer = 0.00;

  temperatureServer = CurrentInletTemperature();
  
  diff = temperaturePredictionServer[0] - temperatureServer;
  RMSE += pow(diff,2);
  numberElements += 1;

  errorMean += fabs(1-(temperaturePredictionServer[0]/temperatureServer));

  varianceErrorMean.push_back(fabs(1-(temperaturePredictionServer[0]/temperatureServer)));
   
  temperaturePredictionServer.erase(temperaturePredictionServer.begin());
  timePredictionServer.erase(timePredictionServer.begin());
}

FLOATINGPOINT Server::ReturnRMSE(void)
{
  return sqrt(RMSE / numberElements);
}

FLOATINGPOINT Server::ReturnErrorMean(void)
{
  return errorMean / numberElements;
}

FLOATINGPOINT Server::ReturnVarianceErrorMean(void)
{
 FLOATINGPOINT var = 0.00;
 FLOATINGPOINT mean = 0.00;

 mean = ReturnErrorMean();

  for(unsigned int i=0; i < varianceErrorMean.size(); i++) {
      var += pow(varianceErrorMean[i] - mean,2);
  }
  
  return var / (varianceErrorMean.size()-1);
}

FLOATINGPOINT Server::ReturnSDErrorMean(void){

	return sqrt(ReturnVarianceErrorMean());
}

void Server::InsertRackInServer(int Rck)
{
  rack = Rck;
}

bool Server::IsHostOverUtilized(string prediction_Algorithm) 
{
	if(prediction_Algorithm=="LR") {
      return LR_CPU();
	}

	return false;
}

bool Server::LR_CPU(void)
{
  unsigned int length = SIZE_WINDOWN_PREDICTION_CPU; // we use 10 to make the regression responsive enough to latest values

  FLOATINGPOINT migrationIntervals = 0;
  FLOATINGPOINT predictedUtilization = 0;
  
  vector<FLOATINGPOINT> utilizationHistoryReversed;	
  
  Maths::Regression::Linear_CodeCogs *estimates;
 
  if (utilizationCPU.size() < length) {
	 return false; 
  }
  
  for (unsigned int i = 0; i < length; i++) {
	  utilizationHistoryReversed.push_back(utilizationCPU[utilizationCPU.size() - i - 1]);
  }
  
  estimates = GetParameterEstimates(utilizationHistoryReversed);
  migrationIntervals = ceil(GetMaximumVmMigrationTime() / 300);
  predictedUtilization = estimates->getValue(0) + estimates->getSlope() * ((double) length + migrationIntervals);
  predictedUtilization *= 1.2;  // Extract Beloglazov
  //cout << " predictedUtilization " << predictedUtilization << " AverageUsageofCPU() " << AverageUsageofCPU() << " ReturnStandardDeviationCPU() " << ReturnStandardDeviationCPU() << endl;  
  if (predictedUtilization >= 1) {
	 return true;
  }
  else {
     return false;
  }
}

Maths::Regression::Linear_CodeCogs* Server::GetParameterEstimates(vector<FLOATINGPOINT> y) 
{
  int n = y.size();
  vector<FLOATINGPOINT> x;
  Maths::Regression::Linear_CodeCogs *LR;

  for (int i = 0; i < n; i++) {
	  x.push_back(i + 1);
  }
  LR = CreateWeigthedLinearRegression(x, y, GetTricubeWeigts(n));
	
  return LR;
}

vector<FLOATINGPOINT> Server::GetTricubeWeigts(int n)
{
 FLOATINGPOINT top = 0;
 FLOATINGPOINT spread = 0;
 FLOATINGPOINT k = 0; 
 vector<FLOATINGPOINT> weights;

 top = (FLOATINGPOINT) n - 1;
 spread = top;

 for (int i = 2; i < n; i++) {
	  k = pow(1 - pow((top - i) / spread, 3), 3);
	  if (k > 0) {
		  weights.push_back(1 / k);
	  }
	  else {
		  weights.push_back(3.4e+38);
	  }
 }
 weights[0] = weights[1] = weights[2];

 return weights;
}

Maths::Regression::Linear_CodeCogs* Server::CreateWeigthedLinearRegression(vector<FLOATINGPOINT> x, vector<FLOATINGPOINT> y, vector<FLOATINGPOINT> weigths)
{
/* int numZeroWeigths = 0;
 FLOATINGPOINT *xW, *yW;

 xW= (FLOATINGPOINT*) malloc(sizeof(FLOATINGPOINT) * x.size()); 
 yW= (FLOATINGPOINT*) malloc(sizeof(FLOATINGPOINT) * y.size()); 


 // As to Flanagan's documentation they perform weigthed regression if the number or non-zero weigths is more than 40%
 for (int i = 0; i < weigths.size(); i++) {
   	 if (weigths[i] <= 0) {
	 	numZeroWeigths++;
	 }
 }
 
 for (int i = 0; i < x.size(); i++) {
  	 if (numZeroWeigths >= 0.4 * weigths.size()) {
		// See: http://www.ncsu.edu/crsc/events/ugw07/Presentations/Crooks_Qiao/Crooks_Qiao_Alt_Presentation.pdf
		 xW[i] = sqrt(weigths[i]) * x[i];
		 yW[i] = sqrt(weigths[i]) * y[i];
	 } else {
		 xW[i] = x[i];
		 yW[i] = y[i];
	 }
 }
 Maths::Regression::Linear *LR;

 LR = new Maths::Regression::Linear (x.size(), xW, yW);*/

 int numZeroWeigths = 0;
 vector<FLOATINGPOINT> xW, yW;

 // As to Flanagan's documentation they perform weigthed regression if the number or non-zero weigths is more than 40%
 for (unsigned int i = 0; i < weigths.size(); i++) {
   	 if (weigths[i] <= 0) {
	 	numZeroWeigths++;
	 }
 }
 
 for (unsigned int i = 0; i < x.size(); i++) {
  	 if (numZeroWeigths >= 0.4 * weigths.size()) {
		// See: http://www.ncsu.edu/crsc/events/ugw07/Presentations/Crooks_Qiao/Crooks_Qiao_Alt_Presentation.pdf
		 xW.push_back(sqrt(weigths[i]) * x[i]);
		 yW.push_back(sqrt(weigths[i]) * y[i]);
	 } else {
		 xW.push_back(x[i]);
		 yW.push_back(y[i]);
	 }
 }
 Maths::Regression::Linear_CodeCogs *LR;

 LR = new Maths::Regression::Linear_CodeCogs (x.size(), xW, yW);


 /*cout << "    Slope = " << LR->getSlope() << endl;
 cout << "Intercept = " << LR->getIntercept() << endl << endl;
 
 cout << "Regression coefficient = " << LR->getCoefficient() << endl;
 
 cout << endl << "Regression line values" << endl << endl;
 for (int i = 0; i < 10; i += 1) 
 {
  cout << "x = " << i << "  y = " << LR->getValue(i);
  cout << endl;
 }*/

 xW.erase(xW.begin(), xW.end());
 yW.erase(yW.begin(), yW.end());

 return LR;

}

FLOATINGPOINT Server::GetMaximumVmMigrationTime()
{
  long int maxRam = 0;
  long int ram = 0;
  double timeMigration = 0.0;

  for (vector<VirtualMachine *>::iterator it = vRunningVMs.begin(); it != vRunningVMs.end(); ++it) {
      ram = (*it)->GetMemUseVM();
      if (ram > maxRam) {
		  maxRam = ram;
	  }
  }

  timeMigration = (((double) maxRam * 8) / (double) bandwidthServer );

  return timeMigration;
}

void Server::InsertDownTimeANDLeaseTimeVM(long double downTimeVM, long int leaseTimeVM)
{
  sumDownTimeVM += downTimeVM;
  sumLeaseTimeVM += leaseTimeVM;

}

void Server::ReturnServer(void)
{
 cout << " Chassi " << idChassi << " Sever " << idServer << endl; 
} 

void Server::ConnectServerNode(string node, fnss::Topology* tp, TrafficMatrixSimDC3D* trafficMS)
{ 
	nodeFNSS = node; 
	pTopologyServer = tp;
	pTrafficMatrixSequence = trafficMS;
}

FLOATINGPOINT Server::ReturnAverageDowmTimeServer(void) 
{

	if (sumLeaseTimeVM == 0) {
		return 0;
	}
	else {
	    return (sumDownTimeVM / (long double) sumLeaseTimeVM);
	}

}

bool Server::ReturnCPUPrediction(void) 
{ 
  if (CPUpredicted) {
     return true;
  }
  else {
	  if (clock < predictedTime) {
		  return true;
	  }
	  else {
		  return false;
	  }
  }
}

FLOATINGPOINT Server::ReturnServerTrafficKBPS(void)
{
 float sumTraffic = 0;

  for (vector<VirtualMachine *>::iterator it = vRunningVMs.begin(); it != vRunningVMs.end(); ++it) {
      sumTraffic += (*it)->ReturnVMUseNetwork();
  }

  return (sumTraffic * 1000); // return traffic in Kbit/s

}

FLOATINGPOINT Server::AverageUsageofCPU(void) 
{ 
	if (utilizationCPU.size() == 0) {
		return 0;
	}
	else {
		return averageCPU / utilizationCPU.size();
	}
}


FLOATINGPOINT Server::AverageServerTrafficKBPS(void) 
{ 
	if (trafficKBPS.size() == 0) {
		return 0;
	}
	else {
		return averagetrafficKBPS / trafficKBPS.size();
	}
}

FLOATINGPOINT Server::CalculationRanking(FLOATINGPOINT futureTemperatureServer, FLOATINGPOINT powerSRV, FLOATINGPOINT DataCenterLoad) 
{
	FLOATINGPOINT TLow = 10.0;
	FLOATINGPOINT THight = 34.00;
	FLOATINGPOINT CPULow = 0.00;
	FLOATINGPOINT CPUHight = 1.00;
	FLOATINGPOINT PowerLow = 130;
	FLOATINGPOINT PowerHight = 305;
	FLOATINGPOINT MemoryLow = 0;
	FLOATINGPOINT MemoryHight = TOTAL_OF_MEMORY_IN_ONE_SERVER;

	FLOATINGPOINT EFF_Temp = 0.00;
	FLOATINGPOINT EFF_CPU = 0;
	FLOATINGPOINT EFF_Power = 0;
	FLOATINGPOINT EFF_Memory = 0;
	FLOATINGPOINT EFF_Traffic = 0;

	FLOATINGPOINT temperature = 0;
	FLOATINGPOINT utilizationCPU = 0;
	long int utilizationMemory = 0;
	long int memoryServer = 0;
	FLOATINGPOINT trafficKBPS = 0;
	long int speedKBPS = 0;
	FLOATINGPOINT powerS = 0;

	temperature = futureTemperatureServer;
	utilizationCPU = this->VMRequiresThisMuchUtilization();
	utilizationMemory = this->VMRequiresThisMemory(); 
	memoryServer = this->GetMemoryServer();
	trafficKBPS = this->ReturnServerTrafficKBPS();
	speedKBPS = this->ReturnBandWidthServerKBPS();
	
	powerS = powerSRV;

	EFF_Temp = 1 - pow(((temperature - TLow) / (THight - TLow)), E_TEMPERATURE);

	EFF_CPU = 1 - pow(((utilizationCPU  - CPULow) / (CPUHight - CPULow)), E_CPU);

	EFF_Power = 1 - pow(((powerS - PowerLow) / (PowerHight - PowerLow)), E_POWER);
	//EFF_Power = 1 - pow((((powerS - GetFanPower()) - PowerLow) / (PowerHight - PowerLow)), E_POWER);

	EFF_Memory = pow((((memoryServer - utilizationMemory) - MemoryLow) / (MemoryHight - MemoryLow)), E_MEMORY);

	EFF_Traffic = 1 - pow(((trafficKBPS - 0) / (FLOATINGPOINT (speedKBPS  - 0))), E_TRAFFIC); 

	return (WEIGHT_TEMPERATURE*(ALPHA_3DMOBFD * EFF_Temp)) + (WEIGHT_CPU*(BETA_3DMOBFD * EFF_CPU)) + (WEIGHT_POWER*(GAMMA_3DMOBFD * EFF_Power)) + (WEIGHT_MEMORY*(DELTA_3DMOBFD * EFF_Memory)) + (WEIGHT_TRAFFIC*(EPSILON_3DMOBFD * EFF_Traffic));

	

}

