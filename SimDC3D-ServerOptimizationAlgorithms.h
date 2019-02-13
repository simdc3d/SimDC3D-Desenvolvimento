#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <queue>

#include "SimDC3D-Constants.h"
#include "SimDC3D-Server.h"
#include "SimDC3D-VirtualMachine.h"
#include "SimDC3D-PoolServers.h"

#include "SimDC3D-Structure.h"

using namespace std;

extern int NUMBER_OF_SERVERS_IN_ONE_CHASSIS;
extern int NUMBER_OF_CHASSIS;
extern int NUMBER_OF_CORES_IN_ONE_SERVER;
extern float THRESHOLD_TOP_OF_USE_CPU;
extern float THRESHOLD_BOTTOM_OF_USE_CPU;
extern bool SIMULATES_POOL_SERVER;
extern bool USING_PREDICTION_TO_OVERLOAD;
extern string PREDICTION_ALGORITHM_OVERLOAD;
extern float EMERGENCY_TEMPERATURE;

extern bool sortDecreasingUtilizationlistVMs(STRUCMIGRATION VM_A, STRUCMIGRATION VM_B);
extern bool sortAscendantUtilizationlistVMs(STRUCMIGRATION VM_A, STRUCMIGRATION VM_B);
extern bool sortDecreasingUtilizationCPU(STRUCMIGRATION VM_A, STRUCMIGRATION VM_B);
extern bool sortDecreasingRanking(STRUCOPTIMIZATION SV_A, STRUCOPTIMIZATION SV_B);
extern bool sortAscendantTemperature(STRUCOPTIMIZATION SV_A, STRUCOPTIMIZATION SV_B);
extern bool sortAscendantTrafficNetWork(STRUCOPTIMIZATION SV_A, STRUCOPTIMIZATION SV_B);
extern bool sort_Higher_IO_NetworkVM(STRUCMIGRATION VM_A, STRUCMIGRATION VM_B);
extern bool sortAscendantWeight(STRUCOPTIMIZATION SV_A, STRUCOPTIMIZATION SV_B);



class ServerOptimizationAlgorithms
{
public:
	~ServerOptimizationAlgorithms(void);

	virtual int ReturnTotalMigrationOverLoaded(void) = 0;
	virtual int ReturnTotalMigrationPrecdiction(void) = 0;
	virtual int ReturnTotalMigrationLowUtilization(void) = 0;
	virtual int ReturnTotalMigrationHighTemperature(void) = 0;
	virtual int ReturnTotalMigrationLinkOverload(void) = 0;
	virtual void ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive) = 0;

protected:
	Server* (*ppoServers)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX];
	FLOATINGPOINT HeatRecirculation[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX]; 
	POOLServers* opoolServers;
	int totalMigrationOverLoaded;
	int totalMigrationPrecdiction;
	int totalMigrationLowUtilization;
	int totalMigrationHighTemperature;
	int totalMigrationLinkOverload;

};

class FFD_Low : public ServerOptimizationAlgorithms
{
public:
	FFD_Low(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX]);
	void ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive);
	int ReturnTotalMigrationOverLoaded(void) { return totalMigrationOverLoaded; }
	int ReturnTotalMigrationPrecdiction(void) { return totalMigrationPrecdiction; }
	int ReturnTotalMigrationLowUtilization(void) { return totalMigrationLowUtilization; }
	int ReturnTotalMigrationHighTemperature(void) { return totalMigrationHighTemperature; }
	int ReturnTotalMigrationLinkOverload(void) { return totalMigrationLinkOverload; }
};

class FFD_OverLoaded : public ServerOptimizationAlgorithms
{
public:
	FFD_OverLoaded(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], POOLServers* opool);
	void ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive);
	int ReturnTotalMigrationOverLoaded(void) { return totalMigrationOverLoaded; }
	int ReturnTotalMigrationPrecdiction(void) { return totalMigrationPrecdiction; }
	int ReturnTotalMigrationLowUtilization(void) { return totalMigrationLowUtilization; }
	int ReturnTotalMigrationHighTemperature(void) { return totalMigrationHighTemperature; }
	int ReturnTotalMigrationLinkOverload(void) { return totalMigrationLinkOverload; }
};


class PABFD_Low : public ServerOptimizationAlgorithms
{
public:
	PABFD_Low(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX]);
	void ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive);
	int ReturnTotalMigrationOverLoaded(void) { return totalMigrationOverLoaded; }
	int ReturnTotalMigrationPrecdiction(void) { return totalMigrationPrecdiction; }
	int ReturnTotalMigrationLowUtilization(void) { return totalMigrationLowUtilization; }
	int ReturnTotalMigrationHighTemperature(void) { return totalMigrationHighTemperature; }
	int ReturnTotalMigrationLinkOverload(void) { return totalMigrationLinkOverload; }
};

class PABFD_OverLoaded : public ServerOptimizationAlgorithms
{
public:
	PABFD_OverLoaded(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], POOLServers* opool);
	void ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive);
	int ReturnTotalMigrationOverLoaded(void) { return totalMigrationOverLoaded; }
	int ReturnTotalMigrationPrecdiction(void) { return totalMigrationPrecdiction; }
	int ReturnTotalMigrationLowUtilization(void) { return totalMigrationLowUtilization; }
	int ReturnTotalMigrationHighTemperature(void) { return totalMigrationHighTemperature; }
	int ReturnTotalMigrationLinkOverload(void) { return totalMigrationLinkOverload; }
};


class ThreeDimensionMultiObj_Low : public ServerOptimizationAlgorithms
{
public:
	ThreeDimensionMultiObj_Low(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX]);
	void ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive);
	int ReturnTotalMigrationOverLoaded(void) { return totalMigrationOverLoaded; }
	int ReturnTotalMigrationPrecdiction(void) { return totalMigrationPrecdiction; }
	int ReturnTotalMigrationLowUtilization(void) { return totalMigrationLowUtilization; }
	int ReturnTotalMigrationHighTemperature(void) { return totalMigrationHighTemperature; }
	int ReturnTotalMigrationLinkOverload(void) { return totalMigrationLinkOverload; }
};

class ThreeDimensionMultiObj_OverLoaded : public ServerOptimizationAlgorithms
{
public:
	ThreeDimensionMultiObj_OverLoaded(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], POOLServers* opool);
	void ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive);
	int ReturnTotalMigrationOverLoaded(void) { return totalMigrationOverLoaded; }
	int ReturnTotalMigrationPrecdiction(void) { return totalMigrationPrecdiction; }
	int ReturnTotalMigrationLowUtilization(void) { return totalMigrationLowUtilization; }
	int ReturnTotalMigrationHighTemperature(void) { return totalMigrationHighTemperature; }
	int ReturnTotalMigrationLinkOverload(void) { return totalMigrationLinkOverload; }
};

class TwoDimensionWeight_Low : public ServerOptimizationAlgorithms
{
public:
	TwoDimensionWeight_Low(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX]);
	void ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive);
	int ReturnTotalMigrationOverLoaded(void) { return totalMigrationOverLoaded; }
	int ReturnTotalMigrationPrecdiction(void) { return totalMigrationPrecdiction; }
	int ReturnTotalMigrationLowUtilization(void) { return totalMigrationLowUtilization; }
	int ReturnTotalMigrationHighTemperature(void) { return totalMigrationHighTemperature; }
	int ReturnTotalMigrationLinkOverload(void) { return totalMigrationLinkOverload; }
};


class TwoDimensionWeight_OverLoaded : public ServerOptimizationAlgorithms
{
public:
	TwoDimensionWeight_OverLoaded(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], POOLServers* opool);
	void ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive);
	int ReturnTotalMigrationOverLoaded(void) { return totalMigrationOverLoaded; }
	int ReturnTotalMigrationPrecdiction(void) { return totalMigrationPrecdiction; }
	int ReturnTotalMigrationLowUtilization(void) { return totalMigrationLowUtilization; }
	int ReturnTotalMigrationHighTemperature(void) { return totalMigrationHighTemperature; }
	int ReturnTotalMigrationLinkOverload(void) { return totalMigrationLinkOverload; }
};


class Police_High_Temperature : public ServerOptimizationAlgorithms
{
public:
	Police_High_Temperature(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX],POOLServers* opool);
	void ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive);
	int ReturnTotalMigrationOverLoaded(void) { return totalMigrationOverLoaded; }
	int ReturnTotalMigrationPrecdiction(void) { return totalMigrationPrecdiction; }
	int ReturnTotalMigrationLowUtilization(void) { return totalMigrationLowUtilization; }
	int ReturnTotalMigrationHighTemperature(void) { return totalMigrationHighTemperature; }
	int ReturnTotalMigrationLinkOverload(void) { return totalMigrationLinkOverload; }
};

class NetworkLinkOverload : public ServerOptimizationAlgorithms
{
public:
	NetworkLinkOverload(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX],POOLServers* opool);
	void ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive);
	int ReturnTotalMigrationOverLoaded(void) { return totalMigrationOverLoaded; }
	int ReturnTotalMigrationPrecdiction(void) { return totalMigrationPrecdiction; }
	int ReturnTotalMigrationLowUtilization(void) { return totalMigrationLowUtilization; }
	int ReturnTotalMigrationHighTemperature(void) { return totalMigrationHighTemperature; }
	int ReturnTotalMigrationLinkOverload(void) { return totalMigrationLinkOverload; }
};

class THREEDMOBFD_OverLoaded : public ServerOptimizationAlgorithms
{
public:
	THREEDMOBFD_OverLoaded(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], POOLServers* opool, const FLOATINGPOINT HeatRecirculationMatrix[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX]);
	void ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive);
	int ReturnTotalMigrationOverLoaded(void) { return totalMigrationOverLoaded; }
	int ReturnTotalMigrationPrecdiction(void) { return totalMigrationPrecdiction; }
	int ReturnTotalMigrationLowUtilization(void) { return totalMigrationLowUtilization; }
	int ReturnTotalMigrationHighTemperature(void) { return totalMigrationHighTemperature; }
	int ReturnTotalMigrationLinkOverload(void) { return totalMigrationLinkOverload; }
};

class THREEDMOBFD_Low : public ServerOptimizationAlgorithms
{
public:
	THREEDMOBFD_Low(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], const FLOATINGPOINT HeatRecirculationMatrix[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX]);
	void ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive);
	int ReturnTotalMigrationOverLoaded(void) { return totalMigrationOverLoaded; }
	int ReturnTotalMigrationPrecdiction(void) { return totalMigrationPrecdiction; }
	int ReturnTotalMigrationLowUtilization(void) { return totalMigrationLowUtilization; }
	int ReturnTotalMigrationHighTemperature(void) { return totalMigrationHighTemperature; }
	int ReturnTotalMigrationLinkOverload(void) { return totalMigrationLinkOverload; }
};

class ThreeDimensionMultiObj_Low_V3 : public ServerOptimizationAlgorithms
{
public:
	ThreeDimensionMultiObj_Low_V3(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], const FLOATINGPOINT HeatRecirculationMatrix[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX]);
	void ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive);
	int ReturnTotalMigrationOverLoaded(void) { return totalMigrationOverLoaded; }
	int ReturnTotalMigrationPrecdiction(void) { return totalMigrationPrecdiction; }
	int ReturnTotalMigrationLowUtilization(void) { return totalMigrationLowUtilization; }
	int ReturnTotalMigrationHighTemperature(void) { return totalMigrationHighTemperature; }
	int ReturnTotalMigrationLinkOverload(void) { return totalMigrationLinkOverload; }
};

class ThreeDimensionMultiObj_OverLoaded_V3 : public ServerOptimizationAlgorithms
{
public:
	ThreeDimensionMultiObj_OverLoaded_V3(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], POOLServers* opool, const FLOATINGPOINT HeatRecirculationMatrix[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX]);
	void ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive);
	int ReturnTotalMigrationOverLoaded(void) { return totalMigrationOverLoaded; }
	int ReturnTotalMigrationPrecdiction(void) { return totalMigrationPrecdiction; }
	int ReturnTotalMigrationLowUtilization(void) { return totalMigrationLowUtilization; }
	int ReturnTotalMigrationHighTemperature(void) { return totalMigrationHighTemperature; }
	int ReturnTotalMigrationLinkOverload(void) { return totalMigrationLinkOverload; }
};

class PABFD_Temperature : public ServerOptimizationAlgorithms
{
public:
	PABFD_Temperature(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], POOLServers* opool);
	void ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive);
	int ReturnTotalMigrationOverLoaded(void) { return totalMigrationOverLoaded; }
	int ReturnTotalMigrationPrecdiction(void) { return totalMigrationPrecdiction; }
	int ReturnTotalMigrationLowUtilization(void) { return totalMigrationLowUtilization; }
	int ReturnTotalMigrationHighTemperature(void) { return totalMigrationHighTemperature; }
	int ReturnTotalMigrationLinkOverload(void) { return totalMigrationLinkOverload; }
};