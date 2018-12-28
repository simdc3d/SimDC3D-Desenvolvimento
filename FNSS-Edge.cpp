#include "FNSS-Edge.h"

namespace fnss {

Edge::Edge(const Quantity &capacity_, const Quantity &delay_,
			const float &weight_, const Quantity &bufferSize_, const bool &isOFF_, const FLOATINGPOINT &sumTraffic_) :
			capacity(capacity_), weight(weight_), delay(delay_),
			bufferSize(bufferSize_), isOFF(isOFF_), sumTraffic(sumTraffic_) {
			
			isOFF = false;
			sumTraffic = 0.00;
			averageTraffic = 0.00;
			higherTraffic = 0.00;
            lowerTraffic = 99999999.00;
			previoustraffic = 0.00;
			numberAdded = 0;
			maximumCapacity = 0;
			varianceTraffic.clear();

			if (pthread_mutex_init(&mutexEdge, NULL) != 0) {
               cout << "SimDC3D-ERROR: Error create mutex Edge !!! " << endl;
               exit(0);
			}
		}

Quantity Edge::getCapacity() const {
	return this->capacity;
}

	

void Edge::setCapacity(const Quantity &capacity_) {
	this->capacity = capacity_;
}

float Edge::getWeight() const {
	return this->weight;
}

void Edge::setWeight(float weight_) {
	this->weight = weight_;
}

Quantity Edge::getDelay() const {
	return this->delay;
}

void Edge::setDelay(const Quantity &delay_) {
	this->delay = delay_;
}

Quantity Edge::getBufferSize() const {
	return this->bufferSize;
}

void Edge::setBufferSize(const Quantity &bufferSize_) {
	this->bufferSize = bufferSize_;
}

void Edge::powerOff(void) { 
	isOFF = true; 
}

void Edge::powerOn(void) { 
	isOFF = false; 
}

bool Edge::ReturnIsOff(void) { 
	return isOFF; 
}

FLOATINGPOINT Edge::ReturnTraffic(void) { 
	return sumTraffic; 
}


void Edge::AddTraffic(float traf) {
	
	 sumTraffic += traf;

	 if (traf > higherTraffic) {
		 higherTraffic = traf;
	 }

	 if ( (traf < lowerTraffic) && (traf != 0) )  {
		lowerTraffic = traf;
	 }
}

void Edge::ClearTraffic(void) {
	averageTraffic += sumTraffic;

	if (sumTraffic != 0) {
	   numberAdded += 1;
	   varianceTraffic.push_back(sumTraffic);
	}

	sumTraffic = 0.00;
}


FLOATINGPOINT Edge::ReturnSumTraffic(void) {
	 return averageTraffic;
}


FLOATINGPOINT Edge::ReturnAverageTraffic(void) { 
	return averageTraffic / numberAdded; 
}

FLOATINGPOINT Edge::ReturnStandardDeviation(void) { 
	
	FLOATINGPOINT avrTraffic = 0 ;
	FLOATINGPOINT dp = 0;
	FLOATINGPOINT vra = 0;

	avrTraffic = ReturnAverageTraffic();

	for (int i = 0; i < varianceTraffic.size(); i++)  {
		vra += pow((varianceTraffic[i] - avrTraffic),2);
    }

	vra = vra / varianceTraffic.size();

	dp = sqrt(vra);

	return dp;

	

}

}