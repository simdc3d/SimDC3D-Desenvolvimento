#include "SimDC3D-Topology.h"

#include "FNSS-Parser.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

TopologySimDC3D::TopologySimDC3D(void)
{
 
  racks.clear();
  switchEdge.clear();
  switchAggregation.clear();
  switchCore.clear();
  activeFlows.clear();
  hostFNSS.clear();

  vTotalEnergyAccessSwitchSparseLog.clear();
  vTotalEnergyAggregationSwitchSparseLog.clear();
  vTotalEnergyCoreSwitchSparseLog.clear();

  vTotalEnergyAccessSwitch = 0.00;
  vTotalEnergyAggregationSwitch = 0.00;
  vTotalEnergyCoreSwitch = 0.00;

  vEnergyAccessSwitch = 0.00;
  vEnergyAggregationSwitch = 0.00;
  vEnergyCoreSwitch = 0.00; 

  nodeFNSS.clear();
  nodeSimDC3D.clear();

  numberSamplesTM = 0;
  totalSamplesTM = NUMBER_OF_SAMPLES_TRAFFIC_MATRIX * NUMBER_OF_PERIODS;
  
  internalCommunication = 0;
  externalCommunication = 0;

  gen = new std::default_random_engine ((int) 2*SEED*time(NULL));

  if (pthread_mutex_init(&mutex_sum, NULL) != 0) {
     cout << "SimDC3D-ERROR: Error create mutex Topology !!! " << endl;
     exit(0);
  }

}

TopologySimDC3D::~TopologySimDC3D(void)
{

  racks.clear();
  switchEdge.clear();
  switchAggregation.clear();
  switchCore.clear();
  hostFNSS.clear();

  activeFlows.clear();

  nodeFNSS.clear();
  nodeSimDC3D.clear();

  vTotalEnergyAccessSwitchSparseLog.clear();
  vTotalEnergyAggregationSwitchSparseLog.clear();
  vTotalEnergyCoreSwitchSparseLog.clear();
}


void TopologySimDC3D::CreateTopology(Server* (*tservers)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], map <VirtualMachine*, std::string> *vmDataFlowKeyVM) 
{

 int NumberOfRacks = 0; 
 int k = 0; 
 int insert = 0;
 int sumHostFNSS = 0;

 set<pair <string, string> >::iterator it;
 list<int>::iterator sw;
 string tierSwitchFirst = "";
 string tierSwitchSecond = "";
 vmDestinationDataFlowKeyVM = vmDataFlowKeyVM;


 if ( (PROPORTION_OF_INTERNAL_COMMUNICATION_BETWEEN_VMS + PROPORTION_OF_EXTERNAL_COMMUNICATION) != 100 ) {
	 cout << "SimDC3D-ERROR: The sum of a PROPORTION_OF_INTERNAL_COMMUNICATION_BETWEEN_VMS + PROPORTION_OF_EXTERNAL_COMMUNICATION is different from 100 !!!" << endl << endl; 
	 exit(0);
 }

 // Import topology from file (FNSS)
 cout << "SIMDC3D-NOTICE: Initializing topology import !!!" << NAME_FILE_TOPOLOGY << endl << endl;
	topology = fnss::Parser::parseTopology(NAME_FILE_TOPOLOGY);
 cout << endl;
 cout << "SIMDC3D-NOTICE: Topology imported and created !!!" << endl << endl;

 if (SIMULATE_TRAFFIC_MATRIX) {
    matrixTraffic = new TrafficMatrixSimDC3D();
	matrixTraffic->GetMatrixTraffic(numberSamplesTM);
 }

 cout << "SIMDC3D-NOTICE: Load energetic consumption in switch !!!" << endl << endl;
 topology.insertEnergyConsumption();
  

 // get list of all nodes
 nodes = topology.getAllNodes();

 for (set<string>::iterator it = nodes.begin(); it != nodes.end(); it++) {
     if (topology.getNode(*it)->getProperty("type") == "host") {
	    sumHostFNSS ++;
		hostFNSS.push_back(stoi(*it));
	 }

	 if (topology.getNode(*it)->getProperty("type") == "switch") {
        if (topology.getNode(*it)->getProperty("tier") == "core") {
	   	   SWITCH_CORE ++;
		   switchCore.push_back(stoi(*it));
		}
		if (topology.getNode(*it)->getProperty("tier") == "aggregation") {
		   SWITCH_AGGREGATION ++;
		   switchAggregation.push_back(stoi(*it));
		}
		if (topology.getNode(*it)->getProperty("tier") == "edge") {
		   SWITCH_ACCESS  ++;
		   switchEdge.push_back(stoi(*it));
		}
	 }
 }

 switchEdge.sort();
 hostFNSS.sort();

  // calculates the number of racks
 NumberOfRacks = ceil((double) NUMBER_OF_CHASSIS /  (double) NUMBER_CHASSI_RACK);

 if ( sumHostFNSS != (NUMBER_OF_CHASSIS*NUMBER_OF_SERVERS_IN_ONE_CHASSIS) ) {
    cout << "SIMDC3D-ERROR: host number is different from the host number of FNSS !!!" << endl;
	cout << "Switch Core " << SWITCH_CORE << endl;
	cout << "Switch Aggregation " << SWITCH_AGGREGATION << endl;
	cout << "Switch Edge " << SWITCH_ACCESS << endl;
	cout << "Hosts " << sumHostFNSS << endl;
	exit(0);
 }
  
 if (SWITCH_ACCESS != NumberOfRacks) {
    cout << "SIMDC3D-ERROR: Number of edge switches is different from the number of racks !!!" << endl;
	cout << "Switch Edge " << SWITCH_ACCESS << endl;
	cout << "Racks " << NumberOfRacks << endl;
	exit(0);
 }

 // create racks
 for(int i=0;i < NumberOfRacks; i++) {
	 racks.push_back(new Rack(i));
 }

 cout << "SIMDC3D-NOTICE: Inicializing created edges of the FNSS simulator. " << endl << endl; 


 // get list of all edges
 edges = topology.getAllEdges();
 
 for (set<pair <string, string> >::iterator it = edges.begin(); it != edges.end(); it++) {

	 topology.getEdge(*it)->setMaximumCapacity(topology.getEdge(*it)->getCapacity().getValue());

	 if (topology.getNode((*it).first)->getProperty("type") == "switch") {
        if (topology.getNode((*it).first)->getProperty("tier") == "core") {
			 tierSwitchFirst = "Core";
		}
        if (topology.getNode((*it).first)->getProperty("tier") == "aggregation") {
			 tierSwitchFirst = "Aggregation";
		}
		if (topology.getNode((*it).first)->getProperty("tier") == "edge") {
			 tierSwitchFirst = "Edge";
		}
	 }
	 else {
		  tierSwitchFirst = "Host";
	 }
	 if (topology.getNode((*it).second)->getProperty("type") == "switch") {
        if (topology.getNode((*it).second)->getProperty("tier") == "core") {
			 tierSwitchSecond = "Core";
		}
        if (topology.getNode((*it).second)->getProperty("tier") == "aggregation") {
			 tierSwitchSecond = "Aggregation";
		}
		if (topology.getNode((*it).second)->getProperty("tier") == "edge") {
			 tierSwitchSecond = "Edge";
		}
	 }
	 else {
		  tierSwitchSecond = "Host";
	 }
     cout << "[" << tierSwitchFirst << " " << (*it).first << " -> " << tierSwitchSecond << " " << (*it).second<<"] " << " Delay " << topology.getNode((*it).first)->getProperty("type") << " " << topology.getNode((*it).first)->getProperty("tier") << " " << topology.getEdge(*it)->getDelay().toString() << " Capacidade " << topology.getEdge(*it)->getCapacity().toString() << endl;
	 if (topology.getNode((*it).first)->getProperty("type") == "switch") {
 	    topology.getNode((*it).first)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
	 }
   	 if (topology.getNode((*it).second)->getProperty("type") == "switch") {
 	    topology.getNode((*it).second)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
	 }
 }
	
 cout << endl;
 cout << "Summary of Switch Connections" << endl << endl;

 for (list<int>::iterator it=switchCore.begin(); it!=switchCore.end(); ++it) {
	 cout << "Switch Core " << *it << " Active Ports " << topology.getNode(to_string(*it))->ReturnsActivePorts() << endl;
 }

 for (list<int>::iterator it=switchAggregation.begin(); it!=switchAggregation.end(); ++it) {
	 cout << "Switch Aggregation  " <<  *it << " Active Ports " << topology.getNode(to_string(*it))->ReturnsActivePorts() << endl;
 }

 for (list<int>::iterator it=switchEdge.begin(); it!=switchEdge.end(); ++it) {
	 cout << "Switch Edge  " <<  *it << " Active Ports " << topology.getNode(to_string(*it))->ReturnsActivePorts() << endl;
 }

 cout << endl;
 cout << "Relation of servers versus FNSS Node " << endl << endl;
 sw = switchEdge.begin();
 it = edges.begin();

 for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
     for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
		 if (insert < (NUMBER_CHASSI_RACK*NUMBER_OF_SERVERS_IN_ONE_CHASSIS)) {
			 racks[k]->InsertServerToRack((*tservers)[i][j]); //Inserts a server into the rack
			 (*tservers)[i][j]->InsertRackInServer(k); // identifies rack in server
			 (*tservers)[i][j]->InsertSwitch(to_string(*sw)); // identifies switch in server

			 // Connect to FNSS node with a SIMDC3D Server
		     while ( ((*it).first != to_string(*sw)) && (it != edges.end()) ) {
			       it++;
		     } 
		     if ( (it != edges.end()) && ((*it).first == to_string(*sw)) ) {
			    if (topology.getNode((*it).second)->getProperty("type") == "host") {
		           (*tservers)[i][j]->ConnectServerNode((*it).second, &topology, matrixTraffic);
				   nodeFNSS.insert(pair<pair<int,int>, string>(make_pair(i,j),(*it).second));
				   nodeSimDC3D.insert(pair<string, pair<int,int>>((*it).second, make_pair(i,j)));
				   cout << "Chassi " << i << " Server " << j << " NodeFNSS " << (*it).second << endl;
   		        }
                it++;
             }
			 else {
                cout << "SimDC3D-ERROR: There is no switch. " << topology.getNode((*it).second)->getProperty("type") << " !!!" << endl;
				exit(0);
			 }
		     insert++;
	     }
		 else {
			 racks[k]->InsertSwitchToRack(to_string(*sw));
			 k++;
			 sw++;
			 it = edges.begin();
			 racks[k]->InsertServerToRack((*tservers)[i][j]); //Inserts a server into the rack
			 (*tservers)[i][j]->InsertRackInServer(k); // identifies rack in server
			 (*tservers)[i][j]->InsertSwitch(to_string(*sw)); // identifies switch in server
			 // Connect to FNSS node with a SIMDC3D Server
		     while ( ((*it).first != to_string(*sw)) && (it != edges.end()) ) {
			       it++;
		     } 
		     if ( (it != edges.end()) && ((*it).first == to_string(*sw)) ) {
			    if (topology.getNode((*it).second)->getProperty("type") == "host") {
		           (*tservers)[i][j]->ConnectServerNode((*it).second, &topology, matrixTraffic);
				   nodeFNSS.insert(pair<pair<int,int>, string>(make_pair(i,j),(*it).second));
				   nodeSimDC3D.insert(pair<string, pair<int,int>>((*it).second, make_pair(i,j)));
				   cout << "Chassi " << i << " Server " << j << " NodeFNSS " << (*it).second << endl;
			    }
                it++;
             }
			 else {
                cout << "SimDC3D-ERROR: There is no switch." << topology.getNode((*it).second)->getProperty("type") << " !!!" << endl;
				exit(0);
			 }
		     insert=1;
		 }
	}
	racks[k]->InsertSwitchToRack(to_string(*sw));
 }
 if (switchEdge.size() != racks.size()) {
	cout << "SIMDC3D-ERROR: Number of edge switches insert is different from the number of racks ("<< switchEdge.size() << " != " << racks.size() << ") !!!" << endl;
	exit(0);
 }

}

void TopologySimDC3D::EveryASecond(int clock_, Server* (*svt)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX])
{
  clock_ = clock_ - 1; //Simulation clock is forward in 1 second
  clockTP = clock_;

  int chassiTemp = 0;
  int serverTemp = 0;
  int numSort = 0;

  set<string>::iterator it;
  
  vector<string> pathFlow;
  vector<VirtualMachine *> vmTemp;
  vector<DataFlow *> dataFlowTemp;
  
  tServers = svt; 

  std::map<VirtualMachine*,string>::iterator iter;

  for (list<int>::iterator it=switchCore.begin(); it!=switchCore.end(); ++it) {
	 if ( topology.getNode(to_string(*it))->isOFFSwitch() ) {
		 continue;
	 }
	 if ( topology.getNode(to_string(*it))->ReturnsActivePorts() == 0 ) {
		topology.getNode(to_string(*it))->powerOff();
	 }
 }

 for (list<int>::iterator it=switchAggregation.begin(); it!=switchAggregation.end(); ++it) {
	 if ( topology.getNode(to_string(*it))->isOFFSwitch() ) {
		 continue;
	 }
	 if ( topology.getNode(to_string(*it))->ReturnsActivePorts() == 0 ) {
	    topology.getNode(to_string(*it))->powerOff();	
	 }
 }

 for (list<int>::iterator it=switchEdge.begin(); it!=switchEdge.end(); ++it) {
	 if ( topology.getNode(to_string(*it))->isOFFSwitch() ) {
		 continue;
	 }
	 if ( topology.getNode(to_string(*it))->ReturnsActivePorts() == 0 ) {
		topology.getNode(to_string(*it))->powerOff();
	 }
 }

  ClearTraffic();
  vmTemp.clear();

  for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
      for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
		  if ( ((*tServers)[i][j]->IsOFF()) || ((*tServers)[i][j]->IsHibernating()) || ((*tServers)[i][j]->IsENDING()) || ((*tServers)[i][j]->IsPOOL()) || ((*tServers)[i][j]->IsINITIALIZING())) { 	
		     continue;
		  }

		  vmTemp = (*tServers)[i][j]->GetALLVMs();		// Retorna todas as Máquinas Virtuais de um Servidor
		  for (unsigned int k=0; k < vmTemp.size(); k++) {

			  dataFlowTemp = vmTemp[k]->ReturnDataFlowVM();		// Retorna todos os fluxos de dados de uma Máquina Virtual

			  for (unsigned int l = 0; l < dataFlowTemp.size(); l++) {
				  if ( !(dataFlowTemp[l]->DataFlowHasDestination()) ) {      // Check if the flow has destination
					 if ( (*vmDestinationDataFlowKeyVM).size() != 0) {
                        dist = new std::uniform_int_distribution<int> (0,(*vmDestinationDataFlowKeyVM).size()-1);
						std::uniform_int_distribution<int> destinationVM(dist->param());

						while (true) {
                              numSort = destinationVM(*gen);
						      iter = (*vmDestinationDataFlowKeyVM).begin();
							  for (int i=0; i < numSort; i++) {
							      ++iter;
						      }
							  if ( ((iter->first)->ReturnChassi() == 99999) && ((iter->first)->ReturnServer() == 99999) ) {  // Waiting for escalation
                                 continue;
							  }
						      if ( iter->first != vmTemp[k] ) {
                                 break;
						      }
						}
						dataFlowTemp[l]->InsertDestinationVM( iter->first );
						dataFlowTemp[l]->UpdateDestinationDataFlow( (iter->first)->ReturnChassi(), (iter->first)->ReturnServer(),(*tServers)[(iter->first)->ReturnChassi()][(iter->first)->ReturnServer()]->ReturnServerNode(), (iter->first)->ReturnNodeFNSSVM() );
					}
				 }

 				 if ( !(dataFlowTemp[l]->PathExists()) ) {
					// Path does not exist
					 InsertPathInFlow(dataFlowTemp[l]);
				    
				 }	
				// Checks whether the destination VM migrated
				if ( (dataFlowTemp[l]->ReturnDestinationVM())->ReturnTimeMigrationSource() != 0 ) { 
				   chassiTemp = ((dataFlowTemp[l]->ReturnDestinationVM())->ReturnNewVM())->ReturnChassi();
				   serverTemp = ((dataFlowTemp[l]->ReturnDestinationVM())->ReturnNewVM())->ReturnServer();
				   dataFlowTemp[l]->UpdateDestinationDataFlow( chassiTemp, serverTemp, (*tServers)[chassiTemp][serverTemp]->ReturnServerNode(), (dataFlowTemp[l]->ReturnDestinationVM())->ReturnNewVM() );
				   InsertPathInFlow(dataFlowTemp[l]);
				}

				// Checks whether the destination VM finish
				if ( (dataFlowTemp[l]->ReturnDestinationVM())->IsFinished() ) { 
				   chassiTemp = 99999;
				   serverTemp = 99999;
				   dataFlowTemp[l]->UpdateDestinationDataFlow( chassiTemp, serverTemp, "");
				   continue;
				}

				// EveryASecond() DataFlow
				dataFlowTemp[l]->EveryASecond();

				// Insert traffic on the link
	            pathFlow = dataFlowTemp[l]->ReturnPath(); // return path 

	            for (unsigned int m = 0; m < pathFlow.size()-1; m++) {
				    if ( !topology.getEdge(pathFlow[m], pathFlow[m+1])->ReturnIsOff() ) { 
		               topology.getEdge(pathFlow[m], pathFlow[m+1])->AddTraffic(dataFlowTemp[l]->ReturnFlowMBPS());
					}
					else {
					   cout << "SimDC3D-ERROR: Data flow on a disconnected link !!!" << endl;
					   exit(0);
					}
				 }
				 pathFlow.clear();
			  }
		  }
		  vmTemp.clear();
	  }
  }

  //
  for (set<pair <string, string> >::iterator it = edges.begin(); it != edges.end(); it++) {
	  if (topology.getEdge(*it)->ReturnIsOff()) {
		 continue;
	  }
	  if ( (clock_%RUNTIME_LINK_ADAPTATION == 0) && (clock_!=0) ) { 
	  	 if ( topology.getEdge(*it)->getCapacity().getValue() == 100) {
            if ( (topology.getEdge(*it)->ReturnTraffic()) <= (10 * (TRAFFIC_LIMIT_FOR_ADAPTATION/100))  ) {
			   topology.getEdge(*it)->setCapacity(fnss::Quantity(10, fnss::Units::Bandwidth));
		    }
			if (topology.getNode((*it).first)->getProperty("type") == "switch") {
 	           topology.getNode((*it).first)->portsUsedSwitch(100, false, '-');
			   topology.getNode((*it).first)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');

			}
   	        if (topology.getNode((*it).second)->getProperty("type") == "switch") {
 	           topology.getNode((*it).second)->portsUsedSwitch(100, false, '-');
  			   topology.getNode((*it).second)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
	        }
		 }

		 if ( topology.getEdge(*it)->getCapacity().getValue() == 1000) {
            if ( (topology.getEdge(*it)->ReturnTraffic()) <= (100 * (TRAFFIC_LIMIT_FOR_ADAPTATION/100)) ) {
			   topology.getEdge(*it)->setCapacity(fnss::Quantity(100, fnss::Units::Bandwidth));
		    }
			if (topology.getNode((*it).first)->getProperty("type") == "switch") {
 	           topology.getNode((*it).first)->portsUsedSwitch(1000, false, '-');
			   topology.getNode((*it).first)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');

			}
   	        if (topology.getNode((*it).second)->getProperty("type") == "switch") {
 	           topology.getNode((*it).second)->portsUsedSwitch(1000, false, '-');
  			   topology.getNode((*it).second)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
	        }
		 }

		 if ( topology.getEdge(*it)->getCapacity().getValue() == 10000) {
            if ( (topology.getEdge(*it)->ReturnTraffic()) <= (1000 * (TRAFFIC_LIMIT_FOR_ADAPTATION/100)) ) {
			   topology.getEdge(*it)->setCapacity(fnss::Quantity(1000, fnss::Units::Bandwidth));
		    }
			if (topology.getNode((*it).first)->getProperty("type") == "switch") {
 	           topology.getNode((*it).first)->portsUsedSwitch(10000, false, '-');
			   topology.getNode((*it).first)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');

			}
   	        if (topology.getNode((*it).second)->getProperty("type") == "switch") {
 	           topology.getNode((*it).second)->portsUsedSwitch(10000, false, '-');
  			   topology.getNode((*it).second)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
	        }
		 }
	  }
        
	  if ( topology.getEdge(*it)->getCapacity().getValue() == 10) {
         if ( (topology.getEdge(*it)->ReturnTraffic()) > (topology.getEdge(*it)->getCapacity().getValue() * (TRAFFIC_LIMIT_FOR_ADAPTATION/100)) ) {
			topology.getEdge(*it)->setCapacity(fnss::Quantity(100, fnss::Units::Bandwidth));
		 }
		 if (topology.getNode((*it).first)->getProperty("type") == "switch") {
 	        topology.getNode((*it).first)->portsUsedSwitch(10, false, '-');
			topology.getNode((*it).first)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');

		 }
   	     if (topology.getNode((*it).second)->getProperty("type") == "switch") {
 	        topology.getNode((*it).second)->portsUsedSwitch(10, false, '-');
  		    topology.getNode((*it).second)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
	     }
	  }
	  if ( topology.getEdge(*it)->getCapacity().getValue() == 100) {
         if ( (topology.getEdge(*it)->ReturnTraffic()) > (topology.getEdge(*it)->getCapacity().getValue() * (TRAFFIC_LIMIT_FOR_ADAPTATION/100)) ) {
			topology.getEdge(*it)->setCapacity(fnss::Quantity(1000, fnss::Units::Bandwidth));
		 }
		 if (topology.getNode((*it).first)->getProperty("type") == "switch") {
 	        topology.getNode((*it).first)->portsUsedSwitch(100, false, '-');
			topology.getNode((*it).first)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');

		 }
   	     if (topology.getNode((*it).second)->getProperty("type") == "switch") {
 	        topology.getNode((*it).second)->portsUsedSwitch(100, false, '-');
  		    topology.getNode((*it).second)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
	     }
	  }
	  if ( topology.getEdge(*it)->getCapacity().getValue() == 1000) {
         if ( (topology.getEdge(*it)->ReturnTraffic()) > (topology.getEdge(*it)->getCapacity().getValue() * (TRAFFIC_LIMIT_FOR_ADAPTATION/100)) ) {
			topology.getEdge(*it)->setCapacity(fnss::Quantity(10000, fnss::Units::Bandwidth));
		 }

		 if (topology.getNode((*it).first)->getProperty("type") == "switch") {
 	        topology.getNode((*it).first)->portsUsedSwitch(1000, false, '-');
			topology.getNode((*it).first)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');

		 }
   	     if (topology.getNode((*it).second)->getProperty("type") == "switch") {
 	        topology.getNode((*it).second)->portsUsedSwitch(1000, false, '-');
  		    topology.getNode((*it).second)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
	     }
      }
  }


  for (list<int>::iterator it=switchEdge.begin(); it!=switchEdge.end(); ++it) {
      vEnergyAccessSwitch += (topology.getNode(to_string(*it))->getEnergyConsumption());  
  }

  for (list<int>::iterator it=switchAggregation.begin(); it!=switchAggregation.end(); ++it) {
   	  vEnergyAggregationSwitch += (topology.getNode(to_string(*it))->getEnergyConsumption());  
  }

  for (list<int>::iterator it=switchCore.begin(); it!=switchCore.end(); ++it) {
   	  vEnergyCoreSwitch += (topology.getNode(to_string(*it))->getEnergyConsumption());  
  }

  if ( (clock_%PERIODIC_LOG_INTERVAL==0) && (clock_ != 0) ) {
	 vTotalEnergyAccessSwitchSparseLog.push_back(vEnergyAccessSwitch/PERIODIC_LOG_INTERVAL);
     vTotalEnergyAggregationSwitchSparseLog.push_back(vEnergyAggregationSwitch/PERIODIC_LOG_INTERVAL);
     vTotalEnergyCoreSwitchSparseLog.push_back(vEnergyCoreSwitch/PERIODIC_LOG_INTERVAL);

	 vTotalEnergyAccessSwitch += vEnergyAccessSwitch;
	 vTotalEnergyAggregationSwitch += vEnergyAggregationSwitch;
	 vTotalEnergyCoreSwitch += vEnergyCoreSwitch;
 
	 vEnergyAccessSwitch = 0.00;
	 vEnergyAggregationSwitch = 0.00;
     vEnergyCoreSwitch = 0.00; 
  } 

};

struct lEdges {
	string left;
	string right;
};

void TopologySimDC3D::ReconnectEdges(){
	for (set<pair <string, string> >::iterator it = edges.begin(); it != edges.end(); it++) {
		if (topology.getEdge((*it).first, (*it).second)->ReturnIsOff()) {
			topology.powerONEdge((*it).first, (*it).second);
		}
	}

}

void TopologySimDC3D::EveryASecondMultiThread_P1(void)
{
  vector<VirtualMachine *> vmTemp;
  vector<DataFlow *> dataFlowTemp;
 
 for (set<pair <string, string> >::iterator it = edges.begin(); it != edges.end(); it++) {
	 if ( topology.getEdge(*it)->ReturnTraffic() > (0.80 * (topology.getEdge((*it).first,(*it).second)->getCapacity().getValue() )) ) {
		 if (TYPE_TOPOLOGY == "3LAYER") {
			 // Turn on links Edge-Aggregation
			 for (list<int>::iterator it2 = switchEdge.begin(); it2 != switchEdge.end(); ++it2) {
				 for (list<int>::iterator iter2 = switchAggregation.begin(); iter2 != switchAggregation.end(); ++iter2) {
					 if ((*it).first == to_string(*iter2) && ((*it).second == to_string(*it2))) {
						 ReconnectEdges(); // Reconnect all Edges
						 // Clears the path
						 for (int j = 0; j < NUMBER_OF_CHASSIS; ++j) {
							 for (int k = 0; k < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++k) {
								 if (((*tServers)[j][k]->IsOFF()) || ((*tServers)[j][k]->IsHibernating()) || ((*tServers)[j][k]->IsENDING()) || ((*tServers)[j][k]->IsPOOL()) || ((*tServers)[j][k]->IsINITIALIZING())) {
									 continue;
								 }
								 vmTemp = (*tServers)[j][k]->GetALLVMs();
								 for (unsigned int l = 0; l < vmTemp.size(); l++) {
									 dataFlowTemp = vmTemp[l]->ReturnDataFlowVM();
									 for (unsigned int key_L = 0; key_L < dataFlowTemp.size(); key_L++) {
										 dataFlowTemp[key_L]->ClearPath();
									 }
								 }
							 }
						 }
					 }
				 }
			 }

			 // Turn on links Aggregation-Core
			 for (list<int>::iterator it3 = switchAggregation.begin(); it3 != switchAggregation.end(); ++it3) {
				 for (list<int>::iterator iter3 = switchCore.begin(); iter3 != switchCore.end(); ++iter3) {
					 if ((*it).first == to_string(*iter3) && ((*it).second == to_string(*it3))) {
						 ReconnectEdges(); // Reconnect all Edges
 					     // Clears the path
						 for (int j = 0; j < NUMBER_OF_CHASSIS; ++j) {
							 for (int k = 0; k < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++k) {
								 if (((*tServers)[j][k]->IsOFF()) || ((*tServers)[j][k]->IsHibernating()) || ((*tServers)[j][k]->IsENDING()) || ((*tServers)[j][k]->IsPOOL()) || ((*tServers)[j][k]->IsINITIALIZING())) {
									 continue;
								 }
								 vmTemp = (*tServers)[j][k]->GetALLVMs();
								 for (unsigned int l = 0; l < vmTemp.size(); l++) {
									 dataFlowTemp = vmTemp[l]->ReturnDataFlowVM();
									 for (unsigned int key_L = 0; key_L < dataFlowTemp.size(); key_L++) {
										 dataFlowTemp[key_L]->ClearPath();
									 }
								 }
							 }
						 }
					 }
				 }
			 }
		 }
		 // It is a two layers topology
		 else {
			 if (TYPE_TOPOLOGY == "2LAYER") {
				 // Turn on links Edge-Core
				 for (list<int>::iterator it2 = switchEdge.begin(); it2 != switchEdge.end(); ++it2) {
					 for (list<int>::iterator iter2 = switchCore.begin(); iter2 != switchCore.end(); ++iter2) {
						 if ((*it).first == to_string(*iter2) && ((*it).second == to_string(*it2))) {
							 ReconnectEdges(); // Reconnect all Edges
 						     // Clears the path
							 for (int j = 0; j < NUMBER_OF_CHASSIS; ++j) {
								 for (int k = 0; k < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++k) {
									 if (((*tServers)[j][k]->IsOFF()) || ((*tServers)[j][k]->IsHibernating()) || ((*tServers)[j][k]->IsENDING()) || ((*tServers)[j][k]->IsPOOL()) || ((*tServers)[j][k]->IsINITIALIZING())) {
									    continue;
									 }
									 vmTemp = (*tServers)[j][k]->GetALLVMs();
									 for (unsigned int l = 0; l < vmTemp.size(); l++) {
										 dataFlowTemp = vmTemp[l]->ReturnDataFlowVM();
										 for (unsigned int key_L = 0; key_L < dataFlowTemp.size(); key_L++) {
											 dataFlowTemp[key_L]->ClearPath();
										 }
									 }
								 }
							 }
						 }
					 }
				 }
			 }
			 else {
				 cout << "SIMDC3D-ERROR: Topology is not implemented !!!" << endl;
				 exit(0);
			 }
		 }
	 }
 }
  
 for (list<int>::iterator it=switchCore.begin(); it!=switchCore.end(); ++it) {
	 if ( topology.getNode(to_string(*it))->isOFFSwitch() ) {
		continue;
	 }
	 if ( topology.getNode(to_string(*it))->ReturnsActivePorts() == 0 ) {
		topology.getNode(to_string(*it))->powerOff();
	 }
 }

 for (list<int>::iterator it=switchAggregation.begin(); it!=switchAggregation.end(); ++it) {
	 if ( topology.getNode(to_string(*it))->isOFFSwitch() ) {
		continue;
	 }
	 if ( topology.getNode(to_string(*it))->ReturnsActivePorts() == 0 ) {
	    topology.getNode(to_string(*it))->powerOff();	
	 }
 }

 for (list<int>::iterator it=switchEdge.begin(); it!=switchEdge.end(); ++it) {
	 if ( topology.getNode(to_string(*it))->isOFFSwitch() ) {
		continue;
	 }
	 if ( topology.getNode(to_string(*it))->ReturnsActivePorts() == 0 ) {
		topology.getNode(to_string(*it))->powerOff();
	 }
 }
 
 ClearTraffic();
}



void TopologySimDC3D::EveryASecondMultiThread_P2(int chassi_i, int chassi_f, Server* (*svt)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], pthread_mutex_t* mtx)
{
  tServers = svt;

  int chassiTemp = 0;
  int serverTemp = 0;
  int numSort = 0;
  double totaltraffic = 0;

  set<string>::iterator it;
  
  vector<string> pathFlow;
  vector<VirtualMachine *> vmTemp;
  vector<DataFlow *> dataFlowTemp;

  std::map<VirtualMachine*,string>::iterator iter;

  if (SIMULATE_TRAFFIC_MATRIX) {

     for (int i = chassi_i; i < chassi_f; i++) {
	     for (int j = 0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; j++) {
  		     if ( ((*svt)[i][j]->IsOFF()) || ((*svt)[i][j]->IsHibernating()) || ((*svt)[i][j]->IsENDING()) || ((*svt)[i][j]->IsPOOL()) || ((*svt)[i][j]->IsINITIALIZING())) { 	
 		        continue;
		     }

		     vmTemp = (*svt)[i][j]->GetALLVMs();

		     for (unsigned int k=0; k < vmTemp.size(); k++) {

		 	     dataFlowTemp = vmTemp[k]->ReturnDataFlowVM();
				 
				 totaltraffic = 0;
			     
				 for (unsigned int key_L = 0; key_L < dataFlowTemp.size(); key_L++) {

			         // Check if the flow has destination
				     if ( !(dataFlowTemp[key_L])->DataFlowHasDestination() ) {
					    if ( internalCommunication < PROPORTION_OF_INTERNAL_COMMUNICATION_BETWEEN_VMS ) {
					       pthread_mutex_lock(&mutex_sum);
					             internalCommunication ++;
					       pthread_mutex_unlock(&mutex_sum);

						   if ( (*vmDestinationDataFlowKeyVM).size() != 0) {
						      dist = new std::uniform_int_distribution<int> (0,(*vmDestinationDataFlowKeyVM).size()-1);
						      std::uniform_int_distribution<int> destinationVM(dist->param());
						      iter = (*vmDestinationDataFlowKeyVM).begin();
                       
						      numSort = destinationVM(*gen);
						      iter = (*vmDestinationDataFlowKeyVM).begin();
						      for (int p=0; p < numSort; p++) {
							     ++iter;
						      }
						      if ( (iter == (*vmDestinationDataFlowKeyVM).end()) || ((iter->first)->ReturnChassi() == 99999) || ((iter->first)->ReturnTimeMigrationSource() != 0) || ((iter->first)->IsFinished()) || (iter->first == vmTemp[k]) ) {
						         continue;
						      }
							  dataFlowTemp[key_L]->InsertDestinationVM( iter->first );
						      dataFlowTemp[key_L]->UpdateDestinationDataFlow( (iter->first)->ReturnChassi(), (iter->first)->ReturnServer(),(*svt)[(iter->first)->ReturnChassi()][(iter->first)->ReturnServer()]->ReturnServerNode(), (iter->first)->ReturnNodeFNSSVM() );
					       }
					       else {
						      continue;
					       }
					    }
					    else {
						   if ( externalCommunication < PROPORTION_OF_EXTERNAL_COMMUNICATION ) {
						 	  pthread_mutex_lock(&mutex_sum);
					                externalCommunication ++;
					          pthread_mutex_unlock(&mutex_sum);
						      dataFlowTemp[key_L]->InsertDestinationVM( NULL );
						      dataFlowTemp[key_L]->UpdateDestinationDataFlow(88888, 88888, to_string(switchCore.front()), to_string(switchCore.front()));
    					   }
						   else {
							  pthread_mutex_lock(&mutex_sum);
					                externalCommunication = 0;
									internalCommunication = 0;
					          pthread_mutex_unlock(&mutex_sum);
							  continue;
						   }
					    }
				     }
				     if ( !(dataFlowTemp[key_L]->PathExists()) ) {
					    // Path does not exist
				        InsertPathInFlow(dataFlowTemp[key_L]);
				     }	
				 
				     if ( dataFlowTemp[key_L]->ReturnServerDestination() != 88888 ) {
				        // Checks whether the destination VM migrated
				        if ( (dataFlowTemp[key_L]->ReturnDestinationVM())->ReturnTimeMigrationSource() != 0 ) { 
				           chassiTemp = ((dataFlowTemp[key_L]->ReturnDestinationVM())->ReturnNewVM())->ReturnChassi();
				           serverTemp = ((dataFlowTemp[key_L]->ReturnDestinationVM())->ReturnNewVM())->ReturnServer();
				           dataFlowTemp[key_L]->UpdateDestinationDataFlow( chassiTemp, serverTemp, (*svt)[chassiTemp][serverTemp]->ReturnServerNode(), (dataFlowTemp[key_L]->ReturnDestinationVM())->ReturnNewVM() );
				           InsertPathInFlow(dataFlowTemp[key_L]);
					       continue;
				        }

				        // Checks whether the destination VM finish
				        if ( (dataFlowTemp[key_L]->ReturnDestinationVM())->IsFinished() ) { 
				           chassiTemp = 99999;
				           serverTemp = 99999;
				           dataFlowTemp[key_L]->UpdateDestinationDataFlow( chassiTemp, serverTemp, "");
					       continue;
				        }
				     }

				     // EveryASecond() DataFlow
				     dataFlowTemp[key_L]->EveryASecond();
				  
					 totaltraffic +=  dataFlowTemp[key_L]->ReturnFlowMBPS();


					 // Verifica se tem algum caminho desligado se tiver chama o insert path
					 if (isLinkOff(dataFlowTemp[key_L]->ReturnPath())) {
						 dataFlowTemp[key_L]->ClearPath();
						 InsertPathInFlow(dataFlowTemp[key_L]);
					 }

				     // Insert traffic on the link
	                 pathFlow = dataFlowTemp[key_L]->ReturnPath(); // return path 

	                 for (unsigned int m = 0; m < pathFlow.size()-1; m++) {
				         if ( !topology.getEdge(pathFlow[m], pathFlow[m+1])->ReturnIsOff() ) { 
					        pthread_mutex_lock( (topology.getEdge(pathFlow[m], pathFlow[m+1])->returnMutex()) );
							//cout << "Path " << pathFlow[m] << " -> " << pathFlow[m+1] << " " << dataFlowTemp[key_L]->ReturnFlowMBPS() << endl;	
					            topology.getEdge(pathFlow[m], pathFlow[m+1])->AddTraffic(dataFlowTemp[key_L]->ReturnFlowMBPS());
					        pthread_mutex_unlock( (topology.getEdge(pathFlow[m], pathFlow[m+1])->returnMutex()) );
					     }
					     else {
					        cout << "SimDC3D-ERROR: Data flow on a disconnected link !!!" << endl;
					        exit(0);
				         }
			         }
			         pathFlow.clear();
			     }
		         dataFlowTemp.clear();
				 vmTemp[k]->SumTrafficVM(totaltraffic);
		     }
		     vmTemp.clear();
	     }
     }
  }
};

void TopologySimDC3D::EveryASecondMultiThread_P3(void)
{
   if ( (DYNAMIC_SPEED_ADAPTATION) && (SIMULATE_TRAFFIC_MATRIX) ) { 
	  for (set<pair <string, string> >::iterator it = edges.begin(); it != edges.end(); it++) {
		  if (topology.getEdge(*it)->ReturnIsOff()) {
		     continue;
	      }

	      if ( (clockTP%RUNTIME_LINK_ADAPTATION == 0) && (clockTP!=0) ) { 
	  	     if ( topology.getEdge(*it)->getCapacity().getValue() == 100) {
                if ( (topology.getEdge(*it)->ReturnTraffic()) <= (10 *  ((float) TRAFFIC_LIMIT_FOR_ADAPTATION / 100))  ) {
			       topology.getEdge(*it)->setCapacity(fnss::Quantity(10, fnss::Units::Bandwidth));
				   if ( topology.getNode((*it).first)->getProperty("type") == "switch" ) {
 	                  topology.getNode((*it).first)->portsUsedSwitch(100, false, '-');
			          topology.getNode((*it).first)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
			       }
   	               if ( topology.getNode((*it).second)->getProperty("type") == "switch" ) {
 	                  topology.getNode((*it).second)->portsUsedSwitch(100, false, '-');
  			          topology.getNode((*it).second)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
	               }
			    }
	         }

		     if ( topology.getEdge(*it)->getCapacity().getValue() == 1000) {
			    if ( (topology.getEdge(*it)->ReturnTraffic()) <= (100*((float) TRAFFIC_LIMIT_FOR_ADAPTATION / 100)) ) {
			       topology.getEdge(*it)->setCapacity(fnss::Quantity(100, fnss::Units::Bandwidth));
		    
			       if (topology.getNode((*it).first)->getProperty("type") == "switch") {
 	                  topology.getNode((*it).first)->portsUsedSwitch(1000, false, '-');
			          topology.getNode((*it).first)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
			       }
   	               if (topology.getNode((*it).second)->getProperty("type") == "switch") {
 	                  topology.getNode((*it).second)->portsUsedSwitch(1000, false, '-');
  			          topology.getNode((*it).second)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
	               }
			    }
		     }

		     if ( topology.getEdge(*it)->getCapacity().getValue() == 10000 ) {
                if ( (topology.getEdge(*it)->ReturnTraffic()) <= (1000*((float) TRAFFIC_LIMIT_FOR_ADAPTATION / 100)) ) {
			       topology.getEdge(*it)->setCapacity(fnss::Quantity(1000, fnss::Units::Bandwidth));
 
			       if ( topology.getNode((*it).first)->getProperty("type") == "switch" ) {
 	                  topology.getNode((*it).first)->portsUsedSwitch(10000, false, '-');
			          topology.getNode((*it).first)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
  			       }

   	               if ( topology.getNode((*it).second)->getProperty("type") == "switch" ) {
 	                  topology.getNode((*it).second)->portsUsedSwitch(10000, false, '-');
  			          topology.getNode((*it).second)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
	               }
		        }
		     }
	      }
        
	      if ( topology.getEdge(*it)->getCapacity().getValue() == 10) {
             if ( (topology.getEdge(*it)->ReturnTraffic()) > (topology.getEdge(*it)->getCapacity().getValue() * ((float) TRAFFIC_LIMIT_FOR_ADAPTATION / 100) ) ) {
				if (100 <= topology.getEdge(*it)->returnMaximumCapacity()) {
				   topology.getEdge(*it)->setCapacity(fnss::Quantity(100, fnss::Units::Bandwidth));
		 
				   if ( topology.getNode((*it).first)->getProperty("type") == "switch" ) {
 					  topology.getNode((*it).first)->portsUsedSwitch(10, false, '-');
					  topology.getNode((*it).first)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
  				   }
   				   if ( topology.getNode((*it).second)->getProperty("type") == "switch" ) {
 					  topology.getNode((*it).second)->portsUsedSwitch(10, false, '-');
  					  topology.getNode((*it).second)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
				   }
				}
		     }
	      }
	      else {
	         if ( topology.getEdge(*it)->getCapacity().getValue() == 100 ) {
                if ( (topology.getEdge(*it)->ReturnTraffic()) > (topology.getEdge(*it)->getCapacity().getValue() * ((float) TRAFFIC_LIMIT_FOR_ADAPTATION / 100) ) ) {
				   if (1000 <= topology.getEdge(*it)->returnMaximumCapacity()) {
					  topology.getEdge(*it)->setCapacity(fnss::Quantity(1000, fnss::Units::Bandwidth));
		 
					  if ( topology.getNode((*it).first)->getProperty("type") == "switch" ) {
 						 topology.getNode((*it).first)->portsUsedSwitch(100, false, '-');
						 topology.getNode((*it).first)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
					  }
   					  if ( topology.getNode((*it).second)->getProperty("type") == "switch" ) {
 						 topology.getNode((*it).second)->portsUsedSwitch(100, false, '-');
  						 topology.getNode((*it).second)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
					  }
				   }
		        }
	         }
		     else {
			    if ( topology.getEdge(*it)->getCapacity().getValue() == 1000 ) {
                   if ( (topology.getEdge(*it)->ReturnTraffic()) > (topology.getEdge(*it)->getCapacity().getValue() * ((float) TRAFFIC_LIMIT_FOR_ADAPTATION / 100) ) ) {
					  if (10000 <= topology.getEdge(*it)->returnMaximumCapacity()) {
						 topology.getEdge(*it)->setCapacity(fnss::Quantity(10000, fnss::Units::Bandwidth));
				   
						 if ( topology.getNode((*it).first)->getProperty("type") == "switch" ) {
 							topology.getNode((*it).first)->portsUsedSwitch(1000, false, '-');
							topology.getNode((*it).first)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
						 }
   						 if ( topology.getNode((*it).second)->getProperty("type") == "switch" ) {
 							topology.getNode((*it).second)->portsUsedSwitch(1000, false, '-');
  							topology.getNode((*it).second)->portsUsedSwitch(topology.getEdge(*it)->getCapacity().getValue(), false, '+');
						 }
					  }
		           }
                }
             } 
	      }
      }
   }

   for (list<int>::iterator it=switchEdge.begin(); it!=switchEdge.end(); ++it) {
       vEnergyAccessSwitch += (topology.getNode(to_string(*it))->getEnergyConsumption());  
   }

   for (list<int>::iterator it=switchAggregation.begin(); it!=switchAggregation.end(); ++it) {
   	   vEnergyAggregationSwitch += (topology.getNode(to_string(*it))->getEnergyConsumption());  
   }

   for (list<int>::iterator it=switchCore.begin(); it!=switchCore.end(); ++it) {
   	   vEnergyCoreSwitch += (topology.getNode(to_string(*it))->getEnergyConsumption());  
   }

   if ( (clockTP%PERIODIC_LOG_INTERVAL==0) && (clockTP != 0) ) {
	  vTotalEnergyAccessSwitchSparseLog.push_back(vEnergyAccessSwitch/PERIODIC_LOG_INTERVAL);
      vTotalEnergyAggregationSwitchSparseLog.push_back(vEnergyAggregationSwitch/PERIODIC_LOG_INTERVAL);
      vTotalEnergyCoreSwitchSparseLog.push_back(vEnergyCoreSwitch/PERIODIC_LOG_INTERVAL);

	  vTotalEnergyAccessSwitch += vEnergyAccessSwitch;
	  vTotalEnergyAggregationSwitch += vEnergyAggregationSwitch;
	  vTotalEnergyCoreSwitch += vEnergyCoreSwitch;
 
	  vEnergyAccessSwitch = 0.00;
	  vEnergyAggregationSwitch = 0.00;
      vEnergyCoreSwitch = 0.00; 
   } 

}

/*void TopologySimDC3D::InsertPathInFlow(DataFlow* dataF)
{
 string switchToRDestination = "";
 string switchToRSource = "";
 stringstream switchC;

 //Insert Source Node
 dataF->InsertPath( dataF->ReturnSourceNodeFNSS() );

 //Insert Switch ToR
 switchToRSource = (*tServers)[dataF->ReturnChassiSource()][dataF->ReturnServerSource()]->ReturnSwitch();
 dataF->InsertPath( switchToRSource );

 
 if ( dataF->ReturnServerDestination() != 88888 ) {
    //Verify that the destination is on the same ToR switch
    if ( (*tServers)[dataF->ReturnChassiSource()][dataF->ReturnServerSource()]->ReturnSwitch() == (*tServers)[dataF->ReturnChassiDestination()][dataF->ReturnServerDestination()]->ReturnSwitch() ) {
       dataF->InsertPath( dataF->ReturnDestinationNodeFNSS() );
    }
    else {
       switchToRDestination = (*tServers)[dataF->ReturnChassiDestination()][dataF->ReturnServerDestination()]->ReturnSwitch();
	   for (list<int>::iterator it=switchCore.begin(); it != switchCore.end(); ++it) {
           if ( (topology.hasEdge(switchToRDestination, to_string(*it)))  &&  (topology.hasEdge(switchToRSource, to_string(*it)))   ) {
		      dataF->InsertPath( to_string(*it) );
		      dataF->InsertPath( switchToRDestination );
		      dataF->InsertPath( dataF->ReturnDestinationNodeFNSS() );
		      break;
	       }
	   }
    }
 }
 else {
    dataF->InsertPath( dataF->ReturnDestinationNodeFNSS() );
 }

 //dataF->ListPath();
 //getchar();
}*/

bool TopologySimDC3D::isLinkOff(vector<string> path) {
	for (unsigned int i = 0; i < path.size() - 1; i++) {
		if (topology.getEdge(path[i], path[i + 1])->ReturnIsOff()) {
			return true;
		}
	}
	return false;
}

void TopologySimDC3D::LoadBalancing(DataFlow* dataF, vector< vector<string> > possiblePaths) // Traffic value calculation and load balancing
{
	vector<string> path;
	vector<string> pathTemp;
	FLOATINGPOINT trafficValue = 1000000000;
	FLOATINGPOINT trafficValueTemp = 0;

	if (possiblePaths.size() == 0) {
		cout << "SimDC3D-ERROR: Empty path list !!! " << endl;
		exit(0);
	}

	if (possiblePaths.size() == 1) {
		path = possiblePaths[0];
		for (unsigned int i = 0; i < path.size(); ++i) {
			dataF->InsertPath(path[i]);
		}
	}
	else {
		for (unsigned int i = 0; i < possiblePaths.size(); ++i) {
			pathTemp = possiblePaths[i];
			for (unsigned int j = 0; j < pathTemp.size() - 1; ++j) {
				if (!topology.getEdge(pathTemp[j], pathTemp[j + 1])->ReturnIsOff()) {
					trafficValueTemp += topology.getEdge(pathTemp[j], pathTemp[j + 1])->returnAverageTraffic();
				}
				else {
					cout << "SimDC3D-ERROR: Data flow on a disconnected link !!!" << endl;
					exit(0);
				}
			}
			if (trafficValueTemp < trafficValue) {
				path = pathTemp;
				trafficValue = trafficValueTemp;
			}
			pathTemp.clear();
			trafficValueTemp = 0;
		}

		for (unsigned int i = 0; i < path.size(); ++i) {
			dataF->InsertPath(path[i]);
		}
	}
	possiblePaths.clear();
	path.clear();
	pathTemp.clear();
}

void TopologySimDC3D::AddPath(DataFlow* dataF, vector< vector<string> > possiblePaths) // Traffic value calculation and load balancing
{
	vector<string> path;

	if (possiblePaths.size() == 0) {
		cout << "SimDC3D-ERROR: Empty path list !!! " << endl;
		exit(0);
	}
	else {
		path = possiblePaths[0];
		for (unsigned int i = 0; i < path.size(); ++i) {
			dataF->InsertPath(path[i]);
		}
	}
	
	possiblePaths.clear();
	path.clear();
}

void TopologySimDC3D::threeLayersTopology(DataFlow* dataF, vector< vector<string> > possiblePaths, string switchToRSource)
{
	string switchToRDestination = "";
	vector<string> pathTemp;
	string firstCore;

	// Verify that the destination is on the same ToR switch
	if ((*tServers)[dataF->ReturnChassiSource()][dataF->ReturnServerSource()]->ReturnSwitch() == (*tServers)[dataF->ReturnChassiDestination()][dataF->ReturnServerDestination()]->ReturnSwitch()) {
		dataF->InsertPath(dataF->ReturnSourceNodeFNSS());
		dataF->InsertPath(switchToRSource);
		dataF->InsertPath(dataF->ReturnDestinationNodeFNSS());
	}
	else {
		switchToRDestination = (*tServers)[dataF->ReturnChassiDestination()][dataF->ReturnServerDestination()]->ReturnSwitch();
		// Dois servidores no mesmo switch agregacao
		for (list<int>::iterator iter0 = switchAggregation.begin(); iter0 != switchAggregation.end(); ++iter0) {
			if ((topology.hasEdge(switchToRDestination, to_string(*iter0))) && (topology.hasEdge(to_string(*iter0), switchToRSource))) {
				if ((!topology.getEdge(switchToRDestination, to_string(*iter0))->ReturnIsOff()) && (!topology.getEdge(to_string(*iter0), switchToRSource)->ReturnIsOff())) { // Verify if the edges isn't off
					if (!topology.getNode(to_string(*iter0))->isOFFSwitch()) {
						pathTemp.push_back(dataF->ReturnSourceNodeFNSS());
						pathTemp.push_back(switchToRSource);
						pathTemp.push_back(to_string(*iter0));
						pathTemp.push_back(switchToRDestination);
						pathTemp.push_back(dataF->ReturnDestinationNodeFNSS());
						possiblePaths.push_back(pathTemp);
						pathTemp.clear();
					}
				}
			}
		}
		if (possiblePaths.size() > 0) {
			if (EXECUTE_LOAD_BALANCING) {
				LoadBalancing(dataF, possiblePaths); // Load Balancing
			}
			else {
				AddPath(dataF, possiblePaths); // Adds path without execute load balancing
			}
		}
		else {
			// Dois servidores em switches de agregacao diferentes
			for (list<int>::iterator it = switchCore.begin(); it != switchCore.end(); ++it) {
				for (list<int>::iterator iter = switchAggregation.begin(); iter != switchAggregation.end(); ++iter) {
					if ((topology.hasEdge(switchToRDestination, to_string(*iter))) && (topology.hasEdge(to_string(*iter), to_string(*it)))) {
						if ((!topology.getEdge(switchToRDestination, to_string(*iter))->ReturnIsOff()) && (!topology.getEdge(to_string(*iter), to_string(*it))->ReturnIsOff())) { // Verify if the edges isn't off
							firstCore = to_string(*it);

							for (list<int>::iterator iter2 = switchAggregation.begin(); iter2 != switchAggregation.end(); ++iter2) {
								if ((topology.hasEdge(switchToRSource, to_string(*iter2))) && (topology.hasEdge(to_string(*iter2), firstCore))) {
									if ((!topology.getEdge(switchToRSource, to_string(*iter2))->ReturnIsOff()) && (!topology.getEdge(to_string(*iter2), firstCore)->ReturnIsOff())) { // Verify if the edges isn't off
										if ((!topology.getNode(to_string(*iter2))->isOFFSwitch()) && (!topology.getNode(to_string(*iter))->isOFFSwitch())) {
											// If the two paths lead to the core
											pathTemp.push_back(dataF->ReturnSourceNodeFNSS());
											pathTemp.push_back(switchToRSource);
											pathTemp.push_back(to_string(*iter2));
											pathTemp.push_back(firstCore);
											pathTemp.push_back(to_string(*iter));
											pathTemp.push_back(switchToRDestination);
											pathTemp.push_back(dataF->ReturnDestinationNodeFNSS());
											possiblePaths.push_back(pathTemp);
											pathTemp.clear();
										}
									}
								}
							}
						}
					}
				}
			}
			if (possiblePaths.size() > 0) {
				if (EXECUTE_LOAD_BALANCING) {
					LoadBalancing(dataF, possiblePaths); // Load balancing
				}
				else {
					AddPath(dataF, possiblePaths); // Adds path without execute load balancing
				}
			}
		}
	}
	possiblePaths.clear();
}

void TopologySimDC3D::twoLayersTopology(DataFlow* dataF, vector< vector<string> > possiblePaths, string switchToRSource)
{
	string switchToRDestination = "";
	vector<string> pathTemp;

	// Verify that the destination is on the same ToR switch
	if ((*tServers)[dataF->ReturnChassiSource()][dataF->ReturnServerSource()]->ReturnSwitch() == (*tServers)[dataF->ReturnChassiDestination()][dataF->ReturnServerDestination()]->ReturnSwitch()) {
		dataF->InsertPath(dataF->ReturnSourceNodeFNSS());
		dataF->InsertPath(switchToRSource);
		dataF->InsertPath(dataF->ReturnDestinationNodeFNSS());
	}
	else {
		switchToRDestination = (*tServers)[dataF->ReturnChassiDestination()][dataF->ReturnServerDestination()]->ReturnSwitch();
		// Destination is on the same Core switch
		for (list<int>::iterator it = switchCore.begin(); it != switchCore.end(); ++it) {
			if ((topology.hasEdge(switchToRDestination, to_string(*it))) && (topology.hasEdge(switchToRSource, to_string(*it)))) {
				if ((!topology.getEdge(switchToRDestination, to_string(*it))->ReturnIsOff()) && (!topology.getEdge(switchToRSource, to_string(*it))->ReturnIsOff())) { // Verify if the edges isn't off
					if (!topology.getNode(to_string(*it))->isOFFSwitch()) {
						pathTemp.push_back(dataF->ReturnSourceNodeFNSS());
						pathTemp.push_back(switchToRSource);
						pathTemp.push_back(to_string(*it));
						pathTemp.push_back(switchToRDestination);
						pathTemp.push_back(dataF->ReturnDestinationNodeFNSS());
						possiblePaths.push_back(pathTemp);
						pathTemp.clear();
					}
				}
			}
		}
		if (possiblePaths.size() > 0) {
			if (EXECUTE_LOAD_BALANCING) {
				LoadBalancing(dataF, possiblePaths); // Load balancing
			}
			else {
				AddPath(dataF, possiblePaths); // Adds path without execute load balancing
			}
		}
	}
	possiblePaths.clear();
}

void TopologySimDC3D::InsertPathInFlow(DataFlow* dataF) // InsertPathInFlowThreeLayers
{
	string switchToRSource = "";
	vector< vector<string> > possiblePaths;
	vector<string> pathTemp;
	vector<VirtualMachine *> vmTemp;
	vector<DataFlow *> dataFlowTemp;

	// Get Switch ToR
	switchToRSource = (*tServers)[dataF->ReturnChassiSource()][dataF->ReturnServerSource()]->ReturnSwitch();
	// Checks if the communicate is internal
	if (dataF->ReturnServerDestination() != 88888) {
		// Check if it is a three layers topology
		if (TYPE_TOPOLOGY == "3LAYER") {
			threeLayersTopology(dataF, possiblePaths, switchToRSource);
		}
		// It is a two layers topology
		else { 
			if (TYPE_TOPOLOGY == "2LAYER") {
			   twoLayersTopology(dataF, possiblePaths, switchToRSource);
		    }
			else {
				cout << "SIMDC3D-ERROR: Topology is not implemented !!!" << endl;
				exit(0);
			}
		}
	}
	// It is an external communication
	else {
		// Check if it is a three layers topology
		if (TYPE_TOPOLOGY == "3LAYER") {
			possiblePaths.clear();
			for (list<int>::iterator iter3 = switchAggregation.begin(); iter3 != switchAggregation.end(); ++iter3) {
				if ((topology.hasEdge(switchToRSource, to_string(*iter3))) && (topology.hasEdge(to_string(*iter3), dataF->ReturnDestinationNodeFNSS()))) {
					if ((!topology.getEdge(switchToRSource, to_string(*iter3))->ReturnIsOff()) && (!topology.getEdge(to_string(*iter3), dataF->ReturnDestinationNodeFNSS())->ReturnIsOff())) { // Verify if the edges isn't off
						if (!topology.getNode(to_string(*iter3))->isOFFSwitch()) {
							pathTemp.push_back(dataF->ReturnSourceNodeFNSS());
							pathTemp.push_back(switchToRSource);
							pathTemp.push_back(to_string(*iter3));
							pathTemp.push_back(dataF->ReturnDestinationNodeFNSS());
							possiblePaths.push_back(pathTemp);
							pathTemp.clear();
						}
					}
				}
			}
			if (possiblePaths.size() > 0) {
				if (EXECUTE_LOAD_BALANCING) {
					LoadBalancing(dataF, possiblePaths); // Load balancing
				}
				else {
					AddPath(dataF, possiblePaths); // Adds path without execute load balancing
				}
			}
			else {
				ReconnectEdges(); // Reconnect all Edges

				InsertPathInFlow(dataF);
			}
		}
		else { 
			if (TYPE_TOPOLOGY == "2LAYER") {
				// It is a two layers topology
				possiblePaths.clear();
				if (EXECUTE_LOAD_BALANCING) {
					for (list<int>::iterator it4 = switchCore.begin(); it4 != switchCore.end(); ++it4) {
						if (topology.hasEdge(switchToRSource, to_string(*it4))) {
							if (!topology.getEdge(switchToRSource, to_string(*it4))->ReturnIsOff()) { // Verify if the edges isn't off
								pathTemp.push_back(dataF->ReturnSourceNodeFNSS());
								pathTemp.push_back(switchToRSource);
								pathTemp.push_back(dataF->ReturnDestinationNodeFNSS());
								possiblePaths.push_back(pathTemp);
								pathTemp.clear();
							}
						}
					}
					if (possiblePaths.size() > 0) {
						LoadBalancing(dataF, possiblePaths); // Load balancing
					}
					else {
						ReconnectEdges(); // Reconnect all Edges
						InsertPathInFlow(dataF);
					}
				}
				else {
					dataF->InsertPath(dataF->ReturnSourceNodeFNSS());
					dataF->InsertPath(switchToRSource);
					dataF->InsertPath(dataF->ReturnDestinationNodeFNSS());
				}
			}
			else {
				cout << "SIMDC3D-ERROR: Topology is not implemented !!!" << endl;
				exit(0);
			}
	    }
	}
	possiblePaths.clear();
}



double TopologySimDC3D::ReturnTotalEnergyAccessSwitchSparseLog(void)
{
  double sum = 0.00;

  for(unsigned int i=0; i < vTotalEnergyAccessSwitchSparseLog.size(); i++) {
	  sum += vTotalEnergyAccessSwitchSparseLog[i];
  }
  return sum;
}

double TopologySimDC3D::ReturnTotalEnergyAggregationSwitchSparseLog(void)
{
  double sum = 0.00;

  for(unsigned int i=0; i < vTotalEnergyAggregationSwitchSparseLog.size(); i++) {
	  sum += vTotalEnergyAggregationSwitchSparseLog[i];
  }
  return sum;
}

double TopologySimDC3D::ReturnTotalEnergyCoreSwitchSparseLog(void)
{
  double sum = 0.00;

  for(unsigned int i=0; i <vTotalEnergyCoreSwitchSparseLog.size(); i++) {
	  sum += vTotalEnergyCoreSwitchSparseLog[i];
  }
  return sum;
}

void TopologySimDC3D::PrintPowerDrawAccessSwitch(void)
{
  for (unsigned int i=0; i<vTotalEnergyAccessSwitchSparseLog.size(); ++i) {
	  cout << vTotalEnergyAccessSwitchSparseLog[i] << "\t";
  }
  cout << endl;
}
void TopologySimDC3D::PrintPowerDrawAggregationSwitch(void)
{
  for (unsigned int i=0; i< vTotalEnergyAggregationSwitchSparseLog.size(); ++i) {
	  cout << vTotalEnergyAggregationSwitchSparseLog[i] << "\t";
  }
  cout << endl;
}
void TopologySimDC3D::PrintPowerDrawCoreSwitch(void)
{
  for (unsigned int i=0; i< vTotalEnergyCoreSwitchSparseLog.size(); ++i) {
	  cout << vTotalEnergyCoreSwitchSparseLog[i] << "\t";
  }
  cout << endl;
}

void TopologySimDC3D::PrintTotalPowerDrawSwitches(void)
{
  for (unsigned int i=0; i< vTotalEnergyAccessSwitchSparseLog.size(); ++i) {
	  cout << vTotalEnergyAccessSwitchSparseLog[i] + vTotalEnergyAggregationSwitchSparseLog[i] + vTotalEnergyCoreSwitchSparseLog[i] << "\t";
  }
  cout << endl;
}

double TopologySimDC3D::ReturnPowerDrawSwitches(int ind)
{
  return vTotalEnergyAccessSwitchSparseLog[ind] + vTotalEnergyAggregationSwitchSparseLog[ind] + vTotalEnergyCoreSwitchSparseLog[ind];
}

void TopologySimDC3D::PrintTotalEnergyAccessSwitchSparseLog(void)
{
  for (unsigned int i=0; i<vTotalEnergyAccessSwitchSparseLog.size(); ++i) {
	  cout << (vTotalEnergyAccessSwitchSparseLog[i] * PERIODIC_LOG_INTERVAL)<< "\t";
  }
  cout << endl;
}
void TopologySimDC3D::PrintTotalEnergyAggregationSwitchSparseLog(void)
{
  for (unsigned int i=0; i< vTotalEnergyAggregationSwitchSparseLog.size(); ++i) {
	  cout << (vTotalEnergyAggregationSwitchSparseLog[i] * PERIODIC_LOG_INTERVAL) << "\t";
  }
  cout << endl;
}
void TopologySimDC3D::PrintTotalEnergyCoreSwitchSparseLog(void)
{
  for (unsigned int i=0; i< vTotalEnergyCoreSwitchSparseLog.size(); ++i) {
	  cout << (vTotalEnergyCoreSwitchSparseLog[i] * PERIODIC_LOG_INTERVAL) << "\t";
  }
  cout << endl;
}

void TopologySimDC3D::PrintTotalEnergySwitchesSparseLog(void)
{
  for (unsigned int i=0; i< vTotalEnergyAccessSwitchSparseLog.size(); ++i) {
	  cout << ((vTotalEnergyAccessSwitchSparseLog[i] + vTotalEnergyAggregationSwitchSparseLog[i] + vTotalEnergyCoreSwitchSparseLog[i]) * PERIODIC_LOG_INTERVAL) << "\t";
  }
  cout << endl;
}

void TopologySimDC3D::ClearTraffic(void) 
{
 edges = topology.getAllEdges();

 for (set<pair <string, string> >::iterator it = edges.begin(); it != edges.end(); it++) {
	 if (!topology.getEdge((*it).first, (*it).second)->ReturnIsOff()) {
		 topology.getEdge((*it).first, (*it).second)->ClearTraffic();
	 }
 }	
}

void TopologySimDC3D::StatisticSwitch(void)
{

 //nodes = topology.getAllNodes();

 cout << "SWITCHES EDGE	 Connect	10		100		1000	10000	Disconect	10	100	 1000	1000" << endl;

 for (list<int>::iterator it=switchEdge.begin(); it!=switchEdge.end(); ++it) {
	 cout << "Switch Edge " << *it << " "; 
	 topology.getNode(to_string(*it))->printStatistic();
	 cout << endl;
 }

 cout << endl << endl;

 cout << "SWITCHES AGGREGATION" << endl;

 for (list<int>::iterator it=switchAggregation.begin(); it!=switchAggregation.end(); ++it) {
      cout << "Switch Aggregation " << *it << endl; 
	  topology.getNode(to_string(*it))->printStatistic();
	  cout << endl;
 }

 cout << endl << endl;

 cout << "SWITCHES CORE" << endl;
 for (list<int>::iterator it=switchCore.begin(); it!=switchCore.end(); ++it) {
      cout << "Switch Core " << *it << " "; 
	  topology.getNode(to_string(*it))->printStatistic();
	  cout << endl;
 }

 cout << endl << endl;

 if ( SIMULATE_TRAFFIC_MATRIX ) {
    cout << "LINKS		Sum Traffic		Average Traffic		Higher Traffic		Lower Traffic" << endl << endl;

    for (set<pair <string, string> >::iterator it = edges.begin(); it != edges.end(); it++) {
        cout << "[EDGE " << (*it).first<< " -> " << (*it).second<<"] " <<  "Traffic Total	" << topology.getEdge(*it)->ReturnSumTraffic() << "		Average " <<  topology.getEdge(*it)->ReturnAverageTraffic() << "		Standard Deviation " << topology.getEdge(*it)->ReturnStandardDeviation();
	    cout << "		Higher Traffic " <<  topology.getEdge(*it)->returnHigherTraffic() << "	";
	    cout << "		Lower Traffic " <<  topology.getEdge(*it)->returnLowerTraffic() << endl;
    }	
 }

 cout << endl << endl;

}

void TopologySimDC3D::LoadTrafficMatriz(void) 
{
	if ( (clockTP%NUMBER_OF_SAMPLES_TRAFFIC_MATRIX == 0) && (clockTP != 0) ) {
       numberSamplesTM += 1;
       if ( numberSamplesTM < totalSamplesTM ) {
          matrixTraffic->GetMatrixTraffic(numberSamplesTM);
	   }
	   else {
		  numberSamplesTM = 0; 
		  matrixTraffic->GetMatrixTraffic(numberSamplesTM);
	   }
    }
}


