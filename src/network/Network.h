#ifndef NeTrainSim_Network_h
#define NeTrainSim_Network_h



#include <cmath>
#include <fstream>
#include <sstream>
#include <regex>
#include <cstdlib>
#include "../util/Vector.h"
#include <unordered_set>
#include <set>
#include "NetNode.h"
#include "NetLink.h"
#include "NetSignal.h"
#include "src/trainDefintion/Train.h"
#include "src/util/Utils.h"
#include "src/Util/Error.h"
#include "ReadWriteNetwork.h"
//#include <qapplication.h>

/**
 * A network.
 * @author	Ahmed
 * @date	2/14/2023
 */
class Network {
private:
    /** The file nodes */
    Vector<std::shared_ptr<NetNode>> theFileNodes;
public:
    /** Holds the name of the network. */
    std::string networkName;
    /** The nodes mapped by its simulator id */
    std::map<int, std::shared_ptr<NetNode>> nodes;
    /** The links */
    Vector<std::shared_ptr<NetLink>> links;
    /** The signals */
    Vector<std::shared_ptr<NetSignal>> networkSignals;

    Network(){}

    /**
     * Constructor
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	nodesFile	The nodes file.
     * @param 	linksFile	The links file.
     */
    Network(const string& nodesFile, const string& linksFile, std::string netName = "") {
        if (netName == "") {
            this->networkName = Utils::getPrefix(Utils::trim(Utils::getFilenameWithoutExtension(linksFile)));
        }
        else {
            this->networkName = netName;
        }
        auto nodesRecords = ReadWriteNetwork::readNodesFile(nodesFile);
        this->theFileNodes = ReadWriteNetwork::generateNodes(nodesRecords);
        auto linksRecords = ReadWriteNetwork::readLinksFile(linksFile);
        this->links = ReadWriteNetwork::generateLinks(this->theFileNodes, linksRecords);
        updateLinksLength();
        defineNodesLinks();
        this->nodes = defineNodes();
        //this->AdjMatrix = defineAdjMatrix();
        this->networkSignals = generateSignals();

    }

    Network(Vector<tuple<int, double, double, std::string, double, double>> nodesRecords,
            Vector<tuple<int, int, int, double, double, int, double, double, int, double,
                              bool, std::string, double, double>> linksRecords, std::string netName = "") {
        if (netName == "") {
            this->networkName = "Unnamed Network";
        }
        else {
            this->networkName = netName;
        }
        this->theFileNodes = ReadWriteNetwork::generateNodes(nodesRecords);
        this->links = ReadWriteNetwork::generateLinks(this->theFileNodes, linksRecords);
        updateLinksLength();
        defineNodesLinks();
        this->nodes = defineNodes();
        //this->AdjMatrix = defineAdjMatrix();
        this->networkSignals = generateSignals();
    }

    Network(Vector<std::shared_ptr<NetNode>> theNodes,
            Vector<std::shared_ptr<NetLink>> theLinks,
            std::string netName = "") {
        if (netName == "") {
            this->networkName = "Unnamed Network";
        }
        else {
            this->networkName = netName;
        }
        this->theFileNodes = theNodes;
        this->links = theLinks;
        updateLinksLength();
        defineNodesLinks();
        this->nodes = defineNodes();
        //this->AdjMatrix = defineAdjMatrix();
        this->networkSignals = generateSignals();
    }


    /**
     * Define nodes
     * @author	Ahmed
     * @date	2/14/2023
     * @returns	A std::map&lt;int,std::shared_ptr&lt;NetNode&gt;&gt;
     */
    std::map<int, std::shared_ptr<NetNode>> defineNodes() {
        std::map<int, std::shared_ptr<NetNode>> nodesMap;
        for (std::shared_ptr<NetNode> &n : (this->theFileNodes)) {
            nodesMap[n->id] = n;
        }
        return nodesMap;
    }

    /**
     * Gets simulator train path
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	userDefinedTrainPath	Full path list of the user defined train file.
     * @returns	The simulator train path.
     */
    Vector<int> getSimulatorTrainPath(Vector<int> userDefinedTrainPath) {
        Vector<int> simulatorTrainPath;
        for (int i = 0; i < userDefinedTrainPath.size(); i++) {
            simulatorTrainPath.push_back(getSimulatorNodeIDByUserID((userDefinedTrainPath)[i]));
        }
        return simulatorTrainPath;
    }

    tuple<double, double, double, double, double> getNetworkStats() {
        double catenaryCumConsumed = 0.0;
        double catenaryCumRegenerated = 0.0;
        int nuOfCatenaryLinks = 0;
        double totalLength = 0.0;
        double totalLinkLengthsWithCatenary = 0.0;
        for (auto& link: this->links) {
            if (link->hasCatenary) {
                nuOfCatenaryLinks += 1;
                totalLinkLengthsWithCatenary += link->length;
            }
            catenaryCumConsumed += link->catenaryCumConsumedEnergy;
            catenaryCumRegenerated += link->catenaryCumRegeneratedEnergy;
            totalLength += link->length;
        }
        double percOfCatenaryLinks = (nuOfCatenaryLinks / this->links.size()) * 100.0;
        return std::make_tuple(percOfCatenaryLinks, catenaryCumConsumed, catenaryCumRegenerated, totalLength, totalLinkLengthsWithCatenary);
    }

    /**
     * Gets node by identifier
     * @author	Ahmed
     * @date	2/14/2023
     * @exception	std::runtime_error	Raised when a runtime error condition occurs.
     * @param [in,out]	id	The identifier.
     * @returns	The node by identifier.
     */
    std::shared_ptr<NetNode> getNodeByID(int& id) {
        if (this->nodes.count(id)) {
            return this->nodes[id];
        }
        throw std::runtime_error(std::string("Error: ") +
                                     std::to_string(static_cast<int>(Error::cannotFindNode)) +
                                     "\nCould not find the simulator node ID: " + std::to_string(id) + "\n");
    }

    /**
     * Gets distance between two nodes
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	train	The train.
     * @param 	node1	The first node.
     * @param 	node2	The second node.
     * @returns	The distance between two nodes.
     */
    double getDistanceBetweenTwoNodes(std::shared_ptr <Train> train, std::shared_ptr<NetNode> node1, std::shared_ptr<NetNode> node2) {
        if (node1 == node2) { return 0.0; }
        int i1 = train->trainPathNodes.index(node1);
        int i2 = train->trainPathNodes.index(node2);
        if (i1 > i2) { swap(i1, i2); }
        double l = 0;
        for (int i = i1; i < i2; i++) {
            int nextI = i + 1;
            l += getLinkByStartandEndNodeID(train, train->trainPathNodes.at(i)->id, train->trainPathNodes.at(nextI)->id, true)->length;
        }
        return l;
    }

    /**
     * Query if 'train' is taking a conflict zone between two nodes.
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	train	The train.
     * @param 	node1	The first node.
     * @param 	node2	The second node.
     * @returns	True if conflict zone, false if not.
     */
    bool isConflictZone(std::shared_ptr<Train> train, std::shared_ptr<NetNode> node1, std::shared_ptr<NetNode> node2) {
        int n1 = train->trainPathNodes.index(node1);
        int n2 = train->trainPathNodes.index(node2);
        if (n1 > n2) { swap(n1, n2); }
        for (int i = n1; i < n2; i++) {
            int nextI = i + 1;
            Vector<std::shared_ptr<NetLink>> l1 = train->trainPathNodes.at(i)->linkTo[train->trainPathNodes.at(nextI)];
            Vector<std::shared_ptr<NetLink>> l2 = train->trainPathNodes.at(nextI)->linkTo[train->trainPathNodes.at(i)];
            std::set<std::shared_ptr<NetLink>> temp;
            if (l1.size() > 0) { temp.insert(l1.begin(), l1.end()); }
            if (l2.size() > 0){ temp.insert(l2.begin(), l2.end()); }
            if (temp.size() <= 1) { return true; }
        }
        return false;
    }

    /**
     * Gets distance by two coordinates
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	position1	The first position.
     * @param 	position2	The second position.
     * @returns	The distance by two coordinates.
     */
    double getDistanceByTwoCoordinates(const std::pair<double, double>& position1, 
                                const std::pair<double, double>& position2) {
        double xDiff = position1.first - position2.first;
        double yDiff = position1.second - position2.second;

        return std::sqrt(xDiff * xDiff + yDiff * yDiff);
    }

    /**
     * The shortest distance to end of all leading trains
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	link 	The link.
     * @param 	train	The train.
     * @returns	True if it succeeds, false if it fails.
     */
    bool DistanceToEndOfAllLinkTrainsIsLarge(const std::shared_ptr <NetLink> link, const std::shared_ptr <Train> train) {
        std::set<double> distanceToOtherTrains;
        for (std::shared_ptr <Train>& t : link->currentTrains) {
            if (t != train) {
                distanceToOtherTrains.insert(
                    getDistanceByTwoCoordinates(t->startEndPoints[1], train->currentCoordinates));
            }
        }
        return std::find_if(distanceToOtherTrains.begin(), distanceToOtherTrains.end(), [](int v) { return v < 2; }) == distanceToOtherTrains.end();
    }

    /**
     * Vectorize a link
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	link	 	The link.
     * @param 	startNode	The start node.
     * @returns	A std::pair&lt;std::pair&lt;double,double&gt;,std::pair&lt;double,double&gt;&gt;
     */
    std::pair<std::pair<double, double>, std::pair<double, double>> vectorizeLink(std::shared_ptr<NetLink> link,
        std::shared_ptr <NetNode> startNode) {
        std::shared_ptr<NetNode> n1 = link->fromLoc;
        std::shared_ptr<NetNode> n2 = link->toLoc;
        if (n1 != startNode) {
            std::swap(n1, n2);
        }
        return std::make_pair(n1->coordinates(), std::make_pair(n2->x - n1->x, n2->y - n1->y));
    }

    /**
     * Gets positionby travelled distance
     * @author	Ahmed
     * @date	2/14/2023
     * @param 		  	train			 	The train.
     * @param [in,out]	travelledDistance	The travelled distance.
     * @returns	The positionby travelled distance.
     */
    pair<double, double> getPositionbyTravelledDistance(std::shared_ptr <Train> train, double &travelledDistance) {
        if (travelledDistance <= 0.0) { return train->trainPathNodes[0]->coordinates(); }
        if (travelledDistance >= train->trainTotalPathLength) { return train->trainPathNodes.back()->coordinates(); }
        if (travelledDistance < train->linksCumLengths[0]) {

        }
        for (int i = 0; i < train->trainPath.size() - 1; i++) {
            if (travelledDistance > train->linksCumLengths[i]) {
                int nextI = i + 1;
                std::shared_ptr<NetNode> startNode = train->trainPathNodes[i];
                std::shared_ptr<NetLink> link = getLinkByStartandEndNodeID(train, train->trainPath[i], 
                    train->trainPath[nextI], true);
                std::pair<std::pair<double, double>, std::pair<double, double>> vector = vectorizeLink(link, startNode);
                double travelledDistanceOnLink = travelledDistance - train->linksCumLengths[i];
                return getPositionOnVector(vector, travelledDistanceOnLink);
            }
        }
        return std::make_pair(0.0, 0.0);
    }

    /**
     * Dots
     * @author	Ahmed
     * @date	2/14/2023
     * @param [in,out]	u	A std::pair&lt;double,double&gt; to process.
     * @param [in,out]	v	A std::pair&lt;double,double&gt; to process.
     * @returns	A double.
     */
    double dot(std::pair<double, double> &u, std::pair<double, double> &v) {
        return u.first * v.first + u.second * v.second;
    }

    /**
     * Dots
     * @author	Ahmed
     * @date	2/14/2023
     * @param 		  	matrix	The matrix.
     * @param [in,out]	v	  	A std::pair&lt;double,double&gt; to process.
     * @returns	A std::pair&lt;double,double&gt;
     */
    std::pair<double, double> dot(Vector<Vector<double>> matrix, std::pair<double, double>& v) {
        return std::make_pair(matrix.at(0).at(0) * v.first + matrix.at(0).at(1) * v.second,
            matrix.at(1).at(0) * v.first + matrix.at(1).at(1) * v.second);
    }

    /**
     * Cross
     * @author	Ahmed
     * @date	2/14/2023
     * @param [in,out]	u	A std::pair&lt;double,double&gt; to process.
     * @param [in,out]	v	A std::pair&lt;double,double&gt; to process.
     * @returns	A double.
     */
    double cross(std::pair<double, double> &u, std::pair<double, double> &v) {
        return u.first * v.second - u.second * v.first;
    }

    /**
     * Project lengthon path vector
     * @author	Ahmed
     * @date	2/14/2023
     * @param [in,out]	vectorEndpoints	The vector endpoints.
     * @param 		  	length		   	The length.
     * @returns	A std::pair&lt;double,double&gt;
     */
    std::pair<double, double> projectLengthonPathVector(std::pair<double, double>& vectorEndpoints, double length) {
        std::pair<double, double> u = { length, 0 };
        std::pair<double, double> v = vectorEndpoints;
        double cosTh = dot(u, v);
        double sinTh = cross(u, v);
        double theta = atan2(sinTh, cosTh);
        Vector<Vector<double>> rotationMatrix;
        rotationMatrix.push_back({ cos(theta), -1 * sin(theta) });
        rotationMatrix.push_back({ sin(theta), cos(theta) });
        return dot(rotationMatrix, u);
    }

    /**
     * Gets position vector
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	travelledDistanceOnXAxis	The travelled distance on x coordinate axis.
     * @returns	The position vector.
     */
    std::pair<double, double> getPositionOnVector(std::pair<std::pair<double, double>, std::pair<double, double>>vector, 
        double travelledDistanceOnXAxis) {
        double pathOriginX = vector.first.first;
        double pathOriginY = vector.first.second;
        // double pathEndX = vector.second.first;
        // double pathEndY = vector.second.second;
        std::pair<double, double> projectedLength = projectLengthonPathVector(vector.second, travelledDistanceOnXAxis);

        return std::make_pair(pathOriginX + projectedLength.first, pathOriginY + projectedLength.second);
    }

    /**
     * Gets signals by current node list
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	nodeList	List of nodes.
     * @returns	The signals by current node list.
     */
    Vector< std::shared_ptr<NetSignal>> getSignalsByCurrentNodeList(const std::vector<std::shared_ptr<NetNode>> nodeList) {
        Vector<std::shared_ptr<NetSignal>> sgnls;
        for (const auto& node : nodeList) {
            for (const auto& s : this->networkSignals) {
                if (s->currentNode.lock() == node) {
                    sgnls.push_back(s);
                }
            }
        }
        return sgnls;
    }

    /**
     * Gets link by startand end node identifier
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	train	 	The train.
     * @param 	startID  	The start identifier.
     * @param 	endID	 	The end identifier.
     * @param 	calcExact	(Optional) True to calculate exact.
     * @returns	The link by startand end node identifier.
     */
    std::shared_ptr<NetLink> getLinkByStartandEndNodeID(const std::shared_ptr<Train> train, int startID, 
        int endID, bool calcExact = true) {
        Vector<std::shared_ptr<NetLink>> betweenNodesLinks = this->getNodeByID(startID)->linkTo.at(this->getNodeByID(endID));
        if (betweenNodesLinks.size() == 1) {
            return betweenNodesLinks.at(0);
        }
        for (std::shared_ptr<NetLink> l : betweenNodesLinks) {
            if (l->currentTrains.exist(train)) {
                return l;
            }
        }
        Vector<std::shared_ptr<NetLink>> betweenNLinks(betweenNodesLinks.begin(), betweenNodesLinks.end());
        std::sort(betweenNLinks.begin(), betweenNLinks.end(), [](std::shared_ptr<NetLink> a, std::shared_ptr<NetLink> b) { return a->cost < b->cost; });
        if (!calcExact) {
            return betweenNLinks[0];
        }
        for (std::shared_ptr <NetLink> l : betweenNLinks) {
            if (l->currentTrains.empty() || (isSameDirection(l, train) && DistanceToEndOfAllLinkTrainsIsLarge(l, train))) {
                return l;
            }
        }
        return betweenNLinks[0];

    }

    Vector<std::shared_ptr<NetLink>> getLinksByStartandEndNode(std::shared_ptr<NetNode> startNode, std::shared_ptr<NetNode> endNode) {
        return startNode->linkTo.at(endNode);
    }

    /**
     * Gets full path length
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	train	The train.
     * @returns	The full path length.
     */
    double getFullPathLength(std::shared_ptr <Train> train) {
        double l = 0.0;
        int prevI = 0;
        for (int i = 1; i < train->trainPath.size(); i++) {
            prevI = i - 1;
            l += getLinkByStartandEndNodeID(train, train->trainPath.at(prevI), train->trainPath.at(i), false)->length;
        }
        return l;
    }

    /**
     * Gets link by start node identifier
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	train	   	The train.
     * @param 	startNodeID	The start node identifier.
     * @returns	The link by start node identifier.
     */
    std::shared_ptr <NetLink> getLinkByStartNodeID(const std::shared_ptr <Train> train, int startNodeID) {
        int i = train->trainPath.index(startNodeID);
        if (i < train->trainPath.size() - 1) {
            int indx = i + 1;
            int EndNodeID = train->trainPath.at(indx);
            return getLinkByStartandEndNodeID(train, startNodeID, EndNodeID);
        }
        return nullptr;
    }

    /**
     * Gets the first train link
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	train	The train.
     * @returns	The first train link.
     */
    std::shared_ptr<NetLink> getFirstTrainLink(const std::shared_ptr <Train> train) {
        if (train->trainPath.size() == 0) {
            throw std::runtime_error("Error: " + std::to_string(static_cast<int>(Error::trainPathCannotBeNull)) +
                                     "\n Train" + std::to_string(train->id) + " path cannot be null!\n");
        }
        else {
            int strartID = train->trainPath.at(0);
            return getLinkByStartNodeID(train, strartID);
        }
    }

    /**
     * Generates a cumulative links lengths
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	train	The train.
     * @returns	The cumulative links lengths.
     */
    Vector<double> generateCumLinksLengths(std::shared_ptr<Train> train) {
        int n = train->trainPath.size();
        Vector<double> linksCumLengths(n, 0);
        for (int i = 1; i < n; i++) {
            linksCumLengths[i] = getDistanceToSpecificNodeFromStart(train, train->trainPath.at(i));
        }
        return linksCumLengths;
    }

    /**
     * Gets the previous node by distance
     * @author	Ahmed
     * @date	2/14/2023
     * @param 		  	train			 	The train.
     * @param 		  	travelledDistance	The travelled distance.
     * @param [in,out]	previousNodeID   	Identifier for the previous node.
     * @returns	The previous node by distance.
     */
    std::shared_ptr <NetNode> getPreviousNodeByDistance(std::shared_ptr<Train> train, double travelledDistance, int &previousNodeID) {
        int nextI = -1;
        for (int i = train->trainPath.index(previousNodeID); train->trainPath.size(); i++) {
            if (train->linksCumLengths.at(i) > travelledDistance) {
                nextI = i;
                break;
            }
        }
        if (nextI == -1) { nextI = train->trainPath.size(); }
        int prevI = nextI - 1;
        return train->trainPathNodes.at(prevI);
    }

    /**
     * Gets link from distance
     * @author	Ahmed
     * @date	2/14/2023
     * @param 		  	train			 	The train.
     * @param [in,out]	travelledDistance	The travelled distance.
     * @param [in,out]	previousNodeID   	Identifier for the previous node.
     * @returns	The link from distance.
     */
    std::shared_ptr <NetLink> getLinkFromDistance(std::shared_ptr <Train> train, double &travelledDistance, int &previousNodeID) {
        int nextI = -1;
        for (int i = train->trainPath.index(previousNodeID); train->trainPath.size(); i++) {
            if (train->linksCumLengths.at(i) > travelledDistance) {
                nextI = i;
                break;
            }
        }
        if (nextI == -1) { nextI = train->trainPath.size(); }
        int prevI = nextI - 1;
        return this->getLinkByStartandEndNodeID(train, train->trainPath.at(prevI), train->trainPath.at(nextI), true);
    }

    /**
     * Gets simulator node identifier by user identifier
     * @author	Ahmed
     * @date	2/14/2023
     * @exception	std::runtime_error	Raised when a runtime error condition occurs.
     * @param 	oldID	Identifier for the old.
     * @returns	The simulator node identifier by user identifier.
     */
    int getSimulatorNodeIDByUserID(int oldID) {
        for (const std::shared_ptr<NetNode>& n : this->theFileNodes) {
            if (n->userID == oldID) {
                return n->id;
            }
        }
        throw std::runtime_error(std::string("Error: ") +
                                     std::to_string(static_cast<int>(Error::cannotFindNode)) +
                                     "\nCould not find the node ID: " + std::to_string(oldID) + "\n");
    }


    /**
     * Gets distance to specific node by travelled distance
     * @author	Ahmed
     * @date	2/14/2023
     * @param 		  	train			 	The train.
     * @param [in,out]	travelledDistance	The travelled distance.
     * @param [in,out]	nodeID			 	Identifier for the node.
     * @returns	The distance to specific node by travelled distance.
     */
    double getDistanceToSpecificNodeByTravelledDistance(std::shared_ptr<Train> train, double& travelledDistance, int& nodeID) {
        int endNodeIndex = train->trainPath.index(nodeID);
        return train->linksCumLengths[endNodeIndex] - travelledDistance;
    }

    /**
     * Ccws
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	A	A std::pair&lt;double,double&gt; to process.
     * @param 	B	A std::pair&lt;double,double&gt; to process.
     * @param 	C	A std::pair&lt;double,double&gt; to process.
     * @returns	True if it succeeds, false if it fails.
     */
    bool ccw(const std::pair<double, double>& A, const std::pair<double, double>& B, 
        const std::pair<double, double>& C) {
        return (C.second - A.second) * (B.first - A.first) > (B.second - A.second) * (C.first - A.first);
    }

    /**
     * Two lines intersect
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	A	A std::pair&lt;double,double&gt; to process.
     * @param 	B	A std::pair&lt;double,double&gt; to process.
     * @param 	C	A std::pair&lt;double,double&gt; to process.
     * @param 	D	A std::pair&lt;double,double&gt; to process.
     * @returns	True if it succeeds, false if it fails.
     */
    bool twoLinesIntersect(const std::pair<double, double>& A, const std::pair<double, double>& B, 
        const std::pair<double, double>& C, const std::pair<double, double>& D) {
        return (ccw(A, C, D) != ccw(B, C, D)) && (ccw(A, B, C) != ccw(A, B, D));
    }

    /**
     * Shortest path search
     * 
     * get the shortest path between two nodes. The first nodes is the start node and the second is
     * the destination. The function returns a Vector&lt;std::shared_ptr&lt;NetNode&gt;&gt;.
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	startNodeID 	The start node identifier.
     * @param 	targetNodeID	Identifier for the target node.
     */
    std::pair<Vector<std::shared_ptr<NetNode>>, double> shortestPathSearch(int startNodeID, int targetNodeID) {
        // clear all the nodes for the new sheart path
        for (auto& n : this->nodes) {
            n.second->clearGraphSearchParams();
        }
        // get the nodes by their user id's
        std::shared_ptr<NetNode> startNode = this->getNodeByID(startNodeID);
        std::shared_ptr<NetNode> targetNode = this->getNodeByID(targetNodeID);
        // override the inf value of the start node
        startNode->graphSearchDistanceFromStart = 0.0;

        // keep looping until the target node is reached
        while (! targetNode->graphSearchVisited) {
            // get the min-distance-unvisited node
            std::shared_ptr<NetNode> currentNode = minimumDistance();
            // if nothing was found, break
            if (currentNode == nullptr) { break; }
            // set the node as visited
            currentNode->graphSearchVisited = true;
            // check all the connected links/nodes
            for (auto& connectedNode : currentNode->getNeighbors()) {
                // check if a new node has a lower value
                if ((currentNode->graphSearchDistanceFromStart + 
                    currentNode->linkTo.at(connectedNode).at(0)->length) < connectedNode->graphSearchDistanceFromStart) {
                    // set the value
                    connectedNode->graphSearchDistanceFromStart = currentNode->graphSearchDistanceFromStart +
                            currentNode->linkTo.at(connectedNode).at(0)->length;
                    //n->graphSearchDistanceFromStart += currentNode->linkTo.at(n).at(0)->length;
                    connectedNode->graphSearchPreviousNode = currentNode;
                }
            }
        }

        Vector<std::shared_ptr<NetNode>> path;
        path.push_back(targetNode);
        while (true) {
            std::shared_ptr<NetNode> node = path.back();
            if (node->graphSearchPreviousNode == nullptr) { break; }
            //if (path.exist(node->graphSearchPreviousNode)) { break; }
            path.push_back(node->graphSearchPreviousNode);
        }
        std::reverse(path.begin(), path.end());
        return std::make_pair(path, targetNode->graphSearchDistanceFromStart);
    }



    //************************************************************************
    // Private functions
    // ***********************************************************************
private:

    /**
     * Node with minimum distance
     * @author	Ahmed
     * @date	2/14/2023
     * @returns	A std::shared_ptr&lt;NetNode&gt;
     */
    std::shared_ptr<NetNode> minimumDistance() {
        std::shared_ptr<NetNode> nextNode = std::shared_ptr<NetNode>();
        double minValue = INFINITY;
        for (auto& node : this->nodes) {
            if (node.second->graphSearchDistanceFromStart < minValue && node.second->graphSearchVisited == false) {
                minValue = node.second->graphSearchDistanceFromStart;
                nextNode = node.second;
            }
        }
        return nextNode;
    }
// ##################################################################
// #               start: read data and organize it                 #
// ##################################################################


    /**
     * Generates the signals
     * @author	Ahmed
     * @date	2/14/2023
     * @returns	The signals.
     */
    Vector<std::shared_ptr<NetSignal>> generateSignals() {
        Vector<std::shared_ptr<NetSignal>> networkSignals;
        //signals.reserve(links.size());
        for (std::shared_ptr<NetLink> l : this->links) {
            if (l->trafficSignalNo != 0 && l->trafficSignalNo != 10001) {
                if (l->direction == 1) {
                    NetSignal networkSignal = NetSignal(l->trafficSignalNo, l, l->fromLoc, l->toLoc);
                    std::shared_ptr<NetSignal> sharedSignal = std::make_shared<NetSignal>(networkSignal);
                    l->toLoc->addSignal(sharedSignal);
                    networkSignals.push_back(sharedSignal);
                }
                else {
                    NetSignal networkSignal1 = NetSignal(l->trafficSignalNo, l, l->fromLoc, l->toLoc);
                    std::shared_ptr<NetSignal> sharedSignal1 = std::make_shared<NetSignal>(networkSignal1);
                    l->toLoc->addSignal(sharedSignal1);
                    networkSignals.push_back(sharedSignal1);

                    NetSignal networkSignal2 = NetSignal(l->trafficSignalNo, l, l->toLoc, l->fromLoc);
                    std::shared_ptr<NetSignal> sharedSignal2 = std::make_shared<NetSignal>(networkSignal2);
                    l->fromLoc->addSignal(sharedSignal2);
                    networkSignals.push_back(sharedSignal2);
                    networkSignals.push_back(sharedSignal2);
                }
                
            }
            else if (l->trafficSignalNo == 10001){
                l->toLoc->isDepot = true;
            }
        }
        return networkSignals;
    }

    /**
     * Define nodes links
     * @author	Ahmed
     * @date	2/14/2023
     */
    void defineNodesLinks() {
        for (std::shared_ptr<NetLink>& l : this->links) {
            if (l->direction == 1){ 
                l->fromLoc.get()->linkTo[l->toLoc].push_back(l);
                //l->fromLoc.get()->linkTo.at(l->toLoc).push_back(l);
            }
            else {
                l->fromLoc.get()->linkTo[l->toLoc].push_back(l);
                l->toLoc.get()->linkTo[l->fromLoc].push_back(l);
                //l->fromLoc.get()->linkTo.at(l->toLoc).push_back(l);
                //l->toLoc.get()->linkTo.at(l->fromLoc).push_back(l);
            }
            
        }
    }


    /**
     * Updates the links length
     * @author	Ahmed
     * @date	2/14/2023
     */
    void updateLinksLength() {
        for (std::shared_ptr<NetLink> &l : this->links) {
            std::shared_ptr<NetNode> n1 = l->fromLoc;
            std::shared_ptr<NetNode> n2 = l->toLoc;
            double dx = n2->x - n1->x;
            double dy = n2->y - n1->y;
            double lLength = sqrt(dx * dx + dy * dy);
            if (lLength <= 0) {
                throw std::runtime_error("Error: " + std::to_string(static_cast<int>(Error::wrongLinksLength)) +
                                         "\nHorizontal distance between nodes should be greater than 0!\nReview the nodes file!... " +
                                         std::to_string(l->id) + "'s length is equal to zero!\n");
            }
            l->length = lLength;
        }
    }

// ##################################################################
// #                 end: read data and organize it                 #
// ##################################################################

    /**
     * Query if 'link' is same direction
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	link 	The link.
     * @param 	train	The train.
     * @returns	True if same direction, false if not.
     */
    bool isSameDirection(std::shared_ptr<NetLink> link, std::shared_ptr <Train> train) {
        std::set<bool> allTrainsAreInSameDirection;
        for (const std::shared_ptr<Train>& t : link->currentTrains) {
            if (t != train) {
                allTrainsAreInSameDirection.insert(
                    std::is_permutation(t->trainPath.begin(), t->trainPath.end(),
                        train->trainPath.begin(), train->trainPath.end()));
            }
        }
        return !allTrainsAreInSameDirection.count(false);
    }

    /**
     * Gets distance to specific node from start
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	train 	The train.
     * @param 	nodeID	Identifier for the node.
     * @returns	The distance to specific node from start.
     */
    double getDistanceToSpecificNodeFromStart(std::shared_ptr<Train> train, int nodeID) {
        double l = 0;
        for (int i = 1; i < train->trainPath.size(); i++) {
            int prevI = i - 1;
            l += getLinkByStartandEndNodeID(train, train->trainPath.at(prevI), train->trainPath.at(i), true)->length;
            if (train->trainPath.at(i) == nodeID) {
                return l;
            }
        }
        return l;
    }

    /**
     * Stream insertion operator
     * @author	Ahmed
     * @date	2/14/2023
     * @param [in,out]	ostr	The ostr.
     * @param [in,out]	stud	The stud.
     * @returns	The shifted result.
     */
    friend ostream& operator<<(ostream& ostr, Network& stud) {
        ostr << "the network has " << stud.nodes.size() << " nodes and " << stud.links.size() << "links." << endl;
        return ostr;
    }

};

#endif // !NeTrainSim_Network_h
