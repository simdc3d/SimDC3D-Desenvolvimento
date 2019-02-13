#include <vector>
#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <cstring>

#include "FNSS-Parser.h"
#include "SimDC3D-TopologyOptimizationAlgorithms.h"

struct lEdges {
	string left;
	string right;
	//FLOATINGPOINT traffic = 0;
};

CarpoOptimizationAlgorithm::CarpoOptimizationAlgorithm(Server* (*ps)[SIZE_OF_HR_MATRIX][NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX], TopologySimDC3D* pTopologySimDC3D)
{
	tServers = ps;
	topologySimDC3D = pTopologySimDC3D;
	topologyFNSS = topologySimDC3D->ReturnFNSSTopology();
}

void CarpoOptimizationAlgorithm::OptimizationTopology()
{
	vector<lEdges> lEdgeAggregation;
	vector<lEdges> lAggregationCore;
	vector<lEdges> lEdgeCore;
	lEdges temp;

	list<int> switchEdge = topologySimDC3D->ReturnSwitchEdge();
	list<int> switchAggregation = topologySimDC3D->ReturnSwitchAggregation();
	list<int> switchCore = topologySimDC3D->ReturnSwitchCore();

	FLOATINGPOINT trafficTemp = 0;
	int k = 0;
	int flag = 0;
	int links = 0;
	int acumulator = 0;
	int capacityEdge = 0;
	int capacityMaxEdge = 0;

	if (TYPE_TOPOLOGY == "3LAYER") {
		// Optimizes bindings edge-aggregation
		for (list<int>::iterator it = switchEdge.begin(); it != switchEdge.end(); ++it) {
			for (list<int>::iterator iter = switchAggregation.begin(); iter != switchAggregation.end(); ++iter) {
				if ((topologyFNSS->hasEdge(to_string(*it), to_string(*iter))) && (!topologyFNSS->getEdge(to_string(*it), to_string(*iter))->ReturnIsOff())) {
					temp.left = to_string(*it);
					temp.right = to_string(*iter);
					lEdgeAggregation.push_back(temp);
					k++;
				}
			}
			k = 0;
			for (unsigned int i = 0; i < lEdgeAggregation.size(); i++) {
				trafficTemp += topologyFNSS->getEdge(lEdgeAggregation[i].left, lEdgeAggregation[i].right)->ReturnTraffic();
			}
			for (unsigned int i = 0; i < lEdgeAggregation.size(); i++) {
				if (flag == 1) {
					break;
				}

				if (trafficTemp <= (0.8 * topologyFNSS->getEdge(lEdgeAggregation[i].left, lEdgeAggregation[i].right)->getCapacity().getValue())) { // alterar para maior limite de capacidade <= 0.8
					for (unsigned int j = 0; j < lEdgeAggregation.size(); j++) {
						if (lEdgeAggregation[j].right != lEdgeAggregation[i].right) {
							topologyFNSS->powerOFFEdge(lEdgeAggregation[j].left, lEdgeAggregation[j].right);
							flag = 1;
						}
					}
				}
			}
			flag = 0;
			trafficTemp = 0;//999999
			lEdgeAggregation.clear();
		}

		// Optimizes bindings aggregation-core
		for (list<int>::iterator it = switchAggregation.begin(); it != switchAggregation.end(); ++it) {
			for (list<int>::iterator iter = switchCore.begin(); iter != switchCore.end(); ++iter) {
				if ((topologyFNSS->hasEdge(to_string(*it), to_string(*iter))) && (!topologyFNSS->getEdge(to_string(*it), to_string(*iter))->ReturnIsOff())) {
					temp.left = to_string(*it);
					temp.right = to_string(*iter);
					lAggregationCore.push_back(temp);
					k++;
				}
			}
			k = 0;

			for (unsigned int i = 0; i < lAggregationCore.size(); i++) {
				trafficTemp += topologyFNSS->getEdge(lAggregationCore[i].left, lAggregationCore[i].right)->ReturnTraffic();
			}

			for (unsigned int i = 0; i < lAggregationCore.size(); i++) {
				if (flag == 1) {
					break;
				}

				if (trafficTemp <= (0.8 * topologyFNSS->getEdge(lAggregationCore[i].left, lAggregationCore[i].right)->getCapacity().getValue())) { // alterar para maior limite de capacidade <= 0.8
					for (unsigned int j = 0; j < lAggregationCore.size(); j++) {
						if (lAggregationCore[j].right != lAggregationCore[i].right) {
							topologyFNSS->powerOFFEdge(lAggregationCore[j].left, lAggregationCore[j].right);
							flag = 1;
						}
					}
				}
				else {
					if (topologyFNSS->getEdge(lAggregationCore[i].left, lAggregationCore[i].right)->getCapacity().getValue() < topologyFNSS->getEdge(lAggregationCore[i].left, lAggregationCore[i].right)->returnMaximumCapacity()) {
						capacityEdge = topologyFNSS->getEdge(lAggregationCore[i].left, lAggregationCore[i].right)->getCapacity().getValue();
						capacityMaxEdge = topologyFNSS->getEdge(lAggregationCore[i].left, lAggregationCore[i].right)->returnMaximumCapacity();

						topologyFNSS->getEdge(lAggregationCore[i].left, lAggregationCore[i].right)->setCapacity(fnss::Quantity(capacityMaxEdge, fnss::Units::Bandwidth));

						if (topologyFNSS->getNode(lAggregationCore[i].left)->getProperty("type") == "switch") {
							topologyFNSS->getNode(lAggregationCore[i].left)->portsUsedSwitch(capacityEdge, false, '-');
							topologyFNSS->getNode(lAggregationCore[i].left)->portsUsedSwitch(capacityMaxEdge, false, '+');
						}
						if (topologyFNSS->getNode(lAggregationCore[i].right)->getProperty("type") == "switch") {
							topologyFNSS->getNode(lAggregationCore[i].right)->portsUsedSwitch(capacityEdge, false, '-');
							topologyFNSS->getNode(lAggregationCore[i].right)->portsUsedSwitch(capacityMaxEdge, false, '+');
						}

					}

					if (trafficTemp <= (0.8 * topologyFNSS->getEdge(lAggregationCore[i].left, lAggregationCore[i].right)->getCapacity().getValue())) { // alterar para maior limite de capacidade <= 0.8
						for (unsigned int j = 0; j < lAggregationCore.size(); j++) {
							if (lAggregationCore[j].right != lAggregationCore[i].right) {
								topologyFNSS->powerOFFEdge(lAggregationCore[j].left, lAggregationCore[j].right);
								flag = 1;
							}
						}
					}
				}

				// Turn off switches aggregation
				for (list<int>::iterator iterat = switchEdge.begin(); iterat != switchEdge.end(); ++iterat) {
					if (topologyFNSS->hasEdge(lAggregationCore[i].left, to_string(*iterat))) {
						links += 1;
						if (!topologyFNSS->getEdge(lAggregationCore[i].left, to_string(*iterat))->ReturnIsOff()) {
							break;
						}
						else {
							acumulator += 1;
						}
					}

				}
				if (links == acumulator) {
					if (!topologyFNSS->getNode(lAggregationCore[i].left)->isOFFSwitch()) {
						topologyFNSS->getNode(lAggregationCore[i].left)->powerOff();
						//cout << "desliguei o switch: " << lAggregationCore[i].left << endl;
						break;
					}
					break;
				}
			}
			flag = 0;
			links = 0;
			acumulator = 0;
			trafficTemp = 0;//999999
			lAggregationCore.clear();
		}
	}
	// It is a two layers topology
	else {
		if (TYPE_TOPOLOGY == "2LAYER") {
			// Optimizes bindings aggregation-core
			for (list<int>::iterator it = switchEdge.begin(); it != switchEdge.end(); ++it) {
				for (list<int>::iterator iter = switchCore.begin(); iter != switchCore.end(); ++iter) {
					if ((topologyFNSS->hasEdge(to_string(*it), to_string(*iter))) && (!topologyFNSS->getEdge(to_string(*it), to_string(*iter))->ReturnIsOff())) {
						temp.left = to_string(*it);
						temp.right = to_string(*iter);
						lEdgeCore.push_back(temp);
						k++;
					}
				}
				k = 0;

				for (unsigned int i = 0; i < lEdgeCore.size(); i++) {
					trafficTemp += topologyFNSS->getEdge(lEdgeCore[i].left, lEdgeCore[i].right)->ReturnTraffic();
				}

				for (unsigned int i = 0; i < lEdgeCore.size(); i++) {
					if (flag == 3) {
						break;
					}

					if (trafficTemp <= (0.8 * topologyFNSS->getEdge(lEdgeCore[i].left, lEdgeCore[i].right)->getCapacity().getValue())) { // alterar para maior limite de capacidade <= 0.8
						for (unsigned int j = 0; j < lEdgeCore.size(); j++) {
							if (lEdgeCore[j].right != lEdgeCore[i].right) {
								topologyFNSS->powerOFFEdge(lEdgeCore[j].left, lEdgeCore[j].right);
								flag += 1;
							}
						}
					}
					else {
						if (topologyFNSS->getEdge(lEdgeCore[i].left, lEdgeCore[i].right)->getCapacity().getValue() < topologyFNSS->getEdge(lEdgeCore[i].left, lEdgeCore[i].right)->returnMaximumCapacity()) {
							capacityEdge = topologyFNSS->getEdge(lEdgeCore[i].left, lEdgeCore[i].right)->getCapacity().getValue();
							capacityMaxEdge = topologyFNSS->getEdge(lEdgeCore[i].left, lEdgeCore[i].right)->returnMaximumCapacity();

							topologyFNSS->getEdge(lEdgeCore[i].left, lEdgeCore[i].right)->setCapacity(fnss::Quantity(capacityMaxEdge, fnss::Units::Bandwidth));

							if (topologyFNSS->getNode(lEdgeCore[i].left)->getProperty("type") == "switch") {
								topologyFNSS->getNode(lEdgeCore[i].left)->portsUsedSwitch(capacityEdge, false, '-');
								topologyFNSS->getNode(lEdgeCore[i].left)->portsUsedSwitch(capacityMaxEdge, false, '+');
							}
							if (topologyFNSS->getNode(lEdgeCore[i].right)->getProperty("type") == "switch") {
								topologyFNSS->getNode(lEdgeCore[i].right)->portsUsedSwitch(capacityEdge, false, '-');
								topologyFNSS->getNode(lEdgeCore[i].right)->portsUsedSwitch(capacityMaxEdge, false, '+');
							}

						}

						if (trafficTemp <= (0.8 * topologyFNSS->getEdge(lEdgeCore[i].left, lEdgeCore[i].right)->getCapacity().getValue())) { // alterar para maior limite de capacidade <= 0.8
							for (unsigned int j = 0; j < lEdgeCore.size(); j++) {
								if (lEdgeCore[j].right != lEdgeCore[i].right) {
									topologyFNSS->powerOFFEdge(lEdgeCore[j].left, lEdgeCore[j].right);
									flag += 1;
								}
							}
						}
					}
				}
				flag = 0;
				trafficTemp = 0;//999999
				lEdgeCore.clear();
			}
		}
		else {
			cout << "SIMDC3D-ERROR: Topology is not implemented !!!" << endl;
			exit(0);
		}
	}
	
}
