CC=g++
CFLAGS=-c -Wall -I. -DNO_DEP_CHECK -std=c++11 -O3
LDFLAGS=-lstdc++ -pthread 
SOURCES=main.cpp SimDC3D-DataCenter.cpp SimDC3D-Server.cpp SimDC3D-JobQueue.cpp SimDC3D-SchedulingAlgorithm.cpp  \
	SimDC3D-Users.cpp SimDC3D-VirtualMachine.cpp SimDC3D-CRAC.cpp SimDC3D-ParametersPrediction.cpp \
	SimDC3D-PoolServers.cpp SimDC3D-QuickSort.cpp SimDC3D-ServerOptimizationAlgorithms.cpp SimDC3D-TrafficMatrix.cpp \
	SimDC3D-Topology.cpp SimDC3D-TopologyOptimizationAlgorithms.cpp SimDC3D-Rack.cpp SimDC3D-DataFlow.cpp TISEAN-AlgorithmPrediction.cpp \
	TISEAN-RoutinesTisean.cpp SimDC3D-PearsonCorrelation.cpp FNSS-Application.cpp FNSS-Event-Schedule.cpp FNSS-Parser.cpp FNSS-Quantity.cpp \
	FNSS-Traffic-Matrix-Sequence.cpp FNSS-Edge.cpp FNSS-Measurement-Unit.cpp FNSS-Property-Container.cpp FNSS-Topology.cpp \
	FNSS-Units.cpp FNSS-Event.cpp FNSS-Node.cpp FNSS-Protocol-Stack.cpp FNSS-Traffic-Matrix.cpp

OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=SimDC3D

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *o SimDC3D
