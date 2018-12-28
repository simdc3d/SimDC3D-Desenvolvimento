#include <algorithm>

#include "SimDC3D-PoolServers.h"


using namespace std;


POOLServers::POOLServers(void)
{
 clockSimulation = 0;
 timeOfEmergency = -7200;
 totalPowerOn = 0;
 totalPowerOff = 0;
 sumOff = 0;

 serversPowerOFF.clear();
 serversPowerON.clear();
 serversPOOL.clear();
 serversON.clear();
 sumPowerOff.clear();
 
 }

POOLServers::~POOLServers(void)
{
 serversPowerOFF.clear();
 serversPowerON.clear();
 serversPOOL.clear();
 serversON.clear();
 sumPowerOff.clear();
 
}

void POOLServers::InsertVectorServersPOOL(int Chassi, int Server, double Temperature)
{
     POOL server;

     server.chassi = Chassi;
	 server.server = Server;
	 server.temperature = Temperature;

	 serversPOOL.push_back(server);
}

void POOLServers::InsertVectorServersPowerOFF(int Chassi, int Server, double Temperature)
{
     POOL server;

	 server.chassi = Chassi;
	 server.server = Server;
	 server.temperature = Temperature;

	 serversPowerOFF.push_back(server);
}


void POOLServers::InsertVectorServersPowerON(int Chassi, int Server, double Temperature)
{
     POOL server;

	 server.chassi = Chassi;
	 server.server = Server;
	 server.temperature = Temperature;

	 serversPowerON.push_back(server);
}


void POOLServers::InsertVectorServersON(int Chassi, int Server, double Temperature)
{
     POOL server;

	 server.chassi = Chassi;
	 server.server = Server;
	 server.temperature = Temperature;

	 serversON.push_back(server);
}

void POOLServers:: SortVectorServersPowerOFF(void)
{

  sort(serversPowerOFF.begin(), serversPowerOFF.end(), sortPOOLServer);
}

void POOLServers::SortVectorServersPOOL(void)
{
  sort(serversPOOL.begin(), serversPOOL.end(), sortPOOLServer);
}

POOL POOLServers::RemoveServerPOOL(Server* (*sv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX])
{
 POOL server;
     
 if (serversPOOL.size() > 0) {
    //update the temperature
	for (int i=0; i < serversPOOL.size(); i++){
        serversPOOL[i].temperature=(*sv)[serversPOOL[i].chassi][serversPOOL[i].server]->CurrentInletTemperature();
    }

    SortVectorServersPOOL();
 
    server.chassi = serversPOOL[0].chassi;
	server.server = serversPOOL[0].server;
	(*sv)[serversPOOL[0].chassi][serversPOOL[0].server]->ServerRead();
	InsertVectorServersON(serversPOOL[0].chassi,serversPOOL[0].server, serversPOOL[0].temperature);
	serversPOOL.erase(serversPOOL.begin());
 }    
 else {
	server.chassi = -1;
	server.server = -1;
 }
 
 return server;
}

void POOLServers::ServerPowerON(Server* (*sv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], int wakeUP)
{
  int wakeUPServers = 0;

  if (serversPowerOFF.size() > 0) {
     //update the temperature
	 for (int i=0; i < serversPowerOFF.size(); i++){
	     serversPowerOFF[i].temperature=(*sv)[serversPowerOFF[i].chassi][serversPowerOFF[i].server]->CurrentInletTemperature();
     }

     SortVectorServersPowerOFF();
 
	 for (int j=0; j < wakeUP; j++) {
         if (serversPowerOFF.size() > 0) {
	        InsertVectorServersPowerON(serversPowerOFF[0].chassi, serversPowerOFF[0].server, serversPowerOFF[0].temperature);
			if (HIBERNATING_SERVER) {
			   (*sv)[serversPowerOFF[0].chassi][serversPowerOFF[0].server]->ExitHibernation(clockSimulation);
			}
			else {
			   (*sv)[serversPowerOFF[0].chassi][serversPowerOFF[0].server]->TurnON(clockSimulation);
			}
		    serversPowerOFF.erase(serversPowerOFF.begin());
	     }
	     else {
            cout << "SimDC3D - Warning: All servers are power on!!!" << endl;
		    break;
	     }  
     }
  

	 if (SIMULATES_POOL_SERVER) {
        //If the size pool is below 30%, increases the pool size
	    if (INCREASE_SIZE_POOL_DYNAMICALLY) {
		   if ((serversPOOL.size() < (0.50 * SIZE_POOL)) && (clockSimulation > (timeOfEmergency + 7200))) {
              wakeUPServers = int (SIZE_POOL * 0.50); // increase the pool size of 50%
	          timeOfEmergency = clockSimulation;
              for (int i=0; i < wakeUPServers; i++) {
                  if (serversPowerOFF.size() > 0) {
	                 InsertVectorServersPowerON(serversPowerOFF[0].chassi, serversPowerOFF[0].server, serversPowerOFF[0].temperature);
				     if (HIBERNATING_SERVER) {
					    (*sv)[serversPowerOFF[0].chassi][serversPowerOFF[0].server]->ExitHibernation(clockSimulation);
				     }
				     else {
					    (*sv)[serversPowerOFF[0].chassi][serversPowerOFF[0].server]->TurnON(clockSimulation);
				     }
		             serversPowerOFF.erase(serversPowerOFF.begin());
	              }
	              else {
                     cout << "All servers are power on!!!" << endl;
		             break;
	             }
              }
           }

           //If the size pool is equal 0%, increases the pool size
           if ((serversPOOL.size() == 0) && (clockSimulation > (timeOfEmergency + POWER_ON))) {
              wakeUPServers = int (SIZE_POOL * 0.50); // increase the pool size of 50%
	          timeOfEmergency = clockSimulation;
              for (int j=0; j < wakeUPServers; j++) {
                  if (serversPowerOFF.size() > 0) {
	                 InsertVectorServersPowerON(serversPowerOFF[0].chassi, serversPowerOFF[0].server, serversPowerOFF[0].temperature);
				     if (HIBERNATING_SERVER) {
					    (*sv)[serversPowerOFF[0].chassi][serversPowerOFF[0].server]->ExitHibernation(clockSimulation);
				     }
				     else {
					    (*sv)[serversPowerOFF[0].chassi][serversPowerOFF[0].server]->TurnON(clockSimulation);
				     }
		             serversPowerOFF.erase(serversPowerOFF.begin());
	              }
	              else {
                     cout << "All servers are power on!!!" << endl;
		             break;
	              }
              }
           }
	    }
	 }
  }
}


void POOLServers::ServerPowerOFF_in_POOL(Server* (*posv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], int powerOFFServer)
{
  if (powerOFFServer < serversPOOL.size()) {
     //update the temperature
	 for (int i=0; i < serversPOOL.size(); i++){
         serversPOOL[i].temperature=(*posv)[serversPOOL[i].chassi][serversPOOL[i].server]->CurrentInletTemperature();
     }

	 SortVectorServersPOOL();

	 if (HIBERNATING_SERVER)
        for (int j=0; j < powerOFFServer; j++) {
	   	    (*posv)[serversPOOL[serversPOOL.size()-1].chassi][serversPOOL[serversPOOL.size()-1].server]->TimeHibernatingServer(clockSimulation);
		    serversPOOL.erase(serversPOOL.end()-1);
	    }
	 else {
        for (int j=0; j < powerOFFServer; j++) {
	   	    (*posv)[serversPOOL[serversPOOL.size()-1].chassi][serversPOOL[serversPOOL.size()-1].server]->TimePowerOffServer(clockSimulation);
		    serversPOOL.erase(serversPOOL.end()-1);
	    }
	 }
  }
}

void POOLServers::EveryASecond(Server* (*psv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX])
{
 int sumInitializing = 0;
 int power_off_server = 0;
 int serverIni = 0;

 //cout << "PowerOFF " << serversPowerOFF.size() << " PowerON " << serversPowerON.size() << " POOL " << serversPOOL.size() << " Servers ON " << serversON.size() << " Total " << serversPowerOFF.size()+serversPowerON.size()+serversPOOL.size()+serversON.size() << endl;


 // Turn OFF servers and server insert in the pool	
 if (clockSimulation == 0) {
	if (SIMULATES_POOL_SERVER) {
	   cout << "SiMDC3D: Turn OFF servers and server insert in the pool !!!" << endl;
       PowerOFFDCWithPOOL(psv);
	}
	else {
	   cout << "SiMDC3D: Turn OFF servers !!!" << endl;
	   PowerOFFDCWithoutPOOL(psv);
	}
 }
 
 for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
      for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
	      if (SIMULATES_POOL_SERVER) {
			 //replaces superheated server in the server pool
		     if (((*psv)[i][j]->IsPOOL()) && ((*psv)[i][j]->IsSuperHeatedInTheServerPOOL())) {
                if (!SwapServer(psv, i, j)) {
   			       (*psv)[i][j]->UpdateSuperHeatedInTheServerPOOL(false);
			    }
		     }
		  }
      
		  // check  power off servers
	  	  if ((*psv)[i][j]->IsENDING()) {
             if ((*psv)[i][j]->ReturnFinishPowerOffServer() == clockSimulation) {
		        InsertVectorServersPowerOFF(i, j, (*psv)[i][j]->CurrentInletTemperature());
			    if (HIBERNATING_SERVER) {
			       (*psv)[i][j]->EnterHibernation();
			    }
			    else {
		           (*psv)[i][j]->TurnOFF();
			    }
				for (int k = 0; k < serversON.size(); k++) {
					if ((serversON[k].chassi == i) && (serversON[k].server == j)) {
					   serversON.erase(serversON.begin()+k);
					   break;
					}
			    } 
			    sumOff += 1;
		     }
	      }

		  if (((*psv)[i][j]->IsOFF()) || ((*psv)[i][j]->IsHibernating()) || ((*psv)[i][j]->IsENDING()) || ((*psv)[i][j]->IsPOOL()) || ((*psv)[i][j]->IsMIGRATING()) || ((*psv)[i][j]->IsINITIALIZING())) {
//			 if ( (*psv)[i][j]->IsINITIALIZING() ) {
//				serverIni ++;
//			 }
		     continue;
	 	  }

		  if (SIMULATES_MIGRATION_VMS) { 
			  if ( (!(*psv)[i][j]->HasVMs()) && (clockSimulation >= 360) )  { 
				 AddPowerOff(1);
			  }
		  }
	  }
  }
  
  if ( ((clockSimulation % PERIODIC_LOG_INTERVAL) == 0) && (clockSimulation !=0 )) {
     sumPowerOff.push_back(sumOff);
	 sumOff = 0;
  }

  power_off_server = totalPowerOff - totalPowerOn;

  //power_off_server = totalPowerOff - totalPowerOn + serverIni;

  if (power_off_server > 0) {
     for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
         for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
		     if (((*psv)[i][j]->IsOFF()) || ((*psv)[i][j]->IsHibernating()) || ((*psv)[i][j]->IsENDING()) || ((*psv)[i][j]->IsPOOL()) || ((*psv)[i][j]->IsMIGRATING()) || ((*psv)[i][j]->IsINITIALIZING())) {
			    continue;
		     }
			 if (SIMULATES_MIGRATION_VMS) {
				 //check power off
				 if (!(*psv)[i][j]->HasVMs()) { // Check if you have VMs
					if (power_off_server != 0) {
						if (HIBERNATING_SERVER) {
						   (*psv)[i][j]->TimeHibernatingServer(clockSimulation);
						}
						else {
						   (*psv)[i][j]->TimePowerOffServer(clockSimulation);
						}
					   power_off_server --;
					}
					else {
					   i = NUMBER_OF_CHASSIS;
					   break;
					}
				 }
			 }
         }
     }
  }
  else {
	  power_off_server = power_off_server * -1;
	  if (power_off_server > 0) {
     	 ServerPowerON(psv, power_off_server);
	  }
  }
	
  totalPowerOff = 0;
  totalPowerOn = 0;

 
 // checks if any server finished power on
 BEFORE_ITERATOR:
 for (vector<POOL>::iterator it = serversPowerON.begin(); it != serversPowerON.end(); ++it) {
	 if ((*psv)[(*it).chassi][(*it).server]->ReturnFinishPowerOnServer() == clockSimulation) {
		(*psv)[(*it).chassi][(*it).server]->FinishInitialization();
		if (SIMULATES_POOL_SERVER) {
  		   (*psv)[(*it).chassi][(*it).server]->AddPOOL();
	       InsertVectorServersPOOL((*it).chassi, (*it).server, (*it).temperature);
  	       serversPowerON.erase(it);
		}
		else {
		   (*psv)[(*it).chassi][(*it).server]->ServerRead();
	       InsertVectorServersON((*it).chassi, (*it).server, (*it).temperature);
  	       serversPowerON.erase(it);
		}
		goto BEFORE_ITERATOR;
	 }
 }
 
 if (SIMULATES_POOL_SERVER) {
    // checks if pool is correct!!!
    for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
        for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
	        if ((*psv)[i][j]->IsINITIALIZING()) {
		       sumInitializing += 1;
		    }
	    }
    }  
    if (serversPOOL.size() < SIZE_POOL) {
	   if ((serversPOOL.size()+sumInitializing) < SIZE_POOL) {
          ServerPowerON(psv, SIZE_POOL - (serversPOOL.size() + sumInitializing));
	   }
    }

	if ((serversPOOL.size() > SIZE_POOL) && (clockSimulation > (timeOfEmergency + 7200))) { // 7200 is the time in seconds that the emergency pool should be active
  	   ServerPowerOFF_in_POOL(psv, ((serversPOOL.size()+sumInitializing)-SIZE_POOL));
    }
 }

}

bool POOLServers::SwapServer(Server* (*psv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], int chassi, int server)
{
  //this method replaces a server that overheated in the pool by a turn off server with the lowest temperature

  if (serversPowerOFF.size() > 0) {
     //update the temperature
	 for (int i=0; i < serversPowerOFF.size(); i++){
	     serversPowerOFF[i].temperature=(*psv)[serversPowerOFF[i].chassi][serversPowerOFF[i].server]->CurrentInletTemperature();
     }

	 SortVectorServersPowerOFF();
  
     if (serversPowerOFF[0].temperature < (EMERGENCY_TEMPERATURE - 1)) { //I am swapping only if server is below the maximum temperature in 1 degree
		// Power ON
        InsertVectorServersPowerON(serversPowerOFF[0].chassi, serversPowerOFF[0].server, serversPowerOFF[0].temperature);
		if (HIBERNATING_SERVER) {
		   (*psv)[serversPowerOFF[0].chassi][serversPowerOFF[0].server]->ExitHibernation(clockSimulation);
		}
		else {
	       (*psv)[serversPowerOFF[0].chassi][serversPowerOFF[0].server]->TurnON(clockSimulation);
		}
	    serversPowerOFF.erase(serversPowerOFF.begin());

		// initializing Power OFF
		if (!(*psv)[chassi][server]->HasVMs()) {
		   if (HIBERNATING_SERVER) {
			  (*psv)[chassi][server]->TimeHibernatingServer(clockSimulation);
		   }
		   else {
		      (*psv)[chassi][server]->TimePowerOffServer(clockSimulation);
		   }
		}
		else {
			cout << "Error: There VMs on a server shutdown request!!!" << endl;
		}
		// remove server in the pool
		for (int j=0; j < serversPOOL.size(); j++) {
			if ((serversPOOL[j].chassi == chassi) && (serversPOOL[j].server == server)) {
				serversPOOL.erase(serversPOOL.begin()+j);
				return true;
			} 
		}
     }
	 return false;
  }
  return false;
}

void POOLServers::PowerOFFDCWithPOOL(Server* (*psv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX])
{
 POOL servers;	
 vector<POOL> sortTempServers;
 int sumPowerOFF = 0;
 int sumPoolServers = 0;
 int servPowerOFF = 0;
 int servPowerON = 0;

 servPowerON = int ((NUMBER_OF_CHASSIS*NUMBER_OF_SERVERS_IN_ONE_CHASSIS) * 0.10);
 servPowerOFF = (NUMBER_OF_CHASSIS*NUMBER_OF_SERVERS_IN_ONE_CHASSIS) - servPowerON - SIZE_POOL; 
	

 for (int i=0; i < NUMBER_OF_CHASSIS; ++i) {
     for (int j=0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
   	     if ((*psv)[i][j]->IsOFF() || ((*psv)[i][j]->IsHibernating()) || (*psv)[i][j]->IsENDING() || (*psv)[i][j]->HasVMs() || (*psv)[i][j]->IsMIGRATING() || (*psv)[i][j]->IsINITIALIZING()) {
            continue;
		 }
		 servers.chassi = i;
		 servers.server = j;
		 servers.temperature = (*psv)[i][j]->CurrentInletTemperature();
		 sortTempServers.push_back(servers);
     }
 }

 //order servers by temperature
 sort(sortTempServers.begin(), sortTempServers.end(), sortTemperature);

 //power off 80% servers
 for (int i=0; i < servPowerOFF; i++) {	
     if (((*psv)[sortTempServers[i].chassi][sortTempServers[i].server]->IsOFF()) || ((*psv)[sortTempServers[i].chassi][sortTempServers[i].server]->IsHibernating()) || ((*psv)[sortTempServers[i].chassi][sortTempServers[i].server]->IsENDING())){
        continue;
	 }
	 if (HIBERNATING_SERVER) {
		(*psv)[sortTempServers[i].chassi][sortTempServers[i].server]->TimeHibernatingServer(clockSimulation);
	 }
	 else {
        (*psv)[sortTempServers[i].chassi][sortTempServers[i].server]->TimePowerOffServer(clockSimulation);
	 }
	 sumPowerOFF += 1;
 }
 
 // delete power off servers
 sortTempServers.erase(sortTempServers.begin(), sortTempServers.begin()+sumPowerOFF);

 // insert servers in the pool
 for (int j=0; j < SIZE_POOL; j++) {	
     if (((*psv)[sortTempServers[j].chassi][sortTempServers[j].server]->IsOFF()) || ((*psv)[sortTempServers[j].chassi][sortTempServers[j].server]->IsHibernating()) || ((*psv)[sortTempServers[j].chassi][sortTempServers[j].server]->IsENDING())){
        continue;
	 }

	 (*psv)[sortTempServers[j].chassi][sortTempServers[j].server]->AddPOOL();
	 InsertVectorServersPOOL(sortTempServers[j].chassi, sortTempServers[j].server, (*psv)[sortTempServers[j].chassi][sortTempServers[j].server]->CurrentInletTemperature());
	 sumPoolServers += 1;
 }

 sortTempServers.erase(sortTempServers.begin(), sortTempServers.begin()+SIZE_POOL);

 // insert servers on

 for (int k=0; k < SIZE_POOL; k++) {	
     if (((*psv)[sortTempServers[k].chassi][sortTempServers[k].server]->IsOFF()) || ((*psv)[sortTempServers[k].chassi][sortTempServers[k].server]->IsHibernating()) || ((*psv)[sortTempServers[k].chassi][sortTempServers[k].server]->IsENDING())){
        continue;
	 }
	 (*psv)[sortTempServers[k].chassi][sortTempServers[k].server]->ServerRead();
	 InsertVectorServersON(sortTempServers[k].chassi, sortTempServers[k].server, (*psv)[sortTempServers[k].chassi][sortTempServers[k].server]->CurrentInletTemperature());
 }

 // clear vector servers
 sortTempServers.clear(); 
}


void POOLServers::PowerOFFDCWithoutPOOL(Server* (*psv)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX])
{
 POOL servers;	
 vector<POOL> sortTempServers;
 int sumPowerOFF = 0;
 int sumPoolServers = 0;
 int servPowerOFF = 0;
 int servPowerON = 0;
 
 servPowerON = int ((NUMBER_OF_CHASSIS*NUMBER_OF_SERVERS_IN_ONE_CHASSIS) * 0.10);
 servPowerOFF = (NUMBER_OF_CHASSIS*NUMBER_OF_SERVERS_IN_ONE_CHASSIS) - servPowerON; 

 for (int i=0; i < NUMBER_OF_CHASSIS; ++i) {
     for (int j=0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
   	     if ((*psv)[i][j]->IsOFF() || ((*psv)[i][j]->IsHibernating()) || (*psv)[i][j]->IsENDING() || (*psv)[i][j]->HasVMs() || (*psv)[i][j]->IsMIGRATING() || (*psv)[i][j]->IsINITIALIZING()) {
            continue;
		 }
		 servers.chassi = i;
		 servers.server = j;
		 servers.temperature = (*psv)[i][j]->CurrentInletTemperature();
		 sortTempServers.push_back(servers);
     }
 }

 //order servers by temperature
 sort(sortTempServers.begin(), sortTempServers.end(), sortTemperature);

 //power off servers
 for (int i=0; i < servPowerOFF; i++) {	
     if (((*psv)[sortTempServers[i].chassi][sortTempServers[i].server]->IsOFF()) || ((*psv)[sortTempServers[i].chassi][sortTempServers[i].server]->IsHibernating()) || ((*psv)[sortTempServers[i].chassi][sortTempServers[i].server]->IsENDING())){
        continue;
	 }
	 if (HIBERNATING_SERVER) {
		(*psv)[sortTempServers[i].chassi][sortTempServers[i].server]->TimeHibernatingServer(clockSimulation);
	 }
	 else {
        (*psv)[sortTempServers[i].chassi][sortTempServers[i].server]->TimePowerOffServer(clockSimulation);
	 }
	 sumPowerOFF += 1;
 }
 
 // delete power off servers
 sortTempServers.erase(sortTempServers.begin(), sortTempServers.begin()+sumPowerOFF);

 // power on servers
 for (int j=0; j < sortTempServers.size(); j++) {	
     if (((*psv)[sortTempServers[j].chassi][sortTempServers[j].server]->IsOFF()) || ((*psv)[sortTempServers[j].chassi][sortTempServers[j].server]->IsHibernating()) || ((*psv)[sortTempServers[j].chassi][sortTempServers[j].server]->IsENDING())){
        continue;
	 }
	 (*psv)[sortTempServers[j].chassi][sortTempServers[j].server]->ServerRead();
	 InsertVectorServersON(sortTempServers[j].chassi, sortTempServers[j].server, (*psv)[sortTempServers[j].chassi][sortTempServers[j].server]->CurrentInletTemperature());
 }

  // clear vector servers
 sortTempServers.clear(); 
}

void POOLServers::UpdateClockSimulation(int clocksimul)
{
  clockSimulation = clocksimul;
}

void POOLServers::PrintPowerOffServer(void)
{
 for (int i=0; i < sumPowerOff.size(); i++) {
	 cout << sumPowerOff[i] << "\t";
 }
 cout << endl << endl; 
}


int POOLServers::TotalPowerOffServer(void)
{
 int sum = 0;

 for (int i=0; i < sumPowerOff.size(); i++) {
	 sum = sum + sumPowerOff[i];
 }
 return sum; 
}
