#pragma once

#include <list>
#include <vector>
#include <queue>
#include <map>
#include <string> 
#include <random>
#include "time.h" 

#include <semaphore.h>
#include <pthread.h>

#include "SimDC3D-Constants.h"
#include "SimDC3D-Server.h"
#include "SimDC3D-Rack.h"
#include "SimDC3D-DataFlow.h"
#include "SimDC3D-TrafficMatrix.h"

#include "FNSS-Topology.h"
#include "FNSS-Edge.h"
#include "FNSS-Pair.h"
#include "FNSS-Parser.h"
#include "FNSS-Traffic-Matrix-Sequence.h"
#include "FNSS-Traffic-Matrix.h"
#include "FNSS-Measurement-Unit.h"
#include "FNSS-Quantity.h"
#include "FNSS-Units.h"
#include "FNSS-Protocol-Stack.h"
#include "FNSS-Node.h"
#include "FNSS-Application.h"

using namespace std;

extern int NUMBER_OF_CHASSIS;
extern int NUMBER_OF_SERVERS_IN_ONE_CHASSIS;

extern int SWITCH_CORE;
extern int SWITCH_AGGREGATION; 	
extern int SWITCH_ACCESS;

extern int NUMBER_CHASSI_RACK;

extern int NUMBER_OF_PORTS_SWITCH_ACCESS;
extern int NUMBER_OF_PORTS_SWITCH_AGGREGATION;
extern int NUMBER_OF_PORTS_SWITCH_CORE;

extern int PROPORTION_OF_INTERNAL_COMMUNICATION_BETWEEN_VMS;
extern int PROPORTION_OF_EXTERNAL_COMMUNICATION;
extern int SEED;

extern int PERIODIC_LOG_INTERVAL;
extern string NAME_FILE_TOPOLOGY;
extern bool SIMULATE_TRAFFIC_MATRIX;
extern bool DYNAMIC_SPEED_ADAPTATION;
extern int TRAFFIC_LIMIT_FOR_ADAPTATION;

extern int NUMBER_OF_SAMPLES_TRAFFIC_MATRIX;
extern int NUMBER_OF_PERIODS;
extern int RUNTIME_LINK_ADAPTATION;

extern string TYPE_TOPOLOGY;
extern bool EXECUTE_LOAD_BALANCING;

class TopologySimDC3D 
{
public:
	TopologySimDC3D(void);
	~TopologySimDC3D(void);
	void CreateTopology(Server* (*tservers)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], map <VirtualMachine*, std::string> *vmDataFlowKeyVM);
	inline double ReturnTotalEnergyAccessSwitch(void) { return vTotalEnergyAccessSwitch; }
	inline double ReturnTotalEnergyAggregationSwitch(void) { return vTotalEnergyAggregationSwitch; }
	inline double ReturnTotalEnergyCoreSwitch(void) { return vTotalEnergyCoreSwitch; }
	inline int ReturnSizeVectorTotalEnergyAccessSwitch(void) { return vTotalEnergyAccessSwitchSparseLog.size(); }
	inline int ReturnSizeVectorTotalEnergyAggregationSwitch(void) { return vTotalEnergyAggregationSwitchSparseLog.size(); }
	inline int ReturnSizeVectorTotalEnergyCoreSwitch(void) { return vTotalEnergyCoreSwitchSparseLog.size(); }
	inline list<int> ReturnSwitchEdge(void) {return switchEdge;}
	inline list<int> ReturnSwitchAggregation(void) { return switchAggregation; }
	inline list<int> ReturnSwitchCore(void) { return switchCore; }
	inline fnss::Topology* ReturnFNSSTopology(void) { return &topology; }
	inline map <VirtualMachine*, std::string> ReturnAllVms(void) { return *vmDestinationDataFlowKeyVM; }
	
	double ReturnTotalEnergyAccessSwitchSparseLog(void);

	double ReturnTotalEnergyAggregationSwitchSparseLog(void);
	double ReturnTotalEnergyCoreSwitchSparseLog(void);
	void PrintTotalEnergyAccessSwitchSparseLog(void);
	void PrintTotalEnergyAggregationSwitchSparseLog(void);
	void PrintTotalEnergyCoreSwitchSparseLog(void);
	void PrintTotalEnergySwitchesSparseLog(void);
	void PrintPowerDrawAccessSwitch(void);
	void PrintPowerDrawAggregationSwitch(void);
	void PrintPowerDrawCoreSwitch(void);
	void PrintTotalPowerDrawSwitches(void);
	double ReturnPowerDrawSwitches(int ind);
	
	void InsertPathInFlow(DataFlow* dataF);
	void LoadBalancing(DataFlow* dataF, vector< vector<string> > possiblePaths);
	void AddPath(DataFlow* dataF, vector< vector<string> > possiblePaths);
	void twoLayersTopology(DataFlow* dataF, vector< vector<string> > possiblePaths, string switchToRSource);
	void threeLayersTopology(DataFlow* dataF, vector< vector<string> > possiblePaths, string switchToRSource);
	void InsertPathInFlowThreeLayers(DataFlow* dataF);
	void Time_ON_OFF(void);
	void ClearTraffic(void);
	void StatisticSwitch(void);
	void ReconnectEdges(void);
	bool isLinkOff(vector<string> path);

	void LoadTrafficMatriz(void); 

	void EveryASecond(int clock_, Server* (*svt)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX]);

	void EveryASecondMultiThread_P1(void);
	void EveryASecondMultiThread_P2(int chassi_i, int chassi_f, Server* (*svt)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], pthread_mutex_t* mtx);
	void EveryASecondMultiThread_P3(void);

	inline void UpdateClock(unsigned int cTP) {clockTP = cTP;}

private:
	Server* (*tServers)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX];
	vector<Rack*> racks;

	map<pair<int, int>, string> nodeFNSS;
	map<string, pair<int, int>> nodeSimDC3D;
	
	TrafficMatrixSimDC3D* matrixTraffic;

	fnss::Topology topology;
	set<pair <string, string> > edges;
	set<string> nodes;

	map <VirtualMachine*, std::string> *vmDestinationDataFlowKeyVM;

	list<int> switchEdge;
	list<int> switchAggregation;
	list<int> switchCore;
	list<int> hostFNSS;

	vector<FLOATINGPOINT> vTotalEnergyAccessSwitchSparseLog;
	vector<FLOATINGPOINT> vTotalEnergyAggregationSwitchSparseLog;
	vector<FLOATINGPOINT> vTotalEnergyCoreSwitchSparseLog;

	FLOATINGPOINT vTotalEnergyAccessSwitch;
	FLOATINGPOINT vTotalEnergyAggregationSwitch;
	FLOATINGPOINT vTotalEnergyCoreSwitch;

	FLOATINGPOINT vEnergyAccessSwitch;
    FLOATINGPOINT vEnergyAggregationSwitch;
    FLOATINGPOINT vEnergyCoreSwitch; 

	vector<DataFlow *> activeFlows;

	std::default_random_engine *gen;
	std::uniform_int_distribution<int> *dist;

	unsigned int clockTP;

	int numberSamplesTM;
	int totalSamplesTM;

	int internalCommunication;
	int externalCommunication;

	pthread_mutex_t mutex_sum;

};

