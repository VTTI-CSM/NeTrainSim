#include "netnode.h"
#include <iostream>
#include "../util/vector.h"
#include "netlink.h"
#include <math.h>
#include <algorithm> 
#include <cmath>
using namespace std;

unsigned int NetNode::NumberOfNodesInSimulator = 0;

NetNode::NetNode(int simulatorID, int ID, double xCoord, double yCoord, string desc, double xDirScale, double yDirScale ) {
    this->userID = ID;
    this->id = simulatorID;
    this->alphaDesc = desc;
    this->xScale = static_cast<double>(xDirScale);
    this->yScale = static_cast<double>(yDirScale);
    this->x = static_cast<double>(xCoord) * this->xScale;
    this->y = static_cast<double>(yCoord) * this->yScale;
    this->networkSignals = Vector<std::shared_ptr<NetSignal>>();
    this->isDepot = false;
    this->dwellTimeIfDepot = 0.0;
    this->refillTanksAndBatteries = false;
    NetNode::NumberOfNodesInSimulator++;

    this->graphSearchDistanceFromStart = INFINITY;
    this->graphSearchVisited = false;
    this->graphSearchPreviousNode = std::shared_ptr<NetNode>();
}

NetNode::~NetNode() {
    NetNode::NumberOfNodesInSimulator--;
}

void NetNode::setNodeSimulatorID(int newID) {
    this->id = newID;
}

Vector<std::shared_ptr<NetNode>> NetNode::getNeighbors() {
    return this->linkTo.get_keys();
}

void NetNode::clearGraphSearchParams() {
    this->graphSearchDistanceFromStart = INFINITY;
    this->graphSearchVisited = false;
    this->graphSearchPreviousNode = std::shared_ptr<NetNode>();
}

unsigned int NetNode::getNumberOfNodesInSimulator() {
    return NetNode::NumberOfNodesInSimulator;
}


void NetNode::addSignal(std::shared_ptr<NetSignal> networkSignal) {
    this->networkSignals.push_back(networkSignal);
}

void NetNode::updateXScale(const double& newScale) {
    double oldScale = this->xScale;
    this->x = (this->x / oldScale) * newScale;
    this->xScale = newScale;
}

void NetNode::updateYScale(const double& newScale) {
    double oldScale = this->xScale;
    this->y = (this->y / oldScale) * newScale;
    this->yScale = newScale;
}

pair<double, double> NetNode::coordinates() {
    return { this->x, this->y };
}

ostream& operator<<(ostream& ostr, const NetNode& node) {
    ostr << "Network node :: ID: " << node.id << ", user ID: " << node.userID
        <<", x coord : " << node.x << ", y coord : " << node.y << ", is a depot/stop: "
        << ((node.isDepot) ? "true" : "false");
    ostr << endl;
    return ostr;
}
