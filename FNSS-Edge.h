#ifndef EDGE_H
#define EDGE_H

#include "FNSS-Quantity.h"
#include "FNSS-Units.h"

#include "SimDC3D-Constants.h"

#include <string>
#include <iostream>
#include <vector>
#include <math.h> 

using namespace std;

namespace fnss {

/**
 * Default link capacity, set if no capacity is specified in the constructor
 */
#define DEFAULT_CAPACITY "1Mbps"

/**
 * Default link delay, set if no delay is specified in the constructor
 */
#define DEFAULT_DELAY "1ms"

/**
 * Default link weight, set if no link weight is specified in the constructor
 */
#define DEFAULT_WEIGHT 1

/**
 * Default buffer size, set if no buffer size is specified in the constructor
 */
#define DEFAULT_BUFFER_SIZE "100packets"


#define DEFAULT_ISOFF false


/**
 * Represent an edge of a topology
 *
 * @author Cosmin Cocora
 */
	

class Edge {
public:
	/**
	 * Constructor.
	 */
	Edge(const Quantity &capacity_ = Quantity(DEFAULT_CAPACITY, Units::Bandwidth),
		const Quantity &delay_ = Quantity(DEFAULT_DELAY, Units::Time),
		const float &weight_ = DEFAULT_WEIGHT,
		const Quantity &bufferSize_ = Quantity(DEFAULT_BUFFER_SIZE, Units::BufferSize), 
		const bool &isOFF_ = DEFAULT_ISOFF, const FLOATINGPOINT &sumTraffic_ = 0.00 
		);

	/**
	 * Get the capacity of the link.
	 *
	 * @return the capacity.
	 */
	Quantity getCapacity() const;

	/**
	 * Set the the link's capacity.
	 *
	 * @param capacity the link's capacity.
	 */
	void setCapacity(const Quantity &capacity_);

	/**
	 * Get the weight of the link.
	 *
	 * @return the weight of the link.
	 */
	float getWeight() const;

	/**
	 * Set the weight of the link.
	 *
	 * @param weight the weight of the link.
	 */
	void setWeight(float weight_);

	/**
	 * Get the delay of the link.
	 *
	 * @return the delay of the link.
	 */
	Quantity getDelay() const;

	/**
	 * Set the the link's delay.
	 *
	 * @param delay the link's delay.
	 */
	void setDelay(const Quantity &delay_);

	/**
	 * Get the size of the buffer associated with this link.
	 *
	 * @return the buffer's size.
	 */
	Quantity getBufferSize() const;

	/**
	 * Set the size of the buffer associated with this link.
	 *
	 * @param bufferSize the buffer's size.
	 */
	void setBufferSize(const Quantity &bufferSize_);

	void powerOff(void); 
	void powerOn(void);
	void AddTraffic(float traf);
	void ClearTraffic(void);


	inline FLOATINGPOINT returnAverageTraffic(void) { return averageTraffic; } 
	inline FLOATINGPOINT returnHigherTraffic(void) { return higherTraffic; } 
	inline FLOATINGPOINT returnLowerTraffic(void) { return lowerTraffic; } 
	inline FLOATINGPOINT returnpreviousTraffic(void) { return previoustraffic; }
	inline void setMaximumCapacity(int mxCapacity) { maximumCapacity = mxCapacity; }
	inline int returnMaximumCapacity(void) { return maximumCapacity; }
	bool ReturnIsOff(void); 

	FLOATINGPOINT ReturnTraffic(void);
	FLOATINGPOINT ReturnAverageTraffic(void);
	FLOATINGPOINT ReturnSumTraffic(void);
	FLOATINGPOINT ReturnStandardDeviation(void);

	inline pthread_mutex_t* returnMutex(void) { return &mutexEdge; }

private:
	Quantity capacity;
	float weight;
	Quantity delay;
	Quantity bufferSize;
	bool isOFF;
	
	FLOATINGPOINT sumTraffic; 
	FLOATINGPOINT averageTraffic;
	FLOATINGPOINT higherTraffic;
    FLOATINGPOINT lowerTraffic;
	FLOATINGPOINT previoustraffic;
	int maximumCapacity;

	int numberAdded;

	vector<FLOATINGPOINT> varianceTraffic;

	pthread_mutex_t mutexEdge;
};

} //namespace

#endif //EDGE_H
