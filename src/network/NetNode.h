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
 * A network signal.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetSignal;

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
class NetNode{
private:
	/** Number of nodes in simulators */
	static unsigned int NumberOfNodesInSimulator;

public:
	/** Initial value, this values gets updated in the network class */
	int id = -1;
	/** Read user node id */
	int userID;
	/** Read x coordinate */
	double x;
	/** Read y coordinate */
	double y;
	/** Read description */
	string alphaDesc;
	/** Read x - value scale */
	double xScale;
	/** Read y - value scale */
	double yScale;
	/** The signals */
	Vector<std::shared_ptr<NetSignal>> networkSignals;
	/** True if the node is a stopping station / depot for all trains */
	bool isDepot;
	/** Is the dwell time at the depot */
	double dwellTimeIfDepot;
    /** refill trains batteries and batteries if passed by this node. */
    bool refillTanksAndBatteries;
	/** The neighbour nodes with their link connection */
	Map<std::shared_ptr<NetNode>, Vector<std::shared_ptr<NetLink>>> linkTo;

// ##################################################################
// #                 start: graph search variables                  #
// ##################################################################

	/** The graph search distance from start */
	double graphSearchDistanceFromStart;
	/** The graph search isVisited: True if visited */
	bool graphSearchVisited;
	/** The path search previous node */
	std::shared_ptr<NetNode> graphSearchPreviousNode;
// ##################################################################
// #                   end: graph search variables                  #
// ##################################################################

public:

	/**
	 * Gets the neighbors of the nodes.
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @returns	The neighbors.
	 */
	Vector<std::shared_ptr<NetNode>> getNeighbors();

	/**
	 * Clears the graph search parameters. This should be executed before running the graph search.
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 */
	void clearGraphSearchParams();

	/**
	 * Constructor
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
	 * Gets number of nodes in simulator
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @returns	The number of nodes in simulator.
	 */
	static unsigned int getNumberOfNodesInSimulator();

	/**
	 * Adds a network signal
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	networkSignal	The network signal.
	 */
	void addSignal(std::shared_ptr<NetSignal> networkSignal);

	/**
	 * Updates the x coordinate scale described by newScale
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	newScale	The new scale.
	 */
	void updateXScale(const double& newScale);

	/**
	 * Updates the y coordinate scale described by newScale
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	newScale	The new scale.
	 */
	void updateYScale(const double& newScale);

	/**
	 * Gets the coordinates
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @returns	A pair&lt;double,double&gt;
	 */
	pair<double, double> coordinates();

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
	friend ostream& operator<<(ostream& ostr, const NetNode& stud);
};
#endif // !NeTrainSim_NetNode_h
