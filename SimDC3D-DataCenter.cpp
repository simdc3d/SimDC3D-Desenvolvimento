
#include <math.h>
#include <stdio.h>
#include "time.h"
#include <algorithm>

#include "SimDC3D-DataCenter.h"


DataCenter::DataCenter(JobQueue* q, POOLServers* Pool, TopologySimDC3D* topologySW)
{

	pJobQueue = q;
	pPool = Pool;
	pTopology = topologySW;

	fpSupplyTemperatureLog = 0.0;
	vSupplyTemperatureSparseLog.clear();

	fpTotalPowerFromComputingMachinesLog = 0.0;
	fpTotalPowerFromServerFansLog = 0.0;
	vTotalPowerFromComputingMachinesSparseLog.clear();
	vTotalPowerFromServerFansSparseLog.clear();

	fpTotalComputingPowerLog = 0.0;
	vTotalComputingPowerSparseLog.clear();

	fpTotalPowerFromCRACLog = 0.0;
	vTotalPowerFromCRACSparseLog.clear();

	fpUtilizationLog = 0.0;
	vUtilizationSparseLog.clear();

	currentSupplyTemp = LOWEST_SUPPLY_TEMPERATURE;
	currentSupplyTempBase = LOWEST_SUPPLY_TEMPERATURE;

	vTotalMigrationPolicyLowUtilization = 0;
	vTotalMigrationPolicyTemperature = 0;
	vTotalMigrationPolicyIdle = 0;
	vTotalMigrationPolicyOverLoaded = 0;
	vTotalMigrationPolicyPrediction = 0;
	vTotalMigrationsPolicyLinkOverload = 0;

	vTotalMigrationPolicyLowUtilizationLog = 0;
	vTotalMigrationPolicyTemperatureLog = 0;
	vTotalMigrationPolicyOverLoadedLog = 0;
	vTotalMigrationPolicyPredictionLog = 0;
	vTotalMigrationsPolicyLinkOverload = 0;

	vTotalMigrationsPolicyLowUtilizationSparseLog.clear();
	vTotalMigrationsPolicyTemperatureSparseLog.clear();
	vTotalMigrationsPolicyOverLoadedSparseLog.clear();
	vTotalMigrationsPolicyPredictionSparseLog.clear();
	vTotalMigrationsPolicyLinkOverloadSparseLog.clear();

	varianceArrivalTime.clear();
	varianceRunTime.clear();
	varianceTimeSchedulingQueue.clear();

	vmCorrelation.clear();
	vmDestination.clear();
	
	clock = 0;
	peakPower = 0.0;

	HRFLow = 100000.00;
	HRFHight = 0.00;

	avgArrivalTime = 0;
	clockPrevius = 0;
	avgRunTime = 0;
	avgTimeSchedulingQueue = 0;

	totalVMs = 0;
	totalVMsFinish = 0;
	totalVMsMigratingANDFinish = 0;
	maximumNumberSimultaneousVMs = 0;

	timeIni = time(NULL);

	for (int i=0; i < 8; i++) {
	    thread_L[i] = 0;
	    thread_R[i] = 0;
	}

	//gen = new std::default_random_engine ((int) time(NULL));
	gen = new std::default_random_engine((int)SEED*time(NULL));

	dist = new std::uniform_int_distribution<int>(1, NUMBER_NODES_TRAFFIC_MATRIX);

	for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
		for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			pServers[i][j] = new Server(0, BANDWIDTH_IN_ONE_SERVER, TOTAL_OF_MEMORY_IN_ONE_SERVER, i, j, &vmDestinationDataFlowKeyVM);
		}
	}

	for (int i = 0; i<NUMBER_OF_CHASSIS; ++i)
		perServerPowerLog[i] = perServerTemperatureLog[i] = perServerComputingPowerLog[i] = 0.0;

	if (SCHEDULING_ALGORITHM == "BEST_PERFORMANCE")
		pSchedulingAlgorithm = new BestPerformanceSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD, &clock);
	else if (SCHEDULING_ALGORITHM == "UNIFORM_TASK")
		pSchedulingAlgorithm = new UniformTaskSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD, &clock);
	else if (SCHEDULING_ALGORITHM == "LOW_TEMP_FIRST")
		pSchedulingAlgorithm = new LowTemperatureFirstSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD, &clock);
	else if (SCHEDULING_ALGORITHM == "RANDOM")
		pSchedulingAlgorithm = new RandomSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD, &clock);
	else if (SCHEDULING_ALGORITHM == "MIN_HR")
		pSchedulingAlgorithm = new MinHRSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD, &clock);
	else if (SCHEDULING_ALGORITHM == "CENTER_RACK_FIRST")
		pSchedulingAlgorithm = new CenterRackFirstSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD, &clock);
	else if (SCHEDULING_ALGORITHM == "2D_POOL")
		pSchedulingAlgorithm = new TwoDimensionWithPoolSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD, pPool, &clock);
	else if (SCHEDULING_ALGORITHM == "2D_POOL_AND_PREDICTION")
		pSchedulingAlgorithm = new TwoDimensionWithPoolAndPredictionSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD, pPool, &clock);
	else if (SCHEDULING_ALGORITHM == "2D_PREDICTION")
		pSchedulingAlgorithm = new TwoDimensionWithPredictionSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD, pPool, &clock);
	else if (SCHEDULING_ALGORITHM == "2D")
		pSchedulingAlgorithm = new TwoDimensionSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD, pPool, &clock);
	else if (SCHEDULING_ALGORITHM == "3D_MULTI_OBJ")
		pSchedulingAlgorithm = new ThreeDimensionMultiObjSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD, pPool, &clock);
	else if (SCHEDULING_ALGORITHM == "3D_MULTI_OBJ_POOL")
		pSchedulingAlgorithm = new ThreeDimensionMultiObjAndPoolSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD, pPool, &clock);
	else if (SCHEDULING_ALGORITHM == "3D_MULTI_OBJ_PREDICTION")
		pSchedulingAlgorithm = new ThreeDimensionMultiObjAndPredictionCPUAndTemperatureSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD, pPool, &clock);
	else if (SCHEDULING_ALGORITHM == "3D_MULTI_OBJ_POOL_AND_PREDICTION")
		pSchedulingAlgorithm = new ThreeDimensionMultiObjAndPoolAndPredictionCPUAndTemperatureSchedulingAlgorithm(&pServers, &qWaitingVMs, &HeatRecirculationMatrixD, pPool, &clock);

	else {
		cout << "SIMDC3D-ERROR: Unknown scheduling algorithm!!!" << endl;
		exit(0);
	}

	if ((SCHEDULING_ALGORITHM == "2D_POOL") || (SCHEDULING_ALGORITHM == "2D_POOL_AND_PREDICTION") || (SCHEDULING_ALGORITHM == "2D_PREDICTION") || (SCHEDULING_ALGORITHM == "2D")) {
		if ((SCHEDULER_2D_WEIGHT_TEMPERATURE + SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION + SCHEDULER_2D_WEIGHT_LOAD_CPU) != 1) {
			cout << "SIMDC3D-ERROR: Sum of weights (SCHEDULER_2D_WEIGHT_TEMPERATURE, SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION and SCHEDULER_2D_WEIGHT_LOAD_CPU) is different from one" << endl;
			exit(0);
		}
	}

	if (POLICY_OF_TEMPERATURE_CONTROL == "DEFAULT") {
		pCRAC = new Police_Default(&pServers);
	}
	else {
		if (POLICY_OF_TEMPERATURE_CONTROL == "AGGRESSIVE_MODE") {
			pCRAC = new Agressive_Mode(&pServers);
		}
		else {
			cout << "SIMDC3D-ERROR: Unknown Policy of Temperature. Use default value (Default)" << endl;
			pCRAC = new Police_Default(&pServers);
		}
	}

	if (SIMULATES_MIGRATION_VMS) {
		//LOW UTILIZATION
		if (OPTIMIZATION_ALGORITHM_LOW_UTILIZATION == "FFD")
			policyLow = new FFD_Low(&pServers);
		else if (OPTIMIZATION_ALGORITHM_LOW_UTILIZATION == "PABFD")
			policyLow = new PABFD_Low(&pServers);
		else if (OPTIMIZATION_ALGORITHM_LOW_UTILIZATION == "3DMOBFD")
			policyLow = new ThreeDimensionMultiObj_Low_V2(&pServers,  HeatRecirculationMatrixD);
		else if (OPTIMIZATION_ALGORITHM_LOW_UTILIZATION == "2D_WEIGHT")
			policyLow = new TwoDimensionWeight_Low(&pServers);
		else {
			cout << "SIMDC3D-ERROR: Unknown optimization algorithm Low Utilization. Use default value (3D_MULTI_OBJ)" << endl;
			policyLow = new ThreeDimensionMultiObj_Low_V2(&pServers,  HeatRecirculationMatrixD);
		}

		// OVERLOAD
		if (OPTIMIZATION_ALGORITHM_OVERLOAD_UTILIZATION == "FFD")
			policyOverLoaded = new FFD_OverLoaded(&pServers, pPool);
		else if (OPTIMIZATION_ALGORITHM_OVERLOAD_UTILIZATION == "PABFD")
			policyOverLoaded = new PABFD_OverLoaded(&pServers, pPool);
		else if (OPTIMIZATION_ALGORITHM_OVERLOAD_UTILIZATION == "3DMOBFD")
			policyOverLoaded = new ThreeDimensionMultiObj_OverLoaded_V2(&pServers, pPool, HeatRecirculationMatrixD);
		else if (OPTIMIZATION_ALGORITHM_OVERLOAD_UTILIZATION == "2D_WEIGHT")
			policyOverLoaded = new TwoDimensionWeight_OverLoaded(&pServers, pPool);
		else {
			cout << "SIMDC3D-ERROR: Unknown optimization algorithm Overload Utilization. Use default value (3D_MULTI_OBJ)" << endl;
			policyOverLoaded = new ThreeDimensionMultiObj_OverLoaded_V2(&pServers, pPool, HeatRecirculationMatrixD);
		}
		// TEMPERATURE
		if (OPTIMIZATION_ALGORITHM_OVERHEATING_TEMPERATURE == "HIGH_TEMPERATURE")
			policyHightTemperature = new Police_High_Temperature(&pServers, pPool);
		else if (OPTIMIZATION_ALGORITHM_OVERHEATING_TEMPERATURE == "PABFD")
			policyHightTemperature = new PABFD_Temperature(&pServers, pPool);
		else if (OPTIMIZATION_ALGORITHM_OVERHEATING_TEMPERATURE == "3D_MULTI_OBJ")
			policyHightTemperature = new ThreeDimensionMultiObj_OverLoaded_V2(&pServers, pPool, HeatRecirculationMatrixD);
		else {
			cout << "SIMDC3D-ERROR: Unknown optimization algorithm Temperature. Use default value (Default)" << endl;
			policyHightTemperature = new Police_High_Temperature(&pServers, pPool);
		}
		// NETWORK
		if (OPTIMIZATION_ALGORITHM_LINK_OVERLOAD == "DEFAULT") {
			policyNetwork = new NetworkLinkOverload(&pServers, pPool);
		}
		else {
			cout << "SIMDC3D-ERROR: Unknown optimization algorithm Network. Use default value (Default)" << endl;
			exit(0);
		}


		if ((VMS_ALGORITHM_SELECTION == "HIGHER_UTILIZATION_CPU/LESS_MEMORY") || (VMS_ALGORITHM_SELECTION == "HIGHER_UTILIZATION_AVERAGE_CPU/LESS_MEMORY") ||
			(VMS_ALGORITHM_SELECTION == "HIGHER_UTILIZATION_AVERAGE_CPU/LESS_MEMORY/HIGHER_IO_NETWORK") || (VMS_ALGORITHM_SELECTION == "HIGHER_UTILIZATION_AVERAGE_CPU") ||
			(VMS_ALGORITHM_SELECTION == "LESS_MEMORY") || (VMS_ALGORITHM_SELECTION == "RANDOM_SELECTION") || (VMS_ALGORITHM_SELECTION_OVERLOADED_LINK == "HIGHER_IO_NETWORK")) {

		}
		else {
			cout << "Error: unknown VMS_ALGORITHM_SELECTION. Use default value (HIGHER_UTILIZATION_AVERAGE_CPU/LESS_MEMORY)" << endl;
			VMS_ALGORITHM_SELECTION = "HIGHER_UTILIZATION_AVERAGE_CPU/LESS_MEMORY";
		}

		if ((VMS_ALGORITHM_SELECTION_OVERLOADED_LINK == "HIGHER_UTILIZATION_CPU/LESS_MEMORY") || (VMS_ALGORITHM_SELECTION_OVERLOADED_LINK == "HIGHER_UTILIZATION_AVERAGE_CPU/LESS_MEMORY") ||
			(VMS_ALGORITHM_SELECTION_OVERLOADED_LINK == "HIGHER_UTILIZATION_AVERAGE_CPU/LESS_MEMORY/HIGHER_IO_NETWORK") || (VMS_ALGORITHM_SELECTION_OVERLOADED_LINK == "HIGHER_UTILIZATION_AVERAGE_CPU") ||
			(VMS_ALGORITHM_SELECTION_OVERLOADED_LINK == "LESS_MEMORY") || (VMS_ALGORITHM_SELECTION_OVERLOADED_LINK == "RANDOM_SELECTION") || (VMS_ALGORITHM_SELECTION_OVERLOADED_LINK == "HIGHER_IO_NETWORK")) {
		}
		else {
			cout << "Error: unknown VMS_ALGORITHM_SELECTION. Use default value (HIGHER_UTILIZATION_AVERAGE_CPU/LESS_MEMORY)" << endl;
			VMS_ALGORITHM_SELECTION_OVERLOADED_LINK = "HIGHER_IO_NETWORK";
		}
	}

	if ((TYPE_TOPOLOGY == "2LAYER") || (TYPE_TOPOLOGY == "3LAYER") || (TYPE_TOPOLOGY == "FAT-TREE") || (TYPE_TOPOLOGY == "BCUBE")) {
	}
	else {
		 cout << "SIMDC3D-ERROR: Unknown VMS_ALGORITHM_SELECTION. Use default value (HIGHER_UTILIZATION_AVERAGE_CPU/LESS_MEMORY)" << endl;
		 exit(0);
	}


	if ( (OPTIMIZATION_ALGORITHM_OVERLOAD_UTILIZATION == "3DMOBFD") || (OPTIMIZATION_ALGORITHM_LOW_UTILIZATION == "3DMOBFD") || (SCHEDULING_ALGORITHM == "3D_MULTI_OBJ") ) {
	   if ( (WEIGHT_TEMPERATURE != 1) || (WEIGHT_TEMPERATURE != 1) || (WEIGHT_TEMPERATURE != 1) || (WEIGHT_TEMPERATURE != 1) || (WEIGHT_TEMPERATURE != 1) ) {
		   float wght = 0;
		   if (ALPHA_3DMOBFD == 1) 
			   wght += WEIGHT_TEMPERATURE;

		    if (BETA_3DMOBFD == 1) 
			   wght += WEIGHT_CPU;

			if (GAMMA_3DMOBFD == 1) 
			   wght += WEIGHT_POWER;

			if (DELTA_3DMOBFD == 1) 
			   wght += WEIGHT_MEMORY;

			if (EPSILON_3DMOBFD == 1) 
			   wght += WEIGHT_TRAFFIC;

			if (wght != 1) {
				
				cout << "SIMDC3D-ERROR: Sum of weights (" << wght << ") is different from 1 !!!" << endl;
				exit(0);
			}
	   }
	}


	// Create Topology
	if (SIMULATES_NETWORK) {
		pTopology->CreateTopology(&pServers, &vmDestinationDataFlowKeyVM);

		if (SIMULATE_TRAFFIC_MATRIX) {
			if (SIMULATE_TOPOLOGY_OPTIMIZATION) {
				if (OPTIMIZATION_ALGORITHM_TOPOLOGY == "DEFAULT") {
				//topologyOptimization = new CarpoOptimizationAlgorithm(&pServers, &clock, &vmDestinationDataFlowKeyVM);
				}
				else { if (OPTIMIZATION_ALGORITHM_TOPOLOGY == "CARPO") {
					topologyOptimization = new CarpoOptimizationAlgorithm(&pServers, pTopology);
					}
					else {
						cout << "SIMDC3D-ERROR: The selected topology does not exist!!!" << endl;
						exit(0);
					}
				}
			}
		}
	}

	perDataCenterUtilization = 0.0;

	for (int i = 0; i<SIZE_OF_HR_MATRIX; ++i) {
		ModedAirTravelTimeFromCRAC[i] = AirTravelTimeFromCRAC[i] * COOL_AIR_TRAVEL_TIME_MULTIPLIER;
	}

	for (int i = 0; i<SIZE_OF_HR_MATRIX; ++i) {
		if (ModedAirTravelTimeFromCRAC[i] > SIZE_OF_HEAT_TIMING_BUFFER)
			cout << "SIMDC3D-ERROR: HeatRecirculationTimingFromCRAC[" << i << "] > " << SIZE_OF_HEAT_TIMING_BUFFER << endl;
	}

	for (int i = 0; i < 101; ++i) {
		HowManySecondsInThisInletAirTemperature[i] = 0;
		HowManySecondsInThisUtilization[i] = 0;
	}

	for (int i = 0; i<SIZE_OF_HR_MATRIX; ++i) {
		HRM[i] = 0.0;
		for (int j = 0; j<SIZE_OF_HR_MATRIX; ++j) {
			HRM[i] += (HeatRecirculationMatrixD[j][i]);
		}
	}
	for (int i = 0; i<SIZE_OF_HR_MATRIX; ++i) {
		if (HRM[i] < HRFLow) {
			HRFLow = HRM[i];
		}
		if (HRM[i] > HRFHight) {
			HRFHight = HRM[i];
		}
	}

	//CRACDischargeAirTempChangeRate = 0.001 * CRAC_DISCHARGE_CHANGE_RATE_0_00x;
		
	if (RUN_MULTITHREAD) {
		if (NUMBER_OF_THREADS < 3) {
			NUMBER_OF_THREADS = 3;
		}

		// CREATE THREADS EVERY A SECOND SERVER 
		sem_init(&semaphore_SV, 0, 0);
		pthread_barrier_init(&barrier_SV, NULL, NUMBER_OF_THREADS);


		int processing_threads = NUMBER_OF_THREADS - 1;
		int cover_interval = NUMBER_OF_CHASSIS / processing_threads;

		for (int i = 0; i < processing_threads; i++) {
			paramThread_SV[i].l = i * cover_interval;
			paramThread_SV[i].r = i * cover_interval + cover_interval;
			paramThread_SV[i].tServers = &pServers;
			paramThread_SV[i].clock = &clock;
			paramThread_SV[i].semaphore_Thread = &semaphore_SV;
			paramThread_SV[i].barrier_Thread = &barrier_SV;

			pthread_create(&cThread_SV[i], NULL, DataCenter::EveryASecondServer, &paramThread_SV[i]);
		}
		paramThread_SV[processing_threads - 1].r = NUMBER_OF_CHASSIS;
		// FINISH


		// CREATE THREADS TEMPERATURE
		sem_init(&semaphore_TP, 0, 0);
		pthread_barrier_init(&barrier_TP, NULL, NUMBER_OF_THREADS);

		for (int i = 0; i < processing_threads; i++) {
			paramThread_TP[i].l = i * cover_interval;
			paramThread_TP[i].r = i * cover_interval + cover_interval;
			paramThread_TP[i].tServers = &pServers;
			paramThread_TP[i].clock = &clock;
			paramThread_TP[i].currentSupplyTemp = &currentSupplyTemp;
			for (int w = 0; w < SIZE_OF_HR_MATRIX; w++) {
				paramThread_TP[i].ModedAirTravelTimeFromCRAC[w] = ModedAirTravelTimeFromCRAC[w];
			}
			for (int w = 0; w < 101; w++) {
				paramThread_TP[i].HowManySecondsInThisInletAirTemperature[w] = 0;
				paramThread_TP[i].HowManySecondsInThisUtilization[w] = 0;
			}
			paramThread_TP[i].semaphore_Thread = &semaphore_TP;
			paramThread_TP[i].barrier_Thread = &barrier_TP;

			pthread_create(&cThread_TP[i], NULL, DataCenter::WriteTemperatureToEveryServer, &paramThread_TP[i]);
		}
		paramThread_TP[processing_threads - 1].r = NUMBER_OF_CHASSIS;
		// FINISH

		if (SIMULATES_NETWORK) {
			// CREATE THREADS TOPOLOGY
			sem_init(&semaphore_TO, 0, 0);
			pthread_barrier_init(&barrier_TO, NULL, NUMBER_OF_THREADS);

			if (pthread_mutex_init(&mutex_TO, NULL) != 0) {
				cout << "SimDC3D-ERROR: Error create mutex Data Center (mutex_TO)!!! " << endl;
				exit(0);
			}

			for (int i = 0; i < processing_threads; i++) {
				paramThread_TO[i].l = i * cover_interval;
				paramThread_TO[i].r = i * cover_interval + cover_interval;
				paramThread_TO[i].tServers = &pServers;
				paramThread_TO[i].topologySimDC3D = pTopology;
				paramThread_TO[i].semaphore_Thread = &semaphore_TO;
				paramThread_TO[i].barrier_Thread = &barrier_TO;
				paramThread_TO[i].mutex = &mutex_TO;

				pthread_create(&cThread_TO[i], NULL, DataCenter::RunTopologySimDC3DEveryASecond, &paramThread_TO[i]);
			}
			paramThread_TO[processing_threads - 1].r = NUMBER_OF_CHASSIS;
		}

		if (SIMULATES_NETWORK) {
		   if (SIMULATE_TRAFFIC_MATRIX) {
			   sem_init(&semaphore_CO, 0, 0);

			   pthread_barrier_init(&barrier_CO, NULL, 9);

			   if (pthread_mutex_init(&mutex_CO, NULL) != 0) {
				cout << "SimDC3D-ERROR: Error create mutex Data Center (mutex_CO) !!! " << endl;
				exit(0);
			   }

			   for (int i = 0; i < 8; i++) {
				   paramThread_CO[i].l = &thread_L[i];
				   paramThread_CO[i].r = &thread_R[i];
				   paramThread_CO[i].semaphore_Thread = &semaphore_CO;
				   paramThread_CO[i].barrier_Thread = &barrier_CO;
				   paramThread_CO[i].mutex = &mutex_CO;
				   paramThread_CO[i].vmDestinationDFlowKeyVM = &vmDestinationDataFlowKeyVM;
				   paramThread_CO[i].vmCorrelation_th = &vmCorrelation; 
				   paramThread_CO[i].vmDestinationDFlowVector = &vmDestination;

				   pthread_create(&cThread_CO[i], NULL, DataCenter::CalculateCorrelation_MT, &paramThread_CO[i]);
			   }
		   }
		}

		// FINISH
		/*
		// CREATE THREADS VARIANCE
		//processing_threads = (NUMBER_OF_THREADS) - 1;
		//cover_interval = NUMBER_OF_CHASSIS / processing_threads;


		sem_init(&semaphore_VA, 0, 0);
		pthread_barrier_init(&barrier_VA, NULL, NUMBER_OF_THREADS);

		for (int i = 0; i < processing_threads; i++) {
		paramThread_VA[i].l = i * cover_interval;
		paramThread_VA[i].r = i * cover_interval + cover_interval;
		paramThread_VA[i].tServers = &pServers;
		paramThread_VA[i].semaphore_Thread = &semaphore_VA;
		paramThread_VA[i].barrier_Thread = &barrier_VA;
		pthread_create(&cThread_VA[i], NULL, DataCenter::CalculateVarianceServer, &paramThread_VA[i]);
		}
		paramThread_VA[processing_threads-1].r = NUMBER_OF_CHASSIS;
		// FINISH */
	}
}

DataCenter::~DataCenter(void)
{
	for (int i = 0; i<NUMBER_OF_CHASSIS; ++i)
		for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j)
			delete pServers[i][j];


	delete pSchedulingAlgorithm;

/*	if (SIMULATES_MIGRATION_VMS) {
		delete policyLow;
		delete policyOverLoaded;
		delete policyHightTemperature;
	}*/


	vTotalMigrationsPolicyLowUtilizationSparseLog.clear();
	vTotalMigrationsPolicyTemperatureSparseLog.clear();
	vTotalMigrationsPolicyOverLoadedSparseLog.clear();
	vTotalMigrationsPolicyPredictionSparseLog.clear();
	vTotalMigrationsPolicyLinkOverloadSparseLog.clear();

	vSupplyTemperatureSparseLog.clear();
	vTotalPowerFromComputingMachinesSparseLog.clear();
	vTotalPowerFromServerFansSparseLog.clear();
	vTotalComputingPowerSparseLog.clear();
	vTotalPowerFromCRACSparseLog.clear();
	vUtilizationSparseLog.clear();


	varianceArrivalTime.clear();
	varianceRunTime.clear();
	varianceTimeSchedulingQueue.clear();

	vmCorrelation.clear();
	vmDestination.clear();

	if (RUN_MULTITHREAD) {
		for (int i = 0; i < (NUMBER_OF_THREADS - 1); i++) {
			pthread_cancel(cThread_SV[i]);
		}
	}

}

void DataCenter::EveryASecond(void)
{
	std::uniform_int_distribution<int> NodeFNSSVM(dist->param());
	map <VirtualMachine*, std::string>::iterator iteMapKeyVM;

	// updated simulation clock
	pPool->UpdateClockSimulation(clock);

	// calculate variance
	if (!pJobQueue->IsFinished()) {
		avgArrivalTime += (clock - clockPrevius);
		varianceArrivalTime.push_back(clock - clockPrevius);
		clockPrevius = clock;
	}

	while (!pJobQueue->IsFinished()) { // JobQueue is not empty
		SingleJob* sj = pJobQueue->TakeFirst();
		for (int i = 0; i < sj->numCPUs; ++i) {
			qWaitingVMs.push(new VirtualMachine(sj->runTimeSec, sj->avgCPUTimeSec, sj->jobNumber, sj->usedMemKB, IMAGE_SIZE_VIRTUAL_MACHINE, GenerateSeed(), clock, to_string(NodeFNSSVM(*gen))));
			totalVMs += 1; // vms total arrived DC
			avgRunTime += sj->runTimeSec;
			varianceRunTime.push_back(sj->runTimeSec);
			if ( (SIMULATES_NETWORK) && (SIMULATE_TRAFFIC_MATRIX) ) {
				vmDestinationDataFlowKeyVM.insert(pair<VirtualMachine*, string>(qWaitingVMs.back(), (qWaitingVMs.back())->ReturnNodeFNSSVM()));
			}
		}
	}

	if (REASSIGN_VMS && (clock % 16384 == 0)) { // re arranging
		for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j = 0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ((pServers[i][j]->IsOFF()) ||
					(pServers[i][j]->IsHibernating()) ||
					(pServers[i][j]->IsENDING()))
					continue;
				VirtualMachine* pvm;
				while ((pvm = (pServers[i][j]->TakeAVM())) != NULL) {
					qWaitingVMs.push(pvm);
				}
			}
		}
	}


	// Call Controller 
	Controller();

	// call EveryASecond to every server instance
	if (RUN_MULTITHREAD) {
		for (int i = 0; i < NUMBER_OF_THREADS - 1; i++) {
			sem_post(&semaphore_SV);
		}
		pthread_barrier_wait(&barrier_SV);
	}
	else {
		// call EveryASecond to every server instance
		for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if (SCHEDULING_WITH_PREDICTION) {
					if (clock == pServers[i][j]->ReturnFirstTimePredictionServer()) { // calculte RMSD
						pServers[i][j]->CalculateRMSE();
					}
				}
				pServers[i][j]->EveryASecond(clock);
			}
		}

		// checks completed the transfer of vms
		if (SIMULATES_MIGRATION_VMS) {
			for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
				for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
					if ((pServers[i][j]->IsOFF()) || (pServers[i][j]->IsHibernating()) || (pServers[i][j]->IsENDING()) || (!pServers[i][j]->IsMIGRATING()) || (pServers[i][j]->IsPOOL()) || (pServers[i][j]->IsINITIALIZING())) {
						continue;
					}


					if (SIMULATES_MIGRATION_VMS) { 
						// check if a full migration finished
						pServers[i][j]->CheckFinishMove(clock);

						// modify status server 
						if (!pServers[i][j]->HasVMsToMigrate()) {
							pServers[i][j]->SetMigrating(false);
						}
					}
				}
			}
		}
	}

	/*
	// Calculate Variance Server
	if (RUN_MULTITHREAD){
	for (int i=0; i < (NUMBER_OF_THREADS-1); i++) {
	sem_post(&semaphore_VA);
	}
	pthread_barrier_wait(&barrier_VA);
	}
	else {
	// call EveryASecond to every server instance
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
	for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
	if ( (pServers[i][j]->IsOFF()) || (pServers[i][j]->IsHibernating()) || (pServers[i][j]->IsENDING()) || (pServers[i][j]->IsINITIALIZING()) ) {
	continue;
	}
	pServers[i][j]->CalculatingStandardDeviationCPU();
	}
	}
	}
	*/

	// verifies that the servers power off or servers power on
	pPool->EveryASecond(&pServers);


	// increases the clock simulator
	++clock;

	RecalculateHeatDistribution();

	// decide supply temperature for next 1 second
	currentSupplyTemp = pCRAC->PoliceTemperature();

	// Write supply temperature to every server
	if (RUN_MULTITHREAD) {
		for (int i = 0; i < NUMBER_OF_THREADS - 1; i++) {
			sem_post(&semaphore_TP);
		}
		pthread_barrier_wait(&barrier_TP);
	}
	else {
		// Write supply temperature to every server
		for (int i = 0; i < NUMBER_OF_CHASSIS; ++i) {
			for (int j = 0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				// use matrix only 
				if (INSTANT_COOL_AIR_TRAVEL_TIME) {
					pServers[i][j]->SetSupplyTempToTimingBuffer(currentSupplyTemp, 0);
				}
				else {
					pServers[i][j]->SetSupplyTempToTimingBuffer(currentSupplyTemp, ModedAirTravelTimeFromCRAC[i]);
				}
			}
		}

		// record current temperature of servers
		for (int i = 0; i < NUMBER_OF_CHASSIS; ++i) {
			for (int j = 0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if (pServers[i][j]->IsOFF() || (pServers[i][j]->IsHibernating())) {
					continue;
				}
				unsigned int local_temperature = (unsigned int)(pServers[i][j]->CurrentInletTemperature());

				if (local_temperature > 100) {
					cout << "SimDC3D-ERROR: Local_temperature too big: Servidor " << i << " " << j << " Temperature " << local_temperature << endl;
					exit(0);
				}
				else {
					++HowManySecondsInThisInletAirTemperature[local_temperature];
				}

				unsigned int local_utilization = (unsigned int)(pServers[i][j]->CurrentUtilization() * 100);
				if (local_utilization > 100) {
					cout << "SimDC3D-ERROR: Local_utilization too big: " << local_utilization << endl;
					exit(0);
				}
				else {
					++HowManySecondsInThisUtilization[local_utilization];
				}
			}
		}
	}


	// This part of the code is not worth paralelizing. The mutex kills the performance.
	// I commented out the parallel code. It will always run on the main thread.

	unsigned int totalVMsDC = 0;

	// delete finished VMs
	for (int i = 0; i < NUMBER_OF_CHASSIS; ++i) {
		for (int j = 0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			if ((pServers[i][j]->IsOFF()) || (pServers[i][j]->IsHibernating()) || (pServers[i][j]->IsENDING()) || (pServers[i][j]->IsPOOL()) || (pServers[i][j]->IsINITIALIZING())) {
				continue;
			}

			vector<VirtualMachine*>* pvFinishedVMs = pServers[i][j]->GetFinishedVMVector();

		INI_:
			for (vector<VirtualMachine *>::iterator it = pvFinishedVMs->begin(); it != pvFinishedVMs->end(); ++it) {
				if (SIMULATES_MIGRATION_VMS) {
					if ((*it)->ReturnIsMove()) {
						if ((*it)->ReturnTimeMigrationDestination() != 0) {  // The VMs exist on two servers during the migration. To calculate the statistics only one is selected.
							pServers[i][j]->InsertDownTimeANDLeaseTimeVM((*it)->ReturnTDownTime(), (*it)->ReturnLeaseTime());
							pJobQueue->OneVMFinishedOnASingleJob((*it)->GetJobNumber(), (int)clock, (float)(i + j / 10.0));
							avgTimeSchedulingQueue += (*it)->ReturnTimeInTheShedulingQueue();
							varianceTimeSchedulingQueue.push_back((*it)->ReturnTimeInTheShedulingQueue());
							totalVMsMigratingANDFinish++;

							if ((SIMULATES_NETWORK) && (SIMULATE_TRAFFIC_MATRIX) )  {
								iteMapKeyVM = (vmDestinationDataFlowKeyVM.find(*it));
								if (iteMapKeyVM != vmDestinationDataFlowKeyVM.end()) {
									vmDestinationDataFlowKeyVM.erase(iteMapKeyVM);
								}
							}
							pvFinishedVMs->erase(it);
							goto INI_;
						}
						else {
							if ((SIMULATES_NETWORK) && (SIMULATE_TRAFFIC_MATRIX) )  {
								iteMapKeyVM = (vmDestinationDataFlowKeyVM.find(*it));
								//cout << "Apagando a VM " << *it << endl;
								if (iteMapKeyVM != vmDestinationDataFlowKeyVM.end()) {
									vmDestinationDataFlowKeyVM.erase(iteMapKeyVM);
								}
							}
							pvFinishedVMs->erase(it);
							goto INI_;
						}
					}
					else {
						pServers[i][j]->InsertDownTimeANDLeaseTimeVM((*it)->ReturnTDownTime(), (*it)->ReturnLeaseTime());
						pJobQueue->OneVMFinishedOnASingleJob((*it)->GetJobNumber(), (int)clock, (float)(i + j / 10.0));
						avgTimeSchedulingQueue += (*it)->ReturnTimeInTheShedulingQueue();
						varianceTimeSchedulingQueue.push_back((*it)->ReturnTimeInTheShedulingQueue());
						totalVMsFinish++;

						if ((SIMULATES_NETWORK) && (SIMULATE_TRAFFIC_MATRIX) )  {
							iteMapKeyVM = (vmDestinationDataFlowKeyVM.find(*it));
							//cout << "Apagando a VM " << *it << endl;
							if (iteMapKeyVM != vmDestinationDataFlowKeyVM.end()) {
								vmDestinationDataFlowKeyVM.erase(iteMapKeyVM);
							}
						}
						pvFinishedVMs->erase(it);
						goto INI_;
					}
				}
				else {
					pJobQueue->OneVMFinishedOnASingleJob((*it)->GetJobNumber(), (int)clock, (float)(i + j / 10.0));
					avgTimeSchedulingQueue += (*it)->ReturnTimeInTheShedulingQueue();
					varianceTimeSchedulingQueue.push_back((*it)->ReturnTimeInTheShedulingQueue());
					totalVMsFinish++;

					if ((SIMULATES_NETWORK) && (SIMULATE_TRAFFIC_MATRIX) )  {
						iteMapKeyVM = (vmDestinationDataFlowKeyVM.find(*it));
						//cout << "Apagando a VM " << *it << endl;
						if (iteMapKeyVM != vmDestinationDataFlowKeyVM.end()) {
							vmDestinationDataFlowKeyVM.erase(iteMapKeyVM);
						}
					}

					pvFinishedVMs->erase(it);
					goto INI_;
				}
			}

			if (pvFinishedVMs->size() != 0) {
				cout << "SimDC3D-ERROR: pvFinishedVMs <> zero!!! " << pvFinishedVMs->size() << endl;
			}

			pvFinishedVMs->clear();

			if ((pServers[i][j]->IsMIGRATING()) && (!pServers[i][j]->HasVMsToMigrate())) {
				pServers[i][j]->SetMigrating(false);
			}

			totalVMsDC += pServers[i][j]->HowManyVMs();
		}
	}

	// Calculates the energy consumption of switches and optimizes network topology
	if (SIMULATES_NETWORK) {
		pTopology->UpdateClock(clock);  // update clock simulation 
		if (RUN_MULTITHREAD) {
			if (SIMULATE_TRAFFIC_MATRIX) {
				pTopology->LoadTrafficMatriz();
			
			    pTopology->EveryASecondMultiThread_P1();
			    for (int i = 0; i < (NUMBER_OF_THREADS - 1); i++) {
				    sem_post(&semaphore_TO);
			    }
			    pthread_barrier_wait(&barrier_TO);
			}
			pTopology->EveryASecondMultiThread_P3();
		}
		else {
			pTopology->EveryASecond(clock, &pServers);
		}
	
		// *---------------------- calculate correlation ------------------------------------
		if (CALCULATE_TRAFFIC_CORRELATION) {
			if (SIMULATE_TRAFFIC_MATRIX) {
			   if (clock%120 == 0) { 
				   vmCorrelation.clear();
  				   if (RUN_MULTITHREAD) {
					   if (vmDestinationDataFlowKeyVM.size() != 0) {
						  for (iteMapKeyVM = vmDestinationDataFlowKeyVM.begin(); iteMapKeyVM != vmDestinationDataFlowKeyVM.end(); ++iteMapKeyVM) {
							  vmDestination.push_back(iteMapKeyVM->first);
						  }
						  if (vmDestination.size() < 8) {
							 thread_L[0] = 0;
							 thread_R[0] = vmDestination.size();
							 for (int i = 1; i < 8; i++) {
								 thread_L[i] = 0;
								 thread_R[i] = 0;
							 }
							 for (int i = 0; i < 8; i++) {
								 sem_post(&semaphore_CO);
							 }
							 pthread_barrier_wait(&barrier_CO);
						  }
						  else {
  							 int c_interval = vmDestination.size() / 8;
							 for (int i = 0; i < 8; i++) {
								 thread_L[i] = i * c_interval;
								 thread_R[i] = i * c_interval + c_interval;
							 }
							 thread_R[7] = vmDestination.size();
							 for (int i = 0; i < 8; i++) {
								 sem_post(&semaphore_CO);
							 }
							 pthread_barrier_wait(&barrier_CO);
						  }
						  vmDestination.clear();
					   }
				   }
				   else {
					   if (vmDestinationDataFlowKeyVM.size() != 0) {
						  CalculateCorrelation();
					   }
				   }
			   }
			}
		}
		//-------------------executar otimizacao de topologia---------------------------------
		if (SIMULATE_TRAFFIC_MATRIX) {
			if (SIMULATE_TOPOLOGY_OPTIMIZATION) {
				if ( (clock != 0) && (clock%PERIODIC_INTERVAL_EXECUTION_TOPOLOGY == 0) ) {
				   topologyOptimization->OptimizationTopology();
				}
			}
		}
	}

	if (totalVMsDC > maximumNumberSimultaneousVMs) {
		maximumNumberSimultaneousVMs = totalVMsDC;
	}

	// logs
	if ((clock % PERIODIC_LOG_INTERVAL) == 0) {
		vSupplyTemperatureSparseLog.push_back(fpSupplyTemperatureLog / PERIODIC_LOG_INTERVAL);
		fpSupplyTemperatureLog = 0.0;

		vTotalPowerFromComputingMachinesSparseLog.push_back(fpTotalPowerFromComputingMachinesLog / PERIODIC_LOG_INTERVAL);
		fpTotalPowerFromComputingMachinesLog = 0.0;
		vTotalPowerFromServerFansSparseLog.push_back(fpTotalPowerFromServerFansLog / PERIODIC_LOG_INTERVAL);
		fpTotalPowerFromServerFansLog = 0.0;

		vTotalComputingPowerSparseLog.push_back(fpTotalComputingPowerLog / PERIODIC_LOG_INTERVAL);
		fpTotalComputingPowerLog = 0.0;

		vTotalPowerFromCRACSparseLog.push_back(fpTotalPowerFromCRACLog / PERIODIC_LOG_INTERVAL);
		fpTotalPowerFromCRACLog = 0.0;

		vUtilizationSparseLog.push_back(fpUtilizationLog / PERIODIC_LOG_INTERVAL / NUMBER_OF_CHASSIS / NUMBER_OF_SERVERS_IN_ONE_CHASSIS);
		fpUtilizationLog = 0.0;

		vTotalMigrationsPolicyLowUtilizationSparseLog.push_back(vTotalMigrationPolicyLowUtilizationLog);
		vTotalMigrationsPolicyTemperatureSparseLog.push_back(vTotalMigrationPolicyTemperatureLog);
		vTotalMigrationsPolicyOverLoadedSparseLog.push_back(vTotalMigrationPolicyOverLoadedLog);
		vTotalMigrationsPolicyPredictionSparseLog.push_back(vTotalMigrationPolicyPredictionLog);
		vTotalMigrationsPolicyLinkOverloadSparseLog.push_back(vTotalMigrationsPolicyLinkOverloadLog);
		
		vTotalMigrationPolicyLowUtilization += vTotalMigrationPolicyLowUtilizationLog;
		vTotalMigrationPolicyTemperature += vTotalMigrationPolicyTemperatureLog;
		vTotalMigrationPolicyOverLoaded += vTotalMigrationPolicyOverLoadedLog;
		vTotalMigrationPolicyPrediction += vTotalMigrationPolicyPredictionLog;
		vTotalMigrationsPolicyLinkOverload += vTotalMigrationsPolicyLinkOverloadLog;

		
		vTotalMigrationPolicyLowUtilizationLog = 0;
		vTotalMigrationPolicyTemperatureLog = 0;
		vTotalMigrationPolicyOverLoadedLog = 0;
		vTotalMigrationPolicyPredictionLog = 0;
		vTotalMigrationsPolicyLinkOverloadLog = 0;
	}

	fpSupplyTemperatureLog += currentSupplyTemp;
	FLOATINGPOINT totalPowerDrawIT = TotalPowerDrawFromComputingMachines();
	FLOATINGPOINT totalPowerDrawFans = TotalPowerDrawFromServerFans();
	fpTotalPowerFromComputingMachinesLog += totalPowerDrawIT;
	fpTotalPowerFromServerFansLog += totalPowerDrawFans;
	FLOATINGPOINT totalPowerDrawCRAC = CalculateCurrentCRACPower(totalPowerDrawIT);
	fpTotalPowerFromCRACLog += totalPowerDrawCRAC;
	if (peakPower < (totalPowerDrawCRAC + totalPowerDrawIT))
		peakPower = totalPowerDrawCRAC + totalPowerDrawIT;

	fpTotalComputingPowerLog += TotalComputingPower();
	for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
		for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			perServerPowerLog[i] += pServers[i][j]->GetPowerDraw();
			perServerComputingPowerLog[i] += pServers[i][j]->MaxUtilization();
		}
		perServerTemperatureLog[i] += pServers[i][0]->CurrentInletTemperature();
	}
	FLOATINGPOINT sum = 0.0;
	for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
		for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			sum += pServers[i][j]->CurrentUtilization();
		}
	}
	perDataCenterUtilization += sum;
	fpUtilizationLog += sum;
}

void DataCenter::Finish(void)
{

		vTotalMigrationsPolicyLowUtilizationSparseLog.push_back(vTotalMigrationPolicyLowUtilizationLog);
		vTotalMigrationsPolicyTemperatureSparseLog.push_back(vTotalMigrationPolicyTemperatureLog);
		vTotalMigrationsPolicyOverLoadedSparseLog.push_back(vTotalMigrationPolicyOverLoadedLog);
		vTotalMigrationsPolicyPredictionSparseLog.push_back(vTotalMigrationPolicyPredictionLog);
		vTotalMigrationsPolicyLinkOverloadSparseLog.push_back(vTotalMigrationsPolicyLinkOverloadLog);
		
		vTotalMigrationPolicyLowUtilization += vTotalMigrationPolicyLowUtilizationLog;
		vTotalMigrationPolicyTemperature += vTotalMigrationPolicyTemperatureLog;
		vTotalMigrationPolicyOverLoaded += vTotalMigrationPolicyOverLoadedLog;
		vTotalMigrationPolicyPrediction += vTotalMigrationPolicyPredictionLog;
		vTotalMigrationsPolicyLinkOverload += vTotalMigrationsPolicyLinkOverloadLog;

		vTotalMigrationPolicyLowUtilizationLog = 0;
		vTotalMigrationPolicyTemperatureLog = 0;
		vTotalMigrationPolicyOverLoadedLog = 0;
		vTotalMigrationPolicyPredictionLog = 0;
		vTotalMigrationsPolicyLinkOverloadLog = 0;

}

void DataCenter::Controller(void)
{
	vector<STRUCOPTIMIZATION> ServerActiveDC;
	vector<STRUCMIGRATION> listVMsOverLoaded;
	vector<STRUCMIGRATION> listVMsLow;
	vector<STRUCMIGRATION> listVMsTemperature;
	vector<STRUCMIGRATION> listVMsLinkOverload;

	vector<VirtualMachine *> VM;

	vector<double> predictionTemp;

	STRUCOPTIMIZATION temp;
	STRUCMIGRATION add;

	FLOATINGPOINT utilDC = 0;

	// Assign jobs to the servers
	pSchedulingAlgorithm->AssignVMs();

	// Select VMs to Migrate
	if (SIMULATES_MIGRATION_VMS) {
		
		for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
		    for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
                if ((pServers[i][j]->IsOFF()) || (pServers[i][j]->IsHibernating()) || (pServers[i][j]->IsENDING()) || (pServers[i][j]->IsPOOL()) || (pServers[i][j]->IsINITIALIZING())) {
					continue;
				}
			    utilDC += pServers[i][j]->CurrentUtilization();
		    }
	    }
		utilDC = utilDC / 500;

		for (int i = 0; i<NUMBER_OF_CHASSIS; i++) {
			for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; j++) {

				if ((pServers[i][j]->IsOFF()) || (pServers[i][j]->IsHibernating()) || (pServers[i][j]->IsENDING()) || (pServers[i][j]->IsPOOL()) || (pServers[i][j]->IsMIGRATING()) || (pServers[i][j]->IsINITIALIZING())) {
					continue;
				}

				temp.chassi = i;
				temp.server = j; 
				temp.temperature = pServers[i][j]->CurrentInletTemperature();
				temp.utilizationCPU = pServers[i][j]->VMRequiresThisMuchUtilization();
				temp.averageUtilizationCPU = pServers[i][j]->AverageUsageofCPU();
				temp.utilizationMemory = pServers[i][j]->VMRequiresThisMemory(); 
				temp.memoryServer = pServers[i][j]->GetMemoryServer();
				temp.trafficKBPS = pServers[i][j]->ReturnServerTrafficKBPS();
				temp.averageTrafficKBPS = pServers[i][j]->AverageServerTrafficKBPS();
				temp.speedKBPS = pServers[1][j]->ReturnBandWidthServerKBPS();
				temp.power = pServers[i][j]->GetPowerDraw();
				temp.temperatureFuture = 0; 
				temp.predictedMigration = false; 
				temp.isMigrate = false;
				temp.dcload = utilDC;

				temp.ranking = pServers[i][j]->CalculationRanking(pServers[i][j]->CurrentInletTemperature(), pServers[i][j]->GetPowerDraw(), utilDC); 
				temp.rankingWeight = (SCHEDULER_2D_WEIGHT_TEMPERATURE * (temp.temperature/34)) + (SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION * (HRM[i]/HRFHight)) + (SCHEDULER_2D_WEIGHT_LOAD_CPU * (temp.utilizationCPU));
					
				
				
				// Begin Emergency DC
				if (temp.utilizationCPU >= 1) {
					VM = pServers[i][j]->GetNVMs(1, VMS_ALGORITHM_SELECTION);
					if (VM.size() != 0) {
						add.chassi = i; add.server = j; add.VM = VM[0]; add.utilizationCPU = temp.utilizationCPU;
						listVMsOverLoaded.push_back(add);
						temp.isMigrate = true;
						VM.clear();
					}
				}

				if ((temp.temperature >= EMERGENCY_TEMPERATURE) && (!temp.isMigrate)) {
					if (pServers[i][j]->HasVMs()) {
						VM = pServers[temp.chassi][temp.server]->GetNVMs(1, VMS_ALGORITHM_SELECTION);
						if (VM.size() != 0) {
							add.chassi = i; add.server = j; add.VM = VM[0]; add.utilizationCPU = temp.utilizationCPU;
							listVMsTemperature.push_back(add);
							temp.isMigrate = true;
							VM.clear();
						}
					}
				}

				if ( (SIMULATES_NETWORK) && (SIMULATE_TRAFFIC_MATRIX) ) {
					if (pServers[i][j]->ReturnServerTrafficKBPS() >= pServers[1][j]->ReturnBandWidthServerKBPS()) {
						VM = pServers[temp.chassi][temp.server]->GetNVMs(1, VMS_ALGORITHM_SELECTION_OVERLOADED_LINK);
						if (VM.size() != 0) {
							add.chassi = i; add.server = j; add.VM = VM[0]; add.utilizationCPU = temp.utilizationCPU;
							listVMsLinkOverload.push_back(add);
							temp.isMigrate = true;
							VM.clear();
						}
					}
				}
				// End Emergency DC


				if (clock%PERIODIC_INTERVAL_EXECUTION_OVERLOAD == 0) {
				   if (USING_PREDICTION_TO_OVERLOAD) {
					  if ((temp.utilizationCPU > 0.75) && (!temp.isMigrate)) {
						 if (pServers[i][j]->ReturnCPUPrediction()) {
						 	VM = pServers[i][j]->GetNVMs(1, VMS_ALGORITHM_SELECTION);
							if (VM.size() != 0) {
							   add.chassi = i; add.server = j; add.VM = VM[0]; add.utilizationCPU = temp.utilizationCPU;
							   listVMsOverLoaded.push_back(add);
							   temp.isMigrate = true;
							   VM.clear();
							}
						 }
					  }
				   }

				   if ((temp.utilizationCPU > THRESHOLD_TOP_OF_USE_CPU) && (!temp.isMigrate)) {
				      if ((temp.averageUtilizationCPU + pServers[i][j]->ReturnStandardDeviationCPU()) > 1) {
					     VM = pServers[i][j]->GetNVMs(1, VMS_ALGORITHM_SELECTION);
						 if (VM.size() != 0) {
						    add.chassi = i; add.server = j; add.VM = VM[0]; add.utilizationCPU = temp.utilizationCPU;
							listVMsOverLoaded.push_back(add);
							temp.isMigrate = true;
							VM.clear();
						 }
					  }
				   }
				}

				if (clock%PERIODIC_INTERVAL_EXECUTION_LOW == 0) {
					if ((temp.utilizationCPU < THRESHOLD_BOTTOM_OF_USE_CPU) && (!temp.isMigrate)) {
						VM = pServers[i][j]->GetALLVMs();
						if (VM.size() != 0) {
							for (int k = 0; k <  static_cast<int> (VM.size()); k++) {
								add.chassi = i; add.server = j; add.VM = VM[k]; add.utilizationCPU = temp.utilizationCPU;
								listVMsLow.push_back(add);
							}
							temp.isMigrate = true;
							VM.clear();
						}
					}
				}

				if (clock%PERIODIC_INTERVAL_EXECUTION_TEMPERATURE == 0) {
					if ((temp.temperature > EMERGENCY_TEMPERATURE - 0.10) && (!temp.isMigrate)) {
						if (pServers[i][j]->HasVMs()) {
							VM = pServers[temp.chassi][temp.server]->GetNVMs(1, VMS_ALGORITHM_SELECTION);
							if (VM.size() != 0) {
								add.chassi = i; add.server = j; add.VM = VM[0]; add.utilizationCPU = temp.utilizationCPU;
								listVMsTemperature.push_back(add);
								temp.isMigrate = true;
								VM.clear();
							}
						}
					}
				}

				if ( (SIMULATES_NETWORK) && (SIMULATE_TRAFFIC_MATRIX) ) {
					if (clock%PERIODIC_INTERVAL_EXECUTION_LINK_OVERLOAD == 0) {
						if ((!temp.isMigrate)) {
							if (pServers[i][j]->HasVMs()) {
								if (temp.trafficKBPS > (pServers[1][j]->ReturnBandWidthServerKBPS() * 0.80)) {
									VM = pServers[temp.chassi][temp.server]->GetNVMs(1, VMS_ALGORITHM_SELECTION_OVERLOADED_LINK);
									if (VM.size() != 0) {
										add.chassi = i; add.server = j; add.VM = VM[0]; add.utilizationCPU = temp.utilizationCPU;
										listVMsLinkOverload.push_back(add);
										temp.isMigrate = true;
										VM.clear();
									}
								}
							}
						}
					}
				}

				if (!temp.isMigrate) {
					ServerActiveDC.push_back(temp);
				}
			}
		}
	}

	// migration of VMs and Optimization of servers
	if (SIMULATES_MIGRATION_VMS) {
		if ((clock%PERIODIC_INTERVAL_EXECUTION_OVERLOAD == 0) && (clock != 0)) {
			policyOverLoaded->ServerOptimization(clock, &listVMsOverLoaded, &ServerActiveDC);
			vTotalMigrationPolicyOverLoadedLog += policyOverLoaded->ReturnTotalMigrationOverLoaded();
			vTotalMigrationPolicyPredictionLog += policyOverLoaded->ReturnTotalMigrationPrecdiction();
		}

		if ((clock%PERIODIC_INTERVAL_EXECUTION_LOW == 0) && (clock != 0)) {
			policyLow->ServerOptimization(clock, &listVMsLow, &ServerActiveDC);
			vTotalMigrationPolicyLowUtilizationLog += policyLow->ReturnTotalMigrationLowUtilization();
		}
		if ((clock%PERIODIC_INTERVAL_EXECUTION_TEMPERATURE == 0) && (clock != 0)) {
			policyHightTemperature->ServerOptimization(clock, &listVMsTemperature, &ServerActiveDC);
			vTotalMigrationPolicyTemperatureLog += policyHightTemperature->ReturnTotalMigrationHighTemperature();
		}

		if ((clock%PERIODIC_INTERVAL_EXECUTION_LINK_OVERLOAD == 0) && (clock != 0)) {
			policyNetwork->ServerOptimization(clock, &listVMsLinkOverload, &ServerActiveDC);
			vTotalMigrationsPolicyLinkOverloadLog += policyNetwork->ReturnTotalMigrationLinkOverload();
		}

		listVMsLow.clear();
		listVMsOverLoaded.clear();
		listVMsTemperature.clear();
		listVMsLinkOverload.clear();
		ServerActiveDC.clear();
	}
}

void * DataCenter::EveryASecondServer(void *args)
{
	struct Parametros *par = (struct Parametros*)args;

	while (true) {
		sem_wait(par->semaphore_Thread);
		for (int i = par->l; i < par->r; i++) {
			for (int j = 0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if (SCHEDULING_WITH_PREDICTION) {
					if (*par->clock == (*par->tServers)[i][j]->ReturnFirstTimePredictionServer()) { // calculte RMSD
						(*par->tServers)[i][j]->CalculateRMSE();
					}
				}

				(*par->tServers)[i][j]->EveryASecond(*par->clock);


				if (((*par->tServers)[i][j]->IsOFF()) || ((*par->tServers)[i][j]->IsHibernating()) || ((*par->tServers)[i][j]->IsENDING()) || (!(*par->tServers)[i][j]->IsMIGRATING()) || ((*par->tServers)[i][j]->IsPOOL()) || ((*par->tServers)[i][j]->IsINITIALIZING())) {
					continue;
				}

				if (SIMULATES_MIGRATION_VMS) {
					// check if a full migration finished
					(*par->tServers)[i][j]->CheckFinishMove(*par->clock);

					// modify status server 
					if (!(*par->tServers)[i][j]->HasVMsToMigrate()) {
						(*par->tServers)[i][j]->SetMigrating(false);
					}
				}
			}
		}
		pthread_barrier_wait(par->barrier_Thread);
	}
}



void * DataCenter::WriteTemperatureToEveryServer(void *args)
{
	struct Parametros_TP *par = (struct Parametros_TP*)args;

	unsigned int local_temperature = 0;
	unsigned int local_utilization = 0;

	while (true) {
		sem_wait(par->semaphore_Thread);

		// Write supply temperature to every server
		for (int i = par->l; i < par->r; i++) {
			for (int j = 0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; j++) {
				// use matrix only 
				if (INSTANT_COOL_AIR_TRAVEL_TIME) {
					(*par->tServers)[i][j]->SetSupplyTempToTimingBuffer(*par->currentSupplyTemp, 0);
				}
				else {
					(*par->tServers)[i][j]->SetSupplyTempToTimingBuffer(*par->currentSupplyTemp, par->ModedAirTravelTimeFromCRAC[i]);
				}
			}
		}

		for (int i = par->l; i < par->r; i++) {
			for (int j = 0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; j++) {
				if (((*par->tServers)[i][j]->IsOFF()) || ((*par->tServers)[i][j]->IsHibernating())) {
					continue;
				}

				local_temperature = (unsigned int)((*par->tServers)[i][j]->CurrentInletTemperature());

				if (local_temperature > 100) {
					cout << "SimDC3D-ERROR: Local_temperature too big: Servidor " << i << " " << j << " Temperature " << local_temperature << " !!!" << endl;
					exit(0);
				}
				else {
					par->HowManySecondsInThisInletAirTemperature[local_temperature]++;
				}

				local_utilization = (unsigned int)((*par->tServers)[i][j]->CurrentUtilization() * 100);


				if (local_utilization > 100) {
					cout << "SimDC3D-ERROR: Local_utilization too big !!! " << local_utilization << endl;
					exit(0);
				}
				else {
					par->HowManySecondsInThisUtilization[local_utilization]++;
				}

			}
		}
		pthread_barrier_wait(par->barrier_Thread);
	}
}

void * DataCenter::RunTopologySimDC3DEveryASecond(void *args)
{
	struct Parametros_TO *par = (struct Parametros_TO*)args;

	while (true) {
		sem_wait(par->semaphore_Thread);
		par->topologySimDC3D->EveryASecondMultiThread_P2(par->l, par->r, par->tServers, par->mutex);
		pthread_barrier_wait(par->barrier_Thread);
	}
}

void * DataCenter::CalculateVarianceServer(void *args)
{
	struct Parametros_VA *par = (struct Parametros_VA*)args;

	while (true) {
		sem_wait(par->semaphore_Thread);
		for (int i = par->l; i < par->r; i++) {
			for (int j = 0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if (((*par->tServers)[i][j]->IsOFF()) || ((*par->tServers)[i][j]->IsHibernating()) || ((*par->tServers)[i][j]->IsENDING()) || ((*par->tServers)[i][j]->IsINITIALIZING())) {
					continue;
				}
				(*par->tServers)[i][j]->CalculatingStandardDeviationCPU();
			}
		}
		pthread_barrier_wait(par->barrier_Thread);
	}
}


FLOATINGPOINT DataCenter::CalculateCurrentCRACPower(FLOATINGPOINT totalPowerDrawIT)
{
	FLOATINGPOINT cop = 0.0068 * currentSupplyTemp * currentSupplyTemp + 0.0008 * currentSupplyTemp + 0.458;
	return (totalPowerDrawIT / cop);
}

FLOATINGPOINT DataCenter::TotalComputingPower()
{
	FLOATINGPOINT retval = 0.0;
	for (int i = 0; i<NUMBER_OF_CHASSIS; ++i)
		for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j)
			retval += pServers[i][j]->MaxUtilization();
	return retval;
}

FLOATINGPOINT DataCenter::TotalPowerDrawFromComputingMachines()
{
	FLOATINGPOINT retval = 0.0;
	for (int i = 0; i<NUMBER_OF_CHASSIS; ++i)
		for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j)
			retval += pServers[i][j]->GetPowerDraw();
	return retval;
}

FLOATINGPOINT DataCenter::TotalPowerDrawFromServerFans()
{
	FLOATINGPOINT retval = 0.0;
	for (int i = 0; i<NUMBER_OF_CHASSIS; ++i)
		for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j)
			retval += pServers[i][j]->GetFanPower();
	return retval;
}

void DataCenter::RecalculateHeatDistribution()
{
	// calcuate power draw of each CHASSIS
	FLOATINGPOINT powerDraw[SIZE_OF_HR_MATRIX];
	for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
		powerDraw[i] = 0.0;
		for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			powerDraw[i] += pServers[i][j]->GetPowerDraw();
		}
	}

	// calculate added heat
	for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
		for (int j = 0; j<NUMBER_OF_CHASSIS; ++j) {
			FLOATINGPOINT heatFromJtoI = powerDraw[j] * HeatRecirculationMatrixD[i][j];
			for (int k = 0; k<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++k) {
				pServers[i][k]->AddHeatToTimingBuffer(heatFromJtoI, 0);
			}
		}
	}
}

bool DataCenter::IsFinished()
{
	for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
		for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			if (!(pServers[i][j]->IsFinished()))
				return false;
		}
	}
	return true;
}

void DataCenter::PrintResults(POOLServers* pPool)
{
	double var = 0.0000;
	double averageArrivalTime = 0.00;
	double averageRunTime = 0.00;
	double averageTimeSchedulingQueue = 0.00;
	unsigned int sumErrorPrediction = 0;
	unsigned int sumHitPrediction = 0;
	//double eMean = 0.00;
	double sladowntime = 0.00;
	double slat = 0.00;
	int tVMsMigrate = 0;
	int min = 0;
	int hou = 0;
	int day = 0;

	min = (int)(clock / 60);
	hou = (int)(min / 60);
	day = (int)(hou / 24);

	// print timing information
	//cout << "Total time clock running simulator seg / min / hours / day : " << clock << " / " << (clock/60) << " / " << (clock/60)/60 << ((clock/60)/60)/24 << endl;
	cout << "Total time clock running simulator seg / min / hours / day : " << clock << " / " << min << " / " << hou << " / " << day << endl;
	cout << "Total Computing time seg / min / hours : " << difftime(time(NULL), timeIni) << " / " << (difftime(time(NULL), timeIni) / 60) << " / " << ((difftime(time(NULL), timeIni) / 60) / 60) << endl;

	//Trace Statistics

	cout << endl;

	cout << "Trace Statistics" << endl << endl;

	averageArrivalTime = (double)avgArrivalTime / (double)varianceArrivalTime.size();

	cout << "Average time between arrivals of VMs: " << averageArrivalTime << endl;

	for (unsigned int i = 0; i < varianceArrivalTime.size(); i++) {
		var += (pow((double(varianceArrivalTime[i]) - averageArrivalTime), 2));
	}
	cout << "Variance between arrivals of VMs: " << (var / (varianceArrivalTime.size())) << endl;
	cout << "Standard deviation arrivals of VMs: " << sqrt(var / (varianceArrivalTime.size())) << endl << endl;

	averageRunTime = avgRunTime / (double)varianceRunTime.size();
	cout << "Average execution time of the VMs: " << averageRunTime << endl;

	var = 0.0000;
	for (unsigned int i = 0; i < varianceRunTime.size(); i++) {
		var += (pow((double(varianceRunTime[i]) - averageRunTime), 2));
	}
	cout << "Variance execution time of the VMs: " << (var / (varianceRunTime.size())) << endl;
	cout << "Standard deviation arrivals of VMs: " << sqrt((var / (varianceRunTime.size()))) << endl << endl;

	cout << endl;
	cout << "Maximum number of simultaneous VMs: " << maximumNumberSimultaneousVMs << endl;

	cout << endl;

	averageTimeSchedulingQueue = avgTimeSchedulingQueue / varianceTimeSchedulingQueue.size();

	cout << "Average time in the scheduling queue: " << averageTimeSchedulingQueue << endl;

	var = 0.0000;
	for (unsigned int i = 0; i < varianceTimeSchedulingQueue.size(); i++) {
		var += (pow((double(varianceTimeSchedulingQueue[i]) - averageTimeSchedulingQueue), 2));
	}

	cout << "Variance in the scheduling queue: " << (var / (varianceTimeSchedulingQueue.size())) << endl;
	cout << "Standard deviation in the scheduling queue: " << sqrt((var / (varianceTimeSchedulingQueue.size()))) << endl;

	cout << endl;

	if (SCHEDULING_WITH_PREDICTION) {
		cout << "Prediction Statistics" << endl << endl;

		for (int i = 0; i < NUMBER_OF_CHASSIS; ++i) {
			for (int j = 0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				sumErrorPrediction += pServers[i][j]->ReturnErrorPrediction();
				sumHitPrediction += pServers[i][j]->ReturnHitPrediction();
			}
		}

		cout << "Total of hits in the Prediciton Range: " << sumHitPrediction << endl;
		cout << "Total of error in the Prediciton Range: " << sumErrorPrediction << endl << endl;

		cout << "Root Mean Squared Error (RMSE) " << endl << endl;

		for (int i = 0; i < NUMBER_OF_CHASSIS; ++i) {
			cout << "Chassi " << i;
			for (int j = 0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if (pServers[i][j]->ReturnRMSE() >= 0) {
					cout << " " << pServers[i][j]->ReturnRMSE();
				}
				else {
					cout << " " << "0.000000";
				}
			}
			cout << endl;
		}

		cout << endl << endl;

		cout << "Average Error of predicted values and real " << endl << endl;

		for (int i = 0; i < NUMBER_OF_CHASSIS; ++i) {
			cout << "Chassi " << i;
			for (int j = 0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if (pServers[i][j]->ReturnErrorMean() >= 0) {
					cout << " " << pServers[i][j]->ReturnErrorMean() * 100 << "%";
				}
				else {
					cout << " " << "0.00000" << "%";
				}
			}
			cout << endl;
		}

		cout << endl << endl;

		cout << "Variance of predicted values and real " << endl << endl;

		for (int i = 0; i < NUMBER_OF_CHASSIS; ++i) {
			cout << "Chassi " << i;
			for (int j = 0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if (pServers[i][j]->ReturnVarianceErrorMean() > 0) {
					cout << " " << pServers[i][j]->ReturnVarianceErrorMean();
				}
				else {
					cout << " " << "0.000000000";
				}
			}
			cout << endl;
		}

		cout << endl << endl;

		cout << "Standard Deviation of predicted values and real " << endl << endl;

		for (int i = 0; i < NUMBER_OF_CHASSIS; ++i) {
			cout << "Chassi " << i;
			for (int j = 0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if (pServers[i][j]->ReturnSDErrorMean() > 0) {
					cout << " " << pServers[i][j]->ReturnSDErrorMean();
				}
				else {
					cout << " " << "0.0000000";
				}
			}
			cout << endl;
		}

		cout << endl << endl;

		cout << "Coefficient of variation of predicted values and real " << endl << endl;

		for (int i = 0; i < NUMBER_OF_CHASSIS; ++i) {
			cout << "Chassi " << i;
			for (int j = 0; j < NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ((pServers[i][j]->ReturnSDErrorMean() / pServers[i][j]->ReturnErrorMean()) >= 0) {
					cout << " " << pServers[i][j]->ReturnSDErrorMean() / pServers[i][j]->ReturnErrorMean();
				}
				else {
					cout << " " << "0.00000";
				}
			}
			cout << endl;
		}
		cout << endl << endl;
	}

	if (SIMULATES_MIGRATION_VMS) {
		cout << "Statistics of Migration Policies " << endl << endl;

		cout << "Total Migrations Overloaded Utilization: " << vTotalMigrationPolicyOverLoaded << endl;

		cout << "Total Migrations  Overloaded Utilization (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
		for (unsigned int i = 0; i<vTotalMigrationsPolicyOverLoadedSparseLog.size(); ++i)
			cout << vTotalMigrationsPolicyOverLoadedSparseLog[i] << "\t";
		cout << endl << endl;

		cout << "Total Migrations Precdition: " << vTotalMigrationPolicyPrediction << endl;

		cout << "Total Migrations  Precdition (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
		for (unsigned int i = 0; i<vTotalMigrationsPolicyPredictionSparseLog.size(); ++i)
			cout << vTotalMigrationsPolicyPredictionSparseLog[i] << "\t";
		cout << endl << endl;

		cout << "Total Migrations Low Utilization: " << vTotalMigrationPolicyLowUtilization << endl;

		cout << "Total Migrations Low Utilization (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
		for (unsigned int i = 0; i<vTotalMigrationsPolicyLowUtilizationSparseLog.size(); ++i)
			cout << vTotalMigrationsPolicyLowUtilizationSparseLog[i] << "\t";
		cout << endl << endl;

		cout << "Total Migrations Hight Temperature: " << vTotalMigrationPolicyTemperature << endl;

		cout << "Total Migrations Hight Temperature (every " << PERIODIC_LOG_INTERVAL << " secs)" << endl;
		for (unsigned int i = 0; i<vTotalMigrationsPolicyTemperatureSparseLog.size(); ++i)
			cout << vTotalMigrationsPolicyTemperatureSparseLog[i] << "\t";
		cout << endl << endl;


		cout << "Total Migrations Link OverloadHight: " << vTotalMigrationsPolicyLinkOverload << endl;

		cout << "Total Migrations Link OverloadHight (every " << PERIODIC_LOG_INTERVAL << " secs)" << endl;
		for (unsigned int i = 0; i<vTotalMigrationsPolicyLinkOverloadSparseLog.size(); ++i)
			cout << vTotalMigrationsPolicyLinkOverloadSparseLog[i] << "\t";
		cout << endl << endl;


		cout << "Total idle servers turn off: " << pPool->TotalPowerOffServer() << endl;
		pPool->PrintPowerOffServer();

		cout << endl;

		cout << "Total Migrations per server" << endl;
		for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
			cout << " Chassi " << i << " ";
			for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				cout << " " << pServers[i][j]->ReturnTotalVMsMigrate();
			}
			cout << endl;
		}
		cout << endl << endl;

		cout << "Statistics of Migration Policies " << endl << endl;
		cout << "SLA Violation " << endl;

		for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
			cout << " Chassi " << i << " ";
			for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if (pServers[i][j]->ReturntimeServerPowerOn() != 0) {
					cout << " " << ((double)pServers[i][j]->ReturnTimeServerOverLoaded() / (double)pServers[i][j]->ReturntimeServerPowerOn());
					slat += ((double)pServers[i][j]->ReturnTimeServerOverLoaded() / (double)pServers[i][j]->ReturntimeServerPowerOn());
				}
				else {
					cout << " " << "0.00";
				}
			}
			cout << endl;
		}
		cout << endl;
		cout << "Sum slat " << slat << endl;
		cout << "SLAT " << (((double)1 / ((double)(NUMBER_OF_CHASSIS*NUMBER_OF_SERVERS_IN_ONE_CHASSIS)))*slat) * 100;
		cout << endl << endl;

		cout << "VM DownTime " << endl;

		for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
			cout << " Chassi " << i << " ";
			for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if (pServers[i][j]->ReturnAverageDowmTimeServer() != 0) {
					cout << " " << ((double)1 - pServers[i][j]->ReturnAverageDowmTimeServer()) * 100;
					sladowntime += (double)1 - pServers[i][j]->ReturnAverageDowmTimeServer();
				}
				else {
					cout << " " << "0.00";
					sladowntime += 0;
				}
			}
			cout << endl;
		}
		cout << endl << endl;

		cout << "Average SLA DownTime per Active Host " << (sladowntime / ((double)(NUMBER_OF_CHASSIS*NUMBER_OF_SERVERS_IN_ONE_CHASSIS))) * 100;
		cout << endl << endl;
	}


	cout << "Statistics of the Virtual Machines " << endl << endl;

	cout << "Total VMs arrived in the Data Center: " << totalVMs << endl;
	cout << "Total VMs scheduling in the Data Center: " << pSchedulingAlgorithm->returnTotalScheduling() << endl;

	for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
		for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			tVMsMigrate += pServers[i][j]->ReturnTotalVMsMigrate();
		}
	}
	cout << "Total VMs finish in the Data Center: " << totalVMsFinish << endl;
	if (SIMULATES_MIGRATION_VMS)
		cout << "Total VMs that have not completed the migration because finish: " << totalVMsMigratingANDFinish << endl;
	cout << "Total VMs Migrates: " << tVMsMigrate << endl;
	cout << endl << endl;

	cout << "Statistics from the average energy consumption of data centers" << endl << endl;
	// print average supply temperature
	FLOATINGPOINT sum = 0.0;
	FLOATINGPOINT sum2 = 0.0;
	FLOATINGPOINT sum3 = 0.0;
	FLOATINGPOINT sum4 = 0.0;
	FLOATINGPOINT sum5 = 0.0;
	FLOATINGPOINT sum6 = 0.0;
	FLOATINGPOINT sum7 = 0.0;
	FLOATINGPOINT sum8 = 0.0;
	FLOATINGPOINT sum9 = 0.0;

	for (size_t i = 0; i<vSupplyTemperatureSparseLog.size(); ++i)
		sum += vSupplyTemperatureSparseLog[i];
	cout << "Average Supply Temperature: " << (sum / vSupplyTemperatureSparseLog.size()) << endl << endl;;

	// print peakPower
	cout << "peakPower: " << peakPower << endl << endl;;

	// Average Power Consumption
	sum = 0.0;
	for (size_t i = 0; i<vTotalPowerFromComputingMachinesSparseLog.size(); ++i)
		sum += vTotalPowerFromComputingMachinesSparseLog[i];
	cout << "Average Power Consumption (Servers = Fans + Computing components): " << (sum / vTotalPowerFromComputingMachinesSparseLog.size()) << endl;

	sum3 = 0.0;
	for (size_t i = 0; i<vTotalPowerFromServerFansSparseLog.size(); ++i)
		sum3 += vTotalPowerFromServerFansSparseLog[i];
	cout << "\tAverage Power Consumption (Fans): " << (sum3 / vTotalPowerFromServerFansSparseLog.size()) << endl;
	cout << "\tAverage Power Consumption (Computing components): " << (sum / vTotalPowerFromComputingMachinesSparseLog.size()) - (sum3 / vTotalPowerFromServerFansSparseLog.size()) << endl << endl;

	sum2 = 0.0;
	for (size_t i = 0; i<vTotalPowerFromCRACSparseLog.size(); ++i)
		sum2 += vTotalPowerFromCRACSparseLog[i];
	cout << "Average Power Consumption (CRAC): " << (sum2 / vTotalPowerFromCRACSparseLog.size()) << endl << endl;

	if (SIMULATES_NETWORK) {
		sum4 = pTopology->ReturnTotalEnergyAccessSwitchSparseLog() / pTopology->ReturnSizeVectorTotalEnergyAccessSwitch();
		cout << "Average Power Consumption (Switch Access): " << sum4 << endl;;
		sum5 = pTopology->ReturnTotalEnergyAggregationSwitchSparseLog() / pTopology->ReturnSizeVectorTotalEnergyAggregationSwitch();
		cout << "Average Power Consumption (Switch Aggregation): " << sum5 << endl;
		sum6 = pTopology->ReturnTotalEnergyCoreSwitchSparseLog() / pTopology->ReturnSizeVectorTotalEnergyCoreSwitch();
		cout << "Average Power Consumption (Switch Core): " << sum6 << endl;
		sum7 = (pTopology->ReturnTotalEnergyAccessSwitchSparseLog() + pTopology->ReturnTotalEnergyAggregationSwitchSparseLog() + pTopology->ReturnTotalEnergyCoreSwitchSparseLog()) / pTopology->ReturnSizeVectorTotalEnergyAccessSwitch();
		cout << "Average Power Consumption (Total = Switch Access + Switch Aggregation + Switch Core): " << sum7 << endl << endl;

		cout << "Average Power Consumption (Total = CRAC + Servers + Switches): " << ((sum + sum2) / vTotalPowerFromCRACSparseLog.size()) + sum7 << endl << endl;
	}
	else {
		cout << "Average Power Consumption (Total = CRAC + Servers): " << ((sum + sum2) / vTotalPowerFromCRACSparseLog.size()) << endl << endl;
	}


	cout << "Statistical Data Center Energy Consumption" << endl << endl;


	cout << "Energy (Servers = Fans + Computing components): " << (sum*PERIODIC_LOG_INTERVAL) << endl;
	cout << "\tEnergy (Fans): " << (sum3*PERIODIC_LOG_INTERVAL) << endl;
	cout << "\tEnergy (Computing components): " << ((sum - sum3)*PERIODIC_LOG_INTERVAL) << endl << endl;

	cout << "Energy (CRAC): " << (sum2*PERIODIC_LOG_INTERVAL) << endl << endl;

	if (SIMULATES_NETWORK) {
		sum8 = (pTopology->ReturnTotalEnergyAccessSwitch() + pTopology->ReturnTotalEnergyAggregationSwitch() + pTopology->ReturnTotalEnergyCoreSwitch());
		cout << "Energy (Total = Switch Access + Switch Aggregation + Switch Core): " << sum8 << endl;
		cout << "\tEnergy (Switch Access): " << pTopology->ReturnTotalEnergyAccessSwitch() << endl;
		cout << "\tEnergy (Switch Aggregation): " << pTopology->ReturnTotalEnergyAggregationSwitch() << endl;
		cout << "\tEnergy (Switch Core): " << pTopology->ReturnTotalEnergyCoreSwitch() << endl << endl;

		cout << "Energy (Total = CRAC + Servers + Switches): " << (((sum + sum2)*PERIODIC_LOG_INTERVAL) + sum8) << endl << endl;

		sum9 = pTopology->ReturnTotalEnergyAccessSwitchSparseLog() + pTopology->ReturnTotalEnergyAggregationSwitchSparseLog() + pTopology->ReturnTotalEnergyCoreSwitchSparseLog();
		cout << "PUE: " << ((sum + sum2 + sum9) / (sum + sum9)) << endl;
		cout << "tPUE: " << ((sum + sum2) / (sum - sum3)) << endl;
		cout << endl;
	}
	else {
		cout << "PUE: " << ((sum + sum2) / (sum)) << endl;
		cout << "tPUE: " << ((sum + sum2) / (sum - sum3)) << endl;
	}

	// average data center utilization
	cout << "Average Utilization (Data Center Level): " << (perDataCenterUtilization / NUMBER_OF_CHASSIS / NUMBER_OF_SERVERS_IN_ONE_CHASSIS / clock) << endl;

	// utilization log
	cout << "Utilization of the data center (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	for (size_t i = 0; i<vUtilizationSparseLog.size(); ++i)
		cout << vUtilizationSparseLog[i] << "\t";
	cout << endl << endl;

	// computing power log
	cout << "Total computing power (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	for (size_t i = 0; i<vTotalComputingPowerSparseLog.size(); ++i)
		cout << vTotalComputingPowerSparseLog[i] << "\t";
	cout << endl << endl;

	// per Chassis log
	cout << "Per Chassis Avg Power: " << endl;
	for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
		cout << (perServerPowerLog[i] / clock / NUMBER_OF_SERVERS_IN_ONE_CHASSIS) << "\t";
		if (i % 10 == 9)
			cout << endl;
	}
	cout << endl;
	cout << "Per Chassis Avg Computing Power (max utilization):" << endl;
	for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
		cout << (perServerComputingPowerLog[i] / clock / NUMBER_OF_SERVERS_IN_ONE_CHASSIS) << "\t";
		if (i % 10 == 9)
			cout << endl;
	}
	cout << endl;
	cout << "Per Chassis Avg Inlet Temperature: " << endl;
	for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
		cout << (perServerTemperatureLog[i] / clock) << "\t";
		if (i % 10 == 9)
			cout << endl;
	}
	cout << endl;
	cout << "Per Chassis Seconds (avg) Over Emergency Temp: " << endl;
	for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
		unsigned int timeOverEmergencyTemp = 0;
		for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j)
			timeOverEmergencyTemp += pServers[i][j]->HowManySecondsOverEmergencyTemp();
		cout << ((FLOATINGPOINT)timeOverEmergencyTemp / (FLOATINGPOINT)NUMBER_OF_SERVERS_IN_ONE_CHASSIS) << "\t";
		if (i % 10 == 9)
			cout << endl;
	}
	cout << endl;

	cout << "Per Server Seconds Over Emergency Temp: " << endl;
	for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
		cout << "Chassi " << i;
		for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			cout << " " << pServers[i][j]->HowManySecondsOverEmergencyTemp() << "\t";
		}
		cout << endl;
	}
	cout << endl;

	cout << "Per Server - How many times DVFS changed:" << endl;
	for (int i = 0; i<NUMBER_OF_CHASSIS; ++i) {
		for (int j = 0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j)
			cout << pServers[i][j]->HowManyTimesDVFSChanged() << "\t";
		if (i % 10 == 9)
			cout << endl;
	}
	cout << endl;

	// Power (every)
	cout << "Total Power Draw (Computing) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	for (size_t i = 0; i<vTotalPowerFromComputingMachinesSparseLog.size(); ++i)
		cout << vTotalPowerFromComputingMachinesSparseLog[i] << "\t";
	cout << endl << endl;

	cout << "Total Power Draw (CRAC) (every " << PERIODIC_LOG_INTERVAL << " secs): " << endl;
	for (size_t i = 0; i<vTotalPowerFromCRACSparseLog.size(); ++i)
		cout << (vTotalPowerFromCRACSparseLog[i]) << "\t";
	cout << endl << endl;

	if (SIMULATES_NETWORK) {
		cout << "Total Power Draw (Switch Access) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
		pTopology->PrintPowerDrawAccessSwitch();
		cout << endl;

		cout << "Total Power Draw (Switch Aggregation) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
		pTopology->PrintPowerDrawAggregationSwitch();
		cout << endl;

		cout << "Total Power Draw (Switch Core) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
		pTopology->PrintPowerDrawCoreSwitch();
		cout << endl;

		cout << "Total Power Draw (Total = Switch Access + Switch Aggregation + Switch Core) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
		pTopology->PrintTotalPowerDrawSwitches();
		cout << endl;

		cout << "Total Power Draw (Computing + CRAC + Switches) (every " << PERIODIC_LOG_INTERVAL << " secs)" << endl;
		for (size_t i = 0; i<vTotalPowerFromCRACSparseLog.size(); ++i)
			cout << (vTotalPowerFromComputingMachinesSparseLog[i] + vTotalPowerFromCRACSparseLog[i] + pTopology->ReturnPowerDrawSwitches(i)) << "\t";
		cout << endl << endl;
	}

	// energy (every)
	cout << "Total Energy Draw (Computing) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	for (size_t i = 0; i<vTotalPowerFromComputingMachinesSparseLog.size(); ++i)
		cout << (vTotalPowerFromComputingMachinesSparseLog[i] * PERIODIC_LOG_INTERVAL) << "\t";
	cout << endl << endl;

	cout << "Total Energy Draw (CRAC) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	for (size_t i = 0; i<vTotalPowerFromCRACSparseLog.size(); ++i)
		cout << (vTotalPowerFromCRACSparseLog[i] * PERIODIC_LOG_INTERVAL) << "\t";
	cout << endl << endl;

	if (SIMULATES_NETWORK) {
		cout << "Total Energy Draw (Switch Access) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
		pTopology->PrintTotalEnergyAccessSwitchSparseLog();
		cout << endl;

		cout << "Total Energy Draw (Switch Aggregation) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
		pTopology->PrintTotalEnergyAggregationSwitchSparseLog();
		cout << endl;

		cout << "Total Energy Draw (Switch Core) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
		pTopology->PrintTotalEnergyCoreSwitchSparseLog();
		cout << endl;

		cout << "Total Energy Draw (Total = Switch Access + Switch Aggregation + Switch Core) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
		pTopology->PrintTotalEnergySwitchesSparseLog();
		cout << endl;

		cout << "Total Energy Draw (Computing + CRAC + Switches) (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
		for (size_t i = 0; i<vTotalPowerFromCRACSparseLog.size(); ++i)
			cout << ((vTotalPowerFromComputingMachinesSparseLog[i] + vTotalPowerFromCRACSparseLog[i] + pTopology->ReturnPowerDrawSwitches(i))*PERIODIC_LOG_INTERVAL) << "\t";
		cout << endl << endl;
	}

	// print supply temperature log
	cout << "Supply Temperature (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	for (size_t i = 0; i<vSupplyTemperatureSparseLog.size(); ++i)
		cout << vSupplyTemperatureSparseLog[i] << "\t";
	cout << endl << endl;

	// print HowManySecondsInThisInletAirTemperature

	if (RUN_MULTITHREAD) {
		for (int i = 0; i < NUMBER_OF_THREADS - 1; i++) {
			for (int w = 0; w < 101; w++) {
				HowManySecondsInThisInletAirTemperature[w] += paramThread_TP[i].HowManySecondsInThisInletAirTemperature[w];
				HowManySecondsInThisUtilization[w] += paramThread_TP[i].HowManySecondsInThisUtilization[w];
			}
		}
	}
	cout << "Inlet Air Temperature Distribution of the server at the highest temperature (seconds):" << endl;
	for (unsigned int i = 0; i<100; ++i) {
		if (HowManySecondsInThisInletAirTemperature[i] != 0)
			cout << HowManySecondsInThisInletAirTemperature[i] << "\t" << i << endl;
	}
	cout << endl;

	// print HowManySecondsInThisUtilization
	cout << "Utilization Distribution (seconds) :" << endl;
	for (unsigned int i = 0; i<100; ++i) {
		cout << HowManySecondsInThisUtilization[i] << "\t" << i << endl;
	}
	cout << endl;

	if (SIMULATES_NETWORK) {
		cout << "Statistic Switches / Links " << endl << endl;
		pTopology->StatisticSwitch();
	}
}
/*
void DataCenter::PrintVector(void)
{
for (int i=0; i<NUMBER_OF_CHASSIS; i++) {
for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; j++) {
cout << "Chassi " << i << " Servidor " << j << "Numero de elementos no vetor" << pServers[i][j]->returnSizeVectorUtilizationCPU() << endl;
for (int k=0; k < pServers[i][j]->returnSizeVectorUtilizationCPU(); k++) {
cout << pServers[i][j]->returnPositionVectorUtilizationCPU(k) << " " << endl;
}
}
}

for (int i=0; i<NUMBER_OF_CHASSIS; i++) {
for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; j++) {
cout << "Chassi " << i << " Servidor " << j << "Numero de elementos no vetor" << pServers[i][j]->returnSizeVectorMemoryUseOfServe() << endl;
for (int k=0; k < pServers[i][j]->returnSizeVectorMemoryUseOfServe(); k++) {
cout << pServers[i][j]->returnPositionVectorMemoryUseOfServer(k) << " " << endl;
}
}
}

}*/

void DataCenter::CalculateCorrelation(void)
{
	//int ncolum = vmDestinationDataFlowKeyVM.size(); // Number of lines and columns in the matrix will be equal to the number of flows
	//int vmnumb = 0;

	vector<FLOATINGPOINT> vectorFirstCorr; // Stores the first 10 values of the first flow for the correlation calculation
	vector<FLOATINGPOINT> vectorSecondCorr; // Stores the first 10 values of the next flow for the correlation calculation
	
	vector<DataFlow *> vectorvmsdataflow1;
	vector<DataFlow *> vectorvmsdataflow2;

	map <VirtualMachine*, double> datavm;

	double correl = 0;

	std::map<VirtualMachine*, string>::iterator iter;
	std::map<VirtualMachine*, string>::iterator itertemp;
	map <VirtualMachine*, map<VirtualMachine*, double>>::iterator it;
	map <VirtualMachine*, double>::iterator iterat;

	vmCorrelation.clear();


	if (vmDestinationDataFlowKeyVM.size() == 0) {
		return;
	}

	for (iter = vmDestinationDataFlowKeyVM.begin(); iter != vmDestinationDataFlowKeyVM.end(); ++iter) {
		for (itertemp = vmDestinationDataFlowKeyVM.begin(); itertemp != vmDestinationDataFlowKeyVM.end(); ++itertemp) {
			if (iter->first == itertemp->first) {
				correl = 1;
				datavm.emplace(pair<VirtualMachine*, double>(itertemp->first, correl));
			}
			else {
				vectorvmsdataflow1 = (iter->first)->ReturnDataFlowVM();
				vectorvmsdataflow2 = (itertemp->first)->ReturnDataFlowVM();
				if (vectorvmsdataflow1.size() != 0) {
					vectorFirstCorr = vectorvmsdataflow1[0]->ReturnTrafficNetwork();
				}
				if (vectorvmsdataflow2.size() != 0) {
				   vectorSecondCorr = vectorvmsdataflow2[0]->ReturnTrafficNetwork();
				}
  		       if (vectorFirstCorr.size() < 10 || vectorSecondCorr.size() < 10) {
				  correl = 2; // If slidewindow < 10
				  datavm.emplace(pair<VirtualMachine*, double>(itertemp->first, correl));
			   }
			   else {
				  correl = pearson_correlation(&vectorFirstCorr[0], &vectorSecondCorr[0], vectorFirstCorr.size());
				  datavm.emplace(pair<VirtualMachine*, double>(itertemp->first, correl));
			   }
			}
		}
		vmCorrelation.emplace(pair<VirtualMachine*, map<VirtualMachine*, double>>(iter->first, datavm));
		datavm.clear();
	}
}

void * DataCenter::CalculateCorrelation_MT(void *args)
{
	struct Parametros_CO *par = (struct Parametros_CO*)args;

	vector<FLOATINGPOINT> vectorFirstCorr; // Stores the first 10 values of the first flow for the correlation calculation
	vector<FLOATINGPOINT> vectorSecondCorr; // Stores the first 10 values of the next flow for the correlation calculation
	vector<DataFlow *> vectorvmsdataflow1;
	vector<DataFlow *> vectorvmsdataflow2;
	
	map <VirtualMachine*, double> datavm;
	double correl = 0;

	std::map<VirtualMachine*, string>::iterator itertemp;


	while (true) {
		sem_wait(par->semaphore_Thread);
		for (int i = *par->l; i < *par->r; i++) {
			for (itertemp = (*par->vmDestinationDFlowKeyVM).begin(); itertemp != (*par->vmDestinationDFlowKeyVM).end(); ++itertemp) {
				if ((*par->vmDestinationDFlowVector)[i] == itertemp->first) {
					correl = 1;
					datavm.emplace(pair<VirtualMachine*, double>(itertemp->first, correl));
				}
				else {
					vectorvmsdataflow1 =  ((*par->vmDestinationDFlowVector)[i])->ReturnDataFlowVM();
				    vectorvmsdataflow2 = (itertemp->first)->ReturnDataFlowVM();
				    if (vectorvmsdataflow1.size() != 0) {
					   vectorFirstCorr = vectorvmsdataflow1[0]->ReturnTrafficNetwork();
				    }
				    if (vectorvmsdataflow2.size() != 0) {
				       vectorSecondCorr = vectorvmsdataflow2[0]->ReturnTrafficNetwork();
				    }
  					if (vectorFirstCorr.size() < 10 || vectorSecondCorr.size() < 10) {
						correl = 2; // If slidewindow < 10
						datavm.emplace(pair<VirtualMachine*, double>(itertemp->first, correl));
					}
					else {
						correl = pearson_correlation(&vectorFirstCorr[0], &vectorSecondCorr[0], vectorFirstCorr.size());
						datavm.emplace(pair<VirtualMachine*, double>(itertemp->first, correl));
					}	
				}
				vectorvmsdataflow1.clear();
				vectorvmsdataflow2.clear();
				vectorFirstCorr.clear();
				vectorSecondCorr.clear();

			}
			//critical section 
			pthread_mutex_lock(par->mutex);
					(*par->vmCorrelation_th).emplace(pair<VirtualMachine*, map<VirtualMachine*, double>>((*par->vmDestinationDFlowVector)[i], datavm));
			pthread_mutex_unlock(par->mutex);

			datavm.clear();
	    }
		pthread_barrier_wait(par->barrier_Thread);
	}
}


int DataCenter::GenerateSeed(void)
{
	std::uniform_int_distribution<int> distribution(1, 1000000);
	return distribution(*gen);
}


