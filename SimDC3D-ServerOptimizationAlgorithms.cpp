#include <vector>
#include <algorithm>
#include <iostream>
#include <cstring>

#include "SimDC3D-ServerOptimizationAlgorithms.h"

ServerOptimizationAlgorithms::~ServerOptimizationAlgorithms(void)
{
	totalMigrationOverLoaded = 0;
	totalMigrationLowUtilization = 0;
	totalMigrationHighTemperature = 0;
}

FFD_Low::FFD_Low(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX])
{
	ppoServers = pso;
}

void FFD_Low::ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive)
{
   bool migrate;
   vector<STRUCMIGRATION> consolidate;

   totalMigrationLowUtilization = 0;

   if ((ServerActive->size() == 0) || (listVMs->size() == 0)) {
      return;
   }

   sort(listVMs->begin(), listVMs->end(), sortDecreasingUtilizationlistVMs);

   for (int k=0; k < static_cast<int> (listVMs->size()); k++) {
	   migrate = false;
	   if (ServerActive->size() > 0) {
          for (int l=0; l < static_cast<int> (ServerActive->size()); l++) {
			  if ((((*ServerActive)[l].utilizationCPU + ((*listVMs)[k].VM->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) >= THRESHOLD_TOP_OF_USE_CPU) || ( ((*ServerActive)[l].utilizationMemory + (*listVMs)[k].VM->GetMemUseVM()) >= (*ServerActive)[l].memoryServer ) || ((*ServerActive)[l].isMigrate)) {
 			     continue;
			  }
			  //cout << "migrando " << (*listVMs)[k].chassi << " Server " << (*listVMs)[k].server << " para " << (*ServerActive)[l].chassi << " server " <<  (*ServerActive)[l].server << endl; 
		      (*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, (*ServerActive)[l].chassi, (*ServerActive)[l].server, (*listVMs)[k].VM);
			  (*ServerActive)[l].isMigrate = true;
			  totalMigrationLowUtilization++;
			  migrate = true;
			  break;
		  }
	   }
	   if (!migrate) {
	      consolidate.push_back((*listVMs)[k]);
       }
   }
   
   if (consolidate.size() >= 2) {
      sort(consolidate.begin(), consolidate.end(), sortDecreasingUtilizationCPU); 
      for(int k = 1; k < static_cast<int> (consolidate.size()); k++) {
		 if ((consolidate[0].chassi != consolidate[k].chassi) && (consolidate[0].server != consolidate[k].server))  {
   		    //cout << "migrando consolidando " << consolidate[k].chassi<< " Server " << consolidate[k].server << " para " << consolidate[0].chassi << " server " <<  consolidate[0].server << endl; 
			(*ppoServers)[consolidate[k].chassi][consolidate[k].server]->MoveVMTo(ppoServers, consolidate[0].chassi, consolidate[0].server, consolidate[k].VM);
			totalMigrationLowUtilization++;
            break; 
		 }
	  }
   }
   consolidate.clear();
}

FFD_OverLoaded::FFD_OverLoaded(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], POOLServers* opool)
{
	ppoServers = pso;
	opoolServers = opool;
}

void FFD_OverLoaded::ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive)
{
   POOL sv;
   bool removePOOL;
   int sumRemovePOOL = 0;
   int selected = 0;

   totalMigrationOverLoaded = 0;
   totalMigrationPrecdiction = 0;

   if ((ServerActive->size() == 0) || (listVMs->size() == 0)) {
      return;
   }

   sort(listVMs->begin(), listVMs->end(), sortDecreasingUtilizationlistVMs);

   for (int k=0; k < static_cast<int> (listVMs->size()); k++) {
	   removePOOL=true;
	   if (ServerActive->size() > 0) {
          for (int l=0; l < static_cast<int> (ServerActive->size()); l++) {
			  if ((((*ServerActive)[l].utilizationCPU + ((*listVMs)[k].VM->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) >= THRESHOLD_TOP_OF_USE_CPU) || ( ((*ServerActive)[l].utilizationMemory + (*listVMs)[k].VM->GetMemUseVM()) >= (*ServerActive)[l].memoryServer ) || ((*ServerActive)[l].isMigrate)) {
			     continue;
			  }
			  selected = l; 
			  (*ServerActive)[l].isMigrate = true;
			  removePOOL=false;
			  break;
		  }
	   }
       else {
		  removePOOL=true;	
	   }
	   if (removePOOL) {
 		  // Remove server POOL
          if (SIMULATES_POOL_SERVER) {
		     sv = opoolServers->RemoveServerPOOL(ppoServers);
		     if (sv.chassi != -1) {
				(*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, sv.chassi, sv.server, (*listVMs)[k].VM);
				if ((*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->ReturnCPUPrediction()) {
				   totalMigrationPrecdiction ++;
			    }
			    else {
			   	   totalMigrationOverLoaded ++;
			    }
			    sumRemovePOOL++;
		     }
		     else{
		        // cout << "Warning! no servers in the pool - PoliceFFD" << endl;
		     }
		  }  
		  else {
		     // cout << "Warning! no servers !!! - PoliceFFD" << endl;
		  }
	   }
	   else {
		   (*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, (*ServerActive)[selected].chassi, (*ServerActive)[selected].server, (*listVMs)[k].VM);
		   if ((*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->ReturnCPUPrediction()) {
			  totalMigrationPrecdiction ++;
		   }
		   else {
			  totalMigrationOverLoaded ++;
		   }
	   }
   }

   if (SIMULATES_POOL_SERVER) {
      if (sumRemovePOOL > 0) {
	     opoolServers->AddPowerOn(sumRemovePOOL);
	  }
   }

}

PABFD_Low::PABFD_Low(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX])
{
  ppoServers = pso;
}

void PABFD_Low::ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive)
{
   vector<STRUCMIGRATION> consolidate;
   int selected = 0; 
   double powerServer = 0.00; 
   double minPowerServer = 9999999.00;
   bool migrate;
   
   totalMigrationLowUtilization = 0;

   if ((ServerActive->size() == 0) || (listVMs->size() == 0)) {
      return;
   }

   sort(listVMs->begin(), listVMs->end(), sortDecreasingUtilizationlistVMs);

   for (int k=0; k < static_cast<int> (listVMs->size()); k++) {
	   minPowerServer = 9999999;
	   migrate = false;
	   if (ServerActive->size() > 0) {
          for (int l=0; l < static_cast<int> (ServerActive->size()); l++) {
			 if ((((*ServerActive)[l].utilizationCPU + ((*listVMs)[k].VM->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) >= THRESHOLD_TOP_OF_USE_CPU) || ( ((*ServerActive)[l].utilizationMemory + (*listVMs)[k].VM->GetMemUseVM()) >= (*ServerActive)[l].memoryServer ) || ((*ServerActive)[l].isMigrate)) {
				continue;
			  }
			  powerServer =  (*ppoServers)[(*ServerActive)[l].chassi][(*ServerActive)[l].server]->EstimatePowerDraw((*ServerActive)[l].utilizationCPU, ((*listVMs)[k].VM->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER));
			  if (powerServer < minPowerServer) {
				 selected = l;
				 minPowerServer = powerServer;  
				 migrate = true;
			  }
		  }
	   }
 	   if (migrate) {
		  (*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, (*ServerActive)[selected].chassi, (*ServerActive)[selected].server, (*listVMs)[k].VM);
		  totalMigrationLowUtilization++;
		  (*ServerActive)[selected].isMigrate = true;
       }
	   else {
		  consolidate.push_back((*listVMs)[k]);
	   }
   }

   if (consolidate.size() >= 2) {
      sort(consolidate.begin(), consolidate.end(), sortDecreasingUtilizationCPU); 
      for(int k = 1; k < static_cast<int> (consolidate.size()); k++) {
		 if ((consolidate[0].chassi != consolidate[k].chassi) && (consolidate[0].server != consolidate[k].server))  {
 			(*ppoServers)[consolidate[k].chassi][consolidate[k].server]->MoveVMTo(ppoServers, consolidate[0].chassi, consolidate[0].server, consolidate[k].VM);
			totalMigrationLowUtilization++;
            break; 
		 }
	  }
   }
   consolidate.clear();
}


PABFD_OverLoaded::PABFD_OverLoaded(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], POOLServers* opool)
{
	ppoServers = pso;
	opoolServers = opool;
}

void PABFD_OverLoaded::ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive)
{
   POOL sv;
   bool removePOOL;
   int sumRemovePOOL = 0;
   int selected = 0;
   double powerServer = 0.00;
   double minPowerServer = 9999999.00;

   totalMigrationOverLoaded = 0;
   totalMigrationPrecdiction = 0;


   if ((ServerActive->size() == 0) || (listVMs->size() == 0)) {
      return;
   }

   sort(listVMs->begin(), listVMs->end(), sortDecreasingUtilizationlistVMs);

   for (int k=0; k < static_cast<int> (listVMs->size()); k++) {
	   removePOOL=true;
	   minPowerServer = 9999999;
	   if (ServerActive->size() > 0) {
          for (int l=0; l < static_cast<int> (ServerActive->size()); l++) {
			  if ((((*ServerActive)[l].utilizationCPU + ((*listVMs)[k].VM->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) >= THRESHOLD_TOP_OF_USE_CPU) || ( ((*ServerActive)[l].utilizationMemory + (*listVMs)[k].VM->GetMemUseVM()) >= (*ServerActive)[l].memoryServer ) || ((*ServerActive)[l].isMigrate)) {
				  continue;
			  }
			  powerServer =  (*ppoServers)[(*ServerActive)[l].chassi][(*ServerActive)[l].server]->EstimatePowerDraw((*ServerActive)[l].utilizationCPU, ((*listVMs)[k].VM->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER));
			  if (powerServer < minPowerServer) {
				 selected = l;
				 minPowerServer = powerServer;   
                 removePOOL=false;
			  }
		  }
	   }
       else {
		  removePOOL=true;	
	   }
	   if (removePOOL) {
 		  // Remove server POOL
          if (SIMULATES_POOL_SERVER) {
		     sv = opoolServers->RemoveServerPOOL(ppoServers);
		     if (sv.chassi != -1) {
				(*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, sv.chassi, sv.server, (*listVMs)[k].VM);
				if ((*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->ReturnCPUPrediction()) {
				   totalMigrationPrecdiction ++;
			    }
			    else {
			   	   totalMigrationOverLoaded ++;
			    }
				sumRemovePOOL++;
		     }
		     else{
		        //cout << "Warning! no servers in the pool - PoliceFFD" << endl;
		     }
		  }  
		  else {
		        // cout << "Warning! no servers !!! - PABFD_OverLoaded" << endl;
		  }
	   }
	   else {
          (*ServerActive)[selected].isMigrate = true;
		  (*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, (*ServerActive)[selected].chassi, (*ServerActive)[selected].server, (*listVMs)[k].VM);
		  if ((*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->ReturnCPUPrediction()) {
			 totalMigrationPrecdiction ++;
		  }
		  else {
		     totalMigrationOverLoaded ++;
		  }
	  }
   }

   if (SIMULATES_POOL_SERVER) {
      if (sumRemovePOOL > 0) {
	     opoolServers->AddPowerOn(sumRemovePOOL);
	  }
   }

}


Police_High_Temperature::Police_High_Temperature(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], POOLServers* opool)
{
	ppoServers = pso;
	opoolServers = opool;


}

void Police_High_Temperature::ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive)
{
   POOL sv;
   bool removePOOL;
   int sumRemovePOOL = 0;
   int selected = 0;

   totalMigrationHighTemperature = 0;

   if ((ServerActive->size() == 0) || (listVMs->size() == 0)) {
      return;
   }

   sort(listVMs->begin(), listVMs->end(), sortDecreasingUtilizationlistVMs);

   sort(ServerActive->begin(), ServerActive->end(), sortAscendantTemperature);

    for (int k=0; k < static_cast<int> (listVMs->size()); k++) {
	   removePOOL=true;
	   if (ServerActive->size() > 0) {
          for (int l=0; l < static_cast<int> (ServerActive->size()); l++) {
			  if ((((*ServerActive)[l].utilizationCPU + ((*listVMs)[k].VM->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) >= THRESHOLD_TOP_OF_USE_CPU) || ( ((*ServerActive)[l].utilizationMemory + (*listVMs)[k].VM->GetMemUseVM()) > (*ServerActive)[l].memoryServer ) || ((*ServerActive)[l].temperature > EMERGENCY_TEMPERATURE-0.30) || ((*ServerActive)[l].trafficKBPS >= (0.80 * (*ServerActive)[l].speedKBPS)) || ((*ServerActive)[l].isMigrate)) {
//			  if ((((*ServerActive)[l].averageUtilizationCPU + ((*listVMs)[k].VM->AverageCPU()/NUMBER_OF_CORES_IN_ONE_SERVER)) >= THRESHOLD_TOP_OF_USE_CPU) || ( ((*ServerActive)[l].utilizationMemory + (*listVMs)[k].VM->GetMemUseVM()) >= (*ServerActive)[l].memoryServer ) || ((*ServerActive)[l].temperature > (EMERGENCY_TEMPERATURE-0.70)) || (((*ServerActive)[l].averageTrafficKBPS + (*listVMs)[k].VM->ReturnAverageTrafficVM()) >= (0.80 * (*ServerActive)[l].speedKBPS)) || ((*ServerActive)[l].isMigrate)) {
				 continue;
			  }
			  selected = l; 
			  (*ServerActive)[l].isMigrate = true;
			  removePOOL=false;
			  break;
		  }
	   }
       else {
		  removePOOL=true;	
	   }
	   if (removePOOL) {
 		  // Remove server POOL
          if (SIMULATES_POOL_SERVER) {
		     sv = opoolServers->RemoveServerPOOL(ppoServers);
		     if (sv.chassi != -1) {
				(*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, sv.chassi, sv.server, (*listVMs)[k].VM);
				totalMigrationHighTemperature ++;
			    sumRemovePOOL++;
		     }
		     else{
		        // cout << "Warning! no servers in the pool - Police_High_Temperature" << endl;
		     }
		  }  
		  else {
		     // cout << "Warning! no servers !!! - Police_High_Temperature" << endl;
		  }
	   }
	   else {
		  (*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, (*ServerActive)[selected].chassi, (*ServerActive)[selected].server, (*listVMs)[k].VM);
   		  totalMigrationHighTemperature ++;
	   }
   }

   if (SIMULATES_POOL_SERVER) {
      if (sumRemovePOOL > 0) {
	     opoolServers->AddPowerOn(sumRemovePOOL);
	  }
   }

}


NetworkLinkOverload::NetworkLinkOverload(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], POOLServers* opool)
{
	ppoServers = pso;
	opoolServers = opool;
}

void NetworkLinkOverload::ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive)
{
   POOL sv;
   bool removePOOL;
   int sumRemovePOOL = 0;
   int selected = 0;

   totalMigrationLinkOverload = 0;
   
   sort(listVMs->begin(), listVMs->end(), sort_Higher_IO_NetworkVM);

   sort(ServerActive->begin(), ServerActive->end(), sortAscendantTrafficNetWork);

   for (int k=0; k < static_cast<int> (listVMs->size()); k++) {
	   removePOOL=true;
	   if (ServerActive->size() > 0) {
          for (int l=0; l < static_cast<int> (ServerActive->size()); l++) {
			  if ((((*ServerActive)[l].utilizationCPU + ((*listVMs)[k].VM->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) >= THRESHOLD_TOP_OF_USE_CPU) || ( ((*ServerActive)[l].utilizationMemory + (*listVMs)[k].VM->GetMemUseVM()) >= (*ServerActive)[l].memoryServer ) || ((*ServerActive)[l].temperature > EMERGENCY_TEMPERATURE-0.30) || ((*ServerActive)[l].trafficKBPS >= (0.80 * (*ServerActive)[l].speedKBPS)) || ((*ServerActive)[l].isMigrate)) {
//			  if ((((*ServerActive)[l].averageUtilizationCPU + ((*listVMs)[k].VM->AverageCPU()/NUMBER_OF_CORES_IN_ONE_SERVER)) >= THRESHOLD_TOP_OF_USE_CPU) || ( ((*ServerActive)[l].utilizationMemory + (*listVMs)[k].VM->GetMemUseVM()) >= (*ServerActive)[l].memoryServer ) || ((*ServerActive)[l].temperature > (EMERGENCY_TEMPERATURE-0.70)) || (((*ServerActive)[l].averageTrafficKBPS + (*listVMs)[k].VM->ReturnAverageTrafficVM()) >= (0.80 * (*ServerActive)[l].speedKBPS)) || ((*ServerActive)[l].isMigrate)) {
			     continue;
			  }
			  selected = l; 
			  (*ServerActive)[l].isMigrate = true;
			  removePOOL=false;
			  break;
		  }
	   }
       else {
		  removePOOL=true;	
	   }
	   if (removePOOL) {
 		  // Remove server POOL
          if (SIMULATES_POOL_SERVER) {
		     sv = opoolServers->RemoveServerPOOL(ppoServers);
		     if (sv.chassi != -1) {
				 (*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, sv.chassi, sv.server, (*listVMs)[k].VM);
				 totalMigrationLinkOverload ++;
			     sumRemovePOOL++;
		     }
		     else{
		        // cout << "Warning! no servers in the pool - ThreeDimensionSelection_OverLoaded" << endl;
		     }
		  }  
		  else {
		        // cout << "Warning! no servers !!! - ThreeDimensionSelection_OverLoaded" << endl;
		  }
	   }
	   else {
		   (*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, (*ServerActive)[selected].chassi, (*ServerActive)[selected].server, (*listVMs)[k].VM);
	       totalMigrationLinkOverload ++;
	   }
   }

   if (SIMULATES_POOL_SERVER) {
      if (sumRemovePOOL > 0) {
	     opoolServers->AddPowerOn(sumRemovePOOL);
	  }
   }
}


TwoDimensionWeight_Low::TwoDimensionWeight_Low(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX])
{
	ppoServers = pso;
}

void TwoDimensionWeight_Low::ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive)
{
	bool migrate;
	vector<STRUCMIGRATION> consolidate;

	totalMigrationLowUtilization = 0;


    if ((ServerActive->size() == 0) && (listVMs->size() == 1)) {
       return;
    }

    sort(listVMs->begin(), listVMs->end(), sortDecreasingUtilizationlistVMs);

    sort(ServerActive->begin(), ServerActive->end(), sortAscendantWeight);

    for (int k=0; k < static_cast<int> (listVMs->size()); k++) {
	    migrate = false;
	    if (ServerActive->size() > 0) {
           for (int l=0; l < static_cast<int> (ServerActive->size()); l++) {
//		 	   if ((((*ServerActive)[l].utilizationCPU + ((*listVMs)[k].VM->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) >= THRESHOLD_TOP_OF_USE_CPU) || ( ((*ServerActive)[l].utilizationMemory + (*listVMs)[k].VM->GetMemUseVM()) >= (*ServerActive)[l].memoryServer ) || ((*ServerActive)[l].temperature > EMERGENCY_TEMPERATURE-0.30) || ((*ServerActive)[l].trafficKBPS >= (0.80 * (*ServerActive)[l].speedKBPS)) || ((*ServerActive)[l].isMigrate)) {
		 	   if ((((*ServerActive)[l].utilizationCPU + ((*listVMs)[k].VM->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) >= THRESHOLD_TOP_OF_USE_CPU) || ( ((*ServerActive)[l].utilizationMemory + (*listVMs)[k].VM->GetMemUseVM()) >= (*ServerActive)[l].memoryServer ) || ((*ServerActive)[l].isMigrate)) {
				  continue;
			   }
		       (*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, (*ServerActive)[l].chassi, (*ServerActive)[l].server, (*listVMs)[k].VM);
			   (*ServerActive)[l].isMigrate = true;
			   totalMigrationLowUtilization++;
			   migrate = true;
			   break;
		   }
	    }
	    if (!migrate) {
	       consolidate.push_back((*listVMs)[k]);
        }
    }
   
    if (consolidate.size() >= 2) {
       sort(consolidate.begin(), consolidate.end(), sortDecreasingUtilizationCPU); 
       for(int k = 1; k < static_cast<int> (consolidate.size()); k++) {
		  if ((consolidate[0].chassi != consolidate[k].chassi) && (consolidate[0].server != consolidate[k].server))  {
		 	 (*ppoServers)[consolidate[k].chassi][consolidate[k].server]->MoveVMTo(ppoServers, consolidate[0].chassi, consolidate[0].server, consolidate[k].VM);
		 	 totalMigrationLowUtilization++;
             break; 
		  }
	   }
    }
    consolidate.clear();
}

TwoDimensionWeight_OverLoaded::TwoDimensionWeight_OverLoaded(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], POOLServers* opool)
{
	ppoServers = pso;
	opoolServers = opool;
}

void TwoDimensionWeight_OverLoaded::ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive)
{
   POOL sv;
   bool removePOOL;
   int sumRemovePOOL = 0;
   int selected = 0;

   totalMigrationOverLoaded = 0;
   totalMigrationPrecdiction = 0;

   sort(listVMs->begin(), listVMs->end(), sortDecreasingUtilizationlistVMs);
   sort(ServerActive->begin(), ServerActive->end(), sortAscendantWeight);

   for (int k=0; k < static_cast<int> (listVMs->size()); k++) {
	   removePOOL=true;
	   if (ServerActive->size() > 0) {
          for (int l=0; l < static_cast<int> (ServerActive->size()); l++) {
//			  if ((((*ServerActive)[l].utilizationCPU + ((*listVMs)[k].VM->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) >= THRESHOLD_TOP_OF_USE_CPU) || ( ((*ServerActive)[l].utilizationMemory + (*listVMs)[k].VM->GetMemUseVM()) >= (*ServerActive)[l].memoryServer ) || ((*ServerActive)[l].temperature > EMERGENCY_TEMPERATURE-0.30) || ((*ServerActive)[l].trafficKBPS >= (0.80 * (*ServerActive)[l].speedKBPS)) || ((*ServerActive)[l].isMigrate)) {
			  if ((((*ServerActive)[l].utilizationCPU + ((*listVMs)[k].VM->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) >= THRESHOLD_TOP_OF_USE_CPU) || ( ((*ServerActive)[l].utilizationMemory + (*listVMs)[k].VM->GetMemUseVM()) >= (*ServerActive)[l].memoryServer ) || ((*ServerActive)[l].isMigrate)) {

				  continue;
			  }
			  selected = l; 
			  (*ServerActive)[l].isMigrate = true;
			  removePOOL=false;
			  break;
		  }
	   }
       else {
		  removePOOL=true;	
	   }
	   if (removePOOL) {
 		  // Remove server POOL
          if (SIMULATES_POOL_SERVER) {
		     sv = opoolServers->RemoveServerPOOL(ppoServers);
		     if (sv.chassi != -1) {
				 (*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, sv.chassi, sv.server, (*listVMs)[k].VM);
				if ((*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->ReturnCPUPrediction()) {
				   totalMigrationPrecdiction ++;
			    }
			    else {
			   	   totalMigrationOverLoaded ++;
			    }
			    sumRemovePOOL++;
		     }
		     else{
		        // cout << "Warning! no servers in the pool - ThreeDimensionSelection_OverLoaded" << endl;
		     }
		  }  
		  else {
		        // cout << "Warning! no servers !!! - ThreeDimensionSelection_OverLoaded" << endl;
		  }
	   }
	   else {
		   (*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, (*ServerActive)[selected].chassi, (*ServerActive)[selected].server, (*listVMs)[k].VM);
		   if ((*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->ReturnCPUPrediction()) {
			  totalMigrationPrecdiction ++;
		   }
		   else {
		      totalMigrationOverLoaded ++;
		   }
	   }
   }

   if (SIMULATES_POOL_SERVER) {
      if (sumRemovePOOL > 0) {
	     opoolServers->AddPowerOn(sumRemovePOOL);
	  }
   }
}


ThreeDimensionMultiObj_OverLoaded_V2::ThreeDimensionMultiObj_OverLoaded_V2(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], POOLServers* opool, const FLOATINGPOINT HeatRecirculationMatrix[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX])
{
	ppoServers = pso;
	opoolServers = opool;

	for (int i=0; i < SIZE_OF_HR_MATRIX; i++) {
		for (int j=0; j < SIZE_OF_HR_MATRIX; j++) {
			HeatRecirculation[i][j] = HeatRecirculationMatrix[i][j];
		}
	}

}

void ThreeDimensionMultiObj_OverLoaded_V2::ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive)
{
   POOL sv;
   STRUCOPTIMIZATION addServer;
   bool removePOOL;
   int sumRemovePOOL = 0;
   int selected = 0;
   int powerON = 0;

   FLOATINGPOINT power[SIZE_OF_HR_MATRIX]; 
   FLOATINGPOINT futureTemperature = 0.00; 
   FLOATINGPOINT powerServer = 0.0;  
   FLOATINGPOINT estimateTemperatureServer[CHASSIS][SERVERS];


   totalMigrationOverLoaded = 0;
   totalMigrationPrecdiction = 0;

 
   if ((ServerActive->size() == 0) || (listVMs->size() == 0)) {
      return;
   }

   sort(listVMs->begin(), listVMs->end(), sortDecreasingUtilizationlistVMs);
   
   for (int k=0; k < static_cast<int> (listVMs->size()); k++) {
	   removePOOL=true;

	   sort(ServerActive->begin(), ServerActive->end(), sortDecreasingRanking);

	   // Calcula o consumo de todos os chassis
	   for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
	       power[i] = 0.0;
	       for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
		       power[i] += (*ppoServers)[i][j]->GetPowerDraw();
	       }
       }

	   if (ServerActive->size() > 0) {
          for (int l=0; l < static_cast<int> (ServerActive->size()); l++) {
 		 	  if ((((*ServerActive)[l].utilizationCPU + ((*listVMs)[k].VM->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) >= THRESHOLD_TOP_OF_USE_CPU) || ( ((*ServerActive)[l].utilizationMemory + (*listVMs)[k].VM->GetMemUseVM()) >= (*ServerActive)[l].memoryServer ) || ((*ServerActive)[l].isMigrate)) {
			     continue;
			  }

			  if ((*listVMs)[k].chassi != (*ServerActive)[l].chassi) {
			     power[(*ServerActive)[l].chassi] = power[(*ServerActive)[l].chassi] - (*ppoServers)[(*ServerActive)[l].chassi][(*ServerActive)[l].server]->GetPowerDraw() + (*ppoServers)[(*ServerActive)[l].chassi][(*ServerActive)[l].server]->EstimatePowerDrawWithTemperature((*ServerActive)[l].utilizationCPU, ((*listVMs)[k].VM->AverageCPU()/NUMBER_OF_CORES_IN_ONE_SERVER),(*ServerActive)[l].temperature);  
			  }
			  memset((void *)estimateTemperatureServer, 0.00, CHASSIS*SERVERS*sizeof(FLOATINGPOINT)); 
			  // calcula o calor de gerado por cada chassi
		      for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		          for (int j=0; j<NUMBER_OF_CHASSIS; ++j) {
  			          FLOATINGPOINT hFromJtoI  = power[j]*HeatRecirculation[i][j];
				      for (int p=0; p < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++p) {
				          estimateTemperatureServer[i][p] += hFromJtoI;
			          }  
		          }
	          }
			  
			  futureTemperature = estimateTemperatureServer[(*ServerActive)[l].chassi][(*ServerActive)[l].server] + (*ppoServers)[(*ServerActive)[l].chassi][(*ServerActive)[l].server]->ReadSupplyTempToTimingBuffer();
			  powerServer =  (*ppoServers)[(*ServerActive)[l].chassi][(*ServerActive)[l].server]->EstimatePowerDrawWithTemperature((*ServerActive)[l].utilizationCPU, (*listVMs)[k].VM->AverageCPU()/NUMBER_OF_CORES_IN_ONE_SERVER,futureTemperature);

			  selected = l; 
			  (*ServerActive)[l].isMigrate = true;
			  removePOOL=false;
			  break;
		  }
	   }
       else {
		  removePOOL=true;	
	   }
	   if (removePOOL) {
 		  // Remove server POOL
          if (SIMULATES_POOL_SERVER) {
		     sv = opoolServers->RemoveServerPOOL(ppoServers);
		     if (sv.chassi != -1) {
			    (*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, sv.chassi, sv.server, (*listVMs)[k].VM);
				if ((*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->ReturnCPUPrediction()) {
				   totalMigrationPrecdiction ++;
			    }
			    else {
			   	   totalMigrationOverLoaded ++;
			    }

				futureTemperature = estimateTemperatureServer[sv.chassi][sv.server] + (*ppoServers)[sv.chassi][sv.server]->ReadSupplyTempToTimingBuffer();
				powerServer =  (*ppoServers)[sv.chassi][sv.server]->EstimatePowerDrawWithTemperature((*ppoServers)[sv.chassi][sv.server]->VMRequiresThisMuchUtilization(), (*listVMs)[k].VM->AverageCPU()/NUMBER_OF_CORES_IN_ONE_SERVER,futureTemperature);

				addServer.chassi = sv.chassi;
				addServer.server = sv.server; 
				addServer.utilizationCPU = (*ppoServers)[sv.chassi][sv.server]->VMRequiresThisMuchUtilization();
				addServer.temperature = futureTemperature;
				addServer.averageUtilizationCPU = 0;
				addServer.utilizationMemory = (*ppoServers)[sv.chassi][sv.server]->VMRequiresThisMemory(); 
				addServer.memoryServer = (*ppoServers)[sv.chassi][sv.server]->GetMemoryServer();
				addServer.trafficKBPS = 0;
				addServer.averageTrafficKBPS = 0;
				addServer.speedKBPS = (*ppoServers)[sv.chassi][sv.server]->ReturnBandWidthServerKBPS();
				addServer.power = powerServer;
				addServer.temperatureFuture = 0; 
				addServer.predictedMigration = false; 
				addServer.isMigrate = false;
				addServer.dcload = (*ServerActive)[0].dcload;
				addServer.ranking  =  (*ppoServers)[addServer.chassi][addServer.server]->CalculationRanking(futureTemperature, powerServer, (*ServerActive)[selected].dcload);
				
				addServer.rankingWeight = 0;
			   	ServerActive->push_back(addServer);

			    sumRemovePOOL++;
		     }
		     else{
		        // cout << "Warning! no servers in the pool - ThreeDimensionSelection_OverLoaded" << endl;
		     }
		  }  
		  else {
		        // cout << "Warning! no servers !!! - ThreeDimensionSelection_OverLoaded" << endl;
		  }
	   }
	   else {
		   (*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, (*ServerActive)[selected].chassi, (*ServerActive)[selected].server, (*listVMs)[k].VM);
		   if ((*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->ReturnCPUPrediction()) {
			  totalMigrationPrecdiction ++;
		   }
		   else {
		      totalMigrationOverLoaded ++;
		   }

		   (*ServerActive)[selected].utilizationCPU = (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->VMRequiresThisMuchUtilization();
		   (*ServerActive)[selected].utilizationMemory = (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->VMRequiresThisMemory(); 
		   (*ServerActive)[selected].trafficKBPS = (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->ReturnServerTrafficKBPS();
		   (*ServerActive)[selected].temperature = futureTemperature;
		   (*ServerActive)[selected].power = powerServer;
		   (*ServerActive)[selected].isMigrate = true;
		   (*ServerActive)[selected].ranking  = (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->CalculationRanking(futureTemperature, powerServer, (*ServerActive)[selected].dcload);
		   

	   }
   }
   if (SIMULATES_POOL_SERVER) {
      if (sumRemovePOOL > 0) {
	     opoolServers->AddPowerOn(sumRemovePOOL);
	  }
   }
   else{
/*	  if (listVMs->size() > (totalMigrationPrecdiction + totalMigrationOverLoaded)) {
		  powerON = (int) ceil(((double) ( listVMs->size() - (totalMigrationPrecdiction + totalMigrationOverLoaded) ) / (double) NUMBER_OF_CORES_IN_ONE_SERVER ));
		  //cout << " listVMs->size() " << listVMs->size() << " totalMigrationPrecdiction + totalMigrationOverLoaded) " << (totalMigrationPrecdiction + totalMigrationOverLoaded) << " powerON " << powerON << endl;
		  opoolServers->AddPowerOn(powerON);
	  }*/
   }
}


 ThreeDimensionMultiObj_Low_V2::ThreeDimensionMultiObj_Low_V2(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], const FLOATINGPOINT HeatRecirculationMatrix[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX])
{
	ppoServers = pso;

	for (int i=0; i < SIZE_OF_HR_MATRIX; i++) {
		for (int j=0; j < SIZE_OF_HR_MATRIX; j++) {
			HeatRecirculation[i][j] = HeatRecirculationMatrix[i][j];
		}
	}
 
}
 

void ThreeDimensionMultiObj_Low_V2::ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive)
{
	bool migrate;
	FLOATINGPOINT power[SIZE_OF_HR_MATRIX]; 
    FLOATINGPOINT estimateTemperatureServer[CHASSIS][SERVERS];
	FLOATINGPOINT powerServer = 0.0;  
    FLOATINGPOINT futureTemperature = 0.00; 
	vector<STRUCMIGRATION> consolidate;
	totalMigrationLowUtilization = 0;

	if ((ServerActive->size() == 0) && (listVMs->size() == 1)) {
       return;
    }

    sort(listVMs->begin(), listVMs->end(), sortDecreasingUtilizationlistVMs);

    for (int k=0; k < static_cast<int> (listVMs->size()); k++) {
	    migrate = false;

		sort(ServerActive->begin(), ServerActive->end(), sortDecreasingRanking);

	    // Calcula o consumo de todos os chassis
	    for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
	        power[i] = 0.0;
	        for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
		        power[i] += (*ppoServers)[i][j]->GetPowerDraw();
	        }
        }

	    if (ServerActive->size() > 0) {
           for (int l=0; l < static_cast<int> (ServerActive->size()); l++) {
		 	   if ((((*ServerActive)[l].utilizationCPU + ((*listVMs)[k].VM->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) >= THRESHOLD_TOP_OF_USE_CPU) || ( ((*ServerActive)[l].utilizationMemory + (*listVMs)[k].VM->GetMemUseVM()) >= (*ServerActive)[l].memoryServer ) || ((*ServerActive)[l].isMigrate)) {
				  continue;
			   }

			   (*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, (*ServerActive)[l].chassi, (*ServerActive)[l].server, (*listVMs)[k].VM);
			   totalMigrationLowUtilization++;

			   if ((*listVMs)[k].chassi != (*ServerActive)[l].chassi) {
			     power[(*ServerActive)[l].chassi] = power[(*ServerActive)[l].chassi] - (*ppoServers)[(*ServerActive)[l].chassi][(*ServerActive)[l].server]->GetPowerDraw() + (*ppoServers)[(*ServerActive)[l].chassi][(*ServerActive)[l].server]->EstimatePowerDrawWithTemperature((*ServerActive)[l].utilizationCPU, ((*listVMs)[k].VM->AverageCPU()/NUMBER_OF_CORES_IN_ONE_SERVER),(*ServerActive)[l].temperature);  
			   }

			   // calcula o calor de gerado por cada chassi
			   memset((void *)estimateTemperatureServer, 0.00, CHASSIS*SERVERS*sizeof(FLOATINGPOINT)); 
		       for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		           for (int j=0; j<NUMBER_OF_CHASSIS; ++j) {
  			           FLOATINGPOINT hFromJtoI  = power[j]*HeatRecirculation[i][j];
				       for (int p=0; p < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++p) {
				           estimateTemperatureServer[i][p] += hFromJtoI;
			           }  
		           }
	           }
			   futureTemperature = estimateTemperatureServer[(*ServerActive)[l].chassi][(*ServerActive)[l].server] + (*ppoServers)[(*ServerActive)[l].chassi][(*ServerActive)[l].server]->ReadSupplyTempToTimingBuffer();
			   powerServer =  (*ppoServers)[(*ServerActive)[l].chassi][(*ServerActive)[l].server]->EstimatePowerDrawWithTemperature((*ServerActive)[l].utilizationCPU, (*listVMs)[k].VM->AverageCPU()/NUMBER_OF_CORES_IN_ONE_SERVER,futureTemperature);

			   (*ServerActive)[l].utilizationCPU = (*ppoServers)[(*ServerActive)[l].chassi][(*ServerActive)[l].server]->VMRequiresThisMuchUtilization();
			   (*ServerActive)[l].utilizationMemory = (*ppoServers)[(*ServerActive)[l].chassi][(*ServerActive)[l].server]->VMRequiresThisMemory(); 
			   (*ServerActive)[l].trafficKBPS = (*ppoServers)[(*ServerActive)[l].chassi][(*ServerActive)[l].server]->ReturnServerTrafficKBPS();
			   (*ServerActive)[l].temperature = futureTemperature;
			   (*ServerActive)[l].power = powerServer;
			   (*ServerActive)[l].ranking  =  (*ppoServers)[(*ServerActive)[l].chassi][(*ServerActive)[l].server]->CalculationRanking(futureTemperature, powerServer, (*ServerActive)[l].dcload);
			   (*ServerActive)[l].isMigrate = true;

			   migrate = true;
			   break;
		   }
	    }
	    if (!migrate) {
	       consolidate.push_back((*listVMs)[k]);
        }
    }
   
    if (consolidate.size() >= 2) {
       sort(consolidate.begin(), consolidate.end(), sortDecreasingUtilizationCPU); 
       for(int k = 1; k < static_cast<int> (consolidate.size()); k++) {
		  if ((consolidate[0].chassi != consolidate[k].chassi) && (consolidate[0].server != consolidate[k].server))  {
		 	 (*ppoServers)[consolidate[k].chassi][consolidate[k].server]->MoveVMTo(ppoServers, consolidate[0].chassi, consolidate[0].server, consolidate[k].VM);
		 	 totalMigrationLowUtilization++;
             break; 
		  }
	   }
    }
    consolidate.clear();
}


PABFD_Temperature::PABFD_Temperature(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], POOLServers* opool)
{
	ppoServers = pso;
	opoolServers = opool;
}

void PABFD_Temperature::ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive)
{
   POOL sv;
   bool removePOOL;
   int sumRemovePOOL = 0;
   int selected = 0;
   double powerServer = 0.00;
   double minPowerServer = 9999999.00;

   totalMigrationHighTemperature = 0;



   if ((ServerActive->size() == 0) || (listVMs->size() == 0)) {
      return;
   }

   sort(listVMs->begin(), listVMs->end(), sortDecreasingUtilizationlistVMs);

   for (int k=0; k < static_cast<int> (listVMs->size()); k++) {
	   removePOOL=true;
	   minPowerServer = 9999999;
	   if (ServerActive->size() > 0) {
          for (int l=0; l < static_cast<int> (ServerActive->size()); l++) {
			  if ((((*ServerActive)[l].utilizationCPU + ((*listVMs)[k].VM->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) >= THRESHOLD_TOP_OF_USE_CPU) || ( ((*ServerActive)[l].utilizationMemory + (*listVMs)[k].VM->GetMemUseVM()) >= (*ServerActive)[l].memoryServer ) || ((*ServerActive)[l].isMigrate)) {
				  continue;
			  }
			  powerServer =  (*ppoServers)[(*ServerActive)[l].chassi][(*ServerActive)[l].server]->EstimatePowerDraw((*ServerActive)[l].utilizationCPU, ((*listVMs)[k].VM->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER));
			  if (powerServer < minPowerServer) {
				 selected = l;
				 minPowerServer = powerServer;   
                 removePOOL=false;
			  }
		  }
	   }
       else {
		  removePOOL=true;	
	   }
	   if (removePOOL) {
 		  // Remove server POOL
          if (SIMULATES_POOL_SERVER) {
		     sv = opoolServers->RemoveServerPOOL(ppoServers);
		     if (sv.chassi != -1) {
				(*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, sv.chassi, sv.server, (*listVMs)[k].VM);
				totalMigrationHighTemperature ++;
				sumRemovePOOL++;
		     }
		     else{
		        //cout << "Warning! no servers in the pool - PoliceFFD" << endl;
		     }
		  }  
		  else {
		        // cout << "Warning! no servers !!! - PABFD_OverLoaded" << endl;
		  }
	   }
	   else {
          (*ServerActive)[selected].isMigrate = true;
		  (*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, (*ServerActive)[selected].chassi, (*ServerActive)[selected].server, (*listVMs)[k].VM);
		  totalMigrationHighTemperature ++;
	  }
   }

   if (SIMULATES_POOL_SERVER) {
      if (sumRemovePOOL > 0) {
	     opoolServers->AddPowerOn(sumRemovePOOL);
	  }
   }

}




ThreeDimensionMultiObj_Low_V3::ThreeDimensionMultiObj_Low_V3(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], const FLOATINGPOINT HeatRecirculationMatrix[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX])
{
	ppoServers = pso;

	for (int i=0; i < SIZE_OF_HR_MATRIX; i++) {
		for (int j=0; j < SIZE_OF_HR_MATRIX; j++) {
			HeatRecirculation[i][j] = HeatRecirculationMatrix[i][j];
		}
	}
 
}

void ThreeDimensionMultiObj_Low_V3::ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive)
{
	bool migrate;
	int maxRanking = 0;
	int selected = 0;

	vector<STRUCMIGRATION> consolidate;

	totalMigrationLowUtilization = 0;

	FLOATINGPOINT powerServer = 0.0;  
    FLOATINGPOINT estimateTemperatureServer[CHASSIS][SERVERS];
	FLOATINGPOINT futureTemperature = 0.00; 
	FLOATINGPOINT power[SIZE_OF_HR_MATRIX]; 

	FLOATINGPOINT TLow = 0.00;
	FLOATINGPOINT THight = 34.00;
	FLOATINGPOINT CPULow = 0.00;
	FLOATINGPOINT CPUHight = 1.00;
	FLOATINGPOINT PowerLow = 130;
	FLOATINGPOINT PowerHight = 305;
	FLOATINGPOINT MemoryLow = 0;
	FLOATINGPOINT MemoryHight = 128000000;
	FLOATINGPOINT EFF_Temp = 0.00;
	FLOATINGPOINT EFF_CPU = 0;
	FLOATINGPOINT EFF_Power = 0;
	FLOATINGPOINT EFF_Memory = 0;
	FLOATINGPOINT EFF_NetWork = 0;

	FLOATINGPOINT loadDC = 0;

	if ((ServerActive->size() == 0) && (listVMs->size() == 1)) {
       return;
    }

	if (ServerActive->size() != 0) {
	   loadDC = (*ServerActive)[0].dcload;  // return processing load Data Center
	}
	else {
	   loadDC = 0;
	}
    sort(listVMs->begin(), listVMs->end(), sortDecreasingUtilizationlistVMs);


	// Calculates the consumption of all chassis
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
	    power[i] = 0.0;
	    for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
	        power[i] += (*ppoServers)[i][j]->GetPowerDraw();
	    }
	}

    for (int k=0; k < static_cast<int> (listVMs->size()); k++) {
	   maxRanking = 0;
	   migrate = false;
	   if (ServerActive->size() > 0) {
          for (int l=0; l < static_cast<int> (ServerActive->size()); l++) {
			  if ((((*ServerActive)[l].utilizationCPU + ((*listVMs)[k].VM->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) >= THRESHOLD_TOP_OF_USE_CPU) || ( ((*ServerActive)[l].utilizationMemory + (*listVMs)[k].VM->GetMemUseVM()) >= (*ServerActive)[l].memoryServer ) || ((*ServerActive)[l].isMigrate)) {
				 continue;
			  }
			  if ((*ServerActive)[l].ranking > maxRanking) {
				 selected = l;
				 maxRanking = (*ServerActive)[l].ranking;  
				 migrate = true;
			  }
		  }
	   }
 	   if (migrate) {
		  (*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, (*ServerActive)[selected].chassi, (*ServerActive)[selected].server, (*listVMs)[k].VM);
		  totalMigrationLowUtilization++;
		  (*ServerActive)[selected].isMigrate = true;


		  if ((*listVMs)[k].chassi != (*ServerActive)[selected].chassi) {
		     power[(*ServerActive)[selected].chassi] = power[(*ServerActive)[selected].chassi] - (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->GetPowerDraw() + (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->EstimatePowerDrawWithTemperature((*ServerActive)[selected].utilizationCPU, ((*listVMs)[k].VM->AverageCPU()/NUMBER_OF_CORES_IN_ONE_SERVER),(*ServerActive)[selected].temperature);  
		  }
		  memset((void *)estimateTemperatureServer, 0.00, CHASSIS*SERVERS*sizeof(FLOATINGPOINT)); 
		  // calcula o calor de gerado por cada chassi
		  for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		      for (int j=0; j<NUMBER_OF_CHASSIS; ++j) {
  			      FLOATINGPOINT hFromJtoI  = power[j]*HeatRecirculation[i][j];
				  for (int p=0; p < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++p) {
				      estimateTemperatureServer[i][p] += hFromJtoI;
			      }  
			  }
	      }

		  futureTemperature = estimateTemperatureServer[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server] + (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->ReadSupplyTempToTimingBuffer();
		  powerServer =  (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->EstimatePowerDrawWithTemperature((*ServerActive)[selected].utilizationCPU, (*listVMs)[k].VM->AverageCPU()/NUMBER_OF_CORES_IN_ONE_SERVER,futureTemperature);

		  (*ServerActive)[selected].temperature = futureTemperature;
	  	  (*ServerActive)[selected].utilizationCPU = (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->VMRequiresThisMuchUtilization();
		  (*ServerActive)[selected].averageUtilizationCPU = 0;
		  (*ServerActive)[selected].utilizationMemory = (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->VMRequiresThisMemory(); 
		  (*ServerActive)[selected].memoryServer = (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->GetMemoryServer();
		  (*ServerActive)[selected].trafficKBPS = (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->ReturnServerTrafficKBPS();
		  (*ServerActive)[selected].averageTrafficKBPS = (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->AverageServerTrafficKBPS();
		  (*ServerActive)[selected].speedKBPS = (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->ReturnBandWidthServerKBPS();
		  (*ServerActive)[selected].power = powerServer;

	      EFF_CPU = 1 - (((*ServerActive)[selected].utilizationCPU - CPULow) / (CPUHight - CPULow));
		  EFF_Power = 1 - (((*ServerActive)[selected].power - PowerLow) / (PowerHight - PowerLow));
		  EFF_Memory = ((((*ServerActive)[selected].memoryServer - (*ServerActive)[selected].utilizationMemory) - MemoryLow) / (MemoryHight - MemoryLow));
		  EFF_Temp = 1 - pow((((*ServerActive)[selected].temperature - TLow) / (THight - TLow)), 3.00);
		  EFF_NetWork = 1 - pow((((*ServerActive)[selected].trafficKBPS - 0) / (FLOATINGPOINT ((*ServerActive)[selected].speedKBPS  - 0))), 2.00); 


		  if (loadDC <= 0.40) {
			 (*ServerActive)[selected].ranking =  EFF_Power; 
		  }
		  else {
			 (*ServerActive)[selected].ranking =  EFF_CPU + EFF_Power + EFF_Memory + EFF_Temp + EFF_NetWork;
			 
		  }
        }
	    else {
		  consolidate.push_back((*listVMs)[k]);
	    }
    }

    if (consolidate.size() >= 2) {
       sort(consolidate.begin(), consolidate.end(), sortDecreasingUtilizationCPU); 
       for(int k = 1; k < static_cast<int> (consolidate.size()); k++) {
		  if ((consolidate[0].chassi != consolidate[k].chassi) && (consolidate[0].server != consolidate[k].server))  {
		 	 (*ppoServers)[consolidate[k].chassi][consolidate[k].server]->MoveVMTo(ppoServers, consolidate[0].chassi, consolidate[0].server, consolidate[k].VM);
		 	 totalMigrationLowUtilization++;
             break; 
		  }
	   }
    }
    consolidate.clear();
}



ThreeDimensionMultiObj_OverLoaded_V3::ThreeDimensionMultiObj_OverLoaded_V3(Server* (*pso)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], POOLServers* opool, const FLOATINGPOINT HeatRecirculationMatrix[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX])
{
	ppoServers = pso;
	opoolServers = opool;

	for (int i=0; i < SIZE_OF_HR_MATRIX; i++) {
		for (int j=0; j < SIZE_OF_HR_MATRIX; j++) {
			HeatRecirculation[i][j] = HeatRecirculationMatrix[i][j];
		}
	}

}

void ThreeDimensionMultiObj_OverLoaded_V3::ServerOptimization(int clockSimulation, vector<STRUCMIGRATION> *listVMs, vector<STRUCOPTIMIZATION> *ServerActive)
{
   POOL sv;
   STRUCOPTIMIZATION addServer;
   bool removePOOL;
   bool migrate;
   int sumRemovePOOL = 0;
   int selected = 0;
   int maxRanking = 0;

   totalMigrationOverLoaded = 0;
   totalMigrationPrecdiction = 0;


   FLOATINGPOINT powerServer = 0.0;  
   FLOATINGPOINT estimateTemperatureServer[CHASSIS][SERVERS];
   FLOATINGPOINT futureTemperature = 0.00; 
   FLOATINGPOINT power[SIZE_OF_HR_MATRIX]; 


   	FLOATINGPOINT TLow = 0.00;
	FLOATINGPOINT THight = 34.00;
	FLOATINGPOINT CPULow = 0.00;
	FLOATINGPOINT CPUHight = 1.00;
	FLOATINGPOINT PowerLow = 130;
	FLOATINGPOINT PowerHight = 305;
	FLOATINGPOINT MemoryLow = 0;
	FLOATINGPOINT MemoryHight = 128000000;
	FLOATINGPOINT EFF_Temp = 0.00;
	FLOATINGPOINT EFF_CPU = 0;
	FLOATINGPOINT EFF_Power = 0;
	FLOATINGPOINT EFF_Memory = 0;
	FLOATINGPOINT EFF_NetWork = 0;
	FLOATINGPOINT loadDC = 0;

   if ((ServerActive->size() == 0) || (listVMs->size() == 0)) {
      return;
   }

   loadDC = (*ServerActive)[0].dcload;  // return processing load Data Center

   sort(listVMs->begin(), listVMs->end(), sortDecreasingUtilizationlistVMs);
 
   for (int k=0; k < static_cast<int> (listVMs->size()); k++) {
	   removePOOL=true;
  	   maxRanking = 0;

	   // Calcula o consumo de todos os chassis
	   for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
	       power[i] = 0.0;
	       for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
		       power[i] += (*ppoServers)[i][j]->GetPowerDraw();
	       }
       }

	   if (ServerActive->size() > 0) {
		   migrate = false;
		   for (int l=0; l < static_cast<int> (ServerActive->size()); l++) {
			   if ((((*ServerActive)[l].utilizationCPU + ((*listVMs)[k].VM->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) >= THRESHOLD_TOP_OF_USE_CPU) || ( ((*ServerActive)[l].utilizationMemory + (*listVMs)[k].VM->GetMemUseVM()) >= (*ServerActive)[l].memoryServer ) || ((*ServerActive)[l].isMigrate)) {
				  continue;
			   }
			  if ((*ServerActive)[l].ranking > maxRanking) {
				 selected = l;
				 maxRanking = (*ServerActive)[l].ranking;  
				 migrate = true;
				 removePOOL=false;
			  }
		  }
		  if (migrate) {

			 (*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, (*ServerActive)[selected].chassi, (*ServerActive)[selected].server, (*listVMs)[k].VM);

		     if ((*listVMs)[k].chassi != (*ServerActive)[selected].chassi) {
			     power[(*ServerActive)[selected].chassi] = power[(*ServerActive)[selected].chassi] - (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->GetPowerDraw() + (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->EstimatePowerDrawWithTemperature((*ServerActive)[selected].utilizationCPU, ((*listVMs)[k].VM->AverageCPU()/NUMBER_OF_CORES_IN_ONE_SERVER),(*ServerActive)[selected].temperature);  
			 }
			 memset((void *)estimateTemperatureServer, 0.00, CHASSIS*SERVERS*sizeof(FLOATINGPOINT)); 
			 // calcula o calor de gerado por cada chassi
		     for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		         for (int j=0; j<NUMBER_OF_CHASSIS; ++j) {
  			         FLOATINGPOINT hFromJtoI  = power[j]*HeatRecirculation[i][j];
				     for (int p=0; p < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++p) {
				         estimateTemperatureServer[i][p] += hFromJtoI;
			         }  
		         }
	         }
			  
			 futureTemperature = estimateTemperatureServer[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server] + (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->ReadSupplyTempToTimingBuffer();
			 powerServer =  (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->EstimatePowerDrawWithTemperature((*ServerActive)[selected].utilizationCPU, (*listVMs)[k].VM->AverageCPU()/NUMBER_OF_CORES_IN_ONE_SERVER,futureTemperature);

			 (*ServerActive)[selected].isMigrate = true;
			 removePOOL=false;

		     (*ServerActive)[selected].temperature = futureTemperature;
		     (*ServerActive)[selected].utilizationCPU = (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->VMRequiresThisMuchUtilization();
		     (*ServerActive)[selected].averageUtilizationCPU = 0;
		     (*ServerActive)[selected].utilizationMemory = (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->VMRequiresThisMemory(); 
		     (*ServerActive)[selected].memoryServer = (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->GetMemoryServer();
		     (*ServerActive)[selected].trafficKBPS = (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->ReturnServerTrafficKBPS();
		     (*ServerActive)[selected].averageTrafficKBPS = (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->AverageServerTrafficKBPS();
		     (*ServerActive)[selected].speedKBPS = (*ppoServers)[(*ServerActive)[selected].chassi][(*ServerActive)[selected].server]->ReturnBandWidthServerKBPS();
		     (*ServerActive)[selected].power = powerServer;
	         EFF_CPU = 1 - (((*ServerActive)[selected].utilizationCPU - CPULow) / (CPUHight - CPULow));
		     EFF_Power = 1 - (((*ServerActive)[selected].power - PowerLow) / (PowerHight - PowerLow));
		     EFF_Memory = ((((*ServerActive)[selected].memoryServer - (*ServerActive)[selected].utilizationMemory) - MemoryLow) / (MemoryHight - MemoryLow));
		     EFF_Temp = 1 - pow((((*ServerActive)[selected].temperature - TLow) / (THight - TLow)), 3.00);
		     EFF_NetWork = 1 - pow((((*ServerActive)[selected].trafficKBPS - 0) / (FLOATINGPOINT ((*ServerActive)[selected].speedKBPS  - 0))), 2.00); 

			 if (loadDC <= 0.40) {
			    (*ServerActive)[selected].ranking =  EFF_Power; 
		     }
		        else {
			    (*ServerActive)[selected].ranking =  EFF_CPU + EFF_Power + EFF_Memory + EFF_Temp + EFF_NetWork;
			 
		     }

		     if ((*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->ReturnCPUPrediction()) {
			    totalMigrationPrecdiction ++;
		     }
		     else {
		        totalMigrationOverLoaded ++;
		     }
		  }
          else {
	          if (removePOOL) {
 		         // Remove server POOL
                 if (SIMULATES_POOL_SERVER) {
		            sv = opoolServers->RemoveServerPOOL(ppoServers);
		            if (sv.chassi != -1) {
			           (*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->MoveVMTo(ppoServers, sv.chassi, sv.server, (*listVMs)[k].VM);

					   addServer.chassi = sv.chassi;
					   addServer.server = sv.server; 
					   addServer.temperature = (*ppoServers)[sv.chassi][sv.server]->CurrentInletTemperature();
					   addServer.utilizationCPU = (*ppoServers)[sv.chassi][sv.server]->VMRequiresThisMuchUtilization();
					   addServer.averageUtilizationCPU = 0;
					   addServer.utilizationMemory = (*ppoServers)[sv.chassi][sv.server]->VMRequiresThisMemory(); 
				 	   addServer.memoryServer = (*ppoServers)[sv.chassi][sv.server]->GetMemoryServer();
				 	   addServer.trafficKBPS = 0;
					   addServer.averageTrafficKBPS = 0;
					   addServer.speedKBPS = (*ppoServers)[sv.chassi][sv.server]->ReturnBandWidthServerKBPS();


					   if ((*listVMs)[k].chassi != addServer.chassi) {
			              power[addServer.chassi] = power[addServer.chassi] - (*ppoServers)[addServer.chassi][addServer.server]->GetPowerDraw() + (*ppoServers)[addServer.chassi][addServer.server]->EstimatePowerDrawWithTemperature(0, ((*listVMs)[k].VM->AverageCPU()/NUMBER_OF_CORES_IN_ONE_SERVER),addServer.temperature);  
			           }
					   memset((void *)estimateTemperatureServer, 0.00, CHASSIS*SERVERS*sizeof(FLOATINGPOINT)); 
			           // calcula o calor de gerado por cada chassi
		               for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		                   for (int j=0; j<NUMBER_OF_CHASSIS; ++j) {
  			                   FLOATINGPOINT hFromJtoI  = power[j]*HeatRecirculation[i][j];
				               for (int p=0; p < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++p) {
				                   estimateTemperatureServer[i][p] += hFromJtoI;
			                   }  
		                  } 
	                   }


					   futureTemperature = estimateTemperatureServer[sv.chassi][sv.server] + (*ppoServers)[sv.chassi][sv.server]->ReadSupplyTempToTimingBuffer();
					   powerServer =  (*ppoServers)[sv.chassi][sv.server]->EstimatePowerDrawWithTemperature(addServer.utilizationCPU, (*listVMs)[k].VM->AverageCPU()/NUMBER_OF_CORES_IN_ONE_SERVER,futureTemperature);

					   addServer.power = powerServer;
					   addServer.temperatureFuture = 0; 
					   addServer.temperature = futureTemperature;
					   addServer.predictedMigration = false; 
					   addServer.isMigrate = false;

					   EFF_CPU = 1 - ((addServer.utilizationCPU  - CPULow) / (CPUHight - CPULow));
					   EFF_Power = 1 - ((addServer.power - PowerLow) / (PowerHight - PowerLow));
					   EFF_Memory = (((addServer.memoryServer - addServer.utilizationMemory) - MemoryLow) / (MemoryHight - MemoryLow));
					   EFF_Temp = 1 - pow(((addServer.temperature- TLow) / (THight - TLow)), 3.00);
					   EFF_NetWork = 1 - pow(((addServer.trafficKBPS - 0) / (FLOATINGPOINT (addServer.speedKBPS - 0))), 2.00); 

					   if (loadDC <= 0.40) {
						  addServer.ranking =  EFF_Power; 
					   }
					   else {
						  addServer.ranking =  EFF_CPU + EFF_Power + EFF_Memory + EFF_Temp + EFF_NetWork;
					   }
					   
			   		   ServerActive->push_back(addServer);

					   if ((*ppoServers)[(*listVMs)[k].chassi][(*listVMs)[k].server]->ReturnCPUPrediction()) {
						  totalMigrationPrecdiction ++;
					   }
					   else {
			   			  totalMigrationOverLoaded ++;
					   }
					   sumRemovePOOL++;
		            }
		            else{
		               // cout << "Warning! no servers in the pool - ThreeDimensionSelection_OverLoaded" << endl;
		            }
				 }
		      }  
		      else {
		          // cout << "Warning! no servers !!! - ThreeDimensionSelection_OverLoaded V3" << endl;
		      }
	      }
	   }
   }

   if (SIMULATES_POOL_SERVER) {
      if (sumRemovePOOL > 0) {
	     opoolServers->AddPowerOn(sumRemovePOOL);
	  }
   }
}

