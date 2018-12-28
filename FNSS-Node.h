#ifndef NODE_H
#define NODE_H

#include "FNSS-Property-Container.h"
#include "FNSS-Protocol-Stack.h"
#include "FNSS-Application.h"

#include <string>
#include <map>
#include <exception>

namespace fnss {

/**
 * Represent a node of a topology
 * 
 * Each node comprises a set of properties (e.g. if the node is a router/switch or host),
 * a protocol stack and multiple applications.
 *  
 * @author Cosmin Cocora
 */

class Node : public PropertyContainer {
public:
	/**
	 * Constructor.
	 *
	 * @param stack the \c ProtocolStack to be deployed on the node.
	 */
	Node(const ProtocolStack &stack = ProtocolStack());

	/**
	 * Get a copy of the \c ProtocolStack currently deployed on the node.
	 *
	 * @return the deployed \c ProtocolStack.
	 */
	ProtocolStack getProtocolStack() const;

	/**
	 * Overwrite the currently deployed \c ProtocolStack.
	 *
	 * @param stack the \c ProtocolStack to deploy.
	 */
	void setProtocolStack(const ProtocolStack &stack);

	/**
	 * Get a a copy of the application with the specified name.
	 * 
	 * Throws an exception if the specified application was not found.
	 * 
	 * @param name name of the application
	 * @return a copy of the \c Application object.
	 */
	Application getApplication(const std::string &name) const;

	/**
	 * Get a a copy of the application with the specified name.
	 * 
	 * @param name name of the application.
	 * @param found set to \c true if the \c Application with the specified
	 * name was found, \c false otherwise.
	 * @return a copy of the \c Application object if present or
	 * \c Application() otherwise.
	 */
	// Application getApplication(const std::string &name, bool &found) const;

	/**
	 * Deploy or overwrite the given \c Application on the node.
	 *
	 * @param application the \c Application to deploy.
	 */
	void setApplication(const Application &application);

	/**
	 * Remove the specified \c Application from the node.
	 * 
	 * Throws an exception if the specified application was not found.
	 *
	 * @param name name of the application to remove.
	 * @return a copy of the removed \c Application object.
	 */
	Application removeApplication(const std::string &name);

	/**
	 * Remove the specified \c Application from the node.
	 *
	 * @param name name of the application to remove.
	 * @param found set to \c true if the \c Application with the specified
	 * name was found, \c false otherwise.
	 * @return a copy of the \c Application object if present or
	 * \c Application() otherwise. 
	 */
	// Application removeApplication(const std::string &name, bool &found);

	/**
	 * Get a \c std::set containing the names of all the application
	 * deployed on the node.
	 *
	 * @return a set with all the names of the stacks deployed.
	 */
	std::set <std::string> getAllApplications() const;

	void insertEnergyConsumption(float chassis, float lineCard, float port10, float port100, float port1000, float port10000);

	double getEnergyConsumption(void);

	void portsUsedSwitch(int speedPort, bool initialize, char operation);

	inline void powerOff(void) { isOFF = true; }
	inline void powerOn(void) { isOFF = false; }
	inline bool isOFFSwitch(void) { return isOFF; }
	inline int ReturnConnectedPorts10(void) { return connectedports10; }
	inline int ReturnConnectedPorts100(void) { return connectedports100; }
	inline int ReturnConnectedPorts1000(void) { return connectedports1000; }
	inline int ReturnConnectedPorts10000(void) { return connectedports10000; }
	inline int ReturnsActivePorts(void) { return connectedports10+connectedports100+connectedports1000+connectedports10000; }

	void printStatistic(void);

	class ApplicationNotFoundException : public std::exception {
	public:
		ApplicationNotFoundException(const std::string &name) throw() {
			this->exceptionStr = "The application named " + name + " was not found.";
		}

		~ApplicationNotFoundException() throw() {
		}

		const char* what() const throw() {
			return this->exceptionStr.c_str();
		}

	private:
		std::string exceptionStr;
	};

private:
	/**
	 * Map from application name to application
	 */
	typedef std::map <std::string, Application> ApplicationsType;
	std::map <std::string, Application> applications;

	/**
	 * The protocol stack deployed on the node.
	 */
	ProtocolStack stack;

	float eChassis;
    float eLineCard;
    float ePort10;
    float ePort100;
    float ePort1000;
    float ePort10000;

	int connectedports10;
	int connectedports100;
	int connectedports1000;
	int connectedports10000;

	int statisticCP10;
	int statisticCP100;
	int statisticCP1000;
	int statisticCP10000;
	int statisticDP10;
	int statisticDP100;
	int statisticDP1000;
	int statisticDP10000;


    // Switch energy model
    bool switches_eDVFS_enabled;	// enable DVFS
    bool switches_eDNS_enabled;	// enable DNS
    double switches_eDNS_delay;	//  time required to power down the switch defaul = 0.01

    bool isOFF;
};

} //namespace

#endif	//NODE_H
