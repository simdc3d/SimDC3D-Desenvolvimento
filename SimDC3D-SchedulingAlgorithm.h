#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <queue>
#include <math.h>

#include "SimDC3D-Constants.h"
#include "SimDC3D-Server.h"
#include "SimDC3D-VirtualMachine.h"
#include "SimDC3D-PoolServers.h"

using namespace std;

extern int NUMBER_OF_SERVERS_IN_ONE_CHASSIS;
extern int NUMBER_OF_CHASSIS;
extern int NUMBER_OF_CORES_IN_ONE_SERVER;
extern long CLENGTH;
extern string PREDICTION_ALGORITHM;
extern float THRESHOLD_TOP_OF_USE_CPU;
extern bool SCHEDULING_WITH_PREDICTION;
extern string PREDICTION_ALGORITHM_OVERLOAD;
extern float EMERGENCY_TEMPERATURE;

extern float SCHEDULER_2D_WEIGHT_TEMPERATURE;
extern float SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION;
extern float SCHEDULER_2D_WEIGHT_LOAD_CPU;

extern void quickSort(SORTSERVER *server, int begin, int end);
extern vector<double> runRBF(vector<double> vetorPredicao);
extern vector<double> runPolynom(vector<double> vetorPredicao);
extern bool Sort_Ranking(SORTSERVER SV_A, SORTSERVER SV_B);


extern int E_TEMPERATURE;
extern int E_CPU;
extern int E_POWER;
extern int E_MEMORY;
extern int E_TRAFFIC;

extern int ALPHA_3DMOBFD;
extern int BETA_3DMOBFD;
extern int GAMMA_3DMOBFD;
extern int DELTA_3DMOBFD;
extern int EPSILON_3DMOBFD;

extern float WEIGHT_TEMPERATURE;
extern float WEIGHT_CPU;
extern float WEIGHT_POWER;
extern float WEIGHT_MEMORY;
extern float WEIGHT_TRAFFIC;



class SchedulingAlgorithm
{
public:
	~SchedulingAlgorithm(void);
	virtual void AssignVMs() = 0;
	virtual int returnTotalScheduling(void) = 0;

protected:
	Server* (*ppServers)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX];
	queue<VirtualMachine*>* pqVMsToGo;
	const FLOATINGPOINT (*pHeatRecirculationMatrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX];
	POOLServers* ppollServers;
	int totalScheduling;
	unsigned int* clock;
};

class LowTemperatureFirstSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	LowTemperatureFirstSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], unsigned int* clockSimulation);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }
};

class UniformTaskSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	UniformTaskSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], unsigned int* clockSimulation);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }
};

class BestPerformanceSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	BestPerformanceSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], unsigned int* clockSimulation);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }
};

class RandomSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	RandomSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], unsigned int* clockSimulation);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }
};

class MinHRSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	MinHRSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], unsigned int* clockSimulation);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }

private:
	FLOATINGPOINT HRF[SIZE_OF_HR_MATRIX];
	int HRFSortedIndex[SIZE_OF_HR_MATRIX];
};

class XintSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	XintSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], unsigned int* clockSimulation);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }

private:
	FLOATINGPOINT GetHighestTemperatureIncrease();
};

class CenterRackFirstSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	CenterRackFirstSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], unsigned int* clockSimulation);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }

private:
	int HRFSortedIndex[SIZE_OF_HR_MATRIX];

};

class TwoDimensionWithPoolSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	TwoDimensionWithPoolSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], POOLServers* ppool, unsigned int* clockSimulation);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }

private:
	FLOATINGPOINT HRF[SIZE_OF_HR_MATRIX];
};

class TwoDimensionWithPoolAndPredictionSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	TwoDimensionWithPoolAndPredictionSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], POOLServers* ppool, unsigned int* clockSimulation);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }
private:
	FLOATINGPOINT HRF[SIZE_OF_HR_MATRIX];
};


class TwoDimensionWithPredictionSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	TwoDimensionWithPredictionSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], POOLServers* ppool, unsigned int* clockSimulation);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }
private:
	FLOATINGPOINT HRF[SIZE_OF_HR_MATRIX];
};


class TwoDimensionSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	TwoDimensionSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], POOLServers* ppool, unsigned int* clockSimulation);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }
private:
	FLOATINGPOINT HRF[SIZE_OF_HR_MATRIX];
};


class THREEDMOBFDSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	THREEDMOBFDSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, POOLServers* ppool, unsigned int* clockSimulation);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }
private:
};

class THREEDMOBFDAndPredictionCPUAndTemperatureSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	THREEDMOBFDAndPredictionCPUAndTemperatureSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, POOLServers* ppool, unsigned int* clockSimulation);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }
private:
};


class THREEDMOBFDAndPoolAndPredictionCPUAndTemperatureSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	THREEDMOBFDAndPoolAndPredictionCPUAndTemperatureSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, POOLServers* ppool, unsigned int* clockSimulation);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }
private:
};


class THREEDMOBFDAndPoolSchedulingAlgorithm : public SchedulingAlgorithm
{
public:
	THREEDMOBFDAndPoolSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, POOLServers* ppool, unsigned int* clockSimulation);
	void AssignVMs();
	int returnTotalScheduling(void) { return totalScheduling; }
private:
};