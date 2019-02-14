#include <iostream>
#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <iterator>
#include <algorithm>
#include <cstdlib>
//#include <conio.h>
#include <math.h> 
#include <stdlib.h>
#include <time.h>

#include "SimDC3D-Constants.h"
#include "SimDC3D-DataCenter.h"
#include "SimDC3D-Server.h"
#include "SimDC3D-Users.h"
#include "SimDC3D-JobQueue.h"
#include "SimDC3D-PoolServers.h"
#include "SimDC3D-Topology.h"

using namespace std;

// settings General
int PERIODIC_LOG_INTERVAL = 0;						    // in seconds. shorter interval requires more memory space
	int SKIP_x_SECONDS = 0;								// skip first ? seconds
	int FINISHES_AT_DAY = 0;							// stop simulator at day FINISHES_AT_DAY
	bool PRINTSCREEN = false;			
	bool RUN_MULTITHREAD = false;						// Run Multithread;
	int NUMBER_OF_THREADS = 0;							// Number of Threads
	int SEED = 0;										// Data Center SEED
	long int SUBMIT_TIME_SEC = 0;


// setting Server
int NUMBER_OF_CHASSIS = 0;								// how many blade server chassis in the data center? // change SimWare-Constants.h
	int NUMBER_OF_SERVERS_IN_ONE_CHASSIS = 0;
	int NUMBER_OF_CORES_IN_ONE_SERVER = 0;
	long int TOTAL_OF_MEMORY_IN_ONE_SERVER = 0;			// in kbytes 
	long int BANDWIDTH_IN_ONE_SERVER = 0;				// in Kbits/s
	bool CONSTANT_FAN_POWER = false;					// true = fan rpm is constant at max
	bool FAN_RPM_NO_LIMIT = false;						// false = Fan's max rpm to set at 3000, use true for SUPPLY_TEMPERATURE_OFFSET_ALPHA >= 0
	int POWER_ON = 0;									// time to turn on the server
	int POWER_OFF = 0;									// time to turn off the server
	bool HIBERNATING_SERVER = false;					// defines whether the servers will be turned off or hibernated // true = hibernate, false = poweroff  
	float POWER_CONSUMPTION_HIBERNATING = 0.0;			// Power consumption in HIBERNATING (Watts)
	int HIBERNATING_POWER_ON = 0;						// time to remove a server into hibernation
    int HIBERNATING_POWER_OFF = 0;						// time for the server go into hibernation
	string MODEL_OF_ENERGY_CONSUMPTION="";				// Model of energy consumption of the server;



// setting Virtual Machines
vector<long int> MEMORY_VIRTUAL_MACHINE;				// in kbytes
	int INDEX = 0;										// NOT PARAMETRIZAVEL
	long int IMAGE_SIZE_VIRTUAL_MACHINE = 0;			// in kbytes 200 Mbytes
	long int V_TRESHOLD = 0;							// kbytes Calculate the downtime during the migration of a virtual machine. Extracted "Energy-Aware Cloud Management through Progressive SLA Specification".
	long int MEMORY_DIRTYING = 0;						// kbits Calculate the downtime during the migration of a virtual machine. Extracted "Energy-Aware Cloud Management through Progressive SLA Specification".
	int T_RESUME = 0;									// seg  Calculate the downtime during the migration of a virtual machine. Extracted "Energy-Aware Cloud Management through Progressive SLA Specification".


// setting CRAC
int CRAC_SUPPLY_TEMP_IS_CONSTANT_AT = 0;				// 0 = false, use dynamic crac control or enter a constant temperature 
	int SUPPLY_TEMPERATURE_OFFSET_ALPHA = 0;			// T_trigger = T_emergency + alpha
	bool INSTANT_COOL_AIR_TRAVEL_TIME = false;			// true = cool air from CRAC arrives instantly to the servers
	bool INSTANT_CHANGE_CRAC_SUPPLY_AIR = false;		// true = CRAC changes discharge air temperature instantly (e.g., from 10C to 15C or 
    int CRAC_DISCHARGE_CHANGE_RATE_0_00x = 0;
	string POLICY_OF_TEMPERATURE_CONTROL="";			// Default, Aggressive_Mode
	float EMERGENCY_TEMPERATURE = 0.00;					// Emergency Temperature

// setting scheduling Algorithm
string SCHEDULING_ALGORITHM = "";						// best_performance, uniform_task, low_temp_first, random, min_hr, center_rack_first, 2D_POOL, 2D_POOL_AND_Prediction, 2D_Prediction
    bool SCHEDULING_WITH_PREDICTION = false;			// 
	bool OPTIMIZATION_WITH_PREDICTION = false;			//
    string PREDICTION_ALGORITHM = "";					// POLYNOM, RBF
	string VARIABLE_PREDICTED = "";						// Temperature
	float SCHEDULER_2D_WEIGHT_TEMPERATURE = 0.00;		// The sum of the weights of the 3 parameters (2D_SCHEDULER_WEIGHT_TEMPERATURE, 2D_SCHEDULER_HEAT_RECIRCULATION and 2D_SCHEDULER_LOAD_CPU)  must be equal to 1 	
	float SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION = 0.00;		//
	float SCHEDULER_2D_WEIGHT_LOAD_CPU = 0.00;					//

	//parameters prediction algorithm RBF and POLYNOM
	unsigned int SIZE_WINDOWN_PREDICTION = 0;			//	Size Windown Prediction
	int MONITORINGTIME = 0;								//  Monitoring Time in Seconds

	// parameters prediction algorithm RBF 
	unsigned int DIM = 0;										//-m embedding dimension [default: 2]
	unsigned int DELAY = 0;										//-d delay [default: 1]
	int CENTER = 0;										//-p number of centers [default: 10]
	int STEP = 0;										//-s steps to forecast [default: 1]
	unsigned int COLUMN = 0;							//-c column to read [default: 1]
	unsigned long LENGTH = ULONG_MAX;					// NOT PARAMETRIZAVEL //-l # of data to use [default: all from vector]  
	long CLENGTH = 0;									//-L steps to cast [default: none]
	char MAKECAST = 0;									// not change

	//parameters prediction algorithm POLYNOM
	int N = 0;											// -p order of the polynomial [default: 2]\n
	char CAST = 0;										// not change


// setting workload
bool DYNAMIC_WORKLOAD_VMS = false;
	string WORKLOAD_DISTRIBUTION_MODEL = "";			// uniform_real_distribution, poisson_distribution

// setting migration parameters
bool SIMULATES_MIGRATION_VMS = false;
	string OPTIMIZATION_ALGORITHM_LOW_UTILIZATION = "";
	int PERIODIC_INTERVAL_EXECUTION_LOW = 0; 
	
	string OPTIMIZATION_ALGORITHM_OVERLOAD_UTILIZATION = "";
	int PERIODIC_INTERVAL_EXECUTION_OVERLOAD = 0; 
	
	string OPTIMIZATION_ALGORITHM_OVERHEATING_TEMPERATURE = "";
	int PERIODIC_INTERVAL_EXECUTION_TEMPERATURE = 0; 

	string VMS_ALGORITHM_SELECTION = "";				//defines the algorithm for selecting a VM for migration 

	string OPTIMIZATION_ALGORITHM_LINK_OVERLOAD = "";
	int PERIODIC_INTERVAL_EXECUTION_LINK_OVERLOAD= 0; 
	string VMS_ALGORITHM_SELECTION_OVERLOADED_LINK = "";

	float THRESHOLD_TOP_OF_USE_CPU = 0;
	float THRESHOLD_BOTTOM_OF_USE_CPU = 0;

	bool USING_PREDICTION_TO_OVERLOAD = false;
		string PREDICTION_ALGORITHM_OVERLOAD = "";
		unsigned int SIZE_WINDOWN_PREDICTION_CPU = 0;


// setting 3DMOBFD

int ALPHA_3DMOBFD = 0;
int BETA_3DMOBFD = 0;
int GAMMA_3DMOBFD = 0;
int DELTA_3DMOBFD = 0;
int EPSILON_3DMOBFD = 0;

float WEIGHT_TEMPERATURE = 0;
float WEIGHT_CPU = 0;
float WEIGHT_POWER = 0;
float WEIGHT_MEMORY = 0;
float WEIGHT_TRAFFIC = 0;

int E_TEMPERATURE = 0;	
int E_CPU = 0;	
int E_MEMORY = 0;
int E_TRAFFIC = 0;	
int E_POWER = 0;	


// setting network parameters
bool SIMULATES_NETWORK = false;
	string TYPE_TOPOLOGY = "";
    string NAME_FILE_TOPOLOGY = "";

	int NUMBER_CHASSI_RACK = 0;						// Number of Chassis on a rack. Size Chassi is 10U
	int NUMBER_OF_PORTS_SWITCH_ACCESS = 0;			// Number of Ports of Access Switch
	int NUMBER_OF_PORTS_SWITCH_AGGREGATION = 0;		// Number of Ports of Access Aggregation
	int NUMBER_OF_PORTS_SWITCH_CORE = 0;			// Number of Ports of Access Core

	float EACCE_CHASSIS = 0;						// power consumption of the chassis
	float EACCE_LINECARD = 0;						// power consumption of the line card	
	float EACCE_PORT10 = 0;							// power consumption of the port 10Mbps
	float EACCE_PORT100 = 0;						// power consumption of the port 100Mbps
	float EACCE_PORT1000 = 0;						// power consumption of the port 1000Mbps
	float EACCE_PORT10000 = 0;						// power consumption of the port 10000Mbps
	
	float EAGGR_CHASSIS = 0;						// power consumption of the chassis
	float EAGGR_LINECARD = 0;						// power consumption of the line card
	float EAGGR_PORT10 = 0;							// power consumption of the port 10Mbps
	float EAGGR_PORT100 = 0;						// power consumption of the port 100Mbps
	float EAGGR_PORT1000 = 0;						// power consumption of the port 1000Mbps 
	float EAGGR_PORT10000 = 0;						// power consumption of the port 10000Mbps

	float ECORE_CHASSIS = 0;						// power consumption of the chassis 
	float ECORE_LINECARD = 0;						// power consumption of the line card
	float ECORE_PORT10 = 0;							// power consumption of the port 10 Mbps
	float ECORE_PORT100 = 0;						// power consumption of the port 10 Mbps
	float ECORE_PORT1000 = 0;						// power consumption of the port 10 Mbps
	float ECORE_PORT10000 = 0;						// power consumption of the port 10 Mbps

	int SWITCH_ACCESS = 0;  						// Number switches in ACCESS network - calculate TopologySimware::CreateTopology()              // NOT PARAMETRIZAVEL
	int SWITCH_AGGREGATION = 0; 					// Number of Switches in AGGREGATION network - calculate TopologySimware::CreateTopology()		// NOT PARAMETRIZAVEL
	int SWITCH_CORE	= 0; 							// Number of L3 Switches in the CORE network - calculate TopologySimware::CreateTopology()		// NOT PARAMETRIZAVEL

// parameters traffic matrix
bool SIMULATE_TRAFFIC_MATRIX = false;
	string NAME_FILE_MATRIX_TRAFFIC = "";
	bool CALCULATE_TRAFFIC_CORRELATION = false;
	bool EXECUTE_LOAD_BALANCING = false;
	int	NUMBER_NODES_TRAFFIC_MATRIX = 0;
	int NUMBER_OF_SAMPLES_TRAFFIC_MATRIX = 0;
	int NUMBER_OF_PERIODS = 0;
	float K = 3;												// Variable K of the lognormal distribution	
	int SEED_PERIOD_ON = 0;										// Seed in the generation of the traffic period on
	int SEED_PERIOD_OFF = 0;									// Seed in the generation of the traffic period on
	int PROPORTION_OF_INTERNAL_COMMUNICATION_BETWEEN_VMS = 0;	// Percentage of internal communication between VMs. Note, the summation between internal and external communication must be equal to 100
	int PROPORTION_OF_EXTERNAL_COMMUNICATION = 0;				// Percentage of external communication

	bool DYNAMIC_SPEED_ADAPTATION = false;
	int RUNTIME_LINK_ADAPTATION = 0; 							// Time in seconds to execute the link adaptation
	int TRAFFIC_LIMIT_FOR_ADAPTATION = 0;


	bool SIMULATE_TOPOLOGY_OPTIMIZATION = false;
	string OPTIMIZATION_ALGORITHM_TOPOLOGY="";					// Topology optimization algorithm;
	int PERIODIC_INTERVAL_EXECUTION_TOPOLOGY = 0;				// Period of execution of the topology optimization algorithm


// setting POOL
bool SIMULATES_POOL_SERVER = false;
	unsigned int SIZE_POOL = 0;								// The initial size POOL
	bool INCREASE_SIZE_POOL_DYNAMICALLY = false;

// parameters for future work
int COOL_AIR_TRAVEL_TIME_MULTIPLIER = 0;
	bool TEMPERATURE_SENSING_PERFORMANCE_CAPPING = false;
	int TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS = 0;
	bool REASSIGN_VMS = false;
	

long int MAX_TIME = 0;								// NOT PARAMETRIZAVEL



bool To_Bool(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    std::istringstream is(str);
    bool b;
    is >> std::boolalpha >> b;
    return b;
}


int SetParameters(std::map <string, string> par)
{
  std::map<string,string>::iterator it;

    // PERIODIC_LOG_INTERVAL
    if(par.find("PERIODIC_LOG_INTERVAL") != par.end()) {
      PERIODIC_LOG_INTERVAL = stoi(par.find("PERIODIC_LOG_INTERVAL")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! PERIODIC_LOG_INTERVAL " << endl;
	  return 1;
	}

  // SKIP_x_SECONDS
    if(par.find("SKIP_x_SECONDS") != par.end()) {
      SKIP_x_SECONDS = stoi(par.find("SKIP_x_SECONDS")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! SKIP_x_SECONDS " << endl;
	  return 1;
	}

  // FINISHES_AT_DAY
    if(par.find("FINISHES_AT_DAY") != par.end()) {
      FINISHES_AT_DAY = stoi(par.find("FINISHES_AT_DAY")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! FINISHES_AT_DAY " << endl;
	  return 1;
	}


  // PRINTSCREEN
    if(par.find("PRINTSCREEN") != par.end()) {
      PRINTSCREEN = To_Bool(par.find("PRINTSCREEN")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! PRINTSCREEN " << endl;
	  return 1;
	}

	// RUN_MULTITHREAD
    if(par.find("RUN_MULTITHREAD") != par.end()) {
      RUN_MULTITHREAD = To_Bool(par.find("RUN_MULTITHREAD")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! RUN_MULTITHREAD " << endl;
	  return 1;
	}
	
	// NUMBER_OF_THREADS
    if(par.find("NUMBER_OF_THREADS") != par.end()) {
      NUMBER_OF_THREADS = stoi(par.find("NUMBER_OF_THREADS")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! NUMBER_OF_THREADS " << endl;
	  return 1;
	}

	
	// DATA CENTER SEED
    if(par.find("SEED") != par.end()) {
      SEED = stoi(par.find("SEED")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! SEED " << endl;
	  return 1;
	}

	// SUBMIT_TIME_SEC
    if(par.find("SUBMIT_TIME_SEC") != par.end()) {
     SUBMIT_TIME_SEC = stol(par.find("SUBMIT_TIME_SEC")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! SUBMIT_TIME_SEC " << endl;
	  return 1;
	}


	// NUMBER_OF_CHASSIS
    if(par.find("NUMBER_OF_CHASSIS") != par.end()) {
      NUMBER_OF_CHASSIS = stoi(par.find("NUMBER_OF_CHASSIS")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! NUMBER_OF_CHASSIS " << endl;
	  return 1;
	}

  // NUMBER_OF_SERVERS_IN_ONE_CHASSIS
    if(par.find("NUMBER_OF_SERVERS_IN_ONE_CHASSIS") != par.end()) {
      NUMBER_OF_SERVERS_IN_ONE_CHASSIS = stoi(par.find("NUMBER_OF_SERVERS_IN_ONE_CHASSIS")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! NUMBER_OF_SERVERS_IN_ONE_CHASSIS " << endl;
	  return 1;
	}

  // NUMBER_OF_CORES_IN_ONE_SERVER
    if(par.find("NUMBER_OF_CORES_IN_ONE_SERVER") != par.end()) {
      NUMBER_OF_CORES_IN_ONE_SERVER = stoi(par.find("NUMBER_OF_CORES_IN_ONE_SERVER")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! NUMBER_OF_CORES_IN_ONE_SERVER " << endl;
	  return 1;
	}

  // TOTAL_OF_MEMORY_IN_ONE_SERVER
    if(par.find("TOTAL_OF_MEMORY_IN_ONE_SERVER") != par.end()) {
      TOTAL_OF_MEMORY_IN_ONE_SERVER = stol(par.find("TOTAL_OF_MEMORY_IN_ONE_SERVER")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! TOTAL_OF_MEMORY_IN_ONE_SERVER " << endl;
	  return 1;
	}

  // BANDWIDTH_IN_ONE_SERVER 
    if(par.find("BANDWIDTH_IN_ONE_SERVER") != par.end()) {
      BANDWIDTH_IN_ONE_SERVER  = stol(par.find("BANDWIDTH_IN_ONE_SERVER")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! BANDWIDTH_IN_ONE_SERVER  " << endl;
	  return 1;
	}

  // CONSTANT_FAN_POWER
    if(par.find("CONSTANT_FAN_POWER") != par.end()) {
      CONSTANT_FAN_POWER = To_Bool(par.find("CONSTANT_FAN_POWER")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! CONSTANT_FAN_POWER " << endl;
	  return 1;
	}


  // FAN_RPM_NO_LIMIT
    if(par.find("FAN_RPM_NO_LIMIT") != par.end()) {
      FAN_RPM_NO_LIMIT = To_Bool(par.find("FAN_RPM_NO_LIMIT")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! FAN_RPM_NO_LIMIT " << endl;
	  return 1;
	}


  // POWER_ON 
    if(par.find("POWER_ON") != par.end()) {
      POWER_ON  = stoi(par.find("POWER_ON")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! POWER_ON  " << endl;
	  return 1;
	}

  // POWER_OFF
    if(par.find("POWER_OFF") != par.end()) {
      POWER_OFF  = stoi(par.find("POWER_OFF")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! POWER_OFF  " << endl;
	  return 1;
	}

    // HIBERNATING_SERVER
    if(par.find("HIBERNATING_SERVER") != par.end()) {
      HIBERNATING_SERVER = To_Bool(par.find("HIBERNATING_SERVER")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! HIBERNATING_SERVER " << endl;
	  return 1;
	}

  // POWER_CONSUMPTION_HIBERNATING
	if(par.find("POWER_CONSUMPTION_HIBERNATING") != par.end()) {
      POWER_CONSUMPTION_HIBERNATING = stof(par.find("POWER_CONSUMPTION_HIBERNATING")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! POWER_CONSUMPTION_HIBERNATING " << endl;
	  return 1;
	}

  // HIBERNATING_POWER_ON
    if(par.find("HIBERNATING_POWER_ON") != par.end()) {
      HIBERNATING_POWER_ON = stoi(par.find("HIBERNATING_POWER_ON")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! HIBERNATING_POWER_ON " << endl;
	  return 1;
	}

  // HIBERNATING_POWER_OFF
    if(par.find("HIBERNATING_POWER_OFF") != par.end()) {
      HIBERNATING_POWER_OFF = stoi(par.find("HIBERNATING_POWER_OFF")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! HIBERNATING_POWER_OFF " << endl;
	  return 1;
	}

	// MODEL_OF_ENERGY_CONSUMPTION
	if(par.find("MODEL_OF_ENERGY_CONSUMPTION") != par.end()) {
	  boost::to_upper(par.find("MODEL_OF_ENERGY_CONSUMPTION")->second);
      MODEL_OF_ENERGY_CONSUMPTION = par.find("MODEL_OF_ENERGY_CONSUMPTION")->second;
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! MODEL_OF_ENERGY_CONSUMPTION " << endl;
	  return 1;
	}

  // MEMORY_VIRTUAL_MACHINE
    if(par.find("MEMORY_VIRTUAL_MACHINE") != par.end()) {
	   string convert;
	   convert = par.find("MEMORY_VIRTUAL_MACHINE")->second + ",";
	   while (convert.length() != 0) {
              long int value;
			  value = stol(convert.substr(0,convert.find(",")));
              MEMORY_VIRTUAL_MACHINE.push_back(value);
			  convert.erase(0, convert.find(',')+1);	
      }
    }
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! MEMORY_VIRTUAL_MACHINE " << endl;
	  return 1;
	}

  // IMAGE_SIZE_VIRTUAL_MACHINE
    if(par.find("IMAGE_SIZE_VIRTUAL_MACHINE") != par.end()) {
      IMAGE_SIZE_VIRTUAL_MACHINE = stol(par.find("IMAGE_SIZE_VIRTUAL_MACHINE")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! IMAGE_SIZE_VIRTUAL_MACHINE " << endl;
	  return 1;
	}

  // V_TRESHOLD
    if(par.find("V_TRESHOLD") != par.end()) {
      V_TRESHOLD = stol(par.find("V_TRESHOLD")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! V_TRESHOLD " << endl;
	  return 1;
	}

  // MEMORY_DIRTYING
    if(par.find("MEMORY_DIRTYING") != par.end()) {
      MEMORY_DIRTYING = stol(par.find("MEMORY_DIRTYING")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! MEMORY_DIRTYING " << endl;
	  return 1;
	}

  // T_RESUME
    if(par.find("T_RESUME") != par.end()) {
      T_RESUME = stoi(par.find("T_RESUME")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! T_RESUME " << endl;
	  return 1;
	}

  // CRAC_SUPPLY_TEMP_IS_CONSTANT_AT
    if(par.find("CRAC_SUPPLY_TEMP_IS_CONSTANT_AT") != par.end()) {
      CRAC_SUPPLY_TEMP_IS_CONSTANT_AT = stoi(par.find("CRAC_SUPPLY_TEMP_IS_CONSTANT_AT")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! CRAC_SUPPLY_TEMP_IS_CONSTANT_AT " << endl;
	  return 1;
	}

  // SUPPLY_TEMPERATURE_OFFSET_ALPHA
    if(par.find("SUPPLY_TEMPERATURE_OFFSET_ALPHA") != par.end()) {
      SUPPLY_TEMPERATURE_OFFSET_ALPHA = stoi(par.find("SUPPLY_TEMPERATURE_OFFSET_ALPHA")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! SUPPLY_TEMPERATURE_OFFSET_ALPHA " << endl;
	  return 1;
	}

	 // INSTANT_COOL_AIR_TRAVEL_TIME
    if(par.find("INSTANT_COOL_AIR_TRAVEL_TIME") != par.end()) {
      INSTANT_COOL_AIR_TRAVEL_TIME = To_Bool(par.find("INSTANT_COOL_AIR_TRAVEL_TIME")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! INSTANT_COOL_AIR_TRAVEL_TIME " << endl;
	  return 1;
	}

	 // INSTANT_CHANGE_CRAC_SUPPLY_AIR
    if(par.find("INSTANT_CHANGE_CRAC_SUPPLY_AIR") != par.end()) {
      INSTANT_CHANGE_CRAC_SUPPLY_AIR = To_Bool(par.find("INSTANT_CHANGE_CRAC_SUPPLY_AIR")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! INSTANT_CHANGE_CRAC_SUPPLY_AIR " << endl;
	  return 1;
	}

	// CRAC_DISCHARGE_CHANGE_RATE_0_00x
	if(par.find("CRAC_DISCHARGE_CHANGE_RATE_0_00x") != par.end()) {
      CRAC_DISCHARGE_CHANGE_RATE_0_00x = stoi(par.find("CRAC_DISCHARGE_CHANGE_RATE_0_00x")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! CRAC_DISCHARGE_CHANGE_RATE_0_00x " << endl;
	  return 1;
	}

	// EMERGENCY_TEMPERATURE

	if(par.find("EMERGENCY_TEMPERATURE") != par.end()) {
      EMERGENCY_TEMPERATURE = stof(par.find("EMERGENCY_TEMPERATURE")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! EMERGENCY_TEMPERATURE " << endl;
	  return 1;
	}
	
	// POLICY_OF_TEMPERATURE_CONTROL
	if(par.find("POLICY_OF_TEMPERATURE_CONTROL") != par.end()) {
	  boost::to_upper(par.find("POLICY_OF_TEMPERATURE_CONTROL")->second);
      POLICY_OF_TEMPERATURE_CONTROL = par.find("POLICY_OF_TEMPERATURE_CONTROL")->second;
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! POLICY_OF_TEMPERATURE_CONTROL " << endl;
	  return 1;
	}

	//SCHEDULING_ALGORITHM
	if(par.find("SCHEDULING_ALGORITHM") != par.end()) {
	  boost::to_upper(par.find("SCHEDULING_ALGORITHM")->second);
      SCHEDULING_ALGORITHM = par.find("SCHEDULING_ALGORITHM")->second;
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! SCHEDULING_ALGORITHM " << endl;
	  return 1;
	}

	// SCHEDULING_WITH_PREDICTION
    if(par.find("SCHEDULING_WITH_PREDICTION") != par.end()) {
      SCHEDULING_WITH_PREDICTION = To_Bool(par.find("SCHEDULING_WITH_PREDICTION")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! SCHEDULING_WITH_PREDICTION " << endl;
	  return 1;
	}

	// OPTIMIZATION_WITH_PREDICTION
    if(par.find("OPTIMIZATION_WITH_PREDICTION") != par.end()) {
      OPTIMIZATION_WITH_PREDICTION = To_Bool(par.find("OPTIMIZATION_WITH_PREDICTION")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! OPTIMIZATION_WITH_PREDICTION " << endl;
	  return 1;
	}

	// PREDICTION_ALGORITHM
	if(par.find("PREDICTION_ALGORITHM") != par.end()) {
	  boost::to_upper(par.find("PREDICTION_ALGORITHM")->second);
      PREDICTION_ALGORITHM = par.find("PREDICTION_ALGORITHM")->second;
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! PREDICTION_ALGORITHM " << endl;
	  return 1;
	}

	// VARIABLE_PREDICTED
	if(par.find("VARIABLE_PREDICTED") != par.end()) {
	  boost::to_upper(par.find("VARIABLE_PREDICTED")->second);
      VARIABLE_PREDICTED = par.find("VARIABLE_PREDICTED")->second;
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! VARIABLE_PREDICTED " << endl;
	  return 1;
	}

  // SCHEDULER_2D_WEIGHT_TEMPERATURE
	if(par.find("SCHEDULER_2D_WEIGHT_TEMPERATURE") != par.end()) {
      SCHEDULER_2D_WEIGHT_TEMPERATURE = stof(par.find("SCHEDULER_2D_WEIGHT_TEMPERATURE")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! SCHEDULER_2D_WEIGHT_TEMPERATURE " << endl;
	  return 1;
	}

  // SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION
	if(par.find("SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION") != par.end()) {
      SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION = stof(par.find("SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION " << endl;
	  return 1;
	}

  // SCHEDULER_2D_WEIGHT_LOAD_CPU
	if(par.find("SCHEDULER_2D_WEIGHT_LOAD_CPU") != par.end()) {
      SCHEDULER_2D_WEIGHT_LOAD_CPU = stof(par.find("SCHEDULER_2D_WEIGHT_LOAD_CPU")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! SCHEDULER_2D_WEIGHT_LOAD_CPU " << endl;
	  return 1;
	}

	// SIZE_WINDOWN_PREDICTION
	if(par.find("SIZE_WINDOWN_PREDICTION") != par.end()) {
      SIZE_WINDOWN_PREDICTION = stoi(par.find("SIZE_WINDOWN_PREDICTION")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! SIZE_WINDOWN_PREDICTION " << endl;
	  return 1;
	}

	// MONITORINGTIME
	if(par.find("MONITORINGTIME") != par.end()) {
      MONITORINGTIME = stoi(par.find("MONITORINGTIME")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! MONITORINGTIME " << endl;
	  return 1;
	}

	// DIM
	if(par.find("DIM") != par.end()) {
      DIM = stoi(par.find("DIM")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! DIM " << endl;
	  return 1;
	}

	// DELAY
	if(par.find("DELAY") != par.end()) {
      DELAY = stoi(par.find("DELAY")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! DELAY " << endl;
	  return 1;
	}
	
	// CENTER
	if(par.find("CENTER") != par.end()) {
      CENTER = stoi(par.find("CENTER")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! CENTER " << endl;
	  return 1;
	}

	// STEP
	if(par.find("STEP") != par.end()) {
      STEP = stoi(par.find("STEP")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! STEP " << endl;
	  return 1;
	}

	// COLUMN
	if(par.find("COLUMN") != par.end()) {
      COLUMN = stoi(par.find("COLUMN")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! COLUMN " << endl;
	  return 1;
	}

	// CLENGTH
	if(par.find("CLENGTH") != par.end()) {
      CLENGTH = stoi(par.find("CLENGTH")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! CLENGTH " << endl;
	  return 1;
	}

	// MAKECAST
	if(par.find("MAKECAST") != par.end()) {
		MAKECAST = par.find("MAKECAST")->second[0];
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! MAKECAST " << endl;
	  return 1;
	}


	// N
	if(par.find("N") != par.end()) {
      N = stoi(par.find("N")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! N " << endl;
	  return 1;
	}

	// CAST
	if(par.find("CAST") != par.end()) {
		CAST = par.find("CAST")->second[0];
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! CAST " << endl;
	  return 1;
	}

	// DYNAMIC_WORKLOAD_VMS
    if(par.find("DYNAMIC_WORKLOAD_VMS") != par.end()) {
      DYNAMIC_WORKLOAD_VMS = To_Bool(par.find("DYNAMIC_WORKLOAD_VMS")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! DYNAMIC_WORKLOAD_VMS " << endl;
	  return 1;
	}

	// WORKLOAD_DISTRIBUTION_MODEL
	if(par.find("WORKLOAD_DISTRIBUTION_MODEL") != par.end()) {
	  boost::to_upper(par.find("WORKLOAD_DISTRIBUTION_MODEL")->second);
      WORKLOAD_DISTRIBUTION_MODEL = par.find("WORKLOAD_DISTRIBUTION_MODEL")->second;
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! WORKLOAD_DISTRIBUTION_MODEL " << endl;
	  return 1;
	}

	// SIMULATES_MIGRATION_VMS
    if(par.find("SIMULATES_MIGRATION_VMS") != par.end()) {
      SIMULATES_MIGRATION_VMS = To_Bool(par.find("SIMULATES_MIGRATION_VMS")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! SIMULATES_MIGRATION_VMS " << endl;
	  return 1;
	}	

	// OPTIMIZATION_ALGORITHM_LOW_UTILIZATION
	if(par.find("OPTIMIZATION_ALGORITHM_LOW_UTILIZATION") != par.end()) {
	  boost::to_upper(par.find("OPTIMIZATION_ALGORITHM_LOW_UTILIZATION")->second);
      OPTIMIZATION_ALGORITHM_LOW_UTILIZATION = par.find("OPTIMIZATION_ALGORITHM_LOW_UTILIZATION")->second;
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! OPTIMIZATION_ALGORITHM_LOW_UTILIZATION " << endl;
	  return 1;
	}

	// PERIODIC_INTERVAL_EXECUTION_LOW
	if(par.find("PERIODIC_INTERVAL_EXECUTION_LOW") != par.end()) {
      PERIODIC_INTERVAL_EXECUTION_LOW = stoi(par.find("PERIODIC_INTERVAL_EXECUTION_LOW")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! PERIODIC_INTERVAL_EXECUTION_LOW " << endl;
	  return 1;
	}


	// OPTIMIZATION_ALGORITHM_OVERLOAD_UTILIZATION
	if(par.find("OPTIMIZATION_ALGORITHM_OVERLOAD_UTILIZATION") != par.end()) {
	  boost::to_upper(par.find("OPTIMIZATION_ALGORITHM_OVERLOAD_UTILIZATION")->second);
      OPTIMIZATION_ALGORITHM_OVERLOAD_UTILIZATION = par.find("OPTIMIZATION_ALGORITHM_OVERLOAD_UTILIZATION")->second;
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! OPTIMIZATION_ALGORITHM_OVERLOAD_UTILIZATION " << endl;
	  return 1;
	}

	// PERIODIC_INTERVAL_EXECUTION_OVERLOAD
	if(par.find("PERIODIC_INTERVAL_EXECUTION_OVERLOAD") != par.end()) {
      PERIODIC_INTERVAL_EXECUTION_OVERLOAD = stoi(par.find("PERIODIC_INTERVAL_EXECUTION_OVERLOAD")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! PERIODIC_INTERVAL_EXECUTION_OVERLOAD " << endl;
	  return 1;
	}


	// OPTIMIZATION_ALGORITHM_OVERHEATING_TEMPERATURE
	if(par.find("OPTIMIZATION_ALGORITHM_OVERHEATING_TEMPERATURE") != par.end()) {
	  boost::to_upper(par.find("OPTIMIZATION_ALGORITHM_OVERHEATING_TEMPERATURE")->second);
      OPTIMIZATION_ALGORITHM_OVERHEATING_TEMPERATURE = par.find("OPTIMIZATION_ALGORITHM_OVERHEATING_TEMPERATURE")->second;
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! OPTIMIZATION_ALGORITHM_OVERHEATING_TEMPERATURE " << endl;
	  return 1;
	}


	// PERIODIC_INTERVAL_EXECUTION_TEMPERATURE
	if(par.find("PERIODIC_INTERVAL_EXECUTION_TEMPERATURE") != par.end()) {
      PERIODIC_INTERVAL_EXECUTION_TEMPERATURE = stoi(par.find("PERIODIC_INTERVAL_EXECUTION_TEMPERATURE")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! PERIODIC_INTERVAL_EXECUTION_TEMPERATURE " << endl;
	  return 1;
	}

	//VMS_ALGORITHM_SELECTION
	if(par.find("VMS_ALGORITHM_SELECTION") != par.end()) {
	  boost::to_upper(par.find("VMS_ALGORITHM_SELECTION")->second);
      VMS_ALGORITHM_SELECTION = par.find("VMS_ALGORITHM_SELECTION")->second;
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! VMS_ALGORITHM_SELECTION " << endl;
	  return 1;
	}

	// OPTIMIZATION_ALGORITHM_LINK_OVERLOAD
	if(par.find("OPTIMIZATION_ALGORITHM_LINK_OVERLOAD") != par.end()) {
	  boost::to_upper(par.find("OPTIMIZATION_ALGORITHM_LINK_OVERLOAD")->second);
      OPTIMIZATION_ALGORITHM_LINK_OVERLOAD = par.find("OPTIMIZATION_ALGORITHM_LINK_OVERLOAD")->second;
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! OPTIMIZATION_ALGORITHM_LINK_OVERLOAD " << endl;
	  return 1;
	}

	// PERIODIC_INTERVAL_EXECUTION_LINK_OVERLOAD
	if(par.find("PERIODIC_INTERVAL_EXECUTION_LINK_OVERLOAD") != par.end()) {
      PERIODIC_INTERVAL_EXECUTION_LINK_OVERLOAD = stoi(par.find("PERIODIC_INTERVAL_EXECUTION_LINK_OVERLOAD")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! PERIODIC_INTERVAL_EXECUTION_LINK_OVERLOAD" << endl;
	  return 1;
	}

	// VMS_ALGORITHM_SELECTION_OVERLOADED_LINK
	if(par.find("VMS_ALGORITHM_SELECTION_OVERLOADED_LINK") != par.end()) {
	  boost::to_upper(par.find("VMS_ALGORITHM_SELECTION_OVERLOADED_LINK")->second);
      VMS_ALGORITHM_SELECTION_OVERLOADED_LINK = par.find("VMS_ALGORITHM_SELECTION_OVERLOADED_LINK")->second;
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! VMS_ALGORITHM_SELECTION_OVERLOADED_LINK " << endl;
	  return 1;
	}



	// THRESHOLD_TOP_OF_USE_CPU
	if(par.find("THRESHOLD_TOP_OF_USE_CPU") != par.end()) {
      THRESHOLD_TOP_OF_USE_CPU = stof(par.find("THRESHOLD_TOP_OF_USE_CPU")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! THRESHOLD_TOP_OF_USE_CPU " << endl;
	  return 1;
	}

	// THRESHOLD_BOTTOM_OF_USE_CPU
	if(par.find("THRESHOLD_BOTTOM_OF_USE_CPU") != par.end()) {
      THRESHOLD_BOTTOM_OF_USE_CPU = stof(par.find("THRESHOLD_BOTTOM_OF_USE_CPU")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! THRESHOLD_BOTTOM_OF_USE_CPU " << endl;
	  return 1;
	}

	// USING_PREDICTION_TO_OVERLOAD
	if(par.find("USING_PREDICTION_TO_OVERLOAD") != par.end()) {
      USING_PREDICTION_TO_OVERLOAD = To_Bool(par.find("USING_PREDICTION_TO_OVERLOAD")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! USING_PREDICTION_TO_OVERLOAD " << endl;
	  return 1;
	}

	//PREDICTION_ALGORITHM_OVERLOAD
	if(par.find("PREDICTION_ALGORITHM_OVERLOAD") != par.end()) {
	  boost::to_upper(par.find("PREDICTION_ALGORITHM_OVERLOAD")->second);
      PREDICTION_ALGORITHM_OVERLOAD = par.find("PREDICTION_ALGORITHM_OVERLOAD")->second;
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! PREDICTION_ALGORITHM_OVERLOAD " << endl;
	  return 1;
	}

	//SIZE_WINDOWN_PREDICTION_CPU 
	if(par.find("SIZE_WINDOWN_PREDICTION_CPU") != par.end()) {
      SIZE_WINDOWN_PREDICTION_CPU = stoi(par.find("SIZE_WINDOWN_PREDICTION_CPU")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! SIZE_WINDOWN_PREDICTION_CPU " << endl;
	  return 1;
	}


	//ALPHA_3DMOBFD 
	if(par.find("ALPHA_3DMOBFD") != par.end()) {
      ALPHA_3DMOBFD = stoi(par.find("ALPHA_3DMOBFD")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! ALPHA_3DMOBFD " << endl;
	  return 1;
	}

	//BETA_3DMOBFD 
	if(par.find("BETA_3DMOBFD") != par.end()) {
      BETA_3DMOBFD = stoi(par.find("BETA_3DMOBFD")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! BETA_3DMOBFD " << endl;
	  return 1;
	}

	//GAMMA_3DMOBFD 
	if(par.find("GAMMA_3DMOBFD") != par.end()) {
      GAMMA_3DMOBFD = stoi(par.find("GAMMA_3DMOBFD")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! GAMMA_3DMOBFD " << endl;
	  return 1;
	}

	//DELTA_3DMOBFD 
	if(par.find("DELTA_3DMOBFD") != par.end()) {
      DELTA_3DMOBFD = stoi(par.find("DELTA_3DMOBFD")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! DELTA_3DMOBFD " << endl;
	  return 1;
	}

	//EPSILON_3DMOBFD 
	if(par.find("EPSILON_3DMOBFD") != par.end()) {
      EPSILON_3DMOBFD = stoi(par.find("EPSILON_3DMOBFD")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! EPSILON_3DMOBFD " << endl;
	  return 1;
	}


	// WEIGHT_TEMPERATURE
	if(par.find("WEIGHT_TEMPERATURE") != par.end()) {
      WEIGHT_TEMPERATURE = stof(par.find("WEIGHT_TEMPERATURE")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! WEIGHT_TEMPERATURE " << endl;
	  return 1;
	}

	// WEIGHT_CPU
	if(par.find("WEIGHT_CPU") != par.end()) {
      WEIGHT_CPU = stof(par.find("WEIGHT_CPU")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! WEIGHT_CPU " << endl;
	  return 1;
	}

	// WEIGHT_POWER
	if(par.find("WEIGHT_POWER") != par.end()) {
      WEIGHT_POWER = stof(par.find("WEIGHT_POWER")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! WEIGHT_POWER " << endl;
	  return 1;
	}
	
	// WEIGHT_MEMORY
	if(par.find("WEIGHT_MEMORY") != par.end()) {
      WEIGHT_MEMORY = stof(par.find("WEIGHT_MEMORY")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! WEIGHT_MEMORY " << endl;
	  return 1;
	}

	// WEIGHT_TRAFFIC
	if(par.find("WEIGHT_TRAFFIC") != par.end()) {
      WEIGHT_TRAFFIC = stof(par.find("WEIGHT_TRAFFIC")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! WEIGHT_TRAFFIC " << endl;
	  return 1;
	}


	//E_TEMPERATURE 
	if(par.find("E_TEMPERATURE") != par.end()) {
      E_TEMPERATURE = stoi(par.find("E_TEMPERATURE")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! E_TEMPERATURE " << endl;
	  return 1;
	}

	//E_CPU 
	if(par.find("E_CPU") != par.end()) {
      E_CPU = stoi(par.find("E_CPU")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! E_CPU " << endl;
	  return 1;
	}

	//E_POWER 
	if(par.find("E_POWER") != par.end()) {
      E_POWER = stoi(par.find("E_POWER")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! E_POWER " << endl;
	  return 1;
	}

	//E_MEMORY 
	if(par.find("E_MEMORY") != par.end()) {
      E_MEMORY = stoi(par.find("E_MEMORY")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! E_MEMORY " << endl;
	  return 1;
	}

	//E_TRAFFIC 
	if(par.find("E_TRAFFIC") != par.end()) {
      E_TRAFFIC = stoi(par.find("E_TRAFFIC")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! E_TRAFFIC " << endl;
	  return 1;
	}

	// SIMULATES_NETWORK
	if(par.find("SIMULATES_NETWORK") != par.end()) {
      SIMULATES_NETWORK = To_Bool(par.find("SIMULATES_NETWORK")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! SIMULATES_NETWORK " << endl;
	  return 1;
	}

	//TYPE_TOPOLOGY 
	if(par.find("TYPE_TOPOLOGY") != par.end()) {
	  boost::to_upper(par.find("TYPE_TOPOLOGY")->second);
      TYPE_TOPOLOGY  = par.find("TYPE_TOPOLOGY")->second;
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! TYPE_TOPOLOGY  " << endl;
	  return 1;
	}

	//NAME_FILE_TOPOLOGY
	if(par.find("NAME_FILE_TOPOLOGY") != par.end()) {
	  //boost::to_upper(par.find("NAME_FILE_TOPOLOGY")->second);
      NAME_FILE_TOPOLOGY = par.find("NAME_FILE_TOPOLOGY")->second;
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! NAME_FILE_TOPOLOGY " << endl;
	  return 1;
	}

	// NUMBER_CHASSI_RACK
	if(par.find("NUMBER_CHASSI_RACK") != par.end()) {
      NUMBER_CHASSI_RACK = stoi(par.find("NUMBER_CHASSI_RACK")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! NUMBER_CHASSI_RACK " << endl;
	  return 1;
	}

	// NUMBER_OF_PORTS_SWITCH_ACCESS
	if(par.find("NUMBER_OF_PORTS_SWITCH_ACCESS") != par.end()) {
      NUMBER_OF_PORTS_SWITCH_ACCESS = stoi(par.find("NUMBER_OF_PORTS_SWITCH_ACCESS")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! NUMBER_OF_PORTS_SWITCH_ACCESS " << endl;
	  return 1;
	}

	// NUMBER_OF_PORTS_SWITCH_AGGREGATION
	if(par.find("NUMBER_OF_PORTS_SWITCH_AGGREGATION") != par.end()) {
      NUMBER_OF_PORTS_SWITCH_AGGREGATION = stoi(par.find("NUMBER_OF_PORTS_SWITCH_AGGREGATION")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! NUMBER_OF_PORTS_SWITCH_AGGREGATION " << endl;
	  return 1;
	}

	// NUMBER_OF_PORTS_SWITCH_CORE
	if(par.find("NUMBER_OF_PORTS_SWITCH_CORE") != par.end()) {
      NUMBER_OF_PORTS_SWITCH_CORE = stoi(par.find("NUMBER_OF_PORTS_SWITCH_CORE")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! NUMBER_OF_PORTS_SWITCH_CORE " << endl;
	  return 1;
	}

	// EACCE_CHASSIS
	if(par.find("EACCE_CHASSIS") != par.end()) {
      EACCE_CHASSIS = stof(par.find("EACCE_CHASSIS")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! EACCE_CHASSIS " << endl;
	  return 1;
	}

	// EACCE_LINECARD
	if(par.find("EACCE_LINECARD") != par.end()) {
      EACCE_LINECARD = stof(par.find("EACCE_LINECARD")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! EACCE_LINECARD " << endl;
	  return 1;
	}

	// EACCE_PORT10
	if(par.find("EACCE_PORT10") != par.end()) {
      EACCE_PORT10 = stof(par.find("EACCE_PORT10")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! EACCE_PORT10 " << endl;
	  return 1;
	}

	// EACCE_PORT100
	if(par.find("EACCE_PORT100") != par.end()) {
      EACCE_PORT100 = stof(par.find("EACCE_PORT100")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! EACCE_PORT100 " << endl;
	  return 1;
	}

	// EACCE_PORT1000
	if(par.find("EACCE_PORT1000") != par.end()) {
      EACCE_PORT1000 = stof(par.find("EACCE_PORT1000")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! EACCE_PORT1000 " << endl;
	  return 1;
	}

	// EACCE_PORT10000
	if(par.find("EACCE_PORT10000") != par.end()) {
      EACCE_PORT10000 = stof(par.find("EACCE_PORT10000")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! EACCE_PORT10000 " << endl;
	  return 1;
	}

	// EAGGR_CHASSIS
	if(par.find("EAGGR_CHASSIS") != par.end()) {
      EAGGR_CHASSIS = stof(par.find("EAGGR_CHASSIS")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! EAGGR_CHASSIS " << endl;
	  return 1;
	}

	// EAGGR_LINECARD
	if(par.find("EAGGR_LINECARD") != par.end()) {
      EAGGR_LINECARD = stof(par.find("EAGGR_LINECARD")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! EAGGR_LINECARD " << endl;
	  return 1;
	}

	// EAGGR_PORT10
	if(par.find("EAGGR_PORT10") != par.end()) {
      EAGGR_PORT10 = stof(par.find("EAGGR_PORT10")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! EAGGR_PORT10 " << endl;
	  return 1;
	}

	// EAGGR_PORT100
	if(par.find("EAGGR_PORT100") != par.end()) {
      EAGGR_PORT100 = stof(par.find("EAGGR_PORT100")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! EAGGR_PORT100 " << endl;
	  return 1;
	}

	// EAGGR_PORT1000
	if(par.find("EAGGR_PORT1000") != par.end()) {
      EAGGR_PORT1000 = stof(par.find("EAGGR_PORT1000")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! EAGGR_PORT1000 " << endl;
	  return 1;
	}

	// EAGGR_PORT10000
	if(par.find("EAGGR_PORT10000") != par.end()) {
      EAGGR_PORT10000 = stof(par.find("EAGGR_PORT10000")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! EAGGR_PORT10000 " << endl;
	  return 1;
	}

	// ECORE_CHASSIS
	if(par.find("ECORE_CHASSIS") != par.end()) {
      ECORE_CHASSIS = stof(par.find("ECORE_CHASSIS")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! ECORE_CHASSIS " << endl;
	  return 1;
	}

	// ECORE_LINECARD
	if(par.find("ECORE_LINECARD") != par.end()) {
      ECORE_LINECARD = stof(par.find("ECORE_LINECARD")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! ECORE_LINECARD " << endl;
	  return 1;
	}

	// ECORE_PORT10
	if(par.find("ECORE_PORT10") != par.end()) {
      ECORE_PORT10 = stof(par.find("ECORE_PORT10")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! ECORE_PORT10 " << endl;
	  return 1;
	}

	// ECORE_PORT100
	if(par.find("ECORE_PORT100") != par.end()) {
      ECORE_PORT100 = stof(par.find("ECORE_PORT100")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! ECORE_PORT100 " << endl;
	  return 1;
	}

	// ECORE_PORT1000
	if(par.find("ECORE_PORT1000") != par.end()) {
      ECORE_PORT1000 = stof(par.find("ECORE_PORT1000")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! ECORE_PORT1000 " << endl;
	  return 1;
	}

	// ECORE_PORT10000
	if(par.find("ECORE_PORT10000") != par.end()) {
      ECORE_PORT10000 = stof(par.find("ECORE_PORT10000")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! ECORE_PORT10000 " << endl;
	  return 1;
	}

	// SIMULATE_TRAFFIC_MATRIX
	if(par.find("SIMULATE_TRAFFIC_MATRIX") != par.end()) {
      SIMULATE_TRAFFIC_MATRIX = To_Bool(par.find("SIMULATE_TRAFFIC_MATRIX")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! SIMULATE_TRAFFIC_MATRIX " << endl;
	  return 1;
	}


	//NAME_FILE_MATRIX_TRAFFIC
	if(par.find("NAME_FILE_MATRIX_TRAFFIC") != par.end()) {
	  //boost::to_upper(par.find("NAME_FILE_MATRIX_TRAFFIC")->second);
      NAME_FILE_MATRIX_TRAFFIC = par.find("NAME_FILE_MATRIX_TRAFFIC")->second;
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! NAME_FILE_MATRIX_TRAFFIC " << endl;
	  return 1;
	}

	//CALCULATE_TRAFFIC_CORRELATION
	if(par.find("CALCULATE_TRAFFIC_CORRELATION") != par.end()) {
      CALCULATE_TRAFFIC_CORRELATION = To_Bool(par.find("CALCULATE_TRAFFIC_CORRELATION")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! CALCULATE_TRAFFIC_CORRELATION " << endl;
	  return 1;
	}

	//EXECUTE_LOAD_BALANCING
	if(par.find("EXECUTE_LOAD_BALANCING") != par.end()) {
      EXECUTE_LOAD_BALANCING = To_Bool(par.find("EXECUTE_LOAD_BALANCING")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! EXECUTE_LOAD_BALANCING " << endl;
	  return 1;
	}


	// NUMBER_NODES_TRAFFIC_MATRIX
	if(par.find("NUMBER_NODES_TRAFFIC_MATRIX") != par.end()) {
      NUMBER_NODES_TRAFFIC_MATRIX = stoi(par.find("NUMBER_NODES_TRAFFIC_MATRIX")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! NUMBER_NODES_TRAFFIC_MATRIX " << endl;
	  return 1;
	}

	// NUMBER_OF_SAMPLES_TRAFFIC_MATRIX
	if(par.find("NUMBER_OF_SAMPLES_TRAFFIC_MATRIX") != par.end()) {
      NUMBER_OF_SAMPLES_TRAFFIC_MATRIX = stoi(par.find("NUMBER_OF_SAMPLES_TRAFFIC_MATRIX")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! NUMBER_OF_SAMPLES_TRAFFIC_MATRIX " << endl;
	  return 1;
	}

	// NUMBER_OF_PERIODS
	if(par.find("NUMBER_OF_PERIODS") != par.end()) {
      NUMBER_OF_PERIODS = stoi(par.find("NUMBER_OF_PERIODS")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! NUMBER_OF_PERIODS " << endl;
	  return 1;
	}

	// K
	if(par.find("K") != par.end()) {
      K = stof(par.find("K")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! K " << endl;
	  return 1;
	}
	
	// SEED_PERIOD_ON
	if(par.find("SEED_PERIOD_ON") != par.end()) {
      SEED_PERIOD_ON = stoi(par.find("SEED_PERIOD_ON")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! SEED_PERIOD_ON " << endl;
	  return 1;
	}

	// SEED_PERIOD_OFF
	if(par.find("SEED_PERIOD_OFF") != par.end()) {
      SEED_PERIOD_OFF = stoi(par.find("SEED_PERIOD_OFF")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! SEED_PERIOD_OFF " << endl;
	  return 1;
	}

	// PROPORTION_OF_INTERNAL_COMMUNICATION_BETWEEN_VMS
	if(par.find("PROPORTION_OF_INTERNAL_COMMUNICATION_BETWEEN_VMS") != par.end()) {
      PROPORTION_OF_INTERNAL_COMMUNICATION_BETWEEN_VMS = stoi(par.find("PROPORTION_OF_INTERNAL_COMMUNICATION_BETWEEN_VMS")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! PROPORTION_OF_INTERNAL_COMMUNICATION_BETWEEN_VMS " << endl;
	  return 1;
	}

	// PROPORTION_OF_EXTERNAL_COMMUNICATION
	if(par.find("PROPORTION_OF_EXTERNAL_COMMUNICATION") != par.end()) {
      PROPORTION_OF_EXTERNAL_COMMUNICATION = stoi(par.find("PROPORTION_OF_EXTERNAL_COMMUNICATION")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! PROPORTION_OF_EXTERNAL_COMMUNICATION " << endl;
	  return 1;
	}

	// DYNAMIC_SPEED_ADAPTATION
	if(par.find("DYNAMIC_SPEED_ADAPTATION") != par.end()) {
      DYNAMIC_SPEED_ADAPTATION = To_Bool(par.find("DYNAMIC_SPEED_ADAPTATION")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! DYNAMIC_SPEED_ADAPTATION " << endl;
	  return 1;
	}

	// RUNTIME_LINK_ADAPTATION
	if(par.find("RUNTIME_LINK_ADAPTATION") != par.end()) {
      RUNTIME_LINK_ADAPTATION = stoi(par.find("RUNTIME_LINK_ADAPTATION")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! RUNTIME_LINK_ADAPTATION " << endl;
	  return 1;
	}

	// TRAFFIC_LIMIT_FOR_ADAPTATION
	if(par.find("TRAFFIC_LIMIT_FOR_ADAPTATION") != par.end()) {
      TRAFFIC_LIMIT_FOR_ADAPTATION = stoi(par.find("TRAFFIC_LIMIT_FOR_ADAPTATION")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! TRAFFIC_LIMIT_FOR_ADAPTATION " << endl;
	  return 1;
	}

	// SIMULATE_TOPOLOGY_OPTIMIZATION 
	if(par.find("SIMULATE_TOPOLOGY_OPTIMIZATION") != par.end()) {
      SIMULATE_TOPOLOGY_OPTIMIZATION = To_Bool(par.find("SIMULATE_TOPOLOGY_OPTIMIZATION")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! SIMULATE_TOPOLOGY_OPTIMIZATION " << endl;
	  return 1;
	}

	
	// OPTIMIZATION_ALGORITHM_TOPOLOGY
	if(par.find("OPTIMIZATION_ALGORITHM_TOPOLOGY") != par.end()) {
	  boost::to_upper(par.find("OPTIMIZATION_ALGORITHM_TOPOLOGY")->second);
      OPTIMIZATION_ALGORITHM_TOPOLOGY = par.find("OPTIMIZATION_ALGORITHM_TOPOLOGY")->second;
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! OPTIMIZATION_ALGORITHM_TOPOLOGY " << endl;
	  return 1;
	}

	// PERIODIC_INTERVAL_EXECUTION_TOPOLOGY
	if(par.find("PERIODIC_INTERVAL_EXECUTION_TOPOLOGY") != par.end()) {
      PERIODIC_INTERVAL_EXECUTION_TOPOLOGY = stoi(par.find("PERIODIC_INTERVAL_EXECUTION_TOPOLOGY")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! PERIODIC_INTERVAL_EXECUTION_TOPOLOGY " << endl;
	  return 1;
	}
	
	// SIMULATES_POOL_SERVER 
	if(par.find("SIMULATES_POOL_SERVER") != par.end()) {
      SIMULATES_POOL_SERVER  = To_Bool(par.find("SIMULATES_POOL_SERVER")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! SIMULATES_POOL_SERVER  " << endl;
	  return 1;
	}


	// SIZE_POOL
	if(par.find("SIZE_POOL") != par.end()) {
      SIZE_POOL = stoi(par.find("SIZE_POOL")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! SIZE_POOL " << endl;
	  return 1;
	}

	
	// INCREASE_SIZE_POOL_DYNAMICALLY 
	if(par.find("INCREASE_SIZE_POOL_DYNAMICALLY") != par.end()) {
      INCREASE_SIZE_POOL_DYNAMICALLY  = To_Bool(par.find("INCREASE_SIZE_POOL_DYNAMICALLY")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! INCREASE_SIZE_POOL_DYNAMICALLY  " << endl;
	  return 1;
	}


	// COOL_AIR_TRAVEL_TIME_MULTIPLIER
	if(par.find("COOL_AIR_TRAVEL_TIME_MULTIPLIER") != par.end()) {
      COOL_AIR_TRAVEL_TIME_MULTIPLIER = stoi(par.find("COOL_AIR_TRAVEL_TIME_MULTIPLIER")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! COOL_AIR_TRAVEL_TIME_MULTIPLIER " << endl;
	  return 1;
	}


	// TEMPERATURE_SENSING_PERFORMANCE_CAPPING
	if(par.find("TEMPERATURE_SENSING_PERFORMANCE_CAPPING") != par.end()) {
      TEMPERATURE_SENSING_PERFORMANCE_CAPPING  = To_Bool(par.find("TEMPERATURE_SENSING_PERFORMANCE_CAPPING")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! TEMPERATURE_SENSING_PERFORMANCE_CAPPING  " << endl;
	  return 1;
	}


	// TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS
	if(par.find("TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS") != par.end()) {
      TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS = stoi(par.find("TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS " << endl;
	  return 1;
	}

	// REASSIGN_VMS
	if(par.find("REASSIGN_VMS") != par.end()) {
      REASSIGN_VMS  = To_Bool(par.find("REASSIGN_VMS")->second);
	}
	else {
	  cout << "SimDC3D-ERROR: Parameter not found !!! REASSIGN_VMS  " << endl;
	  return 1;
	}

 return 0;
}


int ReadParameters()
{
  string lineparam, nameparameter, valueparameter;
  std::map <string, string> parameters;


  ifstream config("config.cfg");

  if(!config.is_open()) {
	 cout << "Cannot open a file config.cfg:" << endl;
  	 return 1;
  }

  getline(config,lineparam);

  while (!config.eof()) { 
	    if (lineparam.size() == 0) {
		   getline(config,lineparam);
		   continue;
		}
		if (lineparam[0] == '/')  {
		   getline(config,lineparam);
		   continue;
		}

		nameparameter = lineparam.substr(0,lineparam.find("="));
		boost::trim(nameparameter);
		valueparameter = lineparam.substr(lineparam.find("=")+1, (lineparam.find(";")-lineparam.find("="))-1); 
		boost::trim(valueparameter);
		parameters.insert(pair<string, string> (nameparameter, valueparameter) );

    	getline(config,lineparam);
 }  

  if (lineparam.size() != 0) {
    nameparameter = lineparam.substr(0,lineparam.find("="));
    boost::trim(nameparameter);
	valueparameter = lineparam.substr(lineparam.find("=")+1, (lineparam.find(";")-lineparam.find("="))-1); 
	boost::trim(valueparameter);
 	parameters.insert(pair<string, string> (nameparameter, valueparameter) );
 }

 config.close();

 if (SetParameters(parameters) != 0) {
	 return 1;
 }
 else {
	 return 0;
 }

}

void PrintUsage()
{
	cout << "Usage: SimDC3D.exe <SWF file name>" << endl;
	cout << "\t[-NUMBER_OF_CHASSIS #(1-50)] (default: "<< NUMBER_OF_CHASSIS <<")" << endl;
	cout << "\t[-NUMBER_OF_SERVERS_IN_ONE_CHASSIS #(1-10)] (default: "<<NUMBER_OF_SERVERS_IN_ONE_CHASSIS<<")" << endl;
	cout << "\t[-NUMBER_OF_CORES_IN_ONE_SERVER #(1-)] (default: "<<NUMBER_OF_CORES_IN_ONE_SERVER<<")" << endl;
	cout << "\t[-TOTAL_OF_MEMORY_IN_ONE_SERVER #(1-)] (default: "<<TOTAL_OF_MEMORY_IN_ONE_SERVER<<")" << endl;
	cout << "\t[-PERIODIC_LOG_INTERVAL #(500- in seconds)] (default: "<< PERIODIC_LOG_INTERVAL <<")" << endl;
	cout << "\t[-SKIP_x_SECONDS #(any, in seconds)] (default: "<<SKIP_x_SECONDS<<")" << endl;
	cout << "\t[-FINISHES_AT_DAY #(1-)] (default: "<<FINISHES_AT_DAY<<", until all jobs are done)" << endl;

	cout << "\t[-SCHEDULING_ALGORITHM best_performance | uniform_task | low_temp_first | min_hr | random | center_rack_first ] (default: "<< SCHEDULING_ALGORITHM <<")" << endl;
	cout << "\t[-SUPPLY_TEMPERATURE_OFFSET_ALPHA #(any integer)] (default: " << SUPPLY_TEMPERATURE_OFFSET_ALPHA << ")" << endl;
	cout << "\t[-CRAC_SUPPLY_TEMP_IS_CONSTANT_AT #(1-)] (default: " << CRAC_SUPPLY_TEMP_IS_CONSTANT_AT << ")" << endl;
	cout << "\t[-INSTANT_COOL_AIR_TRAVEL_TIME] (default: "<<INSTANT_COOL_AIR_TRAVEL_TIME<<")" << endl;
	cout << "\t[-CONSTANT_FAN_POWER] (default: "<<CONSTANT_FAN_POWER<<")" << endl;
	cout << "\t[-INSTANT_CHANGE_CRAC_SUPPLY_AIR] (default: "<<INSTANT_CHANGE_CRAC_SUPPLY_AIR<<")" << endl;
	cout << "\t[-CRAC_DISCHARGE_CHANGE_RATE_0_00x #(5,10,20)] (default: " << CRAC_DISCHARGE_CHANGE_RATE_0_00x << ")" << endl;
	cout << "\t[-FAN_RPM_NO_LIMIT] (default: " << FAN_RPM_NO_LIMIT << ")" << endl;
	
	cout << "\t[-SCHEDULING_WITH_PREDICTION] (default: " << SCHEDULING_WITH_PREDICTION << ")" << endl;
	cout << "\t[-PREDICTION_ALGORITHM] (default: " << PREDICTION_ALGORITHM << ")" << endl;

	cout << "\t[-POWER_ON] (default: " << POWER_ON << ")" << endl;

	cout << "\t[-SIZE_WINDOWN_PREDICTION] (default: " << SIZE_WINDOWN_PREDICTION << ")" << endl;
	cout << "\t[-MONITORINGTIME] (default: " << MONITORINGTIME << ")" << endl;


	cout << "\t[-DIM] (default: " << DIM  << ")" << endl;
	cout << "\t[-DELAY] (default: " << DELAY << ")" << endl;
	cout << "\t[-STEP] (default: " << STEP << ")" << endl;
	cout << "\t[-CENTER] (default: " << CENTER << ")" << endl;
	cout << "\t[-STEP] (default: " << STEP << ")" << endl;
	cout << "\t[-COLUMN] (default: " << STEP << ")" << endl;
	cout << "\t[-LENGTH] (default: " << LENGTH << ")" << endl;
	cout << "\t[-CLENGTH] (default: " << CLENGTH << ")" << endl;
	cout << "\t[-MAKECAST] (default: " << MAKECAST << ")" << endl;

	cout << "\t[-N] (default: " << N << ")" << endl;
	cout << "\t[-CAST] (default: " << CAST << ")" << endl;


#ifdef _DEBUG
	cout << "\t[-COOL_AIR_TRAVEL_TIME_MULTIPLIER #1-] (default: "<< COOL_AIR_TRAVEL_TIME_MULTIPLIER <<")" << endl;
	cout << "\t[-TEMPERATURE_SENSING_PERFORMANCE_CAPPING] (default: "<<TEMPERATURE_SENSING_PERFORMANCE_CAPPING<<")" << endl;
	cout << "\t[-TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS #(0-9)] (default: "<<TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS<<")" << endl;
	cout << "\t[-REASSIGN_VMS] (default: "<< REASSIGN_VMS << ", reassign all vms every 4.5 hours)" << endl;
#endif

}

bool ParsingArguments(int argc, char* argv[])
{

	if (argc < 2) {
		PrintUsage();
		return false;
	}
	int optind=2;
	while ((optind < argc) && (argv[optind][0]=='-')) {
		string sw = argv[optind];
		if (sw=="-TEMPERATURE_SENSING_PERFORMANCE_CAPPING") {
			TEMPERATURE_SENSING_PERFORMANCE_CAPPING = true;
		}
		else if (sw=="-TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS") {
			optind++;
			TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS = atoi(argv[optind]);
			if (TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS > 20) {
				cout << "Error: -TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS has to be less than or equal to 9" << endl;
				return false;
			} else if (TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS < 0) {
				cout << "Error: -TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS has to be larger than or equal to 0" << endl;
				return false;
			}
			if (TEMPERATURE_SENSING_PERFORMANCE_CAPPING==false) {
				cout << "Error: You have to enable -TEMPERATURE_SENSING_PERFORMANCE_CAPPING before setting -TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS" << endl;
				return false;
			}
		}
		else if (sw=="-NUMBER_OF_SERVERS_IN_ONE_CHASSIS") {
			optind++;
			NUMBER_OF_SERVERS_IN_ONE_CHASSIS = atoi(argv[optind]);
			if (NUMBER_OF_SERVERS_IN_ONE_CHASSIS > NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX) {
				cout << "Error: -NUMBER_OF_SERVERS_IN_ONE_CHASSIS too big" << endl;
				return false;
			} else if (NUMBER_OF_SERVERS_IN_ONE_CHASSIS < 1) {
				cout << "Error: -NUMBER_OF_SERVERS_IN_ONE_CHASSIS too small" << endl;
				return false;
			}
		}
		else if (sw=="-NUMBER_OF_CORES_IN_ONE_SERVER") {
			optind++;
			NUMBER_OF_CORES_IN_ONE_SERVER = atoi(argv[optind]);
			if (NUMBER_OF_CORES_IN_ONE_SERVER < 1) {
				cout << "Error: -NUMBER_OF_CORES_IN_ONE_SERVER too small" << endl;
				return false;
			}
		}
		else if (sw=="-SCHEDULING_ALGORITHM") {
			optind++;
			SCHEDULING_ALGORITHM = argv[optind];
		}
		else if (sw=="-PERIODIC_LOG_INTERVAL") {
			optind++;
			PERIODIC_LOG_INTERVAL = atoi(argv[optind]);
			if (PERIODIC_LOG_INTERVAL < 3600)
				cout << "Warning: -PERIODIC_LOG_INTERVAL too small" << endl;
		}
		else if (sw=="-SKIP_x_SECONDS") {
			optind++;
			SKIP_x_SECONDS = atoi(argv[optind]);
			if (SKIP_x_SECONDS < 0)
				cout << "Warning: -SKIP_x_SECONDS too small" << endl;
		}
		else if (sw=="-NUMBER_OF_CHASSIS") {
			optind++;
			NUMBER_OF_CHASSIS = atoi(argv[optind]);
			if (NUMBER_OF_CHASSIS < 1 || NUMBER_OF_CHASSIS > 50) {
				cout << "Error: -NUMBER_OF_CHASSIS too small or too big. Use 1-50: " << NUMBER_OF_CHASSIS << endl;
				return false;
			}
		}
		else if (sw=="-COOL_AIR_TRAVEL_TIME_MULTIPLIER") {
			optind++;
			COOL_AIR_TRAVEL_TIME_MULTIPLIER = atoi(argv[optind]);
			if (COOL_AIR_TRAVEL_TIME_MULTIPLIER < 0 || COOL_AIR_TRAVEL_TIME_MULTIPLIER > 10) {
				cout << "Error: -COOL_AIR_TRAVEL_TIME_MULTIPLIER too small or too big. " << COOL_AIR_TRAVEL_TIME_MULTIPLIER << endl;
				return false;
			}
		}
		else if (sw=="-SUPPLY_TEMPERATURE_OFFSET_ALPHA") {
			optind++;
			SUPPLY_TEMPERATURE_OFFSET_ALPHA = atoi(argv[optind]);
			if (SUPPLY_TEMPERATURE_OFFSET_ALPHA < -10 || SUPPLY_TEMPERATURE_OFFSET_ALPHA > 20)
				cout << "Warning: -SUPPLY_TEMPERATURE_OFFSET_ALPHA too small or too big: " << SUPPLY_TEMPERATURE_OFFSET_ALPHA << endl;
		}
		else if (sw=="-CRAC_SUPPLY_TEMP_IS_CONSTANT_AT") {
			optind++;
			CRAC_SUPPLY_TEMP_IS_CONSTANT_AT = atoi(argv[optind]);
			if (CRAC_SUPPLY_TEMP_IS_CONSTANT_AT < LOWEST_SUPPLY_TEMPERATURE 
				|| CRAC_SUPPLY_TEMP_IS_CONSTANT_AT > EMERGENCY_TEMPERATURE) {
				cout << "Error: -CRAC_SUPPLY_TEMP_IS_CONSTANT_AT too small or too big. " << CRAC_SUPPLY_TEMP_IS_CONSTANT_AT << endl;
				return false;
			}
		}
		else if (sw=="-CRAC_DISCHARGE_CHANGE_RATE_0_00x") {
			optind++;
			CRAC_DISCHARGE_CHANGE_RATE_0_00x = atoi(argv[optind]);
			if (CRAC_DISCHARGE_CHANGE_RATE_0_00x < 1 
				|| CRAC_DISCHARGE_CHANGE_RATE_0_00x > 50) {
				cout << "Error: -CRAC_DISCHARGE_CHANGE_RATE_0_00x too small or too big. " << CRAC_DISCHARGE_CHANGE_RATE_0_00x << endl;
				return false;
			}
		}
		else if (sw=="-FINISHES_AT_DAY") {
			optind++;
			FINISHES_AT_DAY = atoi(argv[optind]);
			if (FINISHES_AT_DAY < -1) {
				cout << "Error: -FINISHES_AT_DAY too small :" << FINISHES_AT_DAY << endl;
				return false;
			}
		}
		else if (sw=="-REASSIGN_VMS") {
			REASSIGN_VMS = true;
		}
		else if (sw=="-FAN_RPM_NO_LIMIT") {
			FAN_RPM_NO_LIMIT = true;
		}
		else if (sw=="-INSTANT_COOL_AIR_TRAVEL_TIME") {
			INSTANT_COOL_AIR_TRAVEL_TIME = true;
		}
		else if (sw=="-CONSTANT_FAN_POWER") {
			CONSTANT_FAN_POWER = true;
		}
		else if (sw=="-INSTANT_CHANGE_CRAC_SUPPLY_AIR") {
			INSTANT_CHANGE_CRAC_SUPPLY_AIR = true;
		}
		else {
			cout << "Error: Unknown switch: " << argv[optind] << endl;
			PrintUsage();
			return false;
		}
		optind++;
	}

	cout << endl;
	cout << "#Settings General " << endl;
		cout << " -PERIODIC_LOG_INTERVAL : " << PERIODIC_LOG_INTERVAL << endl;
		cout << " -SKIP_x_SECONDS : " << SKIP_x_SECONDS << endl;
		cout << " -FINISHES_AT_DAY : " << FINISHES_AT_DAY << endl;
		cout << " -PRINTSCREEN : " << PRINTSCREEN << endl;
		cout << " -RUN_MULTITHREAD : " << RUN_MULTITHREAD << endl;
		cout << " -NUMBER_OF_THREADS : " << NUMBER_OF_THREADS << endl;
		cout << " -SEED : " << SEED << endl;
		cout << " -SUBMIT_TIME_SEC : " << SUBMIT_TIME_SEC << endl; 

	cout << endl;
	cout << "#Settings Servers " << endl;
		cout << " -NUMBER_de_CHASSIS : " << NUMBER_OF_CHASSIS << endl;
		cout << " -NUMBER_OF_SERVERS_IN_ONE_CHASSIS : " << NUMBER_OF_SERVERS_IN_ONE_CHASSIS << endl;
		cout << " -NUMBER_OF_CORES_IN_ONE_SERVER : " << NUMBER_OF_CORES_IN_ONE_SERVER << endl;		
		cout << " -TOTAL_OF_MEMORY_IN_ONE_SERVER : " << TOTAL_OF_MEMORY_IN_ONE_SERVER << endl;
		cout << " -BANDWIDTH_IN_ONE_SERVER : " << BANDWIDTH_IN_ONE_SERVER << endl << endl;
		cout << " -CONSTANT_FAN_POWER : " << CONSTANT_FAN_POWER << endl;
		cout << " -FAN_RPM_NO_LIMIT : " << FAN_RPM_NO_LIMIT << endl << endl;
		cout << " -POWER_ON : " << POWER_ON << endl;
		cout << " -POWER_OFF : " << POWER_OFF << endl;
		cout << " -HIBERNATING_SERVER : " << HIBERNATING_SERVER <<endl;
		cout << " -POWER_CONSUMPTION_HIBERNATING : " << POWER_CONSUMPTION_HIBERNATING <<endl;
		cout << " -HIBERNATING_POWER_ON : " << HIBERNATING_POWER_ON << endl;
		cout << " -HIBERNATING_POWER_OFF : " << HIBERNATING_POWER_OFF << endl;
		cout << " -MODEL_OF_ENERGY_CONSUMPTION : " << MODEL_OF_ENERGY_CONSUMPTION << endl;

	cout << endl;
	cout << "#Settings Virtual Machine " << endl;
		cout << " -MEMORY_VIRTUAL_MACHINE : ";
		for (unsigned int i=0; i < MEMORY_VIRTUAL_MACHINE.size(); i++){
			cout << MEMORY_VIRTUAL_MACHINE[i] << " ";
		}
		cout << endl;
		cout << " -IMAGE_SIZE_VIRTUAL_MACHINE : " << IMAGE_SIZE_VIRTUAL_MACHINE << endl;
		cout << " -V_TRESHOLD : " << V_TRESHOLD << endl;
		cout << " -MEMORY_DIRTYING : " << MEMORY_DIRTYING  << endl;
		cout << " -T_RESUME : " << T_RESUME << endl;
	cout << endl;

	cout << "#Settings CRAC " << endl;
		cout << " -CRAC_SUPPLY_TEMP_IS_CONSTANT_AT : " << CRAC_SUPPLY_TEMP_IS_CONSTANT_AT << endl;
		cout << " -SUPPLY_TEMPERATURE_OFFSET_ALPHA : " << SUPPLY_TEMPERATURE_OFFSET_ALPHA << endl;
		cout << " -INSTANT_COOL_AIR_TRAVEL_TIME : " << INSTANT_COOL_AIR_TRAVEL_TIME << endl;
		cout << " -INSTANT_CHANGE_CRAC_SUPPLY_AIR : " << INSTANT_CHANGE_CRAC_SUPPLY_AIR << endl;
		cout << " -CRAC_DISCHARGE_CHANGE_RATE_0_00x : " << CRAC_DISCHARGE_CHANGE_RATE_0_00x << " (rate=" << (0.001*CRAC_DISCHARGE_CHANGE_RATE_0_00x) << ")" << endl << endl;
		cout << " -POLICY_OF_TEMPERATURE_CONTROL : " << POLICY_OF_TEMPERATURE_CONTROL << endl;
		cout << " -EMERGENCY_TEMPERATURE : " << EMERGENCY_TEMPERATURE << endl;
	cout << endl;

	cout << "#Settings Scheduling Algorithm " << endl;
		cout << " -SCHEDULING_ALGORITHM : " << SCHEDULING_ALGORITHM  << endl;
		cout << " -SCHEDULING_WITH_PREDICTION : " << SCHEDULING_WITH_PREDICTION << endl;
		cout << " -OPTIMIZATION_WITH_PREDICTION : " << OPTIMIZATION_WITH_PREDICTION << endl;
		cout << " -PREDICTION_ALGORITHM : " << PREDICTION_ALGORITHM << endl;
		cout << " -VARIABLE_PREDICTED : " << VARIABLE_PREDICTED << endl;
		cout << " -SCHEDULER_2D_WEIGHT_TEMPERATURE : " << SCHEDULER_2D_WEIGHT_TEMPERATURE << endl;
		cout << " -SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION  : " << SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION << endl;
		cout << " -SCHEDULER_2D_WEIGHT_LOAD_CPU : " << SCHEDULER_2D_WEIGHT_LOAD_CPU << endl;



	cout << endl;
	cout << "#Parameters Prediction Algorithm RBF and POLYNOM " << endl;
	
	cout << " -SIZE_WINDOWN_PREDICTION : " << SIZE_WINDOWN_PREDICTION << endl;					
	cout << " -MONITORINGTIME : " << MONITORINGTIME << endl;
	
	cout << endl;
	cout << "#Parameters Prediction Algorithm RBF  " << endl;
		cout << " -DIM : " << DIM  << endl;
		cout << " -DELAY : " << DELAY << endl;
		cout << " -CENTER : " << CENTER << endl;
		cout << " -STEP : " << STEP << endl;
		cout << " -COLUMN : " << STEP << endl;
		cout << " -CLENGTH : " << CLENGTH << endl;
		cout << " -MAKECAST : " << MAKECAST << endl;

	cout << endl;
	cout << "#Parameters Prediction Algorithm POLYNOM " << endl;
		cout << " -N : " << N << endl;
		cout << " -CAST : " << CAST << endl;


	cout << endl;
	cout << "#Setting Workload " << endl;
		cout << " -DYNAMIC_WORKLOAD_VMS : " << DYNAMIC_WORKLOAD_VMS << endl;
		cout << " -WORKLOAD_DISTRIBUTION_MODEL : " << WORKLOAD_DISTRIBUTION_MODEL << endl;

	cout << endl;
	cout << "#Setting Migration Parameters " << endl;

		cout << " -SIMULATES_MIGRATION_VMS : " << SIMULATES_MIGRATION_VMS << endl << endl;
		
		cout << " -OPTIMIZATION_ALGORITHM_LOW_UTILIZATION : " << OPTIMIZATION_ALGORITHM_LOW_UTILIZATION << endl;
		cout << " -PERIODIC_INTERVAL_EXECUTION_LOW : " << PERIODIC_INTERVAL_EXECUTION_LOW << endl << endl;
		
		cout << " -OPTIMIZATION_ALGORITHM_OVERLOAD_UTILIZATION : " << OPTIMIZATION_ALGORITHM_OVERLOAD_UTILIZATION << endl;
		cout << " -PERIODIC_INTERVAL_EXECUTION_OVERLOAD : " << PERIODIC_INTERVAL_EXECUTION_OVERLOAD << endl << endl; 
		
		cout << " -OPTIMIZATION_ALGORITHM_OVERHEATING_TEMPERATURE : " << OPTIMIZATION_ALGORITHM_OVERHEATING_TEMPERATURE << endl;
		cout << " -PERIODIC_INTERVAL_EXECUTION_TEMPERATURE : " << PERIODIC_INTERVAL_EXECUTION_TEMPERATURE << endl << endl;

		cout << " -VMS_ALGORITHM_SELECTION : " << 	VMS_ALGORITHM_SELECTION << endl << endl;

		cout << " -OPTIMIZATION_ALGORITHM_LINK_OVERLOAD : " << OPTIMIZATION_ALGORITHM_LINK_OVERLOAD << endl;
		cout << " -PERIODIC_INTERVAL_EXECUTION_LINK_OVERLOAD: " << PERIODIC_INTERVAL_EXECUTION_LINK_OVERLOAD << endl;
		cout << " -VMS_ALGORITHM_SELECTION_OVERLOADED_LINK  : " << VMS_ALGORITHM_SELECTION_OVERLOADED_LINK  << endl << endl;;

		cout << " -THRESHOLD_TOP_OF_USE_CPU : " << THRESHOLD_TOP_OF_USE_CPU << endl;
		cout << " -THRESHOLD_BOTTOM_OF_USE_CPU  : " << THRESHOLD_BOTTOM_OF_USE_CPU  << endl << endl;
		
		cout << " -USING_PREDICTION_TO_OVERLOAD  : " << USING_PREDICTION_TO_OVERLOAD  << endl;
		cout << " -PREDICTION_ALGORITHM_OVERLOAD : " << PREDICTION_ALGORITHM_OVERLOAD << endl;
		cout << " -SIZE_WINDOWN_PREDICTION_CPU : " << SIZE_WINDOWN_PREDICTION_CPU  << endl; 

	cout << endl;
	cout << "#Setting 3DMOBFD Algorithm Parameters " << endl;
		cout << " -ALPHA_3DMOBFD  : " << ALPHA_3DMOBFD  << endl;
		cout << " -BETA_3DMOBFD : " << BETA_3DMOBFD << endl;
		cout << " -GAMMA_3DMOBFD : " << SIZE_WINDOWN_PREDICTION_CPU  << endl; 
		cout << " -DELTA_3DMOBFD : " << DELTA_3DMOBFD << endl;
		cout << " -EPSILON_3DMOBFD : " << EPSILON_3DMOBFD  << endl;
		cout << endl;
		cout << " -WEIGHT_TEMPERATURE  : " << WEIGHT_TEMPERATURE  << endl;
		cout << " -WEIGHT_CPU : " << WEIGHT_CPU << endl;
		cout << " -WEIGHT_POWER : " << WEIGHT_POWER  << endl; 
		cout << " -WEIGHT_MEMORY : " << WEIGHT_MEMORY << endl;
		cout << " -WEIGHT_TRAFFIC : " << WEIGHT_TRAFFIC  << endl;
		cout << endl;
		cout << " -E_TEMPERATURE  : " << E_TEMPERATURE  << endl;
		cout << " -E_CPU : " << E_CPU << endl;
		cout << " -E_POWER : " << E_POWER  << endl; 
		cout << " -E_MEMORY : " << E_MEMORY << endl;
		cout << " -E_TRAFFIC : " << E_TRAFFIC << endl;
		cout << endl;

	cout << "#Setting Network Parameters " << endl;
		cout << " -SIMULATES_NETWORK : " << SIMULATES_NETWORK << endl;
		cout << " -TYPE_TOPOLOGY : " << TYPE_TOPOLOGY << endl;
		cout << " -NAME_FILE_TOPOLOGY : " << NAME_FILE_TOPOLOGY << endl;

	cout << endl;

	cout << "##Parameters Switches " << endl;
		cout << " -NUMBER_CHASSI_RACK : " << NUMBER_CHASSI_RACK << endl;
		cout << " -NUMBER_OF_PORTS_SWITCH_ACCESS : " << NUMBER_OF_PORTS_SWITCH_ACCESS << endl;
		cout << " -NUMBER_OF_PORTS_SWITCH_AGGREGATION : " << NUMBER_OF_PORTS_SWITCH_AGGREGATION << endl; 
		cout << " -NUMBER_OF_PORTS_SWITCH_CORE : " << NUMBER_OF_PORTS_SWITCH_CORE << endl << endl;

		cout << " -EACCE_CHASSIS   : " << EACCE_CHASSIS << endl;
		cout << " -EACCE_LINECARD  : " << EACCE_LINECARD << endl;
		cout << " -EACCE_PORT10    : " << EACCE_PORT10 << endl;
		cout << " -EACCE_PORT100   : " << EACCE_PORT100 << endl;
		cout << " -EACCE_PORT1000  : " << EACCE_PORT1000 << endl;
		cout << " -EACCE_PORT10000 : " << EACCE_PORT10000 << endl << endl;

		cout << " -EAGGR_CHASSIS   : " << EAGGR_CHASSIS << endl;
		cout << " -EAGGR_LINECARD  : " << EAGGR_LINECARD << endl;
		cout << " -EAGGR_PORT10    : " << EAGGR_PORT10 << endl;
		cout << " -EAGGR_PORT100   : " << EAGGR_PORT100 << endl;
		cout << " -EAGGR_PORT1000  : " << EAGGR_PORT1000 << endl;
		cout << " -EAGGR_PORT10000 : " << EAGGR_PORT10000 << endl << endl;

		cout << " -ECORE_CHASSIS   : " << ECORE_CHASSIS << endl;
		cout << " -ECORE_LINECARD  : " << ECORE_LINECARD << endl;
		cout << " -ECORE_PORT10    : " << ECORE_PORT10 << endl;
		cout << " -ECORE_PORT100   : " << ECORE_PORT100 << endl;
		cout << " -ECORE_PORT1000  : " << ECORE_PORT1000 << endl;
		cout << " -ECORE_PORT10000 : " << ECORE_PORT10000 << endl << endl;

	cout << endl;

	cout << "#Parameters Traffic Matrix" << endl;
		cout << " -SIMULATE_TRAFFIC_MATRIX : " << SIMULATE_TRAFFIC_MATRIX << endl;
		cout << " -NAME_FILE_MATRIX_TRAFFIC : " << NAME_FILE_MATRIX_TRAFFIC << endl;
		cout << " -CALCULATE_TRAFFIC_CORRELATION : " << CALCULATE_TRAFFIC_CORRELATION << endl;
		cout << " -EXECUTE_LOAD_BALANCING : " << EXECUTE_LOAD_BALANCING << endl;
		cout << " -NUMBER_NODES_TRAFFIC_MATRIX : " << NUMBER_NODES_TRAFFIC_MATRIX << endl;
		cout << " -NUMBER_OF_SAMPLES_TRAFFIC_MATRIX : " << NUMBER_OF_SAMPLES_TRAFFIC_MATRIX << endl;
		cout << " -NUMBER_OF_PERIODS : " << NUMBER_OF_PERIODS << endl;
		cout << " -K : " << K << endl;
		cout << " -SEED_PERIOD_ON : " << SEED_PERIOD_ON << endl;
		cout << " -SEED_PERIOD_OFF : " << SEED_PERIOD_OFF << endl;
		cout << " -PROPORTION_OF_INTERNAL_COMMUNICATION_BETWEEN_VMS : " << PROPORTION_OF_INTERNAL_COMMUNICATION_BETWEEN_VMS << endl;
		cout << " -PROPORTION_OF_EXTERNAL_COMMUNICATION : " << PROPORTION_OF_EXTERNAL_COMMUNICATION << endl;
		cout << " -DYNAMIC_SPEED_ADAPTATION : " << DYNAMIC_SPEED_ADAPTATION << endl;
		cout << " -RUNTIME_LINK_ADAPTATION : " << RUNTIME_LINK_ADAPTATION << endl; 
		cout << " -TRAFFIC_LIMIT_FOR_ADAPTATION : " << TRAFFIC_LIMIT_FOR_ADAPTATION << endl; 
		cout << " -SIMULATE_TOPOLOGY_OPTIMIZATION : " << SIMULATE_TOPOLOGY_OPTIMIZATION << endl;
		cout << " -OPTIMIZATION_ALGORITHM_TOPOLOGY : " << OPTIMIZATION_ALGORITHM_TOPOLOGY << endl; 
		cout << " -PERIODIC_INTERVAL_EXECUTION_TOPOLOGY : " << PERIODIC_INTERVAL_EXECUTION_TOPOLOGY << endl; 
		cout << endl;


	cout << "#Setting POOL" << endl;
		cout << " -SIMULATES_POOL_SERVER : " << SIMULATES_POOL_SERVER << endl;
		cout << " -SIZE_POOL : " << SIZE_POOL << endl;
		cout << " -INCREASE_SIZE_POOL_DYNAMICALLY : " << INCREASE_SIZE_POOL_DYNAMICALLY << endl;

	cout << endl;


#ifdef _DEBUG
	cout << endl;
	cout << "#Parameters for Future Work" << endl;
	cout << " -COOL_AIR_TRAVEL_TIME_MULTIPLIER : " << COOL_AIR_TRAVEL_TIME_MULTIPLIER << endl;
	cout << " -TEMPERATURE_SENSING_PERFORMANCE_CAPPING : " << TEMPERATURE_SENSING_PERFORMANCE_CAPPING << endl;
	cout << " -TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS : " << TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS << endl;
	cout << " -REASSIGN_VMS : " << REASSIGN_VMS << endl;
#endif

	cout << endl;
	cout << "-SWITCH ACCESS " << SWITCH_ACCESS << " SWITCH AGGREGATION " << SWITCH_AGGREGATION << " SWITCH_CORE " << SWITCH_CORE << endl;


	cout << endl;
	cout << "+ Total cores = " << (NUMBER_OF_CHASSIS*NUMBER_OF_CORES_IN_ONE_SERVER*NUMBER_OF_SERVERS_IN_ONE_CHASSIS) << endl;


	return true;
}

bool ReadMoreLines(ifstream* pIfs, vector<SWFLine*>* pvSWF)
{

	// read swf file into pJobQueueFile
	bool isReadingFinished = false;
	int numberOfLines = 0;
	int index = 0;

	vector<string> pJobQueueFile;
	string aLine;

READ_MORE_LINES:
	if (getline(*pIfs, aLine)) {
		vector<string> vOneLine;
		boost::split(vOneLine, aLine, boost::is_any_of(";"));
		if (vOneLine[0].size()==0) {
			goto READ_MORE_LINES; // skip comments
		} else {
			pJobQueueFile.push_back(aLine);
			numberOfLines++;
			if (numberOfLines < SWF_BUFFER_MIN_LINES)
				goto READ_MORE_LINES;
		}
	} else {
		isReadingFinished = true;
	}
	
	// parse pJobQueueFile into SWFLine (struct)
	for (unsigned int i=0; i<pJobQueueFile.size(); ++i) {
		vector<string> vOneLine;
		boost::split(vOneLine, pJobQueueFile[i], boost::is_any_of("\t "));
		
REMOVE_EMPTY_ELEMENT:
		for (unsigned int j=0; j<vOneLine.size(); ++j) {
			if(vOneLine[j].empty()) {
				vOneLine.erase(vOneLine.begin()+j);
				goto REMOVE_EMPTY_ELEMENT;
			}
		}
		if (vOneLine.size() == 0)
			continue; // could not read anything
		if (vOneLine.size() != 18) {
			cout << "Error: a SWF line has more than or less than 18 cols" << endl;
			continue;
		}

		// parsing SWF file
		SWFLine* pstALine = new SWFLine();
		memset(pstALine, 0x00, sizeof(SWFLine));
		pstALine->jobNumber	= boost::lexical_cast<UINT64>(vOneLine[0].c_str());
		//cout << "jobNumber " << pstALine->jobNumber << endl;
	
		pstALine->submitTimeSec = atoi(vOneLine[1].c_str());

		pstALine->submitTimeSec = pstALine->submitTimeSec - SUBMIT_TIME_SEC;
		// cout << "SubmitTimeSec " << pstALine->submitTimeSec << endl;

		pstALine->waitTimeSec = atoi(vOneLine[2].c_str());
		//cout << "WaitTimeSec " << pstALine->waitTimeSec << endl;
		
		pstALine->runTimeSec = atoi(vOneLine[3].c_str());
		//cout << "RunTimeSec " << pstALine->runTimeSec << endl;
		if ((pstALine->submitTimeSec + pstALine->runTimeSec) > MAX_TIME) {
           MAX_TIME = (pstALine->submitTimeSec + pstALine->runTimeSec);
		}

		pstALine->numCPUs = atoi(vOneLine[4].c_str());
		//cout << "NumCPUs " << pstALine->numCPUs << endl;
		
		pstALine->avgCPUTimeSec = atoi(vOneLine[5].c_str());
		//cout << "AvgCPUTimeSec " << pstALine->avgCPUTimeSec << endl;

		pstALine->usedMemKB = atol(vOneLine[6].c_str());
		//cout << "UsedMemKB  " <<pstALine->usedMemKB << endl;

		pstALine->numCPUsRequested = atoi(vOneLine[7].c_str());
		//cout << "numCPUsRequested  " <<pstALine->numCPUsRequested << endl;


		pstALine->runTimeSecRequested = atoi(vOneLine[8].c_str());
		// cout << "runTimeSecRequested  " <<pstALine->runTimeSecRequested << endl;

		pstALine->usedMemKBRequested = atoi(vOneLine[9].c_str());
		// cout << "usedMemKBRequested  " <<pstALine->usedMemKBRequested << endl;

		pstALine->status = atoi(vOneLine[10].c_str());
		// cout << "status   " <<pstALine->status  << endl;

		pstALine->userID = atoi(vOneLine[11].c_str());
		// cout << "userID   " <<pstALine->userID  << endl;

		pstALine->groupID = atoi(vOneLine[12].c_str());
		// cout << "groupID   " <<pstALine->groupID  << endl;
		
		pstALine->exefileID = atoi(vOneLine[13].c_str());
		// cout << "exefileID  " <<pstALine->exefileID  << endl;
		
		pstALine->queueNum = atoi(vOneLine[14].c_str());
		// cout << "queueNum   " <<pstALine->queueNum   << endl;
		
		pstALine->partitionNum = atoi(vOneLine[15].c_str());
		// cout << "partitionNum   " <<pstALine->partitionNum   << endl;

		if (boost::lexical_cast<long long>(vOneLine[16].c_str()) == -1)
			pstALine->dependencyJobNum = 0;
		else
			pstALine->dependencyJobNum = boost::lexical_cast<UINT64>(vOneLine[16].c_str());
		
		pstALine->thinkTimeAfterDependencySec = atoi(vOneLine[17].c_str());
		// cout << "thinkTimeAfterDependencySec   " <<pstALine->thinkTimeAfterDependencySec   << endl;

		// validate memory

		index = ++INDEX%MEMORY_VIRTUAL_MACHINE.size(); 

		if (pstALine->usedMemKB <= 0) {
			pstALine->usedMemKB = MEMORY_VIRTUAL_MACHINE[index];
		}
		else
		{
			pstALine->usedMemKB = pstALine->usedMemKB + MEMORY_VIRTUAL_MACHINE[index];
		}

		// validate
		if (pstALine->runTimeSec > 0 && pstALine->numCPUs > 0) {
			if (pstALine->avgCPUTimeSec == -1)
				pstALine->avgCPUTimeSec = pstALine->runTimeSec; // no info about cpu utilization. set 100%

			if (pstALine->avgCPUTimeSec == 0)
				pstALine->avgCPUTimeSec = 1; // min value is 1

			if (pstALine->avgCPUTimeSec <= 0) {
				cout << "Warning: avgCPUTimeSec <=0 for " << pstALine->jobNumber << " (skipped)" << endl;
				continue;
			}
			if (pstALine->avgCPUTimeSec > pstALine->runTimeSec)
				pstALine->avgCPUTimeSec = pstALine->runTimeSec;

			pstALine->submitTimeSec -= SKIP_x_SECONDS;
			if (pstALine->submitTimeSec < 0)
				continue;
			if (FINISHES_AT_DAY > 0 && pstALine->submitTimeSec > (FINISHES_AT_DAY*3600*24))
				continue;
			pvSWF->push_back(pstALine);
		}
     
	}
	pJobQueueFile.clear();

	return isReadingFinished;
}

int main(int argc, char* argv[])
{
	time_t timeold, timeini;

	// Reading parameters file
	if (ReadParameters() != 0) {
		return 1;
	}

	// arguments
	cout << "SimDC3D v2.0" << endl;

	for(int i = 0; i < argc; i++)
		cout << argv[i] << " ";
	cout << endl;
	
	if(ParsingArguments(argc, argv)==false)
		return 1;
	
#ifdef SINGLE_PRECISION
	cout << "+ Single Precision: only for debugging" << endl;
#else
	cout << "+ Double Precision" << endl;
#endif
#ifdef NO_DEP_CHECK
	cout << "+ Not checking job dependency" << endl;
#endif
	// current working directory
	char cCurrentPath[FILENAME_MAX];
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) { // yes. intended.
	   cout << "SIMDC3D-NOTICE: + Working Directory: " << cCurrentPath << endl;
	}

	// Reading File
	bool isReadingFinished = false;
	ifstream ifs(argv[1]);
	if(!ifs.is_open()) {
		cout << "SIMDC3D-ERROR: Cannot open a file :" << argv[1] << endl;
		return 1;
	}
	vector<SWFLine*> vSWF;
	isReadingFinished = ReadMoreLines(&ifs, &vSWF);
	if (isReadingFinished)
		ifs.clear();
		
	// Create & run
	cout << endl; 

	JobQueue myJobQueue;
	
	Users myUsers(&myJobQueue, &vSWF);
	
    POOLServers myPoolServers;

	TopologySimDC3D myTopology;

	DataCenter myDataCenter(&myJobQueue, &myPoolServers, &myTopology);

	int myClock = 0;
	int wallClock = 0;
	int lastTime = myUsers.LastJob();
	
	timeold = time(NULL);
    timeini = timeold;

	while (!(myJobQueue.IsFinished() && myDataCenter.IsFinished() && myUsers.IsFinished())) {
		myUsers.EveryASecond();
		myDataCenter.EveryASecond();
		myClock++;
		wallClock++;
		if ((wallClock <= MAX_TIME) && (wallClock <= lastTime))  {
			if (wallClock%3600 == 0) {
			    printf("Simulating ---> %.2f %% / Finish VMS %.2f %% / Simulation Time %d s / computing time %.f s / computing time total %.f s\n", ((double) wallClock / (double) MAX_TIME) * 100, ((double) wallClock / (double) lastTime) * 100, wallClock, difftime(time(NULL),timeold), difftime(time(NULL),timeini));
				timeold = time(NULL);
			}
		}
		else {
			if ((wallClock <= MAX_TIME) && (wallClock > lastTime))  {
			   if (wallClock%3600 == 0) {
			      printf("Simulating ---> %.2f %% / Finish VMS %.2f %% / Simulation Time %d s / computing time %.f s / computing time total %.f s \n", ((double) wallClock / (double) MAX_TIME) * 100, (double) 100, wallClock, difftime(time(NULL),timeold), difftime(time(NULL),timeini));
				  timeold = time(NULL);
			   }
		    }
			else {
			   printf("Ending Simulation / Simulation Time %d s / computing time %.f s / computing time total %.f s \n", wallClock, difftime(time(NULL),timeold), difftime(time(NULL),timeini));
			   timeold = time(NULL);
			} 
		} 
		fflush(stdout);
		if (myClock>(PERIODIC_LOG_INTERVAL<<2)) {
			myClock = 0;
			//cout << "."; // showing progress
			fflush(stdout);
		}
		if (isReadingFinished == false && vSWF.size() < SWF_BUFFER_MIN_LINES)
			isReadingFinished = ReadMoreLines(&ifs, &vSWF);
		if (FINISHES_AT_DAY !=0) {
			if (wallClock > FINISHES_AT_DAY) {
				break;
			}
		}
	}

	//myDataCenter.Finish();

	cout << "SIMDC3D-NOTICE: Finished !!!" << endl << endl;

	//myPoolServers.Print();
	//myDataCenter.PrintVector();

	myDataCenter.PrintResults(&myPoolServers); // Temperature, Power draw
	myJobQueue.PrintResults(wallClock); // Latency

	return 0;
} 

/* Standard Workload Format (SWF)
#  0 - Job Number
#  1 - Submit Time
#  2 - Wait Time
#  3 - Run Time
#  4 - Number of Processors
#  5 - Average CPU Time Used
#  6 - Used Memory
#  7 - Requested Number of Processors
#  8 - Requested Time
#  9 - Requested Memory
# 10 - status (1=completed, 0=killed)
# 11 - User ID
# 12 - Group ID
# 13 - Executable (Application) Number
# 14 - Queue Number
# 15 - Partition Number
# 16 - Preceding Job Number
# 17 - Think Time from Preceding Job
*/
