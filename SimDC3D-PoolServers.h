#pragma once

#include <iostream>
#include <vector>

#include "SimDC3D-Constants.h"
#include "SimDC3D-Server.h"
#include "SimDC3D-Structure.h"

using namespace std;

extern bool sortPOOLServer(POOL S_A, POOL S_B);
extern int NUMBER_OF_CHASSIS;
extern int NUMBER_OF_SERVERS_IN_ONE_CHASSIS;
extern long CLENGTH;
extern int SIZE_POOL;
extern bool INCREASE_SIZE_POOL_DYNAMICALLY;
extern bool sortTemperature(POOL SV_A, POOL SV_B);
extern int PERIODIC_LOG_INTERVAL;
extern bool SIMULATES_POOL_SERVER;
extern float EMERGENCY_TEMPERATURE;
extern bool SIMULATES_MIGRATION_VMS;


class POOLServers
{
public:
	POOLServers(void);
    ~POOLServers(void);
	void InsertVectorServersPOOL(int Chassi, int Server, double Temperature);
	void InsertVectorServersPowerOFF(int Chassi, int Server, double Temperature);
	void InsertVectorServersPowerON(int Chassi, int Server, double Temperature);
	void InsertVectorServersON(int Chassi, int Server, double Temperature);
	void SortVectorServersPowerOFF(void);
	void SortVectorServersPOOL(void);
	void ServerPowerON(Server* (*sv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], int wakeUP);
	POOL RemoveServerPOOL(Server* (*sv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX]);
	void EveryASecond(Server* (*psv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX]);
	void UpdateClockSimulation(int clocksimul);
	void PowerOFFDCWithPOOL(Server* (*psv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX]);
	void PowerOFFDCWithoutPOOL(Server* (*psv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX]);
	inline int ReturnSizePool(void) { return serversPOOL.size(); }
	bool SwapServer(Server* (*psv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], int chassi, int server);
	void ServerPowerOFF_in_POOL(Server* (*posv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], int powerOFFServer);
	inline void AddPowerOn(int power_On) { totalPowerOn += power_On;} 
	inline void AddPowerOff(int power_Off) { totalPowerOff += power_Off;} 
	void PrintPowerOffServer(void);
	int TotalPowerOffServer(void);

private:
	int clockSimulation;
	int timeOfEmergency;
    vector<POOL> serversPowerOFF;
	vector<POOL> serversPowerON;
	vector<POOL> serversPOOL;
	vector<POOL> serversON;

	int totalPowerOn;
	int totalPowerOff;

	vector<int> sumPowerOff;
	int sumOff;
};
