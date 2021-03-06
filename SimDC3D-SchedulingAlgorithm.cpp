#include <vector>
#include <algorithm>

#include "SimDC3D-SchedulingAlgorithm.h"



SchedulingAlgorithm::~SchedulingAlgorithm(void)
{
}

RandomSchedulingAlgorithm::RandomSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], unsigned int* clockSimulation)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;
	clock = clockSimulation;

	srand((unsigned int)time(NULL));
	totalScheduling = 0;
	SCHEDULING_WITH_PREDICTION = false;

	cout << "SimDC3D-NOTICE: The data center scheduler is Random !!!" << endl;
}

void RandomSchedulingAlgorithm::AssignVMs()
{
	while (!pqVMsToGo->empty()) {
		int bestI = rand()%NUMBER_OF_CHASSIS;
		int bestJ = rand()%NUMBER_OF_SERVERS_IN_ONE_CHASSIS;
		if (bestI < 0 || bestJ < 0 || bestI > NUMBER_OF_CHASSIS || bestJ > NUMBER_OF_SERVERS_IN_ONE_CHASSIS)
			cout << "SimDC3D-Warning: No servers to assign a VM !!!" << endl;
		if ( ((*ppServers)[bestI][bestJ]->IsOFF()) || ((*ppServers)[bestI][bestJ]->IsHibernating()) || ((*ppServers)[bestI][bestJ]->IsENDING()) || ((*ppServers)[bestI][bestJ]->IsPOOL()) || ((*ppServers)[bestI][bestJ]->IsMIGRATING()) || ((*ppServers)[bestI][bestJ]->IsINITIALIZING())) { 	
			continue;
		}
		(*ppServers)[bestI][bestJ]->AssignOneVM(pqVMsToGo->front());
		totalScheduling += 1;
		pqVMsToGo->front()->SetClock(*clock);
		pqVMsToGo->pop();
	}
}

LowTemperatureFirstSchedulingAlgorithm::LowTemperatureFirstSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], unsigned int* clockSimulation)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;
	clock = clockSimulation;

	srand((unsigned int)time(NULL));
	totalScheduling = 0;
	SCHEDULING_WITH_PREDICTION = false;
	cout << "SimDC3D-NOTICE: The data center scheduler is Low Temperature First !!!" << endl;
}

void LowTemperatureFirstSchedulingAlgorithm::AssignVMs()
{
	int besttI;
	int besttJ;

	// assign VMs to Servers
	while (!pqVMsToGo->empty()) {
		// assign with qWaitingVMs.top()
		besttI = -1;
		besttJ = -1;
		FLOATINGPOINT bestAvailability = 1000000.0; // any big number
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ( ((*ppServers)[i][j]->IsOFF()) || ((*ppServers)[i][j]->IsHibernating()) || ((*ppServers)[i][j]->IsENDING()) || ((*ppServers)[i][j]->IsPOOL()) || ((*ppServers)[i][j]->IsMIGRATING()) || ((*ppServers)[i][j]->IsINITIALIZING())) { 	
					continue;
				}
				FLOATINGPOINT sumofVM = (*ppServers)[i][j]->VMRequiresThisMuchCPUScale() + pqVMsToGo->front()->GetCPULoadRatio();
				if (sumofVM >= NUMBER_OF_CORES_IN_ONE_SERVER) // it will be over occupied ...
					continue;
				FLOATINGPOINT localInlet = (*ppServers)[i][j]->CurrentInletTemperature();
				if (localInlet < bestAvailability) {
					besttI = i; 
					besttJ = j;
					bestAvailability = localInlet;
				}
			}
		}
		if (besttI < 0 || besttJ < 0) {
			// second iteration. all servers are busy. assign to the lowest inlet temp
			for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
				for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
					if (((*ppServers)[i][j]->IsOFF()) || ((*ppServers)[i][j]->IsHibernating()) || ((*ppServers)[i][j]->IsENDING())) { 
						continue;
					}
					FLOATINGPOINT sumofVM = (*ppServers)[i][j]->VMRequiresThisMuchCPUScale();
					if (sumofVM >= NUMBER_OF_CORES_IN_ONE_SERVER) { // it will be over occupied ...
						continue;
					}
					FLOATINGPOINT localInlet = (*ppServers)[i][j]->CurrentInletTemperature();
					if (localInlet < bestAvailability) {
						besttI = i; 
						besttJ = j;
						bestAvailability = localInlet;
					}
				}
			}
		}
		if (besttI < 0 || besttJ < 0) {
			// still can not decide .. place randomly
			besttI = rand()%NUMBER_OF_CHASSIS;
			besttJ = rand()%NUMBER_OF_SERVERS_IN_ONE_CHASSIS;
		}
		pqVMsToGo->front()->SetClock(*clock);
		(*ppServers)[besttI][besttJ]->AssignOneVM(pqVMsToGo->front());
		totalScheduling += 1;
		pqVMsToGo->pop();
	}
}

UniformTaskSchedulingAlgorithm::UniformTaskSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], unsigned int* clockSimulation)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;
	totalScheduling = 0;
	clock = clockSimulation;

	SCHEDULING_WITH_PREDICTION = false;

	cout << "SimDC3D-NOTICE: The data center scheduler is Uniform Task !!!" << endl;
}

void UniformTaskSchedulingAlgorithm::AssignVMs()
{
	// assign VMs to Servers
	while (!pqVMsToGo->empty()) {
		// assign with qWaitingVMs.top()
		int bestI = -1; 
		int	bestJ = -1;
		FLOATINGPOINT bestAvailability = 1000000.0; // any big number
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ( ((*ppServers)[i][j]->IsOFF()) || ((*ppServers)[i][j]->IsHibernating()) || ((*ppServers)[i][j]->IsENDING()) || ((*ppServers)[i][j]->IsPOOL()) || ((*ppServers)[i][j]->IsMIGRATING()) || ((*ppServers)[i][j]->IsINITIALIZING())) { 	
					continue;
				}
				FLOATINGPOINT sumofVM = (*ppServers)[i][j]->VMRequiresThisMuchCPUScale();
				if (sumofVM < bestAvailability) { // assign to the machine with least jobs
					bestI = i; 
					bestJ = j; 
					bestAvailability = sumofVM;
				}
			}
		}
		if (bestI < 0 || bestJ < 0)
			cout << "SimDC3D-Warning: No servers to assign a VM !!!" << endl;
		pqVMsToGo->front()->SetClock(*clock);
		(*ppServers)[bestI][bestJ]->AssignOneVM(pqVMsToGo->front());
		totalScheduling += 1;
		pqVMsToGo->pop();
	}
}

BestPerformanceSchedulingAlgorithm::BestPerformanceSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], unsigned int* clockSimulation)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;
	totalScheduling = 0;
	clock = clockSimulation;
	SCHEDULING_WITH_PREDICTION = false;
	cout << "SimDC3D-NOTICE: The data center scheduler is Best Performance !!!" << endl;
}

void BestPerformanceSchedulingAlgorithm::AssignVMs()
{
	// assign VMs to Servers
	while (!pqVMsToGo->empty()) {
		// assign with qWaitingVMs.top()
		int bestI = -1; 
		int	bestJ = -1;
		FLOATINGPOINT bestAvailability = 1000000.0; // any big number
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ( ((*ppServers)[i][j]->IsOFF()) || ((*ppServers)[i][j]->IsHibernating()) || ((*ppServers)[i][j]->IsENDING()) || ((*ppServers)[i][j]->IsPOOL()) || ((*ppServers)[i][j]->IsMIGRATING()) || ((*ppServers)[i][j]->IsINITIALIZING())) { 	
					continue;
				}
				FLOATINGPOINT sumofVM = (*ppServers)[i][j]->VMRequiresThisMuchUtilization();
				FLOATINGPOINT maxutil = (*ppServers)[i][j]->MaxUtilization();
				FLOATINGPOINT ratio = (sumofVM/maxutil);
				if (ratio < bestAvailability) {
					bestI = i; 
					bestJ = j;
					bestAvailability = ratio;
				}
			}
		}
		if (bestI < 0 || bestJ < 0) {
			cout << "SimDC3D-Warning: No servers to assign a VM !!!" << endl;
		}
		pqVMsToGo->front()->SetClock(*clock);
		(*ppServers)[bestI][bestJ]->AssignOneVM(pqVMsToGo->front());
		totalScheduling += 1;
		pqVMsToGo->pop();
	}

}

MinHRSchedulingAlgorithm::MinHRSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], unsigned int* clockSimulation)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;

	totalScheduling = 0;
	clock = clockSimulation;
	SCHEDULING_WITH_PREDICTION = false;

	for (int i=0; i<SIZE_OF_HR_MATRIX; ++i) {
		HRF[i] = 0.0;
		for (int j=0; j<SIZE_OF_HR_MATRIX; ++j) {
			HRF[i] += (*pHeatRecirculationMatrixD)[j][i];
		}
	}

	for (int i=0; i<SIZE_OF_HR_MATRIX; ++i) {
		FLOATINGPOINT reference = 100000.0; // some big number
		int toInsert = -1;
		for (int j=0; j<SIZE_OF_HR_MATRIX; ++j) {
			if (HRF[j] < reference) {
				toInsert = j;
				reference = HRF[j];
			}
		}
		HRF[toInsert] = 10000000.0; // some bigger number
		HRFSortedIndex[i] = toInsert;
	}
	
	srand((unsigned int)time(NULL));

	cout << "SimDC3D-NOTICE: The data center scheduler is minimum recirculation !!!" << endl;
}

void MinHRSchedulingAlgorithm::AssignVMs()
{
	// assign VMs to Servers
	while (!pqVMsToGo->empty()) {
		// assign with qWaitingVMs.top()
		int bestI, bestJ = -1;
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ( ((*ppServers)[HRFSortedIndex[i]][j]->IsOFF()) || ((*ppServers)[HRFSortedIndex[i]][j]->IsHibernating()) || ((*ppServers)[HRFSortedIndex[i]][j]->IsENDING()) ) {
					continue;
				}

				FLOATINGPOINT sumofVM = (*ppServers)[HRFSortedIndex[i]][j]->VMRequiresThisMuchCPUScale() + pqVMsToGo->front()->GetCPULoadRatio();
				if (sumofVM >= NUMBER_OF_CORES_IN_ONE_SERVER) // it's full with the new workload
					continue;
				bestI = HRFSortedIndex[i];
				bestJ = j;
				goto ASSIGNING_FINISHED;
			}
		}
		// data center is full. assign similarly but more aggressively.
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ( ((*ppServers)[HRFSortedIndex[i]][j]->IsOFF()) || ((*ppServers)[HRFSortedIndex[i]][j]->IsHibernating()) || ((*ppServers)[HRFSortedIndex[i]][j]->IsENDING()) ) {
				   continue;
				}

				FLOATINGPOINT sumofVM = (*ppServers)[HRFSortedIndex[i]][j]->VMRequiresThisMuchCPUScale();
				if (sumofVM >= NUMBER_OF_CORES_IN_ONE_SERVER) { // it's full even before placing the workload
					continue;
				}
				bestI = HRFSortedIndex[i];
				bestJ = j;
				goto ASSIGNING_FINISHED;
			}
		}
		// still could not decide, which means, every one is full. locate workload randomly.
		bestI = rand()%NUMBER_OF_CHASSIS;
		bestJ = rand()%NUMBER_OF_SERVERS_IN_ONE_CHASSIS;
ASSIGNING_FINISHED:
		pqVMsToGo->front()->SetClock(*clock);
		(*ppServers)[bestI][bestJ]->AssignOneVM(pqVMsToGo->front());
		totalScheduling += 1;
		pqVMsToGo->pop();
	}
}

XintSchedulingAlgorithm::XintSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], unsigned int* clockSimulation)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;
	totalScheduling = 0;
	clock = clockSimulation;
	SCHEDULING_WITH_PREDICTION = false;
	cout << "SimDC3D-NOTICE: The data center scheduler is Xint !!!" << endl;
}

void XintSchedulingAlgorithm::AssignVMs()
{
	// assign VMs to Servers
	while (!pqVMsToGo->empty()) {
		int bestI = -1;
		int bestJ = -1;
		FLOATINGPOINT thishastobemin = 1000.0;
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if (((*ppServers)[i][j]->IsOFF()) || ((*ppServers)[i][j]->IsHibernating()) || ((*ppServers)[i][j]->IsENDING())) {
					continue;
				}
				(*ppServers)[i][j]->AssignOneVM(pqVMsToGo->front()); // temporarilly assign to i,j
				FLOATINGPOINT local_thishastobemin = GetHighestTemperatureIncrease();
				if (thishastobemin > local_thishastobemin) {
					bestI = i; 
					bestJ = j;
					thishastobemin = local_thishastobemin;
				}
				(*ppServers)[i][j]->RemoveTheLastAssignedVM(); // de-assign after calculating hr effect
			}
		}
		pqVMsToGo->front()->SetClock(*clock);
		(*ppServers)[bestI][bestJ]->AssignOneVM(pqVMsToGo->front());
		totalScheduling += 1;
		pqVMsToGo->pop();
	}
}

FLOATINGPOINT XintSchedulingAlgorithm::GetHighestTemperatureIncrease()
{
	FLOATINGPOINT powerDraw[SIZE_OF_HR_MATRIX];
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		powerDraw[i] = 0.0;
		for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			powerDraw[i] += (*ppServers)[i][j]->GetPowerDraw();
		}
	}
	FLOATINGPOINT tempIncrease;
	FLOATINGPOINT biggestTempIncrease = -100.0;
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		tempIncrease = 0.0;
		for (int j=0; j<NUMBER_OF_CHASSIS; ++j) {
			tempIncrease += powerDraw[j]*(*pHeatRecirculationMatrixD)[i][j];
		}
		if (tempIncrease > biggestTempIncrease)
			biggestTempIncrease = tempIncrease;
	}
	return biggestTempIncrease;
}


CenterRackFirstSchedulingAlgorithm::CenterRackFirstSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], unsigned int* clockSimulation)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;
	totalScheduling = 0;
	clock = clockSimulation;
	SCHEDULING_WITH_PREDICTION = false;

	int machineNumber[SIZE_OF_HR_MATRIX] = {
		11,36,37,12,13,38,39,14,15,40,
		6,16,31,41,42,32,17,7,8,18,33,43,44,34,19,9,10,20,35,45,
		1,21,26,46,47,27,22,2,3,23,28,48,49,29,24,4,5,25,30,50
	};
/*
	11,12,13,14,15,
	36,37,38,39,40,

	6,7,8,9,10,
	16,17,18,19,20,
	31,32,33,34,35,
	41,42,43,44,45,

	1,2,3,4,5,
	21,22,23,24,25,
	26,27,28,29,30,
	46,47,48,49,50,
*/

	for (int i=0; i<SIZE_OF_HR_MATRIX; ++i) {
		HRFSortedIndex[i] = machineNumber[i] -1;
	}

	cout << "SimDC3D-NOTICE: The data center scheduler is Center Rack !!!" << endl;

}

void CenterRackFirstSchedulingAlgorithm::AssignVMs()
{
	// assign VMs to Servers
	while (!pqVMsToGo->empty()) {
		// assign with qWaitingVMs.top()
		int bestI, bestJ = -1;
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ( ((*ppServers)[HRFSortedIndex[i]][j]->IsOFF()) || ((*ppServers)[HRFSortedIndex[i]][j]->IsHibernating()) || ((*ppServers)[HRFSortedIndex[i]][j]->IsENDING()) ) continue;

				FLOATINGPOINT sumofVM = (*ppServers)[HRFSortedIndex[i]][j]->VMRequiresThisMuchCPUScale() + pqVMsToGo->front()->GetCPULoadRatio();
				if (sumofVM >= NUMBER_OF_CORES_IN_ONE_SERVER) // it's full with the new workload
					continue;
				bestI = HRFSortedIndex[i];
				bestJ = j;
				goto ASSIGNING_FINISHED;
			}
		}
		// data center is full. assign similarly but more aggressively.
		for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
			for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
				if ( ((*ppServers)[HRFSortedIndex[i]][j]->IsOFF()) || ((*ppServers)[HRFSortedIndex[i]][j]->IsHibernating()) || ((*ppServers)[HRFSortedIndex[i]][j]->IsENDING()) ) continue;

				FLOATINGPOINT sumofVM = (*ppServers)[HRFSortedIndex[i]][j]->VMRequiresThisMuchCPUScale();
				if (sumofVM >= NUMBER_OF_CORES_IN_ONE_SERVER) // it's full even before placing the workload
					continue;
				bestI = HRFSortedIndex[i];
				bestJ = j;
				goto ASSIGNING_FINISHED;
			}
		}
		// still could not decide, which means, every one is full. locate workload randomly.
		bestI = rand()%NUMBER_OF_CHASSIS;
		bestJ = rand()%NUMBER_OF_SERVERS_IN_ONE_CHASSIS;
ASSIGNING_FINISHED:
		pqVMsToGo->front()->SetClock(*clock);
		(*ppServers)[bestI][bestJ]->AssignOneVM(pqVMsToGo->front());
		totalScheduling += 1;
		pqVMsToGo->pop();
	}
}


TwoDimensionWithPoolSchedulingAlgorithm::TwoDimensionWithPoolSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], POOLServers* ppool, unsigned int* clockSimulation)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;
	ppollServers = ppool;
	totalScheduling = 0;
	clock = clockSimulation;

	SCHEDULING_WITH_PREDICTION = false;

	for (int i=0; i<SIZE_OF_HR_MATRIX; ++i) {
		HRF[i] = 0.0;
		for (int j=0; j<SIZE_OF_HR_MATRIX; ++j) {
			HRF[i] += (*pHeatRecirculationMatrixD)[j][i];
		}
	}

	cout << "SimDC3D-NOTICE: The data center scheduler is Two Dimension with Pool Server !!!" << endl;
}

     
void TwoDimensionWithPoolSchedulingAlgorithm::AssignVMs()
{
  POOL sv;
  SORTSERVER serverScheduling[CHASSIxSERVER];

  int k = 0; 
  int RemovePOOL = 0; 
  int full = 0;
  float consumo = 0;

  if (!pqVMsToGo->empty()) {
     for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
	     for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
		     if ( ((*ppServers)[i][j]->IsOFF()) || ((*ppServers)[i][j]->IsHibernating()) || ((*ppServers)[i][j]->IsENDING()) || ((*ppServers)[i][j]->IsPOOL()) || ((*ppServers)[i][j]->IsMIGRATING()) || ((*ppServers)[i][j]->IsINITIALIZING())) { 	
		        continue;
		     }
			 serverScheduling[k].chassi = i;
		     serverScheduling[k].server = j;
		     serverScheduling[k].temperature = (*ppServers)[i][j]->CurrentInletTemperature();
			 serverScheduling[k].temperatureFuture = 0.0;
			 serverScheduling[k].averageUtilizationCPU = (*ppServers)[i][j]->AverageUsageofCPU();
		     serverScheduling[k].utilizationCPU = (*ppServers)[i][j]->VMRequiresThisMuchUtilization();
			 consumo = (*ppServers)[i][j]->GetPowerDraw();
			 serverScheduling[k].utilizationMemory = (*ppServers)[i][j]->VMRequiresThisMemory();
			 serverScheduling[k].memoryServer = (*ppServers)[i][j]->GetMemoryServer();
			 //serverScheduling[k].ranking = (SCHEDULER_2D_WEIGHT_TEMPERATURE * (serverScheduling[k].temperature/34)) + (SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION * (HRF[i]/0.001)) + (SCHEDULER_2D_WEIGHT_LOAD_CPU * (serverScheduling[k].utilizationCPU));
			 serverScheduling[k].ranking = (SCHEDULER_2D_WEIGHT_TEMPERATURE * (serverScheduling[k].temperature/34)) + (SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION * (HRF[i]/0.001)) + (SCHEDULER_2D_WEIGHT_LOAD_CPU * (consumo/305));
			 //serverScheduling[k].ranking = (SCHEDULER_2D_WEIGHT_TEMPERATURE * (serverScheduling[k].temperature/34)) + (SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION * (HRF[i]/0.001)) + (SCHEDULER_2D_WEIGHT_LOAD_CPU * (serverScheduling[k].averageUtilizationCPU));
			 k++;
	     }
     }
     quickSort(serverScheduling, 0, k-1);
  } 

  // assign VMs to Servers
  while (!pqVMsToGo->empty())  {
	    full = 0;
        for (int l=0; l < k; ++l) {
            if (!pqVMsToGo->empty()) {
//			   if (((serverScheduling[l].utilizationCPU + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) <= THRESHOLD_TOP_OF_USE_CPU) && ((serverScheduling[l].utilizationMemory + pqVMsToGo->front()->GetMemUseVM()) < serverScheduling[l].memoryServer) && (serverScheduling[l].temperature < EMERGENCY_TEMPERATURE-0.5)) {
			   if (((serverScheduling[l].utilizationCPU + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) <= THRESHOLD_TOP_OF_USE_CPU) && ((serverScheduling[l].utilizationMemory + pqVMsToGo->front()->GetMemUseVM()) < serverScheduling[l].memoryServer) ) {
				  serverScheduling[l].utilizationCPU += (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER);
				  serverScheduling[l].utilizationMemory += pqVMsToGo->front()->GetMemUseVM();
				  pqVMsToGo->front()->SetClock(*clock);
				  (*ppServers)[serverScheduling[l].chassi][serverScheduling[l].server]->AssignOneVM(pqVMsToGo->front());
				  totalScheduling += 1;
			      pqVMsToGo->pop();
				  break;
		       }
			   else {
                //if ((serverScheduling[l].utilizationCPU + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER) >= THRESHOLD_TOP_OF_USE_CPU) || ((serverScheduling[l].utilizationMemory + pqVMsToGo->front()->GetMemUseVM()) >= serverScheduling[l].memoryServer) || (serverScheduling[l].temperature >= EMERGENCY_TEMPERATURE-0.5)) {
                  if ((serverScheduling[l].utilizationCPU + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER) >= THRESHOLD_TOP_OF_USE_CPU) || ((serverScheduling[l].utilizationMemory + pqVMsToGo->front()->GetMemUseVM()) >= serverScheduling[l].memoryServer) ) {
					  full++;
			      }
			      continue;
		       }
		    }
		    else {
			   break;
		    }
		}
		if (full == k) {
		   sv = ppollServers->RemoveServerPOOL(ppServers);
		   if (sv.chassi != -1) {
              serverScheduling[k].chassi = sv.chassi;
		      serverScheduling[k].server = sv.server;
		      serverScheduling[k].temperature = (*ppServers)[sv.chassi][sv.server]->CurrentInletTemperature();
			  serverScheduling[k].averageUtilizationCPU = (*ppServers)[sv.chassi][sv.server]->AverageUsageofCPU();
		      serverScheduling[k].utilizationCPU = (*ppServers)[sv.chassi][sv.server]->VMRequiresThisMuchUtilization();
			  serverScheduling[k].utilizationMemory = (*ppServers)[sv.chassi][sv.server]->VMRequiresThisMemory();
			  serverScheduling[k].memoryServer = (*ppServers)[sv.chassi][sv.server]->GetMemoryServer();
		      serverScheduling[k].temperatureFuture = 0;
			  consumo =  (*ppServers)[sv.chassi][sv.server]->GetPowerDraw();
			  serverScheduling[k].ranking = (SCHEDULER_2D_WEIGHT_TEMPERATURE * (serverScheduling[k].temperature/34)) + (SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION * (HRF[sv.chassi]/0.001)) + (SCHEDULER_2D_WEIGHT_LOAD_CPU * (consumo/305));
    	      //serverScheduling[k].ranking = (SCHEDULER_2D_WEIGHT_TEMPERATURE * (serverScheduling[k].temperature/34)) + (SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION * (HRF[sv.chassi]/0.001)) + (SCHEDULER_2D_WEIGHT_LOAD_CPU * (serverScheduling[k].utilizationCPU));
			  //serverScheduling[k].ranking = (SCHEDULER_2D_WEIGHT_TEMPERATURE * (serverScheduling[k].temperature/34)) + (SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION * (HRF[sv.chassi]/0.001)) + (SCHEDULER_2D_WEIGHT_LOAD_CPU * (serverScheduling[k].averageUtilizationCPU));
			  pqVMsToGo->front()->SetClock(*clock);
		      (*ppServers)[serverScheduling[k].chassi][serverScheduling[k].server]->AssignOneVM(pqVMsToGo->front());
			  totalScheduling += 1;
		      pqVMsToGo->pop();
		      k++;
		      RemovePOOL++;
		      full = 0;
		   }
		   else{
			   cout << "SimDC3D-NOTICE: No servers in the pool - Scheduling Algorithm - TwoDimensionWithPoolSchedulingAlgorithm !!!" << endl;
			   break;
		   }
	    }
  }
   if (RemovePOOL > 0) {
	 ppollServers->AddPowerOn(RemovePOOL);
  }
}

TwoDimensionWithPoolAndPredictionSchedulingAlgorithm::TwoDimensionWithPoolAndPredictionSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], POOLServers* ppool, unsigned int* clockSimulation)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;
	ppollServers = ppool;
	totalScheduling = 0;
	clock = clockSimulation;
	SCHEDULING_WITH_PREDICTION = true;

	for (int i=0; i<SIZE_OF_HR_MATRIX; ++i) {
		HRF[i] = 0.0;
		for (int j=0; j<SIZE_OF_HR_MATRIX; ++j) {
			HRF[i] += (*pHeatRecirculationMatrixD)[j][i];
		}
	}

	cout << "SimDC3D-NOTICE: The data center scheduler is Two Dimension with Pool Server and Prediction !!!" << endl;
}
void TwoDimensionWithPoolAndPredictionSchedulingAlgorithm::AssignVMs()
{
  SORTSERVER serverScheduling[CHASSIxSERVER];
  vector<double> predictionTemp;
  vector<double> predictionCPU;

  POOL sv;

  int k = 0; 
  int RemovePOOL = 0; 
  int full = 0;
    
  if (!pqVMsToGo->empty()) {
     for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
	     for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) { 
	         if ( ((*ppServers)[i][j]->IsOFF()) || ((*ppServers)[i][j]->IsHibernating()) || ((*ppServers)[i][j]->IsENDING()) || ((*ppServers)[i][j]->IsPOOL()) || ((*ppServers)[i][j]->IsMIGRATING()) || ((*ppServers)[i][j]->IsINITIALIZING())) { 	 
		 	    continue;
		     }
			 serverScheduling[k].chassi = i;
		     serverScheduling[k].server = j;
		     serverScheduling[k].temperature = (*ppServers)[i][j]->CurrentInletTemperature();
			 serverScheduling[k].utilizationCPU = (*ppServers)[i][j]->VMRequiresThisMuchUtilization();
			 serverScheduling[k].utilizationMemory = (*ppServers)[i][j]->VMRequiresThisMemory();
			 serverScheduling[k].memoryServer = (*ppServers)[i][j]->GetMemoryServer();
			 serverScheduling[k].predictedOverload = (*ppServers)[i][j]->ReturnCPUPrediction();

  		     if ((*ppServers)[i][j]->ReturnSizeVectorTemperature() == SIZE_WINDOWN_PREDICTION) {
				if (PREDICTION_ALGORITHM=="POLYNOM"){
				   predictionTemp = runPolynom( (*ppServers)[i][j]->ReturnVectorTemperature() );  // run prediction
				} 
				else{
                   if (PREDICTION_ALGORITHM=="RBF"){
					   predictionTemp = runRBF( (*ppServers)[i][j]->ReturnVectorTemperature() );  // run prediction
				   }
				   else {
					   predictionTemp = runPolynom( (*ppServers)[i][j]->ReturnVectorTemperature() );  // run prediction default
				   }
				}  

				if (!predictionTemp.empty()) {
		           if ( predictionTemp[CLENGTH-1] >= 10 && predictionTemp[CLENGTH-1] <= 35) {
				      serverScheduling[k].temperatureFuture = predictionTemp[CLENGTH-1];
  			          (*ppServers)[i][j]->InsertTemperaturePredictionServer(predictionTemp[CLENGTH-1]);
				      (*ppServers)[i][j]->InsertTimePredictionServer((*ppServers)[i][j]->ReturnClock()+(CLENGTH*MONITORINGTIME));
				      (*ppServers)[i][j]->AddHitPrediction();
			   	      //cout << "Servidor " << i << " " << j << " Temperatura predita " << predictionTemp[CLENGTH-1] << " Tempo de Simula��o " << (*ppServers)[i][j]->ReturnClock() << " Tempo Futuro " << (*ppServers)[i][j]->ReturnClock()+(CLENGTH*MONITORINGTIME) << endl;
				   }
			  	   else {
                      if ( predictionTemp[0] >= 10 && predictionTemp[0] <= 35) {
					     serverScheduling[k].temperatureFuture = predictionTemp[0];
						 (*ppServers)[i][j]->InsertTemperaturePredictionServer(predictionTemp[0]);
				         (*ppServers)[i][j]->InsertTimePredictionServer((*ppServers)[i][j]->ReturnClock()+(CLENGTH*MONITORINGTIME));
				         (*ppServers)[i][j]->AddHitPrediction();
				      }
				      else {
					     (*ppServers)[i][j]->AddErrorPrediction();
					     serverScheduling[k].temperatureFuture = (*ppServers)[i][j]->CurrentInletTemperature();
				      }
				  }
				  serverScheduling[k].ranking = (SCHEDULER_2D_WEIGHT_TEMPERATURE * (serverScheduling[k].temperatureFuture/34)) + (SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION * (HRF[i]/0.001)) + (SCHEDULER_2D_WEIGHT_LOAD_CPU * (serverScheduling[k].utilizationCPU));
				}
				predictionTemp.erase(predictionTemp.begin(), predictionTemp.end());
			 }
			 else {
				serverScheduling[k].temperatureFuture = 0;
    	        serverScheduling[k].ranking = (SCHEDULER_2D_WEIGHT_TEMPERATURE * (serverScheduling[k].temperature/34)) + (SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION * (HRF[i]/0.001)) + (SCHEDULER_2D_WEIGHT_LOAD_CPU * (serverScheduling[k].utilizationCPU));
			 }	 
    	     k++;
	     }
     }
     quickSort(serverScheduling, 0, k-1);
  } 

  // assign VMs to Servers
  while (!pqVMsToGo->empty())  {
	    full = 0;
        for(int i=0; i < k; i++) {
           if (!pqVMsToGo->empty()) {
			  if ((serverScheduling[i].predictedOverload) || (serverScheduling[i].temperatureFuture > EMERGENCY_TEMPERATURE)) {
				 full++;
				 continue;
			  }
			  if (((serverScheduling[i].utilizationCPU + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) < THRESHOLD_TOP_OF_USE_CPU) && ((serverScheduling[i].utilizationMemory + pqVMsToGo->front()->GetMemUseVM()) < serverScheduling[i].memoryServer) && (serverScheduling[i].temperature < EMERGENCY_TEMPERATURE))  {
   		         serverScheduling[i].utilizationCPU += (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER);
				 serverScheduling[i].utilizationMemory += pqVMsToGo->front()->GetMemUseVM();
 			     pqVMsToGo->front()->SetClock(*clock);
			     (*ppServers)[serverScheduling[i].chassi][serverScheduling[i].server]->AssignOneVM(pqVMsToGo->front());
				 totalScheduling += 1;
			     pqVMsToGo->pop();
		      }
			  else {
                 if ((serverScheduling[i].utilizationCPU + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER) >= THRESHOLD_TOP_OF_USE_CPU) || ((serverScheduling[i].utilizationMemory + pqVMsToGo->front()->GetMemUseVM()) > serverScheduling[i].memoryServer) || (serverScheduling[i].temperature > EMERGENCY_TEMPERATURE)) {
					full++;
				 }
				 continue;
		      }
		   }
		   else {
			  break;
		   }
		}
		if (full == k) {
		   sv = ppollServers->RemoveServerPOOL(ppServers);
		   if (sv.chassi != -1) {
              serverScheduling[k].chassi = sv.chassi;
		      serverScheduling[k].server = sv.server;
		      serverScheduling[k].temperature = (*ppServers)[sv.chassi][sv.server]->CurrentInletTemperature();
		      serverScheduling[k].utilizationCPU = (*ppServers)[sv.chassi][sv.server]->VMRequiresThisMuchUtilization();
			  serverScheduling[k].utilizationMemory = (*ppServers)[sv.chassi][sv.server]->VMRequiresThisMemory();
			  serverScheduling[k].memoryServer = (*ppServers)[sv.chassi][sv.server]->GetMemoryServer();
		      serverScheduling[k].temperatureFuture = 0;
			  serverScheduling[k].predictedOverload = false;
    	      serverScheduling[k].ranking = (SCHEDULER_2D_WEIGHT_TEMPERATURE * (serverScheduling[k].temperature/34)) + (SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION * (HRF[sv.chassi]/0.001)) + (SCHEDULER_2D_WEIGHT_LOAD_CPU * (serverScheduling[k].utilizationCPU));

			  pqVMsToGo->front()->SetClock(*clock);
			  (*ppServers)[serverScheduling[k].chassi][serverScheduling[k].server]->AssignOneVM(pqVMsToGo->front());
			  totalScheduling += 1;
		      pqVMsToGo->pop();
		      k++;
		      RemovePOOL++;
		      full = 0;
		   }
		   else{
			   cout << "SimDC3D-NOTICE: No servers in the pool - Scheduling Algorithm - TwoDimensionWithPoolAndPredictionSchedulingAlgorithm !!!" << endl;
			   break;
		   }
		}
  }

  if (RemovePOOL > 0) {
	  ppollServers->AddPowerOn(RemovePOOL);
  }
  predictionTemp.clear();
}



TwoDimensionWithPredictionSchedulingAlgorithm::TwoDimensionWithPredictionSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], POOLServers* ppool, unsigned int* clockSimulation)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;
	ppollServers = ppool;
	totalScheduling = 0;
	clock = clockSimulation;

	SCHEDULING_WITH_PREDICTION = true;
	
	for (int i=0; i<SIZE_OF_HR_MATRIX; ++i) {
		HRF[i] = 0.0;
		for (int j=0; j<SIZE_OF_HR_MATRIX; ++j) {
			HRF[i] += (*pHeatRecirculationMatrixD)[j][i];
		}
	}
	cout << "SimDC3D-NOTICE: The data center scheduler is Two Dimension with Prediction !!!" << endl;
}
void TwoDimensionWithPredictionSchedulingAlgorithm::AssignVMs()
{
  SORTSERVER serverScheduling[CHASSIxSERVER];
  vector<double> predictionTemp;
  queue<VirtualMachine*> VMsScheduler;
  double powerON = 0;
  bool noScheduler;

  int k = 0; 
    
  if (!pqVMsToGo->empty()) {
     for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
	     for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) { 
	         if ( ((*ppServers)[i][j]->IsOFF()) || ((*ppServers)[i][j]->IsHibernating()) || ((*ppServers)[i][j]->IsENDING()) || ((*ppServers)[i][j]->IsPOOL()) || ((*ppServers)[i][j]->IsMIGRATING()) || ((*ppServers)[i][j]->IsINITIALIZING())) { 	 
		 	    continue;
		     }
			 serverScheduling[k].chassi = i;
		     serverScheduling[k].server = j;
		     serverScheduling[k].temperature = (*ppServers)[i][j]->CurrentInletTemperature();
			 serverScheduling[k].utilizationCPU = (*ppServers)[i][j]->VMRequiresThisMuchUtilization();
			 serverScheduling[k].utilizationMemory = (*ppServers)[i][j]->VMRequiresThisMemory();
			 serverScheduling[k].memoryServer = (*ppServers)[i][j]->GetMemoryServer();
			 serverScheduling[k].predictedOverload = (*ppServers)[i][j]->ReturnCPUPrediction();

			 if ((*ppServers)[i][j]->ReturnSizeVectorTemperature() == SIZE_WINDOWN_PREDICTION) {
				if (PREDICTION_ALGORITHM=="POLYNOM"){
				   predictionTemp = runPolynom( (*ppServers)[i][j]->ReturnVectorTemperature() );  // run prediction
				} 
				else{
                   if (PREDICTION_ALGORITHM=="RBF"){
					   predictionTemp = runRBF( (*ppServers)[i][j]->ReturnVectorTemperature() );  // run prediction
				   }
				   else {
					   predictionTemp = runPolynom( (*ppServers)[i][j]->ReturnVectorTemperature() );  // run prediction default
				   }
				}  
			 	if (!predictionTemp.empty()) {
		           if ( predictionTemp[CLENGTH-1] >= 10 && predictionTemp[CLENGTH-1] <= 34) {
				      serverScheduling[k].temperatureFuture = predictionTemp[CLENGTH-1];
  			          (*ppServers)[i][j]->InsertTemperaturePredictionServer(predictionTemp[CLENGTH-1]);
				      (*ppServers)[i][j]->InsertTimePredictionServer((*ppServers)[i][j]->ReturnClock()+(CLENGTH*MONITORINGTIME));
				      (*ppServers)[i][j]->AddHitPrediction();
				   }
			  	   else {
                      if ( predictionTemp[0] >= 10 && predictionTemp[0] <= 34) {
					     serverScheduling[k].temperatureFuture = predictionTemp[0];
						 (*ppServers)[i][j]->InsertTemperaturePredictionServer(predictionTemp[0]);
				         (*ppServers)[i][j]->InsertTimePredictionServer((*ppServers)[i][j]->ReturnClock()+(CLENGTH*MONITORINGTIME));
				         (*ppServers)[i][j]->AddHitPrediction();
				      }
				      else {
					     (*ppServers)[i][j]->AddErrorPrediction();
					     serverScheduling[k].temperatureFuture = (*ppServers)[i][j]->CurrentInletTemperature();
				      }
				  }
				  serverScheduling[k].ranking = (SCHEDULER_2D_WEIGHT_TEMPERATURE * (serverScheduling[k].temperatureFuture/34)) + (SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION * (HRF[i]/0.001)) + (SCHEDULER_2D_WEIGHT_LOAD_CPU * (serverScheduling[k].utilizationCPU));
//				  serverScheduling[k].ranking = (0.40*(serverScheduling[k].temperature/34)) + (0.10*(HRF[i]/0.001)) + (0.50*(serverScheduling[k].utilizationCPU));

				}
				predictionTemp.erase(predictionTemp.begin(), predictionTemp.end());
			 }
			 else {
				serverScheduling[k].temperatureFuture = 0;
     	        serverScheduling[k].ranking = (SCHEDULER_2D_WEIGHT_TEMPERATURE * (serverScheduling[k].temperature/34)) + (SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION * (HRF[i]/0.001)) + (SCHEDULER_2D_WEIGHT_LOAD_CPU * (serverScheduling[k].utilizationCPU));
 			 }	 
    	     k++;
	     }
     }
     quickSort(serverScheduling, 0, k-1);
  } 
  
  // assign VMs to Servers
  while (!pqVMsToGo->empty())  {
	    noScheduler = false;
        for (int i=0; i < k; i++) {
			if ((serverScheduling[i].predictedOverload) || (serverScheduling[i].temperatureFuture > EMERGENCY_TEMPERATURE)) {
			   continue;
			 }
			 if (((serverScheduling[i].utilizationCPU + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) < THRESHOLD_TOP_OF_USE_CPU) && ((serverScheduling[i].utilizationMemory + pqVMsToGo->front()->GetMemUseVM()) < serverScheduling[i].memoryServer) && (serverScheduling[i].temperature < EMERGENCY_TEMPERATURE))  {
   		       serverScheduling[i].utilizationCPU += (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER);
			   serverScheduling[i].utilizationMemory += pqVMsToGo->front()->GetMemUseVM();
			   pqVMsToGo->front()->SetClock(*clock);
			   (*ppServers)[serverScheduling[i].chassi][serverScheduling[i].server]->AssignOneVM(pqVMsToGo->front());
			   pqVMsToGo->pop();
			   totalScheduling += 1;
			   noScheduler = true;
			   break;
		    }
		}
		if (!noScheduler) {
			VMsScheduler.push(pqVMsToGo->front());
			pqVMsToGo->pop();
		}
  }

  if (VMsScheduler.size() > 0) {
     powerON = (int) ceil(((double) VMsScheduler.size() / (double) NUMBER_OF_CORES_IN_ONE_SERVER ));
     ppollServers->AddPowerOn(powerON);
  }

  while (!VMsScheduler.empty())  {
	     pqVMsToGo->push(VMsScheduler.front());
		 VMsScheduler.pop();
  }

  predictionTemp.clear();
}



TwoDimensionSchedulingAlgorithm::TwoDimensionSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, const FLOATINGPOINT (*matrixD)[SIZE_OF_HR_MATRIX][SIZE_OF_HR_MATRIX], POOLServers* ppool, unsigned int* clockSimulation)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	pHeatRecirculationMatrixD = matrixD;
	ppollServers = ppool;
	totalScheduling = 0;
	clock = clockSimulation;

	SCHEDULING_WITH_PREDICTION = false;

	for (int i=0; i<SIZE_OF_HR_MATRIX; ++i) {
		HRF[i] = 0.0;
		for (int j=0; j<SIZE_OF_HR_MATRIX; ++j) {
			HRF[i] += (*pHeatRecirculationMatrixD)[j][i];
		}
	}
	cout << "SimDC3D-NOTICE: The data center scheduler is Two Dimension !!!" << endl;
}
     
void TwoDimensionSchedulingAlgorithm::AssignVMs()
{
  SORTSERVER serverScheduling[CHASSIxSERVER];

  queue<VirtualMachine*> VMsScheduler;

  int k = 0; 
  double powerON = 0;
  bool noScheduler;

  if (!pqVMsToGo->empty()) {
     for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
	     for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
		     if ( ((*ppServers)[i][j]->IsOFF()) || ((*ppServers)[i][j]->IsHibernating()) || ((*ppServers)[i][j]->IsENDING()) || ((*ppServers)[i][j]->IsPOOL()) || ((*ppServers)[i][j]->IsMIGRATING()) || ((*ppServers)[i][j]->IsINITIALIZING())) { 	
		        continue;
		     }
			 serverScheduling[k].chassi = i;
		     serverScheduling[k].server = j;
		     serverScheduling[k].temperature = (*ppServers)[i][j]->CurrentInletTemperature();
			 serverScheduling[k].temperatureFuture = 0.0;
		     serverScheduling[k].utilizationCPU = (*ppServers)[i][j]->VMRequiresThisMuchUtilization();
			 serverScheduling[k].utilizationMemory = (*ppServers)[i][j]->VMRequiresThisMemory();
			 serverScheduling[k].memoryServer = (*ppServers)[i][j]->GetMemoryServer();
			 serverScheduling[k].ranking = (SCHEDULER_2D_WEIGHT_TEMPERATURE * (serverScheduling[k].temperature/34)) + (SCHEDULER_2D_WEIGHT_HEAT_RECIRCULATION * (HRF[i]/0.001)) + (SCHEDULER_2D_WEIGHT_LOAD_CPU * (serverScheduling[k].utilizationCPU));
		     k++;
	     }
     }
     quickSort(serverScheduling, 0, k-1);
  } 

   // assign VMs to Servers
  while (!pqVMsToGo->empty())  {
	    noScheduler = false;
        for(int i=0; i < k; i++) {
			if (((serverScheduling[i].utilizationCPU + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) < THRESHOLD_TOP_OF_USE_CPU) && ((serverScheduling[i].utilizationMemory + pqVMsToGo->front()->GetMemUseVM()) <= serverScheduling[i].memoryServer) && (serverScheduling[i].temperature < EMERGENCY_TEMPERATURE))  {
   		       serverScheduling[i].utilizationCPU += (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER);
			   serverScheduling[i].utilizationMemory += pqVMsToGo->front()->GetMemUseVM();
			   pqVMsToGo->front()->SetClock(*clock);
			   (*ppServers)[serverScheduling[i].chassi][serverScheduling[i].server]->AssignOneVM(pqVMsToGo->front());
			   pqVMsToGo->pop();
			   totalScheduling += 1;
			   noScheduler = true;
			   break;
		    }
		}
		if (!noScheduler) {
			VMsScheduler.push(pqVMsToGo->front());
			pqVMsToGo->pop();
		}
  }

  if (VMsScheduler.size() > 0) {
     powerON = (int) ceil(((double) VMsScheduler.size() / (double) NUMBER_OF_CORES_IN_ONE_SERVER ));
     ppollServers->AddPowerOn(powerON);
  }

  while (!VMsScheduler.empty())  {
	     pqVMsToGo->push(VMsScheduler.front());
		 VMsScheduler.pop();
  }

}

THREEDMOBFDSchedulingAlgorithm::THREEDMOBFDSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, POOLServers* ppool, unsigned int* clockSimulation)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	ppollServers = ppool;
	totalScheduling = 0;
	clock = clockSimulation;

	SCHEDULING_WITH_PREDICTION = false;

	cout << "SimDC3D-NOTICE: The data center scheduler is 3DMOBFD !!!" << endl;
}

void THREEDMOBFDSchedulingAlgorithm::AssignVMs()
{

 FLOATINGPOINT EFF_Temp = 0.00;
 FLOATINGPOINT EFF_CPU = 0;
 FLOATINGPOINT EFF_Power = 0;
 FLOATINGPOINT EFF_Memory = 0;
 FLOATINGPOINT EFF_Traffic = 0;
 
 double powerON = 0;
 
 bool noScheduler;

 vector<SORTSERVER> serverScheduling;
 
 SORTSERVER serverTemp;
 
 queue<VirtualMachine*> VMsScheduler;

 if (!pqVMsToGo->empty()) {
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
	    for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
	        if ( ((*ppServers)[i][j]->IsOFF()) || ((*ppServers)[i][j]->IsHibernating()) || ((*ppServers)[i][j]->IsENDING()) || ((*ppServers)[i][j]->IsPOOL()) || ((*ppServers)[i][j]->IsMIGRATING()) || ((*ppServers)[i][j]->IsINITIALIZING())) { 	
		       continue;
		    }
		    serverTemp.chassi = i;
		    serverTemp.server = j;
		    serverTemp.temperature = (*ppServers)[i][j]->CurrentInletTemperature();
			serverTemp.temperatureFuture = 0.0;
		    serverTemp.utilizationCPU = (*ppServers)[i][j]->VMRequiresThisMuchUtilization(); 
			serverTemp.utilizationMemory = (*ppServers)[i][j]->VMRequiresThisMemory();   
			serverTemp.memoryServer = (*ppServers)[i][j]->GetMemoryServer();
		 	serverTemp.predictedOverload = false;
		 	serverTemp.speedKBPS = (*ppServers)[i][j]->ReturnBandWidthServerKBPS();
			serverTemp.trafficKBPS = (*ppServers)[i][j]->ReturnServerTrafficKBPS();

			EFF_Temp = 1 - pow(((serverTemp.temperature - TEMPLOW) / (TEMPHIGHT - TEMPLOW)), E_TEMPERATURE);
		 	EFF_CPU = 1 - pow(((serverTemp.utilizationCPU - CPULOW) / (CPUHIGHT - CPULOW)), E_CPU);
			EFF_Power = 1 - pow((((*ppServers)[i][j]->GetPowerDraw() - POWERLOW) / (POWERHIGHT - POWERLOW)), E_POWER);
			EFF_Memory = pow(( FLOATINGPOINT ((serverTemp.memoryServer - serverTemp.utilizationMemory) - 0) / FLOATINGPOINT (TOTAL_OF_MEMORY_IN_ONE_SERVER - 0)), E_MEMORY);
			EFF_Traffic = 1 - pow(((serverTemp.trafficKBPS - 0) / (FLOATINGPOINT (serverTemp.speedKBPS  - 0))), E_TRAFFIC); 

			serverTemp.ranking = (WEIGHT_TEMPERATURE*(ALPHA_3DMOBFD * EFF_Temp)) + (WEIGHT_CPU*(BETA_3DMOBFD * EFF_CPU)) + (WEIGHT_POWER*(GAMMA_3DMOBFD * EFF_Power)) + (WEIGHT_MEMORY*(DELTA_3DMOBFD * EFF_Memory)) + (WEIGHT_TRAFFIC*(EPSILON_3DMOBFD * EFF_Traffic));
//			cout << " serverTemp.ranking " << serverTemp.ranking << " " << EFF_Temp << " " << EFF_CPU << " " << EFF_Power << " " << EFF_Memory << " " << EFF_Traffic << endl;
//			cout <<  serverTemp.chassi << " "  << serverTemp.server << " " << "serverTemp.memoryServer " << serverTemp.memoryServer << " serverTemp.utilizationMemory " << serverTemp.utilizationMemory << " TOTAL_OF_MEMORY_IN_ONE_SERVER " << TOTAL_OF_MEMORY_IN_ONE_SERVER << endl;
			serverScheduling.push_back(serverTemp);
	    }
    }

    sort(serverScheduling.begin(), serverScheduling.end(), Sort_Ranking);
 } 

  // assign VMs to Servers
  while (!pqVMsToGo->empty()) {
	    noScheduler = false;
        for (unsigned int i=0; i < serverScheduling.size(); i++) {
			if (((serverScheduling[i].utilizationCPU + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) < THRESHOLD_TOP_OF_USE_CPU) && ((serverScheduling[i].utilizationMemory + pqVMsToGo->front()->GetMemUseVM()) <= serverScheduling[i].memoryServer) && (serverScheduling[i].temperature < EMERGENCY_TEMPERATURE)) {
   		       serverScheduling[i].utilizationCPU += (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER);
			   serverScheduling[i].utilizationMemory += pqVMsToGo->front()->GetMemUseVM();
			   pqVMsToGo->front()->SetClock(*clock);
			   (*ppServers)[serverScheduling[i].chassi][serverScheduling[i].server]->AssignOneVM(pqVMsToGo->front());
			   pqVMsToGo->pop();
			   totalScheduling += 1;
			   noScheduler = true;
			   break;
		    }
		}
		if (!noScheduler) {
			VMsScheduler.push(pqVMsToGo->front());
			pqVMsToGo->pop();
		}
  }

  if (VMsScheduler.size() > 0) {
     powerON = (int) ceil(((double) VMsScheduler.size() / (double) NUMBER_OF_CORES_IN_ONE_SERVER ));
     ppollServers->AddPowerOn(powerON);
  }

  while (!VMsScheduler.empty()) {
	     pqVMsToGo->push(VMsScheduler.front());
		 VMsScheduler.pop();
  }

}

THREEDMOBFDAndPredictionCPUAndTemperatureSchedulingAlgorithm::THREEDMOBFDAndPredictionCPUAndTemperatureSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, POOLServers* ppool, unsigned int* clockSimulation)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	ppollServers = ppool;
	totalScheduling = 0;
	clock = clockSimulation;

	SCHEDULING_WITH_PREDICTION = true;

	cout << "SimDC3D-NOTICE: The data center scheduler is 3DMOBFD with Prediction !!!" << endl;
}

void THREEDMOBFDAndPredictionCPUAndTemperatureSchedulingAlgorithm::AssignVMs()
{

 FLOATINGPOINT EFF_Temp = 0.00;
 FLOATINGPOINT EFF_CPU = 0;
 FLOATINGPOINT EFF_Power = 0;
 FLOATINGPOINT EFF_Memory = 0;
 FLOATINGPOINT EFF_Traffic = 0;
 FLOATINGPOINT powerON = 0;

 bool noScheduler;

 SORTSERVER serverTemp;
 
 queue<VirtualMachine*> VMsScheduler;
 
 vector<double> predictionTemp;
 vector<SORTSERVER> serverScheduling;


 if (!pqVMsToGo->empty()) {
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
	    for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
	        if ( ((*ppServers)[i][j]->IsOFF()) || ((*ppServers)[i][j]->IsHibernating()) || ((*ppServers)[i][j]->IsENDING()) || ((*ppServers)[i][j]->IsPOOL()) || ((*ppServers)[i][j]->IsMIGRATING()) || ((*ppServers)[i][j]->IsINITIALIZING())) { 	
		       continue;
		    }
		    serverTemp.chassi = i;
		    serverTemp.server = j;
		    serverTemp.temperature = (*ppServers)[i][j]->CurrentInletTemperature();
			serverTemp.temperatureFuture = 0.0;
		    serverTemp.utilizationCPU = (*ppServers)[i][j]->VMRequiresThisMuchUtilization(); 
			serverTemp.utilizationMemory = (*ppServers)[i][j]->VMRequiresThisMemory();   
			serverTemp.memoryServer = (*ppServers)[i][j]->GetMemoryServer();
			serverTemp.predictedOverload = (*ppServers)[i][j]->ReturnCPUPrediction();
			serverTemp.speedKBPS = (*ppServers)[i][j]->ReturnBandWidthServerKBPS();
			serverTemp.trafficKBPS = (*ppServers)[i][j]->ReturnServerTrafficKBPS();

			if ((*ppServers)[i][j]->ReturnSizeVectorTemperature() == SIZE_WINDOWN_PREDICTION) {
			   if (PREDICTION_ALGORITHM=="POLYNOM"){
				  predictionTemp = runPolynom( (*ppServers)[i][j]->ReturnVectorTemperature() );  // run prediction
			   } 
			   else {
                  if (PREDICTION_ALGORITHM=="RBF"){
				     predictionTemp = runRBF( (*ppServers)[i][j]->ReturnVectorTemperature() );  // run prediction
				  }
				  else {
					 predictionTemp = runPolynom( (*ppServers)[i][j]->ReturnVectorTemperature() );  // run prediction default
			      }
			   }  
			   if (!predictionTemp.empty()) {
		          if ( predictionTemp[CLENGTH-1] >= 10 && predictionTemp[CLENGTH-1] <= 34) {
				     serverTemp.temperatureFuture = predictionTemp[CLENGTH-1];
  			         (*ppServers)[i][j]->InsertTemperaturePredictionServer(predictionTemp[CLENGTH-1]);
				     (*ppServers)[i][j]->InsertTimePredictionServer((*ppServers)[i][j]->ReturnClock()+(CLENGTH*MONITORINGTIME));
				     (*ppServers)[i][j]->AddHitPrediction();
				  }
			  	  else {
                     if ( predictionTemp[0] >= 10 && predictionTemp[0] <= 34) {
					    serverTemp.temperatureFuture = predictionTemp[0];
						(*ppServers)[i][j]->InsertTemperaturePredictionServer(predictionTemp[0]);
				        (*ppServers)[i][j]->InsertTimePredictionServer((*ppServers)[i][j]->ReturnClock()+(CLENGTH*MONITORINGTIME));
				        (*ppServers)[i][j]->AddHitPrediction();
				     }
				     else {
					    (*ppServers)[i][j]->AddErrorPrediction();
					    serverTemp.temperatureFuture = (*ppServers)[i][j]->CurrentInletTemperature();
				     }
				  }
			   }
			   predictionTemp.erase(predictionTemp.begin(), predictionTemp.end());
			}
			else {
			   serverTemp.temperatureFuture = serverTemp.temperature;
			}



			EFF_Temp = 1 - pow(((serverTemp.temperatureFuture - TEMPLOW) / (TEMPHIGHT - TEMPLOW)), E_TEMPERATURE);
		 	EFF_CPU = 1 - pow(((serverTemp.utilizationCPU - CPULOW) / (CPUHIGHT - CPULOW)), E_CPU);
			EFF_Power = 1 - pow((((*ppServers)[i][j]->GetPowerDraw() - POWERLOW) / (POWERHIGHT - POWERLOW)), E_POWER);
			EFF_Memory = pow(( FLOATINGPOINT ((serverTemp.memoryServer - serverTemp.utilizationMemory) - 0) / FLOATINGPOINT (TOTAL_OF_MEMORY_IN_ONE_SERVER - 0)), E_MEMORY);
			EFF_Traffic = 1 - pow(((serverTemp.trafficKBPS - 0) / (FLOATINGPOINT (serverTemp.speedKBPS  - 0))), E_TRAFFIC); 

			serverTemp.ranking = (WEIGHT_TEMPERATURE*(ALPHA_3DMOBFD * EFF_Temp)) + (WEIGHT_CPU*(BETA_3DMOBFD * EFF_CPU)) + (WEIGHT_POWER*(GAMMA_3DMOBFD * EFF_Power)) + (WEIGHT_MEMORY*(DELTA_3DMOBFD * EFF_Memory)) + (WEIGHT_TRAFFIC*(EPSILON_3DMOBFD * EFF_Traffic));

			//CPU 
			if (serverTemp.predictedOverload)  {
			   serverTemp.ranking = 0;
			}
			serverScheduling.push_back(serverTemp);
	     }
     }

    sort(serverScheduling.begin(), serverScheduling.end(), Sort_Ranking);
   
  } 

  // assign VMs to Servers
  while (!pqVMsToGo->empty())  {
	    noScheduler = false;
        for (unsigned int i=0; i < serverScheduling.size(); i++) {
			if ( (serverScheduling[i].predictedOverload) || (serverScheduling[i].temperatureFuture > EMERGENCY_TEMPERATURE) ) {
			   continue;
			}
			if (((serverScheduling[i].utilizationCPU + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) < THRESHOLD_TOP_OF_USE_CPU) && ((serverScheduling[i].utilizationMemory + pqVMsToGo->front()->GetMemUseVM()) <= serverScheduling[i].memoryServer) && (serverScheduling[i].temperature < EMERGENCY_TEMPERATURE))  {
   		       serverScheduling[i].utilizationCPU += (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER);
			   serverScheduling[i].utilizationMemory += pqVMsToGo->front()->GetMemUseVM();
			   pqVMsToGo->front()->SetClock(*clock);
			   (*ppServers)[serverScheduling[i].chassi][serverScheduling[i].server]->AssignOneVM(pqVMsToGo->front());
			   pqVMsToGo->pop();
			   totalScheduling += 1;
			   noScheduler = true;
			   break;
		    }
		}
		if (!noScheduler) {
			VMsScheduler.push(pqVMsToGo->front());
			pqVMsToGo->pop();
		}
  }

  if (VMsScheduler.size() > 0) {
     powerON = (int) ceil(((double) VMsScheduler.size() / (double) NUMBER_OF_CORES_IN_ONE_SERVER ));
     ppollServers->AddPowerOn(powerON);
  }

  while (!VMsScheduler.empty())  {
	     pqVMsToGo->push(VMsScheduler.front());
		 VMsScheduler.pop();
  }
}

THREEDMOBFDAndPoolSchedulingAlgorithm::THREEDMOBFDAndPoolSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, POOLServers* ppool, unsigned int* clockSimulation)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	ppollServers = ppool;
	totalScheduling = 0;
	clock = clockSimulation;

	SCHEDULING_WITH_PREDICTION = false;

	cout << "SimDC3D-NOTICE: The data center scheduler is 3DMOBFD with Pool Server !!!" << endl;
}

void THREEDMOBFDAndPoolSchedulingAlgorithm::AssignVMs()
{
 
 FLOATINGPOINT EFF_Temp = 0.00;
 FLOATINGPOINT EFF_CPU = 0;
 FLOATINGPOINT EFF_Power = 0;
 FLOATINGPOINT EFF_Memory = 0;
 FLOATINGPOINT EFF_Traffic = 0;

 vector<SORTSERVER> serverScheduling;
 
 SORTSERVER serverTemp;

 POOL sv;

 int RemovePOOL = 0; 
 
 unsigned int full = 0;


 if (!pqVMsToGo->empty()) {
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
	    for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
	        if ( ((*ppServers)[i][j]->IsOFF()) || ((*ppServers)[i][j]->IsHibernating()) || ((*ppServers)[i][j]->IsENDING()) || ((*ppServers)[i][j]->IsPOOL()) || ((*ppServers)[i][j]->IsMIGRATING()) || ((*ppServers)[i][j]->IsINITIALIZING())) { 	
		       continue;
		    }

			serverTemp.chassi = i;
		    serverTemp.server = j;
		    serverTemp.temperature = (*ppServers)[i][j]->CurrentInletTemperature();
			serverTemp.temperatureFuture = 0.0;
		    serverTemp.utilizationCPU = (*ppServers)[i][j]->VMRequiresThisMuchUtilization(); 
			serverTemp.utilizationMemory = (*ppServers)[i][j]->VMRequiresThisMemory();   
			serverTemp.memoryServer = (*ppServers)[i][j]->GetMemoryServer();
		 	serverTemp.predictedOverload = false;
		 	serverTemp.speedKBPS = (*ppServers)[i][j]->ReturnBandWidthServerKBPS();
			serverTemp.trafficKBPS = (*ppServers)[i][j]->ReturnServerTrafficKBPS();

			EFF_Temp = 1 - pow(((serverTemp.temperature - TEMPLOW) / (TEMPHIGHT - TEMPLOW)), E_TEMPERATURE);
		 	EFF_CPU = 1 - pow(((serverTemp.utilizationCPU - CPULOW) / (CPUHIGHT - CPULOW)), E_CPU);
			EFF_Power = 1 - pow((((*ppServers)[i][j]->GetPowerDraw() - POWERLOW) / (POWERHIGHT - POWERLOW)), E_POWER);
			EFF_Memory = pow(( FLOATINGPOINT ((serverTemp.memoryServer - serverTemp.utilizationMemory) - 0) / FLOATINGPOINT (TOTAL_OF_MEMORY_IN_ONE_SERVER - 0)), E_MEMORY);
			EFF_Traffic = 1 - pow(((serverTemp.trafficKBPS - 0) / (FLOATINGPOINT (serverTemp.speedKBPS  - 0))), E_TRAFFIC); 

			serverTemp.ranking = (WEIGHT_TEMPERATURE*(ALPHA_3DMOBFD * EFF_Temp)) + (WEIGHT_CPU*(BETA_3DMOBFD * EFF_CPU)) + (WEIGHT_POWER*(GAMMA_3DMOBFD * EFF_Power)) + (WEIGHT_MEMORY*(DELTA_3DMOBFD * EFF_Memory)) + (WEIGHT_TRAFFIC*(EPSILON_3DMOBFD * EFF_Traffic));
			serverScheduling.push_back(serverTemp);
	     }
     }

    sort(serverScheduling.begin(), serverScheduling.end(), Sort_Ranking);
   
  } 

   // assign VMs to Servers
  while (!pqVMsToGo->empty())  {
	    full = 0;
        for (unsigned int i=0; i < serverScheduling.size(); i++) {
            if (!pqVMsToGo->empty()) {
			   if (((serverScheduling[i].utilizationCPU + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) < THRESHOLD_TOP_OF_USE_CPU) && ((serverScheduling[i].utilizationMemory + pqVMsToGo->front()->GetMemUseVM()) < serverScheduling[i].memoryServer) && (serverScheduling[i].temperature < EMERGENCY_TEMPERATURE))  {
   		          serverScheduling[i].utilizationCPU += (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER);
			 	  serverScheduling[i].utilizationMemory += pqVMsToGo->front()->GetMemUseVM();
 			      pqVMsToGo->front()->SetClock(*clock);
			      (*ppServers)[serverScheduling[i].chassi][serverScheduling[i].server]->AssignOneVM(pqVMsToGo->front());
				  totalScheduling += 1;
			      pqVMsToGo->pop();
		       }
			   else {
                  if ((serverScheduling[i].utilizationCPU + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER) >= THRESHOLD_TOP_OF_USE_CPU) || ((serverScheduling[i].utilizationMemory + pqVMsToGo->front()->GetMemUseVM()) >= serverScheduling[i].memoryServer) || (serverScheduling[i].temperature >= EMERGENCY_TEMPERATURE)) {
					 full++;
				  }
				  continue;
		       }
		    }
		    else {
			  break;
		    }
		}
		if (full == serverScheduling.size()) {
		   sv = ppollServers->RemoveServerPOOL(ppServers);
		   if (sv.chassi != -1) {
              serverTemp.chassi = sv.chassi;
		      serverTemp.server = sv.server;
		      serverTemp.temperature = (*ppServers)[sv.chassi][sv.server]->CurrentInletTemperature();
		      serverTemp.utilizationCPU = (*ppServers)[sv.chassi][sv.server]->VMRequiresThisMuchUtilization();
			  serverTemp.utilizationMemory = (*ppServers)[sv.chassi][sv.server]->VMRequiresThisMemory();
			  serverTemp.memoryServer = (*ppServers)[sv.chassi][sv.server]->GetMemoryServer();
		      serverTemp.temperatureFuture = 0;
			  serverTemp.predictedOverload = false;
			  serverTemp.speedKBPS = (*ppServers)[sv.chassi][sv.server]->ReturnBandWidthServerKBPS();
			  serverTemp.trafficKBPS = (*ppServers)[sv.chassi][sv.server]->ReturnServerTrafficKBPS();


			  EFF_Temp = 1 - pow(((serverTemp.temperature - TEMPLOW) / (TEMPHIGHT - TEMPLOW)), E_TEMPERATURE);
		 	  EFF_CPU = 1 - pow(((serverTemp.utilizationCPU - CPULOW) / (CPUHIGHT - CPULOW)), E_CPU);
		 	  EFF_Power = 1 - pow((((*ppServers)[sv.chassi][sv.server]->GetPowerDraw() - POWERLOW) / (POWERHIGHT - POWERLOW)), E_POWER);
		 	  EFF_Memory = pow(( FLOATINGPOINT ((serverTemp.memoryServer - serverTemp.utilizationMemory) - 0) / FLOATINGPOINT (TOTAL_OF_MEMORY_IN_ONE_SERVER - 0)), E_MEMORY);
			  EFF_Traffic = 1 - pow(((serverTemp.trafficKBPS - 0) / (FLOATINGPOINT (serverTemp.speedKBPS  - 0))), E_TRAFFIC); 

			  serverTemp.ranking = (WEIGHT_TEMPERATURE*(ALPHA_3DMOBFD * EFF_Temp)) + (WEIGHT_CPU*(BETA_3DMOBFD * EFF_CPU)) + (WEIGHT_POWER*(GAMMA_3DMOBFD * EFF_Power)) + (WEIGHT_MEMORY*(DELTA_3DMOBFD * EFF_Memory)) + (WEIGHT_TRAFFIC*(EPSILON_3DMOBFD * EFF_Traffic));

			  serverScheduling.push_back(serverTemp);

			  pqVMsToGo->front()->SetClock(*clock);
			  (*ppServers)[serverScheduling[serverScheduling.size()-1].chassi][serverScheduling[serverScheduling.size()-1].server]->AssignOneVM(pqVMsToGo->front());
			  totalScheduling += 1;
		      pqVMsToGo->pop();
		      RemovePOOL++;
		      full = 0;
		   }
		   else{
			   // cout << "SimDC3D - Warning: No servers in the pool - Scheduling Algorithm - ThreeDimensionMultiObjAndPoolSchedulingAlgorithm!!!" << endl;
			   break;
		   }
		}
  }

  if (RemovePOOL > 0) {
	  ppollServers->AddPowerOn(RemovePOOL);
  }

}

THREEDMOBFDAndPoolAndPredictionCPUAndTemperatureSchedulingAlgorithm::THREEDMOBFDAndPoolAndPredictionCPUAndTemperatureSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, POOLServers* ppool, unsigned int* clockSimulation)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	ppollServers = ppool;
	totalScheduling = 0;
	clock = clockSimulation;

	SCHEDULING_WITH_PREDICTION = true;
	cout << "SimDC3D-NOTICE: The data center scheduler is 3DMOBFD with Pool Server and Prediction !!!" << endl;
}

void THREEDMOBFDAndPoolAndPredictionCPUAndTemperatureSchedulingAlgorithm::AssignVMs()
{

 FLOATINGPOINT EFF_Temp = 0.00;
 FLOATINGPOINT EFF_CPU = 0;
 FLOATINGPOINT EFF_Power = 0;
 FLOATINGPOINT EFF_Memory = 0;
 FLOATINGPOINT EFF_Traffic = 0;

 vector<SORTSERVER> serverScheduling;
 vector<double> predictionTemp;
 
 SORTSERVER serverTemp;
 
 queue<VirtualMachine*> VMsScheduler;

 int RemovePOOL = 0; 
 
 unsigned int full = 0;
 
 POOL sv;

 if (!pqVMsToGo->empty()) {
	for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
	    for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
		    if ( ((*ppServers)[i][j]->IsOFF()) || ((*ppServers)[i][j]->IsHibernating()) || ((*ppServers)[i][j]->IsENDING()) || ((*ppServers)[i][j]->IsPOOL()) || ((*ppServers)[i][j]->IsMIGRATING()) || ((*ppServers)[i][j]->IsINITIALIZING())) { 	
		       continue;
		    }
			serverTemp.chassi = i;
		    serverTemp.server = j;
		    serverTemp.temperature = (*ppServers)[i][j]->CurrentInletTemperature();
			serverTemp.temperatureFuture = 0.0;
		    serverTemp.utilizationCPU = (*ppServers)[i][j]->VMRequiresThisMuchUtilization(); 
			serverTemp.utilizationMemory = (*ppServers)[i][j]->VMRequiresThisMemory();   
			serverTemp.memoryServer = (*ppServers)[i][j]->GetMemoryServer();
			serverTemp.predictedOverload = (*ppServers)[i][j]->ReturnCPUPrediction();
			serverTemp.speedKBPS = (*ppServers)[i][j]->ReturnBandWidthServerKBPS();
			serverTemp.trafficKBPS = (*ppServers)[i][j]->ReturnServerTrafficKBPS();

			if ((*ppServers)[i][j]->ReturnSizeVectorTemperature() == SIZE_WINDOWN_PREDICTION) {
			   if (PREDICTION_ALGORITHM=="POLYNOM"){
				  predictionTemp = runPolynom( (*ppServers)[i][j]->ReturnVectorTemperature() );  // run prediction
			   } 
			   else{
                  if (PREDICTION_ALGORITHM=="RBF"){
					 predictionTemp = runRBF( (*ppServers)[i][j]->ReturnVectorTemperature() );  // run prediction
				  }
				  else {
					 predictionTemp = runPolynom( (*ppServers)[i][j]->ReturnVectorTemperature() );  // run prediction default
				  }
			   }  
			   if (!predictionTemp.empty()) {
		          if ( predictionTemp[CLENGTH-1] >= 10 && predictionTemp[CLENGTH-1] <= 34) {
				     serverTemp.temperatureFuture = predictionTemp[CLENGTH-1];					 
  			         (*ppServers)[i][j]->InsertTemperaturePredictionServer(predictionTemp[CLENGTH-1]);
				     (*ppServers)[i][j]->InsertTimePredictionServer((*ppServers)[i][j]->ReturnClock()+(CLENGTH*MONITORINGTIME));
				     (*ppServers)[i][j]->AddHitPrediction();
				  }
			  	  else {
                     if ( predictionTemp[0] >= 10 && predictionTemp[0] <= 34) {
					    serverTemp.temperatureFuture = predictionTemp[0];
					    (*ppServers)[i][j]->InsertTemperaturePredictionServer(predictionTemp[0]);
				        (*ppServers)[i][j]->InsertTimePredictionServer((*ppServers)[i][j]->ReturnClock()+(CLENGTH*MONITORINGTIME));
				        (*ppServers)[i][j]->AddHitPrediction();
				     }
				     else {
					    (*ppServers)[i][j]->AddErrorPrediction();
					    serverTemp.temperatureFuture = serverTemp.temperature;
				     }
				  }
			   }
			   predictionTemp.erase(predictionTemp.begin(), predictionTemp.end());
			}
			else {
			   serverTemp.temperatureFuture = serverTemp.temperature;
			}

			EFF_Temp = 1 - pow(((serverTemp.temperatureFuture  - TEMPLOW) / (TEMPHIGHT - TEMPLOW)), E_TEMPERATURE);
		 	EFF_CPU = 1 - pow(((serverTemp.utilizationCPU - CPULOW) / (CPUHIGHT - CPULOW)), E_CPU);
			EFF_Power = 1 - pow((((*ppServers)[i][j]->GetPowerDraw() - POWERLOW) / (POWERHIGHT - POWERLOW)), E_POWER);
			EFF_Memory = pow(( FLOATINGPOINT ((serverTemp.memoryServer - serverTemp.utilizationMemory) - 0) / FLOATINGPOINT (TOTAL_OF_MEMORY_IN_ONE_SERVER - 0)), E_MEMORY);
			EFF_Traffic = 1 - pow(((serverTemp.trafficKBPS - 0) / (FLOATINGPOINT (serverTemp.speedKBPS  - 0))), E_TRAFFIC); 

			serverTemp.ranking = (WEIGHT_TEMPERATURE*(ALPHA_3DMOBFD * EFF_Temp)) + (WEIGHT_CPU*(BETA_3DMOBFD * EFF_CPU)) + (WEIGHT_POWER*(GAMMA_3DMOBFD * EFF_Power)) + (WEIGHT_MEMORY*(DELTA_3DMOBFD * EFF_Memory)) + (WEIGHT_TRAFFIC*(EPSILON_3DMOBFD * EFF_Traffic));

			//CPU 
			if (serverTemp.predictedOverload)  {
			   serverTemp.ranking = 0;
			}
  		    serverScheduling.push_back(serverTemp);
	    }
    }
    sort(serverScheduling.begin(), serverScheduling.end(), Sort_Ranking);
  } 

  // assign VMs to Servers
  while (!pqVMsToGo->empty()) {
	    full = 0;
        for (unsigned int i=0; i < serverScheduling.size(); i++) {
			if ( (serverScheduling[i].predictedOverload) || (serverScheduling[i].temperatureFuture > EMERGENCY_TEMPERATURE) ) {
			   full += 1;
			   continue;
			}
            if (!pqVMsToGo->empty()) {
			   if (((serverScheduling[i].utilizationCPU + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) < THRESHOLD_TOP_OF_USE_CPU) && ((serverScheduling[i].utilizationMemory + pqVMsToGo->front()->GetMemUseVM()) < serverScheduling[i].memoryServer) && (serverScheduling[i].temperature < EMERGENCY_TEMPERATURE))  {
   		          serverScheduling[i].utilizationCPU += (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER);
				  serverScheduling[i].utilizationMemory += pqVMsToGo->front()->GetMemUseVM();
 			      pqVMsToGo->front()->SetClock(*clock);
			      (*ppServers)[serverScheduling[i].chassi][serverScheduling[i].server]->AssignOneVM(pqVMsToGo->front());
			 	  totalScheduling += 1;
			      pqVMsToGo->pop();
		       }
			   else {
                  if ((serverScheduling[i].utilizationCPU + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER) >= THRESHOLD_TOP_OF_USE_CPU) || ((serverScheduling[i].utilizationMemory + pqVMsToGo->front()->GetMemUseVM()) >= serverScheduling[i].memoryServer) || (serverScheduling[i].temperature >= EMERGENCY_TEMPERATURE)) {
				 	 full++;
				  }
				  continue;
		       }
		    }
		    else {
			   break;
		    }
		}
		if (full == serverScheduling.size()) {
		   sv = ppollServers->RemoveServerPOOL(ppServers);
		   if (sv.chassi != -1) {
              serverTemp.chassi = sv.chassi;
		      serverTemp.server = sv.server;
		      serverTemp.temperature = (*ppServers)[sv.chassi][sv.server]->CurrentInletTemperature();
		      serverTemp.utilizationCPU = (*ppServers)[sv.chassi][sv.server]->VMRequiresThisMuchUtilization();
			  serverTemp.utilizationMemory = (*ppServers)[sv.chassi][sv.server]->VMRequiresThisMemory();
			  serverTemp.memoryServer = (*ppServers)[sv.chassi][sv.server]->GetMemoryServer();
		      serverTemp.temperatureFuture = 0;
			  serverTemp.predictedOverload = false;
			  serverTemp.speedKBPS = (*ppServers)[sv.chassi][sv.server]->ReturnBandWidthServerKBPS();
			  serverTemp.trafficKBPS = (*ppServers)[sv.chassi][sv.server]->ReturnServerTrafficKBPS();

			  EFF_Temp = 1 - pow(((serverTemp.temperature  - TEMPLOW) / (TEMPHIGHT - TEMPLOW)), E_TEMPERATURE);
		 	  EFF_CPU = 1 - pow(((serverTemp.utilizationCPU - CPULOW) / (CPUHIGHT - CPULOW)), E_CPU);
			  EFF_Power = 1 - pow((((*ppServers)[sv.chassi][sv.server]->GetPowerDraw() - POWERLOW) / (POWERHIGHT - POWERLOW)), E_POWER);
		 	  EFF_Memory = pow(( FLOATINGPOINT ((serverTemp.memoryServer - serverTemp.utilizationMemory) - 0) / FLOATINGPOINT (TOTAL_OF_MEMORY_IN_ONE_SERVER - 0)), E_MEMORY);
			  EFF_Traffic = 1 - pow(((serverTemp.trafficKBPS - 0) / (FLOATINGPOINT (serverTemp.speedKBPS  - 0))), E_TRAFFIC); 

			  serverTemp.ranking = (WEIGHT_TEMPERATURE*(ALPHA_3DMOBFD * EFF_Temp)) + (WEIGHT_CPU*(BETA_3DMOBFD * EFF_CPU)) + (WEIGHT_POWER*(GAMMA_3DMOBFD * EFF_Power)) + (WEIGHT_MEMORY*(DELTA_3DMOBFD * EFF_Memory)) + (WEIGHT_TRAFFIC*(EPSILON_3DMOBFD * EFF_Traffic));

			  serverScheduling.push_back(serverTemp);

			  pqVMsToGo->front()->SetClock(*clock);
			  (*ppServers)[serverScheduling[serverScheduling.size()-1].chassi][serverScheduling[serverScheduling.size()-1].server]->AssignOneVM(pqVMsToGo->front());
			  totalScheduling += 1;
		      pqVMsToGo->pop();
		      RemovePOOL++;
		      full = 0;
		   }
		   else{
			   //cout << "SimDC3D - Warning: No servers in the pool - Scheduling Algorithm - ThreeDimensionMultiObjAndPoolAndPredictionCPUAndTemperatureSchedulingAlgorithm" << endl;
			   break;
		   }
		}
  }

  if (RemovePOOL > 0) {
	  ppollServers->AddPowerOn(RemovePOOL);
  }
}


FFDAndPoolSchedulingAlgorithm::FFDAndPoolSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, POOLServers* ppool, unsigned int* clockSimulation)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	ppollServers = ppool;
	totalScheduling = 0;
	clock = clockSimulation;

	SCHEDULING_WITH_PREDICTION = false;

	cout << "SimDC3D-NOTICE: The data center scheduler is FFD with Pool Server !!!" << endl;
}

void FFDAndPoolSchedulingAlgorithm::AssignVMs()
{
 
 POOL sv;
 int RemovePOOL = 0; 
 bool removePool = false; 

 
 // assign VMs to Servers
 while (!pqVMsToGo->empty()) {
	   // assign with qWaitingVMs.top()
	   removePool = true;
	   for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		   for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			   if ( ((*ppServers)[i][j]->IsOFF()) || ((*ppServers)[i][j]->IsHibernating()) || ((*ppServers)[i][j]->IsENDING()) || ((*ppServers)[i][j]->IsPOOL()) || ((*ppServers)[i][j]->IsMIGRATING()) || ((*ppServers)[i][j]->IsINITIALIZING())) { 	
				  continue;
			   }
			   if ( ((((*ppServers)[i][j]->VMRequiresThisMuchUtilization()) + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) < THRESHOLD_TOP_OF_USE_CPU) && (((*ppServers)[i][j]->VMRequiresThisMemory() + pqVMsToGo->front()->GetMemUseVM()) < (*ppServers)[i][j]->GetMemoryServer()) )  {
				  pqVMsToGo->front()->SetClock(*clock);
			      (*ppServers)[i][j]->AssignOneVM(pqVMsToGo->front());
			      totalScheduling += 1;
			      pqVMsToGo->pop();
			      removePool = false;
				  break;
		       }
	       }
		   if ( !removePool ) {
			  break;
		   }
       }
	   if (removePool) {
		   sv = ppollServers->RemoveServerPOOL(ppServers);
		   if (sv.chassi != -1) {
			  pqVMsToGo->front()->SetClock(*clock);
			  (*ppServers)[sv.chassi][sv.server]->AssignOneVM(pqVMsToGo->front());
			  totalScheduling += 1;
		      pqVMsToGo->pop();
		      RemovePOOL++;
		   }
		   else {
			  break;
		   }
	   }
 }
 if (RemovePOOL > 0) {
	ppollServers->AddPowerOn(RemovePOOL);
 }
} 


PABFDAndPoolSchedulingAlgorithm::PABFDAndPoolSchedulingAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], queue<VirtualMachine*>* pqvm, POOLServers* ppool, unsigned int* clockSimulation)
{
	ppServers = ps;
	pqVMsToGo = pqvm;
	ppollServers = ppool;
	totalScheduling = 0;
	clock = clockSimulation;

	SCHEDULING_WITH_PREDICTION = false;

	cout << "SimDC3D-NOTICE: The data center scheduler is FFD with Pool Server !!!" << endl;
}

void PABFDAndPoolSchedulingAlgorithm::AssignVMs()
{
 
 POOL sv;
 int RemovePOOL = 0; 
 bool removePool = false; 
 
 double powerServer = 0.00;
 double minPowerServer = 9999999.00;
 
 int selectChassi = 0;
 int selectServer = 0;

 // assign VMs to Servers
 while (!pqVMsToGo->empty()) {
	   // assign with qWaitingVMs.top()
	   removePool = true;
	   minPowerServer = 9999999;
	   for (int i=0; i<NUMBER_OF_CHASSIS; ++i) {
		   for (int j=0; j<NUMBER_OF_SERVERS_IN_ONE_CHASSIS; ++j) {
			   if ( ((*ppServers)[i][j]->IsOFF()) || ((*ppServers)[i][j]->IsHibernating()) || ((*ppServers)[i][j]->IsENDING()) || ((*ppServers)[i][j]->IsPOOL()) || ((*ppServers)[i][j]->IsMIGRATING()) || ((*ppServers)[i][j]->IsINITIALIZING())) { 	
				  continue;
			   }
			   if ( ((((*ppServers)[i][j]->VMRequiresThisMuchUtilization()) + (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER)) < THRESHOLD_TOP_OF_USE_CPU) && (((*ppServers)[i][j]->VMRequiresThisMemory() + pqVMsToGo->front()->GetMemUseVM()) < (*ppServers)[i][j]->GetMemoryServer()) )  {
			      powerServer = (*ppServers)[i][j]->EstimatePowerDraw((*ppServers)[i][j]->VMRequiresThisMuchUtilization(), (pqVMsToGo->front()->GetCPULoadRatio()/NUMBER_OF_CORES_IN_ONE_SERVER));
			      if (powerServer < minPowerServer) {
				     selectChassi = i;
				     selectServer = j;
				     minPowerServer = powerServer;   
                     removePool = false;
			      }
			   }
	       }
       }

	   if ( !removePool ) {
 		  pqVMsToGo->front()->SetClock(*clock);
		  (*ppServers)[selectChassi][selectServer]->AssignOneVM(pqVMsToGo->front());
		  totalScheduling += 1;
		  pqVMsToGo->pop();
       }
	   else {
		  sv = ppollServers->RemoveServerPOOL(ppServers);
		  if (sv.chassi != -1) {
			 pqVMsToGo->front()->SetClock(*clock);
			 (*ppServers)[sv.chassi][sv.server]->AssignOneVM(pqVMsToGo->front());
			 totalScheduling += 1;
		     pqVMsToGo->pop();
		     RemovePOOL++;
		  }
		  else {
			  break;
		  }
	   }
 }
 if (RemovePOOL > 0) {
	ppollServers->AddPowerOn(RemovePOOL);
 }
}  
