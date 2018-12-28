#include "stdio.h" 
#include <vector>

#include "SimDC3D-Constants.h"
#include "SimDC3D-VirtualMachine.h"
#include "SimDC3D-Structure.h"


void quickSort(SORTSERVER *server, int begin, int end)
{
   int i, j;
   double half;
   SORTSERVER aux;

   i = begin;
   j = end;

   half = server[(begin + end) / 2].ranking;

   do
   {
      while(server[i].ranking < half) {
           i++;
	  }
      while(server[j].ranking > half) {
           j--;
	  }
      if (i <= j) {
         aux = server[i];
         server[i] = server[j];
         server[j] = aux;
         i++;
         j--;
      }
   }while(i <= j);

   if (begin < j) { 
	  quickSort(server, begin, j);
   }
   if (i < end) {
      quickSort(server, i, end);
   }
}


bool sortTemperature(POOL SV_A, POOL SV_B)
{
	if (SV_A.temperature > SV_B.temperature) // > UP or < DOWN
	  return true;
  return false;
}

bool sortPOOLServer(POOL S_A, POOL S_B)
{
	if (S_A.temperature < S_B.temperature) // > UP or < DOWN
	  return true;
  return false;
}


bool sortCPUAscendant(STRUCTEMP SVOPT_A, STRUCTEMP SVOPT_B)
{
	if (SVOPT_A.utilizationCPU < SVOPT_B.utilizationCPU) // > UP or < DOWN
	  return true;
  return false;
}

bool sortCPUDescendant(STRUCTEMP SVOPT_A, STRUCTEMP SVOPT_B)
{
	if (SVOPT_A.utilizationCPU > SVOPT_B.utilizationCPU) // > UP or < DOWN
	  return true;
  return false;
}


bool sortTempOPT(STRUCTEMP SVOP_A, STRUCTEMP SVOP_B)
{
	if (SVOP_A.temperature > SVOP_B.temperature) // > UP or < DOWN
	  return true;
  return false;
}


bool sortAscendantUtilizationlistVMs(STRUCMIGRATION VM_A, STRUCMIGRATION VM_B)
{
	if (VM_A.VM->GetCPULoadRatio() < VM_B.VM->GetCPULoadRatio()) // > UP or < DOWN
	  return true;
  return false;
}

bool sortDecreasingUtilizationlistVMs(STRUCMIGRATION VM_A, STRUCMIGRATION VM_B)
{
	if (VM_A.VM->GetCPULoadRatio() > VM_B.VM->GetCPULoadRatio()) { // > UP or < DOWN
	  return true;
	}
  return false;
}


bool sortDecreasingRanking(STRUCOPTIMIZATION SV_A, STRUCOPTIMIZATION SV_B)
{
	if (SV_A.ranking > SV_B.ranking) // > UP or < DOWN
	  return true;
  return false;
}


bool sortAscendantWeight(STRUCOPTIMIZATION SV_A, STRUCOPTIMIZATION SV_B)
{
	if (SV_A.rankingWeight < SV_B.rankingWeight) // > UP or < DOWN
	  return true;
  return false;
}




bool sortAscendantTemperature(STRUCOPTIMIZATION SV_A, STRUCOPTIMIZATION SV_B)
{
	if (SV_A.temperature < SV_B.temperature) // > UP or < DOWN
	  return true;
  return false;
}

bool sortDecreasingUtilizationCPU(STRUCMIGRATION VM_A, STRUCMIGRATION VM_B)
{
	if (VM_A.utilizationCPU > VM_B.utilizationCPU) // > UP or < DOWN
	  return true;
  return false;
}

bool sortDecreasingUtilization(VirtualMachine* VM_A, VirtualMachine* VM_B)
{
  if (VM_A->GetCPULoadRatio() > VM_B->GetCPULoadRatio()) // > UP or < DOWN
	  return true;
  return false;
}


bool Sort_HigherCPU_LessMemory(VirtualMachine* VM_A, VirtualMachine* VM_B)
{
  double rankingA, rankingB, efMemoryA, efMemoryB;

  efMemoryA = 1 - (double) pow(((double) (VM_A->GetMemUseVM()-1000000) / (double) (8000000-1000000)),3);
  efMemoryB = 1 - (double) pow(((double) (VM_B->GetMemUseVM()-1000000) / (double) (8000000-1000000)),3);
  
  rankingA = VM_A->HowMuchCPULoadWillThisVMRequire() + efMemoryA;
  rankingB = VM_B->HowMuchCPULoadWillThisVMRequire() + efMemoryB;

  if (rankingA > rankingB) {
     return true;
  }
  return false;
}


bool Sort_HigherAverageCPU_LessMemory(VirtualMachine* VM_A, VirtualMachine* VM_B)
{
  double rankingA, rankingB, efMemoryA, efMemoryB;

  efMemoryA = 1 - pow(((double) (VM_A->GetMemUseVM()-1000000) / (double) (8000000-1000000)),3);
  efMemoryB = 1 - pow(((double) (VM_B->GetMemUseVM()-1000000) / (double) (8000000-1000000)),3);

  rankingA = VM_A->AverageCPU() + efMemoryA;
  rankingB = VM_B->AverageCPU() + efMemoryB;

  if (rankingA > rankingB) {
     return true;
  }
  return false;
}

bool Sort_HigherAverageCPU_LessMemory_HigherNetwork(VirtualMachine* VM_A, VirtualMachine* VM_B)
{
  double rankingA, rankingB, efMemoryA, efMemoryB, efNetworkA, efNetworkB;

  efMemoryA = 1 - pow(((double) (VM_A->GetMemUseVM()-1000000) / (double) (8000000-1000000)),3);		
  efMemoryB = 1 - pow(((double) (VM_B->GetMemUseVM()-1000000) / (double) (8000000-1000000)),3);

  efNetworkA = (double) (VM_A->ReturnVMUseNetwork() / 100);            //max utilization network 100 MB / min utilization network 0.1 MB
  efNetworkB = (double) (VM_B->ReturnVMUseNetwork() / 100);            //max utilization network 100 MB / min utilization network 0.1 MB


  rankingA = ((0.50*VM_A->AverageCPU()) + (0.40*efMemoryA) + (0.10*efNetworkA) );
  rankingB = ((0.50*VM_B->AverageCPU()) + (0.40*efMemoryB) + (0.10*efNetworkB) );

  if (rankingA > rankingB) {
     return true;
  }
  return false;
}

bool  Sort_HigherAverageCPU(VirtualMachine* VM_A, VirtualMachine* VM_B)
{
  if (VM_A->AverageCPU() > VM_B->AverageCPU()) { // > UP or < DOWN
	  return true;
  }
  return false;
}

bool Sort_LessMemory(VirtualMachine* VM_A, VirtualMachine* VM_B)
{
  if (VM_A->GetMemUseVM() < VM_B->GetMemUseVM()) { // > UP or < DOWN
     return true;
  }
  return false;
}

bool Sort_Ranking(SORTSERVER SV_A, SORTSERVER SV_B)
{
  if (SV_A.ranking > SV_B.ranking) { // > UP or < DOWN
     return true;
  }
  return false;
}

bool sort_Higher_IO_NetworkVM(STRUCMIGRATION VM_A, STRUCMIGRATION VM_B)
{
  if (VM_A.VM->ReturnVMUseNetwork() > VM_B.VM->ReturnVMUseNetwork()) { // > UP or < DOWN
	  return true;
  }
  return false;
}

bool sortAscendantTrafficNetWork(STRUCOPTIMIZATION SV_A, STRUCOPTIMIZATION SV_B)
{
	if (SV_A.trafficKBPS < SV_B.trafficKBPS) // > UP or < DOWN
	  return true;
  return false;
}

bool Sort_Higher_IO_Network(VirtualMachine* VM_A, VirtualMachine* VM_B)
{
  if (VM_A->ReturnVMUseNetwork() > VM_B->ReturnVMUseNetwork()) { // > UP or < DOWN
	  return true;
  }
  return false;
}