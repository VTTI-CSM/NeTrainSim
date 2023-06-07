#include <iostream>
#include <math.h>
#include <algorithm> 
//#include "../util/List.h"
//#include "../util/Vector.h"
#include "NetLink.h"
#include "NetNode.h"
//#include "../trainDefintion/Train.h"

using namespace std;

unsigned int NetLink::NumberOfLinksInSimulator = 0;

NetLink::NetLink(int simulatorID, int linkID, std::shared_ptr<NetNode> fromNodeID,
                 std::shared_ptr<NetNode> toNodeID, double linkLength,
                 double maxSpeed, int trafficSignalID, double linkGrade,
                 double linkCurvature, int linkNoOfDirections,
                 double speedVariationfactor, bool isCatenaryAvailable,
                 string linkInRegion, double lengthScale, double maxSpeedScale) {
    this->id = simulatorID;
	this->userID = linkID;
	this->fromLoc = fromNodeID;
	this->toLoc = toNodeID;
	this->length = linkLength;
	this->freeFlowSpeed = maxSpeed;
	this->trafficSignalNo = trafficSignalID;
	this->grade = this->setGrade(linkGrade);
	this->curvature = linkCurvature;
	this->direction = linkNoOfDirections;
	this->speedVariation = speedVariationfactor;
	this->region = linkInRegion;
	this->linksScaleLength = lengthScale;
	this->length *= linksScaleLength;
	this->linksScaleFreeSpeed = maxSpeedScale;
	this->freeFlowSpeed *= linksScaleFreeSpeed;
	this->cost = this->getCost();
    this->hasCatenary = isCatenaryAvailable;
    this->catenaryCumRegeneratedEnergy = 0.0;
    this->catenaryCumConsumedEnergy = 0.0;
	NetLink::NumberOfLinksInSimulator++;
}

NetLink::~NetLink(){
    NetLink::NumberOfLinksInSimulator--;
}

void NetLink::setLinkSimulatorID(int newID) {
    this->id = newID;
}


void NetLink::updateLinksScaleLength(double newScale) {
	double oldScale = this->linksScaleLength;
    this->length = (this->length / oldScale) * newScale;
    this->linksScaleLength = newScale;
}

unsigned int NetLink::getNumberOfLinks()
{
	return NetLink::NumberOfLinksInSimulator;
}

void NetLink::updateLinksScaleFreeSpeed(double newScale) {
	double oldScale = this->linksScaleFreeSpeed;
    this->freeFlowSpeed = (this->freeFlowSpeed / oldScale) * newScale;
    this->linksScaleFreeSpeed = newScale;
}

map<int, double> NetLink::setGrade(double grade) {
	map<int, double> directionalGrade;
	directionalGrade[this->fromLoc->id] = grade;
	directionalGrade[this->toLoc->id] = grade * -1;
	return directionalGrade;
}

double NetLink::getCost() {
	double t = this->length / this->freeFlowSpeed;
	if (direction == 1) {
		return t;
	}
	else {
		return pow(t, 2);
	}
}

ostream& operator<<(ostream& ostr, const NetLink& stud) {
	ostr << "Network link:: id: " << stud.id << ", from node id: " << stud.fromLoc;
	ostr << ", to node id: " << stud.toLoc << endl;
	return ostr;
}
