#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "SimDC3D-Constants.h"
#include "SimDC3D-Server.h"

using namespace std;

extern int CRAC_SUPPLY_TEMP_IS_CONSTANT_AT;
extern bool INSTANT_CHANGE_CRAC_SUPPLY_AIR;
extern int NUMBER_OF_CHASSIS;
extern int NUMBER_OF_SERVERS_IN_ONE_CHASSIS;
extern int CRAC_DISCHARGE_CHANGE_RATE_0_00x;
extern int SUPPLY_TEMPERATURE_OFFSET_ALPHA;
extern float EMERGENCY_TEMPERATURE;

class CRAC
{
public:
	CRAC(void);
	~CRAC(void);

	virtual FLOATINGPOINT PoliceTemperature() = 0;
	inline FLOATINGPOINT ThermostatTemperature () { return supplyTemp; }

protected:
    FLOATINGPOINT supplyTemp;
	FLOATINGPOINT currentSupplyTempBase;
	FLOATINGPOINT CRACDischargeAirTempChangeRate;

	Server* (*pcServers)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX];
};

class Police_Default : public CRAC
{
public:
	Police_Default(Server* (*srv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX]);
	FLOATINGPOINT PoliceTemperature(void);

};

class Agressive_Mode : public CRAC
{
public:
	Agressive_Mode(Server* (*srv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX]);
	FLOATINGPOINT PoliceTemperature(void);

};