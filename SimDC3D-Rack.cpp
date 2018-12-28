#include "SimDC3D-Rack.h"


Rack::Rack(int id)
{
 server_in_Rack.clear();
 identifier = id;
}


Rack::~Rack(void)
{
 server_in_Rack.clear();
}


void Rack::InsertServerToRack(Server* rserver)
{
 server_in_Rack.push_back(rserver);
}

Server* Rack::ReturnServerFromRack(int pos)
{
  return server_in_Rack[pos];
}

int Rack::ReturnTotalServerFromRack(void)
{
  return server_in_Rack.size();
}

void Rack::InsertSwitchToRack(string node)
{
   switchFNSS = node;
}