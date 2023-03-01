//
// Created by Ahmed Aredah
// Version 0.1
//

#ifndef NeTrainSim_NetSignalGroupController_h
#define NeTrainSim_NetSignalGroupController_h

#include <memory>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include "../util/Vector.h"
#include "../util/Map.h"

/**
 * A net link.
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetLink;

/**
 * A net node.
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetNode;

/**
 * A network signal.
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetSignal;
using namespace std;

/**
 * A controller for handling network signal groups.
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetSignalGroupController {
private:
	/** Number of signals in simulators */
	static unsigned int NumberOfSignalsInSimulator;

public:
	/** The network signals */
	Vector<std::shared_ptr <NetSignal>> networkSignalsGroup;
	/** At nodes */
	Vector< std::shared_ptr<NetNode>> atNodes;

	/** Gets the confined links */
	Vector < std::shared_ptr<NetLink>> confinedLinks;

	/** shows which movement is on and which are off. */
	Map < std::shared_ptr<NetSignal>, bool> movements;
	/** The time stamp */
	double timeStamp;
	/** The locked network signal */
	std::shared_ptr<NetSignal> lockedOnSignal;

	/** The signals in other direction */
	Vector<std::shared_ptr<NetSignal>> otherDirectionSignals;

	/**
	 * Constructor
	 * @author	Ahmed
	 * @date	2/14/2023
	 * @param 	nodes	The nodes that are confined by this network signals group controller.
	 */
	NetSignalGroupController(std::set<std::shared_ptr<NetNode>> nodes);

	/**
	 * Clears this object to its blank/initial state
	 * @author	Ahmed
	 * @date	2/14/2023
	 */
	void clear();

	/**
	 * Adds a node to the network signals group controller
	 * @author	Ahmed
	 * @date	2/14/2023
	 * @param 	node	The node.
	 */
	void addNode(std::shared_ptr<NetNode> node);

	/**
	 * Sends a pass request from the train to control a specific networkSignal at time step t
	 * @author	Ahmed
	 * @date	2/14/2023
	 * @param 	networkSignal	The requirested-to-control network signal.
	 */
	void sendPassRequestToControlTo(std::shared_ptr<NetSignal> networkSignal, double& simulationTime);

	/**
	 * Sets signals in same direction for the requested train
	 * @author	Ahmed Aredah
	 * @date	2/21/2023
	 * @param 	SameDirectionSignals	The same direction signals.
	 */
	void setSignalsInSameDirection(Vector<std::shared_ptr<NetSignal>> SameDirectionSignals);

	/**
	 * Keep the previous group feedback as and maintain the current network signal as on and the
	 * rest as off.
	 * @author	Ahmed
	 * @date	2/14/2023
	 * @param [in,out]	timeStep	The time step.
	 */
	void updateTimeStep(double& timeStep);

	/**
	 * Gets feed back of the group controller of which network signal should be turned on.
	 * @author	Ahmed
	 * @date	2/14/2023
	 */
	std::pair<std::shared_ptr<NetSignal>, Vector<std::shared_ptr<NetSignal>>> getFeedback();
	//NetSignal(Vector<std::shared_ptr <NetSignal>> groupSignals, Vector<std::shared_ptr<NetLink>> monitoredLinks);

	/**
	 * Stream insertion operator. It returns a string of the group details
	 * @author	Ahmed
	 * @date	2/14/2023
	 * @param [in,out]	ostr 	The ostr.
	 * @param 		  	group	The group is the instance of the groups.
	 * @returns	The shifted result.
	 */
	friend ostream& operator<<(ostream& ostr, const NetSignalGroupController& group);


};
#endif // !NeTrainSim_NetSignalGroupController_h
