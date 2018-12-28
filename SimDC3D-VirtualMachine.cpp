#include "random"
#include "time.h"

#include "SimDC3D-VirtualMachine.h"

VirtualMachine::VirtualMachine(int runtime, int cputime, UINT64 jobnum) // out of date
{
	if (cputime < 0) // no information about cputime 
		cputime = runtime; // assuming 100% utilization

	runTimeSec = (FLOATINGPOINT)runtime;
	avgCPUTimeSec = (FLOATINGPOINT)cputime;
	cpuLoadRatio = avgCPUTimeSec/runTimeSec;
	dynamicCPULoadRatio = 0;
	memUseVM = 0;
	vmTrafficMBPS = 0;
	imageSizeVM = 0;
	isFinished = true;
	if (runTimeSec > 0.0)
		isFinished = false;
	jobNumber = jobnum;
	isMove = false;
	averageUsingCPU = 0.00;
	timeUsingCPU = 0;
	tDownTime = 0.00;
	timeCompleteMigrationDestination = 0;
	timeCompleteMigrationSource = 0;
	timeOfTheLastMigration = 0;
	adjustCPUtilization = 0;
	arrivalTime = 0;
	steppedTime = 0;
	averageTrafficMBPS = 0;
	timeTraffic = 0;
}

VirtualMachine::VirtualMachine(int runtime, int cputime, UINT64 jobnum, long int memUse, long int imageSize, int seed, unsigned int clockSimul, string nodeF)
{
	if (cputime < 0) // no information about cputime 
		cputime = runtime; // assuming 100% utilization

	runTimeSec = (FLOATINGPOINT)runtime;
	avgCPUTimeSec = (FLOATINGPOINT)cputime;
	
	if (avgCPUTimeSec <= 0) {
	   cpuLoadRatio = 1; // 100% utilization;
	}
	else {
	   cpuLoadRatio = avgCPUTimeSec/runTimeSec;
	}

	dynamicCPULoadRatio = 0;

	memUseVM = memUse;
	vmTrafficMBPS = 0;
	imageSizeVM = imageSize;
	isFinished = true;

	if (runTimeSec > 0.0) {
		isFinished = false;
	}
	jobNumber = jobnum;
	isMove = false;
	averageUsingCPU = 0.0;
	timeUsingCPU = 0;
	tDownTime = 0.00;
	timeCompleteMigrationDestination = 0;
	timeCompleteMigrationSource = 0;
	timeOfTheLastMigration = 0;
	adjustCPUtilization = 0;
	arrivalTime = clockSimul;
	steppedTime = 0;

	averageTrafficMBPS = 0;
	timeTraffic = 0;

	nodeFNSSVM = nodeF;

	dataFlowVM.clear();

	chassiHost = 99999;
    serverHost = 99999; 

	if (DYNAMIC_WORKLOAD_VMS) {
	   generator = new std::default_random_engine ((int) (clock()+seed));
	   //generator = new std::default_random_engine ((int) seed);

	   if (WORKLOAD_DISTRIBUTION_MODEL=="POISSON_DISTRIBUTION") {
		  cpuUsagePD = new std::poisson_distribution<int> (cpuLoadRatio*100);
		  GenerateCPUUtilization(0);
	   } 
	   if (WORKLOAD_DISTRIBUTION_MODEL == "UNIFORM_REAL_DISTRIBUTION") { 
		  cpuUsageURD = new std::uniform_real_distribution<FLOATINGPOINT> (0.01,1.0);
		  GenerateCPUUtilization(0);
	   } 
	}
}

VirtualMachine::VirtualMachine(const VirtualMachine &VM)
{
	this->jobNumber = VM.jobNumber;
	this->runTimeSec = VM.runTimeSec;
	this->avgCPUTimeSec = VM.avgCPUTimeSec;
	this->cpuLoadRatio = VM.cpuLoadRatio;
	this->dynamicCPULoadRatio = VM.dynamicCPULoadRatio;
	this->memUseVM = VM.memUseVM;
	this->vmTrafficMBPS = VM.vmTrafficMBPS;
	this->imageSizeVM = VM.imageSizeVM;
	this->isFinished = VM.isFinished;
	this->isMove = VM.isMove;
	this->averageUsingCPU = VM.averageUsingCPU;
	this->timeUsingCPU = VM.timeUsingCPU;
	this->tDownTime = VM.tDownTime;
	this->generator = VM.generator;
	this->cpuUsagePD = VM.cpuUsagePD;
	this->cpuUsageURD = VM.cpuUsageURD;
	this->nodeFNSSVM = VM.nodeFNSSVM;
	this->timeCompleteMigrationDestination = VM.timeCompleteMigrationDestination;
	this->timeCompleteMigrationSource = VM.timeCompleteMigrationSource;
	this->timeOfTheLastMigration = VM.timeOfTheLastMigration;
	this->adjustCPUtilization = VM.adjustCPUtilization;
	this->arrivalTime =  VM.arrivalTime;
	this->steppedTime = VM.steppedTime;
	this->dataFlowVM = 	VM.dataFlowVM;
	this->chassiHost = VM.chassiHost;
	this->serverHost = VM.serverHost;
	this->averageTrafficMBPS = VM.averageTrafficMBPS;
	this->timeTraffic = VM.timeTraffic;	
}

VirtualMachine::~VirtualMachine(void)
{
	dataFlowVM.clear();
}

UINT64 VirtualMachine::GetJobNumber()
{
	return jobNumber;
}

FLOATINGPOINT VirtualMachine::HowMuchCPULoadWillThisVMRequire()
{
    if (DYNAMIC_WORKLOAD_VMS) {
       return dynamicCPULoadRatio;
	}
	else {
	   return cpuLoadRatio;
	}
}

FLOATINGPOINT VirtualMachine::RunVMAndReturnActualTime(FLOATINGPOINT sec, int chassiVM, int serverVM)
{
	if (isFinished) {
		cout << "Error: Finished VM consumed cpu time!!!! runTimeSec =  " << runTimeSec << "  avgCPUTimeSec = " << avgCPUTimeSec << " sec " << sec << endl;
	}

	if (runTimeSec < sec) {
		isFinished = true;
		runTimeSec = avgCPUTimeSec = 0.0;
		return runTimeSec;
	}

	runTimeSec -= sec;

	if (DYNAMIC_WORKLOAD_VMS) {
 	   avgCPUTimeSec -= sec*dynamicCPULoadRatio;
	}
	else {
	   avgCPUTimeSec -= sec*cpuLoadRatio;
	}

	chassiHost = chassiVM;
    serverHost = serverVM; 

	return sec;
}

bool VirtualMachine::IsFinished()
{
	return isFinished;
}

FLOATINGPOINT VirtualMachine::GetCPULoadRatio()
{
  if (DYNAMIC_WORKLOAD_VMS) {
     return dynamicCPULoadRatio;
  }
  else {
	 return cpuLoadRatio;
  }

}

long int VirtualMachine::GetMemUseVM()
{
	return memUseVM;
}


void VirtualMachine::InsertTimeMigrationDestination(FLOATINGPOINT timeFinish)
{
	isMove = true;
	timeCompleteMigrationDestination = timeFinish;
	timeCompleteMigrationSource = 0;
}

void VirtualMachine::InsertTimeMigrationSource(FLOATINGPOINT timeFinish)
{
	isMove = true;
	timeCompleteMigrationSource = timeFinish;
	timeCompleteMigrationDestination = 0;
}

bool VirtualMachine::ReturnIsMove()
{
	return isMove;
} 

int VirtualMachine::CalculateMigrationTime(long int bandwidth)
{
	int timeMigration = 0;

	timeMigration = (int) (((imageSizeVM + memUseVM) * 8) / bandwidth);

	return timeMigration;
}

void VirtualMachine::GenerateCPUUtilization(long int timeReset)
{
	FLOATINGPOINT cpugen = 0.00;

	if ( WORKLOAD_DISTRIBUTION_MODEL == "POISSON_DISTRIBUTION" ) {
	   std::poisson_distribution<int> cpuUsagePD2(cpuUsagePD->param());
	   cpugen = (FLOATINGPOINT) cpuUsagePD2(*generator);
	   if ( cpugen > 100 ) {
	      cpugen = 100;
	   }
	   if ( cpugen <= 0 ) {
	      cpugen = 1;
	   }
	   dynamicCPULoadRatio = (cpugen/100);
    }

    if ( WORKLOAD_DISTRIBUTION_MODEL=="UNIFORM_REAL_DISTRIBUTION" ) {

	   std::uniform_real_distribution<FLOATINGPOINT> cpuUsageURD2(cpuUsageURD->param());

	   cpugen = adjustCPUtilization + (cpuUsageURD2(*generator) * cpuLoadRatio * 2);

	   if ( cpugen > 1 ) {
		  adjustCPUtilization = cpugen - 1;
		  cpugen = 1;
	   }
	   else {
		  adjustCPUtilization = 0;
	   }
	   dynamicCPULoadRatio = cpugen;
    } 
    averageUsingCPU += dynamicCPULoadRatio;
    timeUsingCPU++;
}

FLOATINGPOINT VirtualMachine::AverageCPU(void)
{
	return (averageUsingCPU / (double) timeUsingCPU);
}

void VirtualMachine::CalculateMigrationDowntime(long int bandwidthServer)
{
	// extracted "Energy-Aware Cloud Management through Progressive SLA Specification" Drazen Lucanin, et al.
	FLOATINGPOINT tdown=0.00, n=0.00;

	MEMORY_DIRTYING = memUseVM * 0.40;
	
	n = log((double) V_TRESHOLD / (double) memUseVM) / log ( (double) (MEMORY_DIRTYING) / (double) bandwidthServer);

	if ((pow(bandwidthServer,n+1) == 0) || ( n > 30)) {
		tdown = T_RESUME;
	}
	else {
	    tdown = ((memUseVM * (pow(MEMORY_DIRTYING,n))) / (pow(bandwidthServer,n+1))) + T_RESUME;
	}

	tDownTime += tdown;
}

void VirtualMachine::SetIsMove(bool updateMove)
{
	isMove = updateMove;
}

void VirtualMachine::InsertTimeOfTheLastMigration(long int time)
{
	timeOfTheLastMigration = time;
}

void VirtualMachine::CreateFlowVM(int chassiO, int serverO, string nodeFNSS_O, fnss::Topology* topologyFNSS, TrafficMatrixSimDC3D* trafficMatrixFNSS)
{
	dataFlowVM.push_back(new DataFlow(chassiO, serverO, nodeFNSS_O, nodeFNSSVM, 99999, topologyFNSS, trafficMatrixFNSS));
}

vector<DataFlow*> VirtualMachine::ReturnDataFlowVM(void)
{
	return dataFlowVM;
}

void VirtualMachine::UpdateDataFlow(int chassiO, int serverO, string nodeFNSS_O) 
{
  for (vector<DataFlow *>::iterator it = dataFlowVM.begin(); it != dataFlowVM.end(); ++it) {
      (*it)->UpdateDataFlow(chassiO, serverO, nodeFNSS_O); 
  }
  chassiHost = chassiO;
  serverHost =  serverO; 
  
}	

void VirtualMachine::InsertNewVM(VirtualMachine* newVirtualMachine)
{ 
  newVM = newVirtualMachine;
  //cout << "newVM " << newVM << endl;
}

VirtualMachine* VirtualMachine::ReturnNewVM(void)
{ 
  return newVM;
}

void VirtualMachine::SumTrafficVM(FLOATINGPOINT sumtraffic) {

	vmTrafficMBPS = sumtraffic; 
	averageTrafficMBPS += sumtraffic;
	timeTraffic++;
}