#pragma once

#include <math.h>
#include <vector>
#include <list>
#include <iostream>
#include <string>
#include <random>

#include "SimDC3D-Constants.h"
#include "SimDC3D-DataFlow.h"
#include "SimDC3D-TrafficMatrix.h"

using namespace std;

extern string WORKLOAD_DISTRIBUTION_MODEL;
extern bool DYNAMIC_WORKLOAD_VMS;
extern long int V_TRESHOLD;
extern long int MEMORY_DIRTYING;
extern int T_RESUME;

class DataFlow;

class VirtualMachine
{
public:
	VirtualMachine(const VirtualMachine &VM);
	VirtualMachine(int runtime, int cputime, UINT64 jobnum);
	VirtualMachine(int runtime, int cputime, UINT64 jobnum, long int memUse, long int imageSize, int seed, unsigned int clockSimul, string nodeF);

	~VirtualMachine(void);

	bool IsFinished();
	bool ReturnIsMove();

	FLOATINGPOINT AverageCPU(void);
	FLOATINGPOINT GetCPULoadRatio();
	FLOATINGPOINT HowMuchCPULoadWillThisVMRequire();
	FLOATINGPOINT RunVMAndReturnActualTime(FLOATINGPOINT sec, int chassiVM, int serverVM);

	inline FLOATINGPOINT ReturnAverageTrafficVM(void) { return averageTrafficMBPS / (double) timeTraffic;}
	inline FLOATINGPOINT ReturnTimeMigrationDestination() {return timeCompleteMigrationDestination; }
	inline FLOATINGPOINT ReturnTimeMigrationSource() {return timeCompleteMigrationSource; }
	inline FLOATINGPOINT ReturnVMUseNetwork(void) {return vmTrafficMBPS; }

	inline int ReturnChassi(void) {return chassiHost; }
	inline int ReturnServer(void) {return serverHost; }

	inline long double ReturnTDownTime() {return tDownTime; }

	inline long int ReturnLeaseTime() { return timeUsingCPU; }
	inline long int ReturnTimeInTheShedulingQueue(void) { return (steppedTime - arrivalTime); }

	inline string ReturnNodeFNSSVM(void) { return nodeFNSSVM; }

	inline void ChangeTimeMigrationDestination(FLOATINGPOINT timeFinish) { timeCompleteMigrationDestination = timeFinish;}
	inline void SetClock(unsigned int clockSimul) { steppedTime = clockSimul; }

	int CalculateMigrationTime(long int bandwidth);

	long int GetMemUseVM();

	UINT64 GetJobNumber();

	vector<DataFlow *> ReturnDataFlowVM(void);

	VirtualMachine* ReturnNewVM(void); 

	void CalculateMigrationDowntime(long int bandwidthServer);
	void CreateFlowVM(int chassiO, int serverO, string nodeFNSS_O, fnss::Topology* topologyFNSS, TrafficMatrixSimDC3D* trafficMatrixFNSS);
	void GenerateCPUUtilization(long int timeReset);
	void InsertNewVM(VirtualMachine* newVirtualMachine);
	void InsertTimeMigrationDestination(FLOATINGPOINT timeFinish);
	void InsertTimeMigrationSource(FLOATINGPOINT timeFinish);
	void InsertTimeOfTheLastMigration(long int time);
	void SetIsMove(bool updatemove);
	void SumTrafficVM(FLOATINGPOINT sumtraffic);
	void UpdateDataFlow(int chassiO, int serverO, string nodeFNSS_O);

private:

	bool isFinished;
	bool isMove;

	FLOATINGPOINT adjustCPUtilization;
	FLOATINGPOINT averageTrafficMBPS;
	FLOATINGPOINT averageUsingCPU;
	FLOATINGPOINT avgCPUTimeSec;
	FLOATINGPOINT cpuLoadRatio;
	FLOATINGPOINT dynamicCPULoadRatio;
	FLOATINGPOINT runTimeSec;
	FLOATINGPOINT timeCompleteMigrationDestination;
	FLOATINGPOINT timeCompleteMigrationSource;
	FLOATINGPOINT vmTrafficMBPS;

	int chassiHost;
	int serverHost;

	long double   tDownTime;

	long int timeTraffic;
	long int imageSizeVM;
	long int memUseVM;
	long int timeOfTheLastMigration;
	long int timeUsingCPU;

	std::default_random_engine *generator;
	std::poisson_distribution<int> *cpuUsagePD;
	std::uniform_real_distribution<FLOATINGPOINT> *cpuUsageURD;

	string nodeFNSSVM;

	UINT64 jobNumber;

	unsigned int arrivalTime;
	unsigned int steppedTime;

	vector<DataFlow *> dataFlowVM;

	VirtualMachine* newVM;
};
