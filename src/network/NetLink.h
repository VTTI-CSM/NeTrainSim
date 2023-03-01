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
 * A net node.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetNode;

/**
 * A train.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class Train;
using namespace std;

/**
 * A net link.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class NetLink {
private:
	/** Number of links in simulators */
	static unsigned int NumberOfLinksInSimulator;

public:
	/** The simulator identifier */
	int id;
	/** Identifier for the user */
	int userID;

	/**
	 * the starting node of the link
	 */
	std::shared_ptr <NetNode> fromLoc;
	/** the end node of the link*/
	std::shared_ptr<NetNode> toLoc;
	/** The length */
	double length;
	/** The free flow speed */
	double freeFlowSpeed;
	/** The traffic network signal no */
	int trafficSignalNo;
	/** The grade */
	map<int, double> grade;
	/** The curvature */
	double curvature;
	/** The direction */
	int direction;
	/** The speed variation */
	double speedVariation;
	/** The region */
	string region;
	/** Length of the links scale */
	double linksScaleLength;
	/** The links scale free speed */
	double linksScaleFreeSpeed;
	/** The current trains */
	Vector< std::shared_ptr<Train>> currentTrains;
	/** The cost */
	double cost;

	/**
	 * Constructor
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
	 * Updates the links scale length described by newScale
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	newScale	The new scale.
	 */
	void updateLinksScaleLength(double newScale);

	/**
	 * Gets number of links
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @returns	The number of links.
	 */
	static unsigned int getNumberOfLinks();

	/**
	 * Updates the links scale free speed described by newScale
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
	 * Sets a grade
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
	 * Gets the cost
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @returns	The cost.
	 */
	double getCost();



};
#endif // !NeTrainSim_NetLink_h