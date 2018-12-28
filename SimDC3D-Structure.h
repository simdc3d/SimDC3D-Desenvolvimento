#include "SimDC3D-VirtualMachine.h"


#pragma once

typedef struct sortserver {
	     int chassi;
		 int server;
		 FLOATINGPOINT temperature;
		 FLOATINGPOINT temperatureFuture;
		 FLOATINGPOINT averageUtilizationCPU;
		 FLOATINGPOINT utilizationCPU;
		 long int utilizationMemory;
		 long int memoryServer;
		 FLOATINGPOINT ranking;
		 bool predictedOverload;
		 long int speedKBPS;
		 FLOATINGPOINT trafficKBPS;
  } SORTSERVER;

typedef struct pool {
	     int chassi;
		 int server;
		 double temperature;
  } POOL;

typedef struct structemp {
	     int chassi;
		 int server;
		 FLOATINGPOINT temperature;
		 FLOATINGPOINT utilizationCPU;
		 bool predictedMigration;
		 bool isMigrate;
  } STRUCTEMP;

typedef struct strucoptimization {
	     int chassi;
		 int server;
		 FLOATINGPOINT temperature;
		 FLOATINGPOINT temperatureFuture;
		 FLOATINGPOINT utilizationCPU;
		 FLOATINGPOINT averageUtilizationCPU;
		 long int utilizationMemory;
		 long int memoryServer;
		 FLOATINGPOINT trafficKBPS;
		 FLOATINGPOINT averageTrafficKBPS;
		 long int speedKBPS;
		 FLOATINGPOINT ranking;
		 FLOATINGPOINT rankingWeight;
		 bool predictedMigration;
		 bool isMigrate;
		 FLOATINGPOINT power;
		 FLOATINGPOINT dcload;
  } STRUCOPTIMIZATION;


typedef struct strucmigration {
	     int chassi;
		 int server;
		 FLOATINGPOINT utilizationCPU;
		 VirtualMachine *VM;
 } STRUCMIGRATION;