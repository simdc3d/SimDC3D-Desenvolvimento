#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <random>

#include "SimDC3D-Constants.h"
#include "SimDC3D-TrafficMatrix.h"
#include "SimDC3D-VirtualMachine.h"

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


extern int NUMBER_OF_CHASSIS;
extern int NUMBER_OF_SERVERS_IN_ONE_CHASSIS;
extern int SEED_PERIOD_ON;
extern int SEED_PERIOD_OFF;
extern float K;

using namespace std;

class VirtualMachine;

class DataFlow 
{
public:
	DataFlow(int idChassiS, int idServerS, string source_ND_FNSS, string source_ND_TrafficMatrix, int timelive, fnss::Topology* tpDF, TrafficMatrixSimDC3D* tffDF);
	~DataFlow(void);

	void InsertPath(string node);
	void ListPath(void);

	bool PathExists(void);

	inline string ReturnSourceNodeFNSS(void) { return sourceNodeFNSS; }
	inline string ReturnDestinationNodeFNSS(void)  { return destinationNodeFNSS; }
	inline int ReturnChassiSource(void) { return idChassiSource; }
	inline int ReturnServerSource(void) { return idServerSource; }
	inline int ReturnChassiDestination(void) { return idChassiDestination; }
	inline int ReturnServerDestination(void) { return idServerDestination; }
	inline FLOATINGPOINT ReturnFlowMBPS(void) { return flowMBPS; }
	inline vector<string> ReturnPath(void) { return path; } 
	inline void DecrementsTimeToLive(void) { timeToLive = timeToLive - 1; }
	inline int ReturnTimeToLive(void) { return timeToLive; }
	inline vector<FLOATINGPOINT> ReturnTrafficNetwork(void) { return trafficNetwork; } 
	inline VirtualMachine* ReturnDestinationVM(void) { return destinationVM; }

	FLOATINGPOINT Period_ON(void);
	FLOATINGPOINT Period_OFF(void);

	void EveryASecond(void);
	void Create_Lognormal_Distribution(void);

	void UpdateDataFlow(int chassi_DF_O, int server_DF_O, string nodeFNSS_DF_O);
	void UpdateDestinationDataFlow (int chassi_D, int server_D, string destNodeFNSS, string destNodeTrafficMatrix);
	void UpdateDestinationDataFlow (int chassi_D, int server_D, string destNodeFNSS, VirtualMachine* destVM);
	void UpdateDestinationDataFlow (int chassi_D, int server_D, string destNodeFNSS);

	bool DataFlowHasDestination(void);
	void InsertDestinationVM(VirtualMachine* destVM);

	void ClearPath(void);

private:
	int idChassiSource;
	int idServerSource;
	int idChassiDestination;
	int idServerDestination;

	string sourceNodeFNSS;
	string destinationNodeFNSS;

	string sourceNodeTrafficMatrix;
	string destinationNodeTrafficMatrix;

	VirtualMachine* destinationVM;

	fnss::Topology* topologyDF;
	TrafficMatrixSimDC3D* matrixTrafficFlow;

	vector<string> path;

	int timeToLive;

	FLOATINGPOINT on_d;
	FLOATINGPOINT off_d;

	FLOATINGPOINT flowMBPS;
	FLOATINGPOINT meanflowMBPS;
	FLOATINGPOINT mean;
	FLOATINGPOINT k;

	std::default_random_engine *generator_on;
	std::default_random_engine *generator_off;

	std::lognormal_distribution<FLOATINGPOINT> *on_distribution;
	std::lognormal_distribution<FLOATINGPOINT> *off_distribution;

	FLOATINGPOINT on_mean;
	FLOATINGPOINT off_mean;

	FLOATINGPOINT on_mean2;
	FLOATINGPOINT off_mean2;

	FLOATINGPOINT period_ON;
    FLOATINGPOINT period_OFF;

	bool nextPeriod_OFF;

	vector<FLOATINGPOINT> trafficNetwork;
	
	FLOATINGPOINT k2;
};

