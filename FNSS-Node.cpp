#include "FNSS-Node.h"
#include <iostream>
using namespace std;

namespace fnss {

Node::Node(const ProtocolStack &stack_) : stack(stack_) {

 eChassis   = 0;
 eLineCard  = 0;
 ePort10    = 0;
 ePort100   = 0;
 ePort1000  = 0;
 ePort10000 = 0;

 connectedports10    = 0;
 connectedports100   = 0;
 connectedports1000  = 0;
 connectedports10000 = 0;

 statisticCP10		= 0;
 statisticCP100		= 0;
 statisticCP1000	= 0;
 statisticCP10000	= 0;
 statisticDP10		= 0;
 statisticDP100		= 0;
 statisticDP1000	= 0;
 statisticDP10000	= 0;
 
 // Switch energy model
 switches_eDVFS_enabled = false;		// enable DVFS
 switches_eDNS_enabled = false;			// enable DNS
 switches_eDNS_delay = 0.01;			// time required to power down the switch defaul = 0.01

 isOFF = false;

}

ProtocolStack Node::getProtocolStack() const {
	return this->stack;
}

void Node::setProtocolStack(const ProtocolStack &stack_) {
	this->stack = stack_;
}

Application Node::getApplication(const std::string &name) const {
	ApplicationsType::const_iterator it;
	it = this->applications.find(name);

	if(it != this->applications.end())
		return Application(it->second);
	else
		throw ApplicationNotFoundException(name);
}


void Node::setApplication(const Application &application) {
	this->applications[application.getName()] = application;
}

Application Node::removeApplication(const std::string &name) {
	ApplicationsType::iterator it;
	it = this->applications.find(name);

	if(it != this->applications.end()) {
		Application ret = it->second;
		this->applications.erase(it);
		return ret;
	} else
		throw ApplicationNotFoundException(name);
}


std::set <std::string> Node::getAllApplications() const {
	std::set<std::string> keys;
	ApplicationsType::const_iterator it;
	for(it = this->applications.begin();it != this->applications.end(); it++)
		keys.insert(it->first);
	return keys;
}

void Node::insertEnergyConsumption(float chassis, float lineCard, float port10, float port100, float port1000, float port10000) {
	eChassis = chassis;
    eLineCard = lineCard;
    ePort10 = port10;
    ePort100 = port100;
    ePort1000 = port1000;
    ePort10000 = port10000;
}


double Node::getEnergyConsumption(void){
  float consumption = 0.00;

  if (isOFF) {
	  return 0;
  }else {
      consumption = ( eChassis + eLineCard + (connectedports10*ePort10) + (connectedports100*ePort100) + (connectedports1000*ePort1000) + (connectedports10000*ePort10000) );
      return consumption;
  }

}

void Node::portsUsedSwitch(int speedPort, bool initialize, char operation) {

   if (initialize) {
	 connectedports10    = 0;
     connectedports100   = 0;
     connectedports1000  = 0;
     connectedports10000 = 0;

	 statisticCP10		= 0;
	 statisticCP100		= 0;
	 statisticCP1000	= 0;
	 statisticCP10000	= 0;
	 statisticDP10		= 0;
	 statisticDP100		= 0;
	 statisticDP1000	= 0;
	 statisticDP10000	= 0;
  }

  if (operation == '+') {
     if (speedPort == 10) {
        connectedports10 ++;
		statisticCP10 ++;
	 }
	 else {
		if (speedPort == 100) {
		    connectedports100 ++; 
			statisticCP100 ++;
		}				
		else {
		   if (speedPort == 1000) {
			  connectedports1000 ++; 
	  		  statisticCP1000 ++;
		   }
		   else {
              connectedports10000 ++; 
			  statisticCP10000 ++;
		   }
		}
	 }
  }

  if (operation == '-') {
     if (speedPort == 10) {
        connectedports10 --;   
		statisticDP10 ++;

	 }
	 else {
		if (speedPort == 100) {
		    connectedports100 --; 
			statisticDP100 ++;
		}				
		else {
		   if (speedPort == 1000) {
			  connectedports1000 --; 
			  statisticDP1000 ++;
		   }
		   else {
              connectedports10000 --;
			  statisticDP10000 ++;
		   }
		}
	 }
  }
}


void Node::printStatistic(void)
{
 cout << "			" << statisticCP10 << "		" << statisticCP100 << "	" << statisticCP1000 << "	" << statisticCP10000 << "			" << statisticDP10 << "	" << statisticDP100 << "	" << statisticDP1000 << "	" << statisticDP10000;
}

}