#include "SimDC3D-CRAC.h"



CRAC::CRAC()
{
  CRACDischargeAirTempChangeRate = 0.001 * CRAC_DISCHARGE_CHANGE_RATE_0_00x;
  supplyTemp = LOWEST_SUPPLY_TEMPERATURE;
  currentSupplyTempBase = LOWEST_SUPPLY_TEMPERATURE;
}

CRAC::~CRAC(void)
{

}


Police_Default::Police_Default(Server* (*srv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX])
{
 pcServers = srv;
}


FLOATINGPOINT Police_Default::PoliceTemperature()
{

  FLOATINGPOINT highestAddedTemp = 0.0;
  FLOATINGPOINT highestInletTemp = 0.0;
  FLOATINGPOINT localInlet = 0.0;
  FLOATINGPOINT local = 0.0;
 

  // decide supply temperature for next 1 second

  if (CRAC_SUPPLY_TEMP_IS_CONSTANT_AT > 0) {
	 supplyTemp = CRAC_SUPPLY_TEMP_IS_CONSTANT_AT;
  } 
  else {
	 if (INSTANT_CHANGE_CRAC_SUPPLY_AIR) {
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				local = (*pcServers)[i][j]->CurrentAddedTemperature();
				if (local > highestAddedTemp)
					highestAddedTemp = local;
			}
		}
		supplyTemp = EMERGENCY_TEMPERATURE-highestAddedTemp;
	 } 
	 else {
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if (((*pcServers)[i][j]->IsOFF()) || ((*pcServers)[i][j]->IsHibernating()) || ((*pcServers)[i][j]->IsENDING())) {
				   continue;
				}
				localInlet = (*pcServers)[i][j]->CurrentInletTemperature();
				if (localInlet > highestInletTemp)
					highestInletTemp = localInlet;
			}
		}

		if ((EMERGENCY_TEMPERATURE + TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS + SUPPLY_TEMPERATURE_OFFSET_ALPHA) > highestInletTemp){
		   currentSupplyTempBase += CRACDischargeAirTempChangeRate;
		}
		else {
		   currentSupplyTempBase -= CRACDischargeAirTempChangeRate;
		}
		
		if (currentSupplyTempBase < LOWEST_SUPPLY_TEMPERATURE) {
		   currentSupplyTempBase = LOWEST_SUPPLY_TEMPERATURE;
		}

		supplyTemp = currentSupplyTempBase;
	 }
  }

   return supplyTemp;
}



Agressive_Mode::Agressive_Mode(Server* (*srv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX])
{
 pcServers = srv;
}


FLOATINGPOINT Agressive_Mode::PoliceTemperature()
{

  FLOATINGPOINT highestAddedTemp = 0.0;
  FLOATINGPOINT highestInletTemp = 0.0;
  FLOATINGPOINT localInlet = 0.0;
  FLOATINGPOINT local = 0.0;

  // decide supply temperature for next 1 second

  if (CRAC_SUPPLY_TEMP_IS_CONSTANT_AT > 0) {
	 supplyTemp = CRAC_SUPPLY_TEMP_IS_CONSTANT_AT;
  } 
  else {
	 if (INSTANT_CHANGE_CRAC_SUPPLY_AIR) {
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				local = (*pcServers)[i][j]->CurrentAddedTemperature();
				if (local > highestAddedTemp)
					highestAddedTemp = local;
			}
		}
		supplyTemp = EMERGENCY_TEMPERATURE-highestAddedTemp;
	 } 
	 else {
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if (((*pcServers)[i][j]->IsOFF()) || ((*pcServers)[i][j]->IsHibernating()) || ((*pcServers)[i][j]->IsENDING())) {
				   continue;
				}
				localInlet = (*pcServers)[i][j]->CurrentInletTemperature();
				if (localInlet > highestInletTemp)
					highestInletTemp = localInlet;
			}
		}
		
		if ( (highestInletTemp < 28) && (currentSupplyTempBase < EMERGENCY_TEMPERATURE) ) {
              currentSupplyTempBase += (CRACDischargeAirTempChangeRate * 10);
		}
		else {
		   if ((highestInletTemp >= 28) && (highestInletTemp < 29.5) && (currentSupplyTempBase < EMERGENCY_TEMPERATURE))  {
              currentSupplyTempBase += (CRACDischargeAirTempChangeRate);
		   }
		   else {
			  if ((highestInletTemp >= 29.5) && (highestInletTemp < EMERGENCY_TEMPERATURE)) {
			     currentSupplyTempBase += 0;
			  }
			  else {
				 currentSupplyTempBase -= CRACDischargeAirTempChangeRate * 20;
				 //cout << "Maior temperatura " << highestInletTemp << " currentSupplyTempBase " << currentSupplyTempBase << endl;
			  }
		   }
		}
		
		if (currentSupplyTempBase < LOWEST_SUPPLY_TEMPERATURE) {
		   currentSupplyTempBase = LOWEST_SUPPLY_TEMPERATURE;
		}

		supplyTemp = currentSupplyTempBase;
	 }
  }
   return supplyTemp;
}