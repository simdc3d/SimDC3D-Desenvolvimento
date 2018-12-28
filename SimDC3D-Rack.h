#pragma once

#include <vector>

#include "SimDC3D-Constants.h"
#include "SimDC3D-Server.h"

using namespace std;

extern int NUMBER_CHASSI_RACK;

class Rack 
{
public:
	Rack(int id);
	~Rack(void);
	void InsertServerToRack(Server* rserver);
	void InsertSwitchToRack(string node);
	Server* ReturnServerFromRack(int pos); 
	int ReturnTotalServerFromRack(void);
	inline int ReturnTotalServerInRack(void) { return server_in_Rack.size(); }
	inline string ReturnSwitchInRack(void) { return switchFNSS; }

private:
	int identifier;
	vector<Server*> server_in_Rack;
	string switchFNSS;
};