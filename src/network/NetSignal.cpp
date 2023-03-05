#include <iostream>
#include "../util/Vector.h"
#include <math.h>
#include <algorithm> 
#include "NetLink.h"
#include "NetSignal.h"
#include "NetNode.h"
using namespace std;


unsigned int NetSignal::NumberOfSignalsInSimulator = 0;

NetSignal::NetSignal(int signalID, std::shared_ptr <NetLink> hostingLink) {
	this->userID = signalID;
	this->id = NumberOfSignalsInSimulator;
	this->isGreen = true;
	this->link = hostingLink;
	this->proximityToActivate = 0.0;
	if (hostingLink) {
		this->currentNode = hostingLink.get()->toLoc;
		this->previousNode = hostingLink.get()->fromLoc;
	}
	else {
		this->currentNode = std::weak_ptr<NetNode>();
		this->previousNode = std::weak_ptr<NetNode>();
	}
	NetSignal::NumberOfSignalsInSimulator++;
}

NetSignal::NetSignal(int signalID, std::shared_ptr<NetLink> hostingLink, 
	std::shared_ptr<NetNode> previousLinkNode, std::shared_ptr<NetNode> currentLinkNode) {
	this->userID = signalID;
	this->id = NumberOfSignalsInSimulator;
	this->proximityToActivate = 0.0;
	this->isGreen = true;
	this->link = hostingLink;
	this->currentNode = currentLinkNode;
	this->previousNode = previousLinkNode;
	NetSignal::NumberOfSignalsInSimulator++;
}

unsigned int NetSignal::getNumberOfSignalsInSimulator()
{
	return NetSignal::NumberOfSignalsInSimulator;
}


ostream& operator<<(ostream& ostr, const NetSignal& networkSignal) {
    ostr << "Network signal:: id: " << networkSignal.userID << ", green: " << ((networkSignal.isGreen)? "true": "false");
	ostr << ", previous node id: " << networkSignal.previousNode.lock()->id;
	ostr << ", current node id: " << networkSignal.currentNode.lock()->id << endl;
	return ostr;
}
