 #pragma once

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <random>
#include <time.h>

#include "SimDC3D-Constants.h"
#include "SimDC3D-VirtualMachine.h"
#include "SimDC3D-TrafficMatrix.h"

#include "FNSS-Topology.h"

#include "CodeCogs-Linear.h"

using namespace std;

extern bool TEMPERATURE_SENSING_PERFORMANCE_CAPPING;
extern int TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS;
extern int NUMBER_OF_CORES_IN_ONE_SERVER;
extern bool CONSTANT_FAN_POWER;
extern bool FAN_RPM_NO_LIMIT;
extern long int TOTAL_OF_MEMORY_IN_ONE_SERVER;
extern long int BANDWIDTH_IN_ONE_SERVER;
extern int SIZE_WINDOWN_PREDICTION;
extern int SIZE_WINDOWN_PREDICTION_CPU;
extern int MONITORINGTIME;
extern bool SCHEDULING_WITH_PREDICTION;
extern int POWER_ON;
extern int POWER_OFF;
extern int HIBERNATING_POWER_ON; 
extern int HIBERNATING_POWER_OFF;
extern bool HIBERNATING_SERVER;
extern float POWER_CONSUMPTION_HIBERNATING;
extern string VMS_ALGORITHM_SELECTION;
extern string MODEL_OF_ENERGY_CONSUMPTION;
extern bool USING_PREDICTION_TO_OVERLOAD;
extern string PREDICTION_ALGORITHM_OVERLOAD;

extern float EMERGENCY_TEMPERATURE;
extern bool SIMULATES_NETWORK;
extern bool OPTIMIZATION_WITH_PREDICTION;

extern vector<double> runRBF(vector<double> vetorPredicao);
extern vector<double> runPolynom(vector<double> vetorPredicao);

extern bool Sort_HigherCPU_LessMemory(VirtualMachine* VM_A, VirtualMachine* VM_B);
extern bool Sort_HigherAverageCPU_LessMemory(VirtualMachine* VM_A, VirtualMachine* VM_B);
extern bool Sort_HigherAverageCPU_LessMemory_HigherNetwork(VirtualMachine* VM_A, VirtualMachine* VM_B);
extern bool Sort_HigherAverageCPU(VirtualMachine* VM_A, VirtualMachine* VM_B);
extern bool Sort_LessMemory(VirtualMachine* VM_A, VirtualMachine* VM_B);
extern bool Sort_Higher_IO_Network(VirtualMachine* VM_A, VirtualMachine* VM_B);



class Server
{
public:
	Server(unsigned int cpuGen, long int bandwidth, long int memory, int ch, int sv, map <VirtualMachine*, std::string> *vmDataFlowKeyVM);
	~Server(void);

	FLOATINGPOINT AverageServerTrafficKBPS(void);
	FLOATINGPOINT AverageUsageofCPU(void);
	FLOATINGPOINT CurrentAddedTemperature();
	FLOATINGPOINT CurrentInletTemperature();
	FLOATINGPOINT CurrentUtilization();
	FLOATINGPOINT EstimatePowerDraw(FLOATINGPOINT utilization, FLOATINGPOINT utilizationNewVM);
	FLOATINGPOINT EstimatePowerDrawWithTemperature(FLOATINGPOINT utilization, FLOATINGPOINT utilizationNewVM, FLOATINGPOINT temperature);
	FLOATINGPOINT GetFanPower();
	FLOATINGPOINT GetMaximumVmMigrationTime(void);
	FLOATINGPOINT GetPowerDraw();
	FLOATINGPOINT MaxUtilization();
	FLOATINGPOINT ReadSupplyTempToTimingBuffer(); 
	FLOATINGPOINT ReturnAverageDowmTimeServer(void);
	FLOATINGPOINT ReturnErrorMean(void);
	FLOATINGPOINT ReturnRMSE( void);
	FLOATINGPOINT ReturnSDErrorMean(void);
	FLOATINGPOINT ReturnServerTrafficKBPS(void);
	FLOATINGPOINT ReturnVarianceErrorMean(void);
	FLOATINGPOINT VMRequiresThisMuchCPUScale();
	FLOATINGPOINT VMRequiresThisMuchUtilization();
	FLOATINGPOINT CalculationRanking(FLOATINGPOINT futureTemperatureServer, FLOATINGPOINT powerSRV, FLOATINGPOINT DataCenterLoad); 

	bool HasVMs(void);
	bool HasVMsToMigrate(void);
	bool IsFinished();
	bool IsHostOverUtilized(string prediction_Algorithm);
	bool LR_CPU(void);
	bool ReturnCPUPrediction(void);

	vector<VirtualMachine *>* GetFinishedVMVector();
	vector<VirtualMachine *> GetALLVMs(void);
	vector<VirtualMachine *> GetNVMs(int N, string Algortimo_Selection);

	VirtualMachine* TakeAVM();
	VirtualMachine* InsertNewVM(VirtualMachine* VM, FLOATINGPOINT timeM);

	vector<FLOATINGPOINT> ReturnVectorTemperature(void);
	
	vector<double> GetTricubeWeigts(int n);

	int HowManyVMs(void);
	int ReturnSizeVectorTemperature(void);

	long int VMRequiresThisMemory();

	unsigned int HowManySecondsOverEmergencyTemp();
	unsigned int HowManyTimesDVFSChanged();
	unsigned int ReturnClock(void);
	unsigned int ReturnErrorPrediction(void);
	unsigned int ReturnFirstTimePredictionServer(void);
	unsigned int ReturnHitPrediction(void);
	

	void AddErrorPrediction(void);
	void AddHeatToTimingBuffer(FLOATINGPOINT temperature, int timing);
	void AddHitPrediction(void);
	void AddPOOL(void);
	void AssignOneVM(VirtualMachine *);
	void CalculateRMSE(void);
	void CalculatingStandardDeviationCPU(void);
	void CheckFinishMove(int clockSim);
	void ConnectServerNode(string node, fnss::Topology* tp, TrafficMatrixSimDC3D* trafficMS); 
	void EnterHibernation();
	void EveryASecond(int clockSimul);
	void ExitHibernation(int clockSimul);
	void FinishInitialization();
	void InsertDownTimeANDLeaseTimeVM(long double downTimeVM, long int leaseTimeVM);
	void InsertRackInServer(int Rck);
	void InsertServerTrafficKBPS(FLOATINGPOINT trafficServer);
	void InsertTemperaturePredictionServer(FLOATINGPOINT temperature);
	void InsertTimePredictionServer(unsigned int timeprediction);
	void InsertVectorTemperature(FLOATINGPOINT tempServer);
	void InsertWorkLoadCPU(FLOATINGPOINT utilCPU);
	void MoveVMTo(Server* (*mserver)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], int chassiDestination, int serverDestination, VirtualMachine* VMSelected);
	void RecalculatePerformanceByTemperature();
	void RemovePOOL(void);
	void RemoveTheLastAssignedVM();
	void ReturnServer(void);  
	void ServerRead();
	void SetSupplyTempToTimingBuffer(FLOATINGPOINT temperature, int timing);
	void TimeHibernatingServer (int clockSimul);
	void TimePowerOffServer (int clockSimul);
	void TurnOFF();
	void TurnON(int clockSimul);


	inline bool IsENDING() { return isEnding;}
	inline bool IsHibernating() { return isHibernating;}
	inline bool IsINITIALIZING() { return isInitializing; }
	inline bool IsMIGRATING() { return isMigrating; }
	inline bool IsOFF() { return isOFF; }
	inline bool IsON() { return isON; }
	inline bool IsPOOL() { return isPOOL; }
	inline bool IsSuperHeatedInTheServerPOOL() { return isSuperHeatedInTheServerPOOL; }


	inline FLOATINGPOINT ReturnStandardDeviationCPU(void) { return standardDeviationCPU; }
	
	inline int ReturnFinishPowerOffServer(void) { return powerOffServer; }
	inline int ReturnFinishPowerOnServer(void) { return powerOnServer; }
	inline int ReturnTotalVMsMigrate(void) { return totalVMsMigrate; }

	inline long int GetMemoryServer() {return memServer; }
	inline long int ReturnBandWidthServerKBPS(void) { return bandwidthServer; }
	inline long int ReturnTimeServerOverLoaded(void) { return timeServerOverLoaded; }
	inline long int ReturntimeServerPowerOn(void) { return timeServerPowerOn;}

	inline string ReturnServerNode(void) { return nodeFNSS; }
	inline string ReturnSwitch(void) { return ToR; }

	inline void InsertSwitch(string node) { ToR = node; }
	inline void SetMigrating(bool Migrat) { isMigrating = Migrat; }
	inline void UpdateSuperHeatedInTheServerPOOL(bool superHeated) {isSuperHeatedInTheServerPOOL = superHeated; }

	Maths::Regression::Linear_CodeCogs* CreateWeigthedLinearRegression(vector<double> x, vector<double> y, vector<double> weigths);
	Maths::Regression::Linear_CodeCogs* GetParameterEstimates(vector<double> y);



private:
	
	bool CPUpredicted;
	bool isEnding;
	bool isHibernating;
	bool isInitializing;
	bool isMigrating;
	bool isOFF;
	bool isON;
	bool isPOOL;
	bool isSuperHeatedInTheServerPOOL;

	double clockPredicted;

	FLOATINGPOINT additionalHeatTimingBuffer[SIZE_OF_HEAT_TIMING_BUFFER];
	FLOATINGPOINT averageCPU;
	FLOATINGPOINT averagetrafficKBPS;
	FLOATINGPOINT coolerMaxDieTemperature;
	FLOATINGPOINT coolerMaxPower;
	FLOATINGPOINT coolerMaxRPM;
	FLOATINGPOINT coolerMinRPM;
	FLOATINGPOINT coolerMinTemperatureGap;
	FLOATINGPOINT cpuTDP;
	FLOATINGPOINT currentCPUPowerFactor;
	FLOATINGPOINT currentFanPowerDraw;
	FLOATINGPOINT currentPerformanceFactor;
	FLOATINGPOINT currentPowerDraw;
	FLOATINGPOINT errorMean;
	FLOATINGPOINT nonLinearEnergyConsumption[11];
	FLOATINGPOINT RMSE;
	FLOATINGPOINT standardDeviationCPU;
	FLOATINGPOINT supplyTempTimingBuffer[SIZE_OF_HEAT_TIMING_BUFFER];
	FLOATINGPOINT varianceCPU;

	fnss::Topology* pTopologyServer;

	int currentPerformanceStateOutof100;
	int idChassi;
	int idServer;
	int numberElements;
	int powerOffServer;
	int powerOnServer;
	int rack;
	int totalVMsMigrate;

	long double sumDownTimeVM;

	long int sumLeaseTimeVM;
	long int bandwidthServer;
	long int memServer;
	long int predictedTime;
	long int timeServerOverLoaded; 
	long int timeServerPowerOn;

	map <VirtualMachine*, std::string> *vmDestinationDataFlowKeyVM;

	string nodeFNSS;
	string ToR;

	TrafficMatrixSimDC3D* pTrafficMatrixSequence;

	unsigned int clock;
	unsigned int cpuGeneration;
	unsigned int errorPrediction;
	unsigned int hitPrediction;
	unsigned int howManySecondsOverEmergencyTemp;
	unsigned int howManyTimesDVFSChanged;

	vector<FLOATINGPOINT> temperaturePredictionServer;
	vector<FLOATINGPOINT> temperatureServer;
	vector<FLOATINGPOINT> trafficKBPS;
	vector<FLOATINGPOINT> utilizationCPU;
	vector<FLOATINGPOINT> varianceErrorMean;
	vector<unsigned int> timePredictionServer;
	vector<VirtualMachine *> vFinishedVMs;
	vector<VirtualMachine *> vRunningVMs;

	void CalculatePowerDraw(FLOATINGPOINT utilization, FLOATINGPOINT temperature);
	void ClockPlusPlus();

	FLOATINGPOINT ReadHeatFromTimingBuffer();
};

