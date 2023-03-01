//
// Created by Ahmed Aredah
// Version 0.1
//

#ifndef NeTrainSim_NetSignal_h
#define NeTrainSim_NetSignal_h

#include <string>
#include <iostream>
#include <map>
#include "../util/Vector.h"

/**
 * A net link.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetLink;

/**
 * A net node.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetNode;
using namespace std;

/**
 * A network signal.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetSignal : public std::enable_shared_from_this<NetSignal> {
private:
	/** Number of signals in simulators */
	static unsigned int NumberOfSignalsInSimulator;

public:
	/** Identifier for the user */
	int userID;
	/** The identifier */
	int id;
	/** True if is green, false if not */
	bool isGreen;
	/** The proximity to activate */
	double proximityToActivate;

	/**
	 * Gets the link
	 *
	 * @returns	The link.
	 */
	std::weak_ptr <NetLink> link;
	/** The previous node */
	std::weak_ptr<NetNode> previousNode;

	/**
	 * Gets the current node
	 *
	 * @returns	The current node.
	 */
	std::weak_ptr <NetNode> currentNode;

	/**
	 * Constructor
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	signalID   	Identifier for the network signal.
	 * @param 	hostingLink	The hosting link.
	 */
	NetSignal(int signalID, std::shared_ptr <NetLink> hostingLink);

	/**
	 * Constructor
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	signalID			Identifier for the network signal.
	 * @param 	hostingLink			The hosting link.
	 * @param 	previousLinkNode	The previous link node.
	 * @param 	currentLinkNode 	The current link node.
	 */
	NetSignal(int signalID, std::shared_ptr<NetLink> hostingLink, 
		std::shared_ptr<NetNode> previousLinkNode, std::shared_ptr<NetNode> currentLinkNode);

	/**
	 * Gets number of signals in simulator
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @returns	The number of signals in simulator.
	 */
	static unsigned int getNumberOfSignalsInSimulator();

	/**
	 * Stream insertion operator
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param [in,out]	ostr	The ostr.
	 * @param 		  	stud	The stud.
	 *
	 * @returns	The shifted result.
	 */
	friend ostream& operator<<(ostream& ostr, const NetSignal& stud);


};
#endif // !NeTrainSim_NetSignal_h