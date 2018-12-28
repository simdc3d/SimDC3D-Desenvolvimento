#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <queue>

#include "FNSS-Edge.h"
#include "FNSS-Parser.h"
#include "FNSS-Pair.h"
#include "FNSS-Topology.h"
#include "SimDC3D-Topology.h"
#include "SimDC3D-Constants.h"
#include "SimDC3D-Structure.h"
#include "SimDC3D-Server.h"
#include "SimDC3D-VirtualMachine.h"
#include "SimDC3D-PoolServers.h"
#include "SimDC3D-PearsonCorrelation.h"

using namespace std;

extern int NUMBER_OF_SERVERS_IN_ONE_CHASSIS;
extern int NUMBER_OF_CHASSIS;
extern int NUMBER_OF_CORES_IN_ONE_SERVER;
extern string NAME_FILE_TOPOLOGY;
extern string TYPE_TOPOLOGY;


class TopologyOptimizationAlgorithms
{
public:
    ~TopologyOptimizationAlgorithms(void);
    virtual void OptimizationTopology() = 0;

protected:
	Server * (*tServers)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX];
    unsigned int* network_clock;
};

class CarpoOptimizationAlgorithm : public TopologyOptimizationAlgorithms
{
public:
    CarpoOptimizationAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], TopologySimDC3D* pTopologySimDC3D);
    void OptimizationTopology();

private:
	TopologySimDC3D* topologySimDC3D;
	fnss::Topology* topologyFNSS;
};