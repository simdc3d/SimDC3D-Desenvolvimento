#include "SimDC3D-TrafficMatrix.h"

using namespace std;


TrafficMatrixSimDC3D::TrafficMatrixSimDC3D(void)
{
 cout << "SimDC3D-NOTICE: Reading matrix traffic !!! " << NAME_FILE_MATRIX_TRAFFIC << endl << endl;
 matrixTraffic_ = fnss::Parser::parseTrafficMatrixSequence(NAME_FILE_MATRIX_TRAFFIC);
 cout << "SimDC3D-NOTICE: Traffic matrix loaded !!!!" << endl << endl;
}

TrafficMatrixSimDC3D::~TrafficMatrixSimDC3D(void)
{

}

void TrafficMatrixSimDC3D::GetMatrixTraffic(int matrixT)
{
	traffic_ = matrixTraffic_.getMatrix(matrixT);
}

FLOATINGPOINT TrafficMatrixSimDC3D::GetDataFlow(string nodeO, string nodeD)
{
	return traffic_.getFlow(nodeO, nodeD).getValue();
}

