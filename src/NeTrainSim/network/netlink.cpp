#include <iostream>
#include <math.h>
#include <algorithm>
#include "netlink.h"
#include "netnode.h"
#include "../util/utils.h"

using namespace std;

unsigned int NetLink::NumberOfLinksInSimulator = 0;

NetLink::NetLink(int simulatorID, int linkID,
                 std::shared_ptr<NetNode> fromNode,
                 std::shared_ptr<NetNode> toNode,
                 double linkLength, double maxSpeed,
                 int trafficSignalID, string signalAtEnd,
                 double linkGrade, double linkCurvature,
                 int linkNoOfDirections,
                 double speedVariationfactor,
                 bool isCatenaryAvailable,
                 string linkInRegion, double lengthScale,
                 double maxSpeedScale)
{
    this->id = simulatorID;
	this->userID = linkID;
    this->fromLoc = fromNode;
    this->toLoc = toNode;
    this->length = linkLength;
	this->freeFlowSpeed = maxSpeed;
	this->trafficSignalNo = trafficSignalID;
    this->direction = linkNoOfDirections;
    auto vec = Utils::split(signalAtEnd, ',');
    if (vec.empty() && this->direction == 2) {
        this->trafficSignalAtEnd.push_back(fromNode->userID);
        this->trafficSignalAtEnd.push_back(toLoc->userID);
    }
    else if (!vec.empty() && this->direction == 2){
        this->trafficSignalAtEnd = Utils::convertStringVectorToIntVector(vec);
    }
    else if (vec.empty() && this->direction == 1) {
        this->trafficSignalAtEnd.push_back(toLoc->userID);
    }
    else if (! vec.empty() && this->direction == 1) {
        this->trafficSignalAtEnd = Utils::convertStringVectorToIntVector(vec);
    }

	this->grade = this->setGrade(linkGrade);
	this->curvature = linkCurvature;
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
    this->updateSimulatorLength();
    if (length == 0.0) {
        simulatorLength = length;
    }
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

void NetLink::updateSimulatorLength() {

    // If there are no intermediate points, calculate position on the direct
    // link between start and end points
    if (intermediatePoints.empty()) {
        this->simulatorLength = Utils::getDistanceByTwoCoordinates(
            fromLoc->coordinates(), toLoc->coordinates());
    }
    else {
        double len = 0.0;
        // The distance from the start node (n1) to the first intermediate point
        len = Utils::getDistanceByTwoCoordinates(
            fromLoc->coordinates(), intermediatePoints[0]->coordinates());

        // Calculate cumulative distances between intermediate points in normal
        // order
        for (int i = 0; i < intermediatePoints.size() - 1; ++i) {
            len +=
                Utils::getDistanceByTwoCoordinates(
                    intermediatePoints[i]->coordinates(),
                    intermediatePoints[i+1]->coordinates());
        }

        len +=
            Utils::getDistanceByTwoCoordinates(
                intermediatePoints.back()->coordinates(),
                toLoc->coordinates());

        this->simulatorLength = len;

    }
}


pair<double, double> NetLink::findPositionOnLink(
    double travelledLength, std::shared_ptr<NetNode> startNode)
{
    double scalledTravelledLength =
        (this->simulatorLength / this->length) * this->length;
    // If the travelled length is greater than the link's length,
    // return infinite coordinates
    if (scalledTravelledLength > this->length || scalledTravelledLength < 0.0)
    {
        return std::make_pair(static_cast<double>(
                                  std::numeric_limits<double>::infinity()),
                              static_cast<double>(
                                  std::numeric_limits<double>::infinity()));
    }

    // If there are no intermediate points, calculate position on the direct
    // link between start and end points
    if (intermediatePoints.empty()) {
        auto out = vectorizeLinkOfOneSegment(startNode);
        return getPositionOnVector(out, scalledTravelledLength);
    }

    // Get nodes of the link
    std::shared_ptr<NetNode> n1 = fromLoc;
    std::shared_ptr<NetNode> n2 = toLoc;

    // Initialize a vector to store distances between consecutive points,
    // including start and end nodes
    std::vector<double> distances(intermediatePoints.size() + 1);

    // If the start node is n1, calculate distances in the normal order
    if (n1 == startNode) {
        // The distance from the start node (n1) to the first intermediate point
        distances[0] = Utils::getDistanceByTwoCoordinates(
                                std::make_pair(n1->x, n1->y),
                                std::make_pair(intermediatePoints[0]->x,
                                               intermediatePoints[0]->y));

        // Calculate cumulative distances between intermediate points in normal
        // order
        for (int i = 0; i < intermediatePoints.size() - 1; ++i) {
            distances[i + 1] = distances[i] +
                               Utils::getDistanceByTwoCoordinates(
                                    std::make_pair(intermediatePoints[i]->x,
                                                   intermediatePoints[i]->y),
                                    std::make_pair(intermediatePoints[i+1]->x,
                                                   intermediatePoints[i+1]->y));
        }

        // Add the distance from the last intermediate point to the
        // end node (n2)
        distances.back() +=
            Utils::getDistanceByTwoCoordinates(
                        std::make_pair(intermediatePoints.back()->x,
                                       intermediatePoints.back()->y),
                                       std::make_pair(n2->x, n2->y));
    }
    // If the start node is n2, calculate distances in the reversed order
    else {
        // The distance from the start node (n2) to the last intermediate point
        distances[0] = Utils::getDistanceByTwoCoordinates(
                                   std::make_pair(n2->x,
                                                  n2->y),
                                   std::make_pair(intermediatePoints.back()->x,
                                                 intermediatePoints.back()->y));

        // Calculate cumulative distances between intermediate points in
        // reverse order
        for (int i = intermediatePoints.size() - 1; i > 0; --i) {
            distances[intermediatePoints.size() - i + 1] =
                distances[intermediatePoints.size() - i] +
                    Utils::getDistanceByTwoCoordinates(
                            std::make_pair(intermediatePoints[i]->x,
                                           intermediatePoints[i]->y),
                            std::make_pair(intermediatePoints[i-1]->x,
                                           intermediatePoints[i-1]->y));
        }

        // Add the distance from the first intermediate point to the end
        // node (n1)
        distances.back() += Utils::getDistanceByTwoCoordinates(
                            std::make_pair(intermediatePoints[0]->x,
                                           intermediatePoints[0]->y),
                            std::make_pair(n1->x, n1->y));
    }

    // Use binary search to find the first distance that is not less
    // than scalledTravelledLength
    auto it = std::lower_bound(distances.begin(), distances.end(),
                               scalledTravelledLength);

    // If the iterator points to the end of distances, the
    // scalledTravelledLength is larger than any accumulated distances
    // Calculate the position on the segment between the last intermediate
    // point and the end node
    if(it == distances.end()) {
        auto vec = std::make_pair(std::make_pair(intermediatePoints.back()->x,
                                                 intermediatePoints.back()->y),
                                  std::make_pair(n2->x -
                                                 intermediatePoints.back()->x,
                                                 n2->y -
                                                 intermediatePoints.back()->y));
        // Use the last accumulated distance
        return getPositionOnVector(vec, scalledTravelledLength - *(it-1));
    }
    // If the iterator points to the beginning of distances, the
    // scalledTravelledLength is less than the first accumulated distance
    // Calculate the position on the segment between the start node
    // and the first intermediate point
    else if(it == distances.begin()) {
        auto vec = std::make_pair(n1->coordinates(),
                                  std::make_pair(intermediatePoints[0]->x -
                                                 n1->x,
                                  intermediatePoints[0]->y - n1->y));
        return getPositionOnVector(vec, scalledTravelledLength);
    }
    // The iterator points to some accumulated distance in the middle of the
    // distances vector
    // Calculate the position on the corresponding segment between two
    // intermediate points
    else {
        int idx = it - distances.begin();
        auto vec = std::make_pair(std::make_pair(intermediatePoints[idx-1]->x,
                                                 intermediatePoints[idx-1]->y),
                                  std::make_pair(intermediatePoints[idx]->x -
                                                 intermediatePoints[idx-1]->x,
                                                 intermediatePoints[idx]->y -
                                                 intermediatePoints[idx-1]->y));
        // Subtract the accumulated distance of the previous point
        return getPositionOnVector(vec, scalledTravelledLength - *(it-1));
    }
}



/**
     * Creates a vector representation of a link with respect to a given start
     * node.
     *
     * @param link A shared_ptr to a NetLink object.
     * @param startNode A shared_ptr to a NetNode object representing the start
     *        node.
     * @return A pair of pairs of doubles, where the first pair represents the
     *          coordinates of the start node, and the second pair represents
     *          the vector from the start node to the end node.
     * @author Ahmed
     * @date 2/14/2023
     */
std::pair<std::pair<double, double>,
          std::pair<double, double>> NetLink::vectorizeLinkOfOneSegment(
    std::shared_ptr <NetNode> startNode) {

    // Get nodes of the link
    std::shared_ptr<NetNode> n1 = fromLoc;
    std::shared_ptr<NetNode> n2 = toLoc;

    // Make sure n1 is the start node
    if (n1 != startNode) {
        std::swap(n1, n2);
    }

    // Return vector representation
    return std::make_pair(n1->coordinates(), std::make_pair(n2->x - n1->x,
                                                            n2->y - n1->y));
}

std::pair<double, double> NetLink::getPositionOnVector(
                        std::pair<std::pair<double, double>,
                        std::pair<double, double>>vector,
                        double travelledDistanceOnXAxis) {
    double pathOriginX = vector.first.first;
    double pathOriginY = vector.first.second;
    // double pathEndX = vector.second.first;
    // double pathEndY = vector.second.second;
    std::pair<double, double> projectedLength =
        this->projectLengthonPathVector(vector.second,
                    travelledDistanceOnXAxis);

    return std::make_pair(pathOriginX + projectedLength.first,
                          pathOriginY + projectedLength.second);
}

std::pair<double, double> NetLink::projectLengthonPathVector(
                                    std::pair<double,
                                    double>& vectorEndpoints,
                                    double length) {
    if (length < 0) {
        throw std::invalid_argument("Length cannot be negative!");
    }
    std::pair<double, double> u = { length, 0 };
    std::pair<double, double> v = vectorEndpoints;
    double cosTh = Utils::dot(u, v);
    double sinTh = Utils::cross(u, v);
    double theta = atan2(sinTh, cosTh);
    Vector<Vector<double>> rotationMatrix;
    rotationMatrix.push_back({ cos(theta), -1 * sin(theta) });
    rotationMatrix.push_back({ sin(theta), cos(theta) });
    return Utils::dot(rotationMatrix, u);
}


ostream& operator<<(ostream& ostr, const NetLink& stud) {
    ostr << "Network link:: id: " << stud.id << ", from node id: " <<
        stud.fromLoc << ", to node id: " << stud.toLoc << endl;
	return ostr;
}
