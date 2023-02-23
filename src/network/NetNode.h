//
// Created by Ahmed Aredah
// Version 0.1
//

#ifndef NeTrainSim_NetNode_h
#define NeTrainSim_NetNode_h

#include <string>
#include <iostream>
#include "../util/Vector.h"
#include "../util/Map.h"
using namespace std;

/**
 * @class	NetSignal NetNode.h C:\Users\Ahmed\source\repos\NeTrainSim\src\network\NetNode.h
 *
 * @brief	A network signal.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetSignal;

/**
 * @class	NetLink NetNode.h C:\Users\Ahmed\source\repos\NeTrainSim\src\network\NetNode.h
 *
 * @brief	A net link.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetLink;

/**
 * @class	NetNode NetNode.h C:\Users\Ahmed\source\repos\NeTrainSim\src\network\NetNode.h
 *
 * @brief	A net node.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetNode{
private:
	/** @brief	Number of nodes in simulators */
	static unsigned int NumberOfNodesInSimulator;

public:
	/** @brief	initial value, this values gets updated in the network class */
	int id = -1;
	/** @brief	read user node id */
	int userID;
	/** @brief	read x coordinate */
	double x;
	/** @brief	read y coordinate */
	double y;
	/** @brief	read description */
	string alphaDesc;
	/** @brief	read x - value scale */
	double xScale;
	/** @brief	read y - value scale */
	double yScale;
	/** @brief	The signals */
	Vector<std::shared_ptr<NetSignal>> networkSignals;
	/** @brief	true if the node is a stopping station / depot for all trains */
	bool isDepot;
	/** @brief is the dwell time at the depot */
	double dwellTimeIfDepot;
	/** @brief	The neighbour nodes with their link connection */
	Map<std::shared_ptr<NetNode>, Vector<std::shared_ptr<NetLink>>> linkTo;

#pragma region graphSearch

	/** @brief	The graph search distance from start */
	double graphSearchDistanceFromStart;
	/** @brief	The graph search isVisited: True if visited */
	bool graphSearchVisited;
	/** @brief	The path search previous node */
	std::shared_ptr<NetNode> graphSearchPreviousNode;
#pragma endregion

public:

	/**
	 * @fn	Vector<std::shared_ptr<NetNode>> NetNode::getNeighbors();
	 *
	 * @brief	Gets the neighbors of the nodes.
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @returns	The neighbors.
	 */
	Vector<std::shared_ptr<NetNode>> getNeighbors();

	/**
	 * @fn	void NetNode::clearGraphSearchParams();
	 *
	 * @brief	Clears the graph search parameters. This should be executed before running the graph search.
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 */
	void clearGraphSearchParams();

	/**
	 * @fn	NetNode::NetNode(int userID, double xCoord, double yCoord, string Desc, double xScale, double yScale);
	 *
	 * @brief	Constructor
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	userID	Identifier for the user.
	 * @param 	xCoord	The coordinate.
	 * @param 	yCoord	The coordinate.
	 * @param 	Desc  	The description.
	 * @param 	xScale	The scale.
	 * @param 	yScale	The scale.
	 */
	NetNode(int userID, double xCoord, double yCoord, string Desc, double xScale, double yScale);

	/**
	 * @fn	static unsigned int NetNode::getNumberOfNodesInSimulator();
	 *
	 * @brief	Gets number of nodes in simulator
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @returns	The number of nodes in simulator.
	 */
	static unsigned int getNumberOfNodesInSimulator();

	/**
	 * @fn	void NetNode::addSignal(std::shared_ptr<NetSignal> networkSignal);
	 *
	 * @brief	Adds a network signal
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	networkSignal	The network signal.
	 */
	void addSignal(std::shared_ptr<NetSignal> networkSignal);

	/**
	 * @fn	void NetNode::updateXScale(const double& newScale);
	 *
	 * @brief	Updates the x coordinate scale described by newScale
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	newScale	The new scale.
	 */
	void updateXScale(const double& newScale);

	/**
	 * @fn	void NetNode::updateYScale(const double& newScale);
	 *
	 * @brief	Updates the y coordinate scale described by newScale
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	newScale	The new scale.
	 */
	void updateYScale(const double& newScale);

	/**
	 * @fn	pair<double, double> NetNode::coordinates();
	 *
	 * @brief	Gets the coordinates
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @returns	A pair&lt;double,double&gt;
	 */
	pair<double, double> coordinates();

	/**
	 * @fn	friend ostream& NetNode::operator<<(ostream& ostr, const NetNode& stud);
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
	friend ostream& operator<<(ostream& ostr, const NetNode& stud);
};
#endif // !NeTrainSim_NetNode_h