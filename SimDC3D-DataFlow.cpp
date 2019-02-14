#include "SimDC3D-DataFlow.h"
#include "time.h"

DataFlow::DataFlow(int idChassiS, int idServerS, string source_ND_FNSS, string source_ND_TrafficMatrix, int timelive, fnss::Topology* tpDF, TrafficMatrixSimDC3D* tffDF)
{
	idChassiSource = idChassiS;
	idServerSource = idServerS;
	sourceNodeFNSS = source_ND_FNSS;

	idChassiDestination = 99999;
	idServerDestination = 99999;
    destinationNodeFNSS = "";

	sourceNodeTrafficMatrix = source_ND_TrafficMatrix;
	destinationNodeTrafficMatrix = "";
	
	topologyDF = tpDF;
	matrixTrafficFlow = tffDF;

	flowMBPS = 0;

	path.clear();

	timeToLive = timelive;

	period_ON = 0.0;
    period_OFF = 1.0;
	
	nextPeriod_OFF = false;

	on_d = 0;
	off_d = 0;

	meanflowMBPS = 0;
	mean = 0;

	k = K;

	generator_on  = new std::default_random_engine ((int) SEED_PERIOD_ON*time(NULL));
	generator_off = new std::default_random_engine ((int) SEED_PERIOD_OFF*time(NULL));

}

void DataFlow::Create_Lognormal_Distribution(void) {

	on_mean  = mean;
	off_mean = 1. - on_mean;

	on_mean2  = on_mean * on_mean;
	off_mean2 = off_mean * off_mean;

	k2 = k * k;
	
	on_distribution = new std::lognormal_distribution<FLOATINGPOINT> ( log(on_mean2/(k2+1)) / 2., sqrt(log(k2 + 1)) );
	off_distribution = new std::lognormal_distribution<FLOATINGPOINT> ( log(off_mean2/(k2+1)) / 2., sqrt(log(k2 + 1)) );
}

DataFlow::~DataFlow(void)
{
  path.clear();
}

void DataFlow::EveryASecond(void) 
{

 if ( timeToLive == 0 )  {
	return; 
 }

 while(true) {
	if ( (period_OFF + period_ON) < 1) {
	   if (nextPeriod_OFF) {
		   period_OFF += Period_OFF();
		   nextPeriod_OFF = false;
	   }
	   else {
		   period_ON += Period_ON();
		   nextPeriod_OFF = true;
	   }
	}
	else {
		if (nextPeriod_OFF) {
		   period_OFF = 0;
		   if ( (1 - period_ON) >=0 ) {
			  flowMBPS = period_ON * 1000;
			  period_ON = 0;
		   }
		   else {
			  flowMBPS = 1 * 1000;
 	          period_ON = period_ON - 1;
		   }
		}
		else {
		   if ( (1 - period_OFF) >=0 ) {
 	          period_OFF = 1 - period_OFF;
		   }
		   else {
 	          period_OFF = period_OFF - 1;
		   }
		   flowMBPS = period_ON * 1000;
		   period_ON = 0;
		}
	  break;	 
	}
 }

 if ( flowMBPS < 0 ) {
	 cout << "SimDC3D-ERROR: Data Flow Value less than zero !!! " << endl; 
	 exit(0);
 }
 
 if ( ( timeToLive > 0 ) || ( timeToLive != 99999) ){
    DecrementsTimeToLive(); 
 }

 trafficNetwork.push_back(flowMBPS);
 if (trafficNetwork.size() > 10) {  // sliding window
	 trafficNetwork.erase(trafficNetwork.begin());
 }

}

void DataFlow::InsertPath(string node)
{
  path.push_back(node);
}

bool DataFlow::PathExists(void)
{
	if (path.size() != 0) {
		return true;
	}
	else {
		return false;
	}
}

void DataFlow::ListPath(void)
{
 for(unsigned int i = 0; i < path.size(); i++) {
	 cout << path[i] << " ";
 }
}

FLOATINGPOINT DataFlow::Period_ON(void)
{
  on_d  = (*on_distribution)(*generator_on);
  return on_d;
}

FLOATINGPOINT DataFlow::Period_OFF(void)
{
  off_d = (*off_distribution)(*generator_off);
  return off_d;
}


void DataFlow::UpdateDataFlow(int chassi_DF_O, int server_DF_O, string nodeFNSS_DF_O)
{
  idChassiSource = chassi_DF_O;
  idServerSource = server_DF_O;
  sourceNodeFNSS = nodeFNSS_DF_O;
  path.clear();
}

bool DataFlow::DataFlowHasDestination(void)
{

  if ( (idChassiDestination == 99999) && (idServerDestination = 99999) ) {
	 return false;
  }

  return true;
}

void DataFlow::InsertDestinationVM(VirtualMachine* destVM)
{
  destinationVM = destVM;
}

void DataFlow::UpdateDestinationDataFlow(int chassi_D, int server_D, string destNodeFNSS, string destNodeTrafficMatrix)
{
   idChassiDestination = chassi_D;
   idServerDestination = server_D;
   destinationNodeFNSS = destNodeFNSS;
   destinationNodeTrafficMatrix = destNodeTrafficMatrix;

   meanflowMBPS = matrixTrafficFlow->GetDataFlow(sourceNodeTrafficMatrix, destinationNodeTrafficMatrix);

   mean = (FLOATINGPOINT) meanflowMBPS / 1000;

   Create_Lognormal_Distribution();
}

void DataFlow::UpdateDestinationDataFlow(int chassi_D, int server_D, string destNodeFNSS, VirtualMachine* destVM)
{
   idChassiDestination = chassi_D;
   idServerDestination = server_D;
   destinationNodeFNSS = destNodeFNSS;
   destinationVM = destVM;
   path.clear();
}

void DataFlow::UpdateDestinationDataFlow(int chassi_D, int server_D, string destNodeFNSS)
{
   idChassiDestination = chassi_D;
   idServerDestination = server_D;
   destinationNodeFNSS = destNodeFNSS;
   destinationVM = NULL;
   path.clear();
}

void DataFlow::ClearPath(void)
{
	path.clear();
}


/*
void TopologySimDC3D::Time_ON_OFF(void)
{
	const int nrolls = 10000;
	const int nstars = 100;
	
	double   mean = 0;
	double   k    = 0;
	unsigned seed = 0;

	double on_mean  = mean;
	double off_mean = 1. - on_mean;

	double on_mean2  = on_mean * on_mean;
	double off_mean2 = off_mean * off_mean;

	double k2 = k * k;
	
	std::default_random_engine generator_on(seed);
	std::default_random_engine generator_off(seed*2);

	std::lognormal_distribution<double> on_distribution( log(on_mean2/(k2+1)) / 2., sqrt(log(k2 + 1)));
	std::lognormal_distribution<double> off_distribution( log(off_mean2/(k2+1)) / 2., sqrt(log(k2 + 1)));
	
	double total_on  = 0.;
	double total_off = 0.;
	
	for (int i =0; i < nrolls; ++i) {
		double on_d, off_d;

		off_d = off_distribution(generator_off);
		on_d  = on_distribution(generator_on);
		total_off += off_d;
		total_on  += on_d;

		std::cout << "OFF: " << off_d << std::endl;
		std::cout << "ON: "  << on_d << std::endl;
	}

	printf("\nInput: on_mean=%.2f off_mean=%.2f", on_mean, off_mean);
	printf("\nTotal time:  %.5f", total_off + total_on);
	printf("\nTotal on: %.5f", total_on);
	printf("\nMean on nrolls: %.5f", total_on / (double)nrolls);
	printf("\nMean on total time: %.5f", total_on / (total_off+total_on));
	printf("\nTotal off: %.5f", total_off);
	printf("\nMean off nrolls: %.5f", total_off / (double)nrolls);
	printf("\nMean off total time: %.5f\n", total_off / (total_off+total_on));
	//return 0;
}
*/