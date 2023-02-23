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
 * @class	NetLink NetSignal.h c:\users\ahmed\source\repos\netrainsim\src\network\NetSignal.h
 *
 * @brief	A net link.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetLink;

/**
 * @class	NetNode NetSignal.h c:\users\ahmed\source\repos\netrainsim\src\network\NetSignal.h
 *
 * @brief	A net node.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetNode;
using namespace std;

/**
 * @class	NetSignal NetSignal.h c:\users\ahmed\source\repos\netrainsim\src\network\NetSignal.h
 *
 * @brief	A network signal.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetSignal : public std::enable_shared_from_this<NetSignal> {
private:
	/** @brief	Number of signals in simulators */
	static unsigned int NumberOfSignalsInSimulator;

public:
	/** @brief	Identifier for the user */
	int userID;
	/** @brief	The identifier */
	int id;
	/** @brief	True if is green, false if not */
	bool isGreen;
	/** @brief	The proximity to activate */
	double proximityToActivate;

	/**
	 * @property	std::weak_ptr <NetLink> link
	 *
	 * @brief	Gets the link
	 *
	 * @returns	The link.
	 */
	std::weak_ptr <NetLink> link;
	/** @brief	The previous node */
	std::weak_ptr<NetNode> previousNode;

	/**
	 * @property	std::weak_ptr <NetNode> currentNode
	 *
	 * @brief	Gets the current node
	 *
	 * @returns	The current node.
	 */
	std::weak_ptr <NetNode> currentNode;

	/**
	 * @fn	NetSignal::NetSignal(int signalID, std::shared_ptr <NetLink> hostingLink);
	 *
	 * @brief	Constructor
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	signalID   	Identifier for the network signal.
	 * @param 	hostingLink	The hosting link.
	 */
	NetSignal(int signalID, std::shared_ptr <NetLink> hostingLink);

	/**
	 * @fn	NetSignal::NetSignal(int signalID, std::shared_ptr<NetLink> hostingLink, std::shared_ptr<NetNode> previousLinkNode, std::shared_ptr<NetNode> currentLinkNode);
	 *
	 * @brief	Constructor
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
	 * @fn	static unsigned int NetSignal::getNumberOfSignalsInSimulator();
	 *
	 * @brief	Gets number of signals in simulator
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @returns	The number of signals in simulator.
	 */
	static unsigned int getNumberOfSignalsInSimulator();

	/**
	 * @fn	friend ostream& NetSignal::operator<<(ostream& ostr, const NetSignal& stud);
	 *
	 * @brief	Stream insertion operator
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