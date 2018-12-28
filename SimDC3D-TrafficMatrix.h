#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream> 

#include "SimDC3D-Constants.h"

#include "FNSS-Topology.h"
#include "FNSS-Edge.h"
#include "FNSS-Pair.h"
#include "FNSS-Parser.h"
#include "FNSS-Traffic-Matrix-Sequence.h"
#include "FNSS-Traffic-Matrix.h"
#include "FNSS-Measurement-Unit.h"
#include "FNSS-Quantity.h"
#include "FNSS-Units.h"
#include "FNSS-Protocol-Stack.h"
#include "FNSS-Node.h"
#include "FNSS-Application.h"

using namespace std;

extern string NAME_FILE_MATRIX_TRAFFIC;

class TrafficMatrixSimDC3D 
{
public:
	TrafficMatrixSimDC3D(void);
	~TrafficMatrixSimDC3D(void);

	void GetMatrixTraffic(int matrixT);

	FLOATINGPOINT GetDataFlow(string nodeO, string nodeD);

private:
	fnss::TrafficMatrixSequence matrixTraffic_;
	fnss::TrafficMatrix traffic_;

};
