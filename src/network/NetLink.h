//
// Created by Ahmed Aredah
// Version 0.1
//

#ifndef NeTrainSim_NetLink_h
#define NeTrainSim_NetLink_h

#include <string>
#include <iostream>
#include <map>
#include "../util/Vector.h"

/**
 * @class	NetNode NetLink.h C:\Users\Ahmed\source\repos\NeTrainSim\src\network\NetLink.h
 *
 * @brief	A net node.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetNode;

/**
 * @class	Train NetLink.h C:\Users\Ahmed\source\repos\NeTrainSim\src\network\NetLink.h
 *
 * @brief	A train.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class Train;
using namespace std;

/**
 * @class	NetLink NetLink.h C:\Users\Ahmed\source\repos\NeTrainSim\src\network\NetLink.h
 *
 * @brief	A net link.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetLink {
private:
	/** @brief	Number of links in simulators */
	static unsigned int NumberOfLinksInSimulator;

public:
	/** @brief	The simulator identifier */
	int id;
	/** @brief	Identifier for the user */
	int userID;

	/** @brief Gets from node*/
	std::shared_ptr <NetNode> fromLoc;
	/** @brief	to node */
	std::shared_ptr<NetNode> toLoc;
	/** @brief	The length */
	double length;
	/** @brief	The free flow speed */
	double freeFlowSpeed;
	/** @brief	The traffic network signal no */
	int trafficSignalNo;
	/** @brief	The grade */
	map<int, double> grade;
	/** @brief	The curvature */
	double curvature;
	/** @brief	The direction */
	int direction;
	/** @brief	The speed variation */
	double speedVariation;
	/** @brief	The region */
	string region;
	/** @brief	Length of the links scale */
	double linksScaleLength;
	/** @brief	The links scale free speed */
	double linksScaleFreeSpeed;
	/** @brief	The current trains */
	Vector< std::shared_ptr<Train>> currentTrains;
	/** @brief	The cost */
	double cost;

	/**
	 * @fn	NetLink::NetLink(int linkID, std::shared_ptr<NetNode> fromNode, std::shared_ptr<NetNode> toNode, double linkLength, double maxSpeed, int trafficSignalID, double linkGrade, double linkCurvature, int linkNoOfDirections, double speedVariationfactor, string linkInRegion, double lengthScale, double maxSpeedScale);
	 *
	 * @brief	Constructor
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	linkID					Identifier for the link.
	 * @param 	fromNode				starting node.
	 * @param 	toNode					ending node.
	 * @param 	linkLength				Length of the link.
	 * @param 	maxSpeed				The maximum speed.
	 * @param 	trafficSignalID			Identifier for the traffic network signal.
	 * @param 	linkGrade				The link grade.
	 * @param 	linkCurvature			The link curvature.
	 * @param 	linkNoOfDirections  	The link no of directions.
	 * @param 	speedVariationfactor	The speed variationfactor.
	 * @param 	linkInRegion			The link in region.
	 * @param 	lengthScale				The length scale.
	 * @param 	maxSpeedScale			The maximum speed scale.
	 */
	NetLink(int linkID, std::shared_ptr<NetNode> fromNode, std::shared_ptr<NetNode> toNode,
		double linkLength, double maxSpeed, int trafficSignalID, double linkGrade, 
		double linkCurvature, int linkNoOfDirections, double speedVariationfactor,
		string linkInRegion, double lengthScale, double maxSpeedScale);

	/**
	 * @fn	void NetLink::updateLinksScaleLength(double newScale);
	 *
	 * @brief	Updates the links scale length described by newScale
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	newScale	The new scale.
	 */
	void updateLinksScaleLength(double newScale);

	/**
	 * @fn	static unsigned int NetLink::getNumberOfLinks();
	 *
	 * @brief	Gets number of links
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @returns	The number of links.
	 */
	static unsigned int getNumberOfLinks();

	/**
	 * @fn	void NetLink::updateLinksScaleFreeSpeed(double newScale);
	 *
	 * @brief	Updates the links scale free speed described by newScale
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	newScale	The new scale.
	 */
	void updateLinksScaleFreeSpeed(double newScale);
	friend ostream& operator<<(ostream& ostr, const NetLink& stud);

private:

	/**
	 * @fn	map<int, double> NetLink::setGrade(double grade);
	 *
	 * @brief	Sets a grade
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	grade	The grade.
	 *
	 * @returns	A map&lt;int,double&gt;
	 */
	map<int, double> setGrade(double grade);

	/**
	 * @fn	double NetLink::getCost();
	 *
	 * @brief	Gets the cost
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @returns	The cost.
	 */
	double getCost();



};
#endif // !NeTrainSim_NetLink_h