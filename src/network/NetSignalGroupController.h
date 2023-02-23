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
 * @class	NetLink NetSignalGroupController.h
 * 			C:\Users\Ahmed\source\repos\NeTrainSim\src\network\NetSignalGroupController.h
 *
 * @brief	A net link.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetLink;

/**
 * @class	NetNode NetSignalGroupController.h
 * 			C:\Users\Ahmed\source\repos\NeTrainSim\src\network\NetSignalGroupController.h
 *
 * @brief	A net node.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetNode;

/**
 * @class	NetSignal NetSignalGroupController.h
 * 			C:\Users\Ahmed\source\repos\NeTrainSim\src\network\NetSignalGroupController.h
 *
 * @brief	A network signal.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetSignal;
using namespace std;

/**
 * @class	NetSignalGroupController NetSignalGroupController.h
 * 			C:\Users\Ahmed\source\repos\NeTrainSim\src\network\NetSignalGroupController.h
 *
 * @brief	A controller for handling network signal groups.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetSignalGroupController {
private:
	/** @brief	Number of signals in simulators */
	static unsigned int NumberOfSignalsInSimulator;

public:
	/** @brief	The network signals */
	Vector<std::shared_ptr <NetSignal>> networkSignalsGroup;
	/** @brief	at nodes */
	Vector< std::shared_ptr<NetNode>> atNodes;

	/**
	 * @property	Vector < std::shared_ptr<NetLink>> confinedLinks
	 *
	 * @brief	Gets the confined links
	 */
	Vector < std::shared_ptr<NetLink>> confinedLinks;

	/** @brief shows which movement is on and which are off.*/
	Map < std::shared_ptr<NetSignal>, bool> movements;
	/** @brief	The time stamp */
	double timeStamp;
	/** @brief	The locked network signal */
	std::shared_ptr<NetSignal> lockedOnSignal;

	/** The signals in other direction */
	Vector<std::shared_ptr<NetSignal>> otherDirectionSignals;

	/**
	 * @fn	NetSignalGroupController::NetSignalGroupController(std::set<std::shared_ptr<NetNode>> nodes);
	 *
	 * @brief	Constructor
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	nodes	The nodes that are confined by this network signals group controller
	 */
	NetSignalGroupController(std::set<std::shared_ptr<NetNode>> nodes);

	/**
	 * @fn	void NetSignalGroupController::clear();
	 *
	 * @brief	Clears this object to its blank/initial state
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 */
	void clear();

	/**
	 * @fn	void NetSignalGroupController::addNode(std::shared_ptr<NetNode> node);
	 *
	 * @brief	Adds a node to the network signals group controller
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	node	The node.
	 */
	void addNode(std::shared_ptr<NetNode> node);

	/**
	 * @fn	void NetSignalGroupController::sendPassRequestToControlTo(std::shared_ptr<NetSignal> networkSignal, double& timeStep);
	 *
	 * @brief	Sends a pass request from the train to control a specific networkSignal at time step t
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 		  	networkSignal  	The requirested-to-control network signal.
	 * @param [in,out]	timeStep		The simulation current time at which the request is sent.
	 */
	void sendPassRequestToControlTo(std::shared_ptr<NetSignal> networkSignal, double& simulationTime);

	/**
	 * Sets signals in same direction for the requested train
	 *
	 * @author	Ahmed Aredah
	 * @date	2/21/2023
	 *
	 * @param 	SameDirectionSignals	The same direction signals.
	 */
	void setSignalsInSameDirection(Vector<std::shared_ptr<NetSignal>> SameDirectionSignals);

	 /**
	  * @fn	void NetSignalGroupController::updateTimeStep(double&amp; timeStep);
	  *
	  * @brief	Keep the previous group feedback as and maintain the current network signal as on and the rest as off.
	  *
	  * @author	Ahmed
	  *
	  * @date	2/14/2023
	  *
	  * @param [in,out]	timeStep	The time step.
	  */
	void updateTimeStep(double& timeStep);

	/**
	 * @fn	void NetSignalGroupController::getFeedBack();
	 *
	 * @brief	Gets feed back of the group controller of which network signal should be turned on.
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 */
	std::pair<std::shared_ptr<NetSignal>, Vector<std::shared_ptr<NetSignal>>> getFeedback();
	//NetSignal(Vector<std::shared_ptr <NetSignal>> groupSignals, Vector<std::shared_ptr<NetLink>> monitoredLinks);

	/**
	 * @fn	friend ostream& NetSignalGroupController::operator<<(ostream& ostr, const NetSignalGroupController& stud);
	 *
	 * @brief	Stream insertion operator. It returns a string of the group details
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param [in,out]	ostr	The ostr
	 * @param 		  	group	The group is the instance of the groups
	 *
	 * @returns	The shifted result.
	 */
	friend ostream& operator<<(ostream& ostr, const NetSignalGroupController& group);


};
#endif // !NeTrainSim_NetSignalGroupController_h
