#ifndef NeTrainSim_Network_h
#define NeTrainSim_Network_h


// Get all required libraries and files
#include <cmath>
#include <fstream>
#include <sstream>
#include <regex>
#include <cstdlib>
#include "../util/vector.h"
#include <unordered_set>
#include <set>
#include "netnode.h"
#include "netlink.h"
#include "netsignal.h"
#include "../traindefinition/train.h"
#include "../util/utils.h"
#include "../util/error.h"
#include "readwritenetwork.h"

/**
 * This class defined a network for trains.
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

    /**
     * Default constructor.
     * Initializes a new instance of the Network class.
     * @author Ahmed Aredah
     * @date 2/14/2023
     */
    Network(){}

    /**
     * Constructor for the Network class.
     * Initializes a new instance of the Network class using node and link
     * files.
     *
     * @param nodesFile A string representing the path to the nodes file.
     * @param linksFile A string representing the path to the links file.
     * @param netName A string for the network name. If not provided, the
     * network name will be generated from the linksFile name.
     * @author Ahmed Aredah
     * @date 2/14/2023
     */
    Network(const string& nodesFile, const string& linksFile,
            std::string netName = "") {
        if (netName == "") {
            this->networkName = Utils::getPrefix(Utils::trim(
                Utils::getFilenameWithoutExtension(linksFile)));
        }
        else {
            this->networkName = netName;
        }
        auto nodesRecords = ReadWriteNetwork::readNodesFile(nodesFile);
        this->theFileNodes = ReadWriteNetwork::generateNodes(nodesRecords);
        auto linksRecords = ReadWriteNetwork::readLinksFile(linksFile);
        this->links = ReadWriteNetwork::generateLinks(this->theFileNodes,
                                                      linksRecords);
        updateLinksLength();
        defineNodesLinks();
        this->nodes = defineNodes();
        //this->AdjMatrix = defineAdjMatrix();
        this->networkSignals = generateSignals();

    }

    /**
     * Constructor for the Network class.
     * Initializes a new instance of the Network class using pre-loaded nodes
     * and links data.
     * @param nodesRecords A vector of tuples representing the nodes data.
     * @param linksRecords A vector of tuples representing the links data.
     * @param netName A string for the network name. If not provided, the
     * network name will be "Unnamed Network".
     * @author Ahmed Aredah
     * @date 2/14/2023
     */
    Network(Vector<Map<std::string, std::string>> nodesRecords,
            Vector<Map<std::string, std::string>> linksRecords,
            std::string netName = "") {

        if (netName == "") {
            this->networkName = "Unnamed Network";
        }
        else {
            this->networkName = netName;
        }
        this->theFileNodes = ReadWriteNetwork::generateNodes(nodesRecords);
        this->links = ReadWriteNetwork::generateLinks(this->theFileNodes,
                                                      linksRecords);
        updateLinksLength();
        defineNodesLinks();
        this->nodes = defineNodes();
        //this->AdjMatrix = defineAdjMatrix();
        this->networkSignals = generateSignals();
    }

    /**
     * Constructor for the Network class.
     * Initializes a new instance of the Network class using shared pointers
     * to NetNode and NetLink.
     * @param theNodes A vector of shared pointers to NetNode objects.
     * @param theLinks A vector of shared pointers to NetLink objects.
     * @param netName A string for the network name. If not provided, the
     * network name will be "Unnamed Network".
     * @author Ahmed Aredah
     * @date 2/14/2023
     */
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
     * @param 	userDefinedTrainPath	Full path list of the user defined
     * train file.
     * @returns	The simulator train path.
     */
    Vector<int> getSimulatorTrainPath(Vector<int> userDefinedTrainPath) {
        Vector<int> simulatorTrainPath;
        for (int i = 0; i < userDefinedTrainPath.size(); i++) {
            simulatorTrainPath.push_back(getSimulatorNodeIDByUserID((
                userDefinedTrainPath)[i]));
        }
        return simulatorTrainPath;
    }

    /**
     * Gets the network statistics.
     * @return A tuple with the percentage of catenary links, the cumulative
     * consumed energy of the catenary,
     * the cumulative regenerated energy of the catenary, the total length of
     * the links and the total length of links with catenary.
     *
     * @author Ahmed Aredah
     * @date 2/14/2023
     */
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
        double percOfCatenaryLinks = (nuOfCatenaryLinks / this->links.size()) *
                                                    100.0;
        return std::make_tuple(percOfCatenaryLinks, catenaryCumConsumed,
                               catenaryCumRegenerated, totalLength,
                               totalLinkLengthsWithCatenary);
    }

    /**
     * Gets node by identifier
     * @author	Ahmed
     * @date	2/14/2023
     * @exception	std::runtime_error	Raised when a runtime error condition
     *              occurs.
     * @param [in,out]	id	The identifier.
     * @returns	The node by identifier.
     */
    std::shared_ptr<NetNode> getNodeByID(int& id) {
        if (this->nodes.count(id)) {
            return this->nodes[id];
        }
        throw std::runtime_error(std::string("Error: ") +
                                     std::to_string(static_cast<int>(
                                                Error::cannotFindNode)) +
                                 "\nCould not find the simulator node ID: " +
                                 std::to_string(id) + "\n");
    }

    /**
     * Computes the distance between two nodes on a given train's path.
     * @param train A shared_ptr to a Train object representing the train for
     * which to compute the distance.
     * @param node1 A shared_ptr to the first NetNode object.
     * @param node2 A shared_ptr to the second NetNode object.
     * @return A double representing the distance between node1 and node2
     * along the train's path.
     * @author Ahmed
     * @date 2/14/2023
     */
    double getDistanceBetweenTwoNodes(std::shared_ptr <Train> train,
                                      std::shared_ptr<NetNode> node1,
                                      std::shared_ptr<NetNode> node2) {
        if (node1 == node2) { return 0.0; }
        int i1 = train->trainPathNodes.index(node1);
        int i2 = train->trainPathNodes.index(node2);
        if (i1 > i2) { swap(i1, i2); }
        double l = 0;
        for (int i = i1; i < i2; i++) {
            int nextI = i + 1;
            l += getLinkByStartandEndNodeID(train,
                                            train->trainPathNodes.at(i)->id,
                                            train->trainPathNodes.at(nextI)->id,
                                            true)->length;
        }
        return l;
    }

    /**
     * Checks if there's a conflict zone between two nodes for the given train.
     * A conflict zone exists when there's only one link between nodes.
     * @param train A shared_ptr to a Train object.
     * @param node1 A shared_ptr to the first NetNode object.
     * @param node2 A shared_ptr to the second NetNode object.
     * @return true if a conflict zone exists, false otherwise.
     * @author Ahmed
     * @date 2/14/2023
     */
    bool isConflictZone(std::shared_ptr<Train> train,
                        std::shared_ptr<NetNode> node1,
                        std::shared_ptr<NetNode> node2) {
        // Get indices of nodes in train path
        int n1 = train->trainPathNodes.index(node1);
        int n2 = train->trainPathNodes.index(node2);

        // Ensure n1 is less than n2
        if (n1 > n2) { swap(n1, n2); }

        for (int i = n1; i < n2; i++) {
            // Get links between consecutive nodes
            int nextI = i + 1;
            Vector<std::shared_ptr<NetLink>> l1 =
                train->trainPathNodes.at(
                       i)->linkTo[train->trainPathNodes.at(nextI)];
            Vector<std::shared_ptr<NetLink>> l2 =
                train->trainPathNodes.at(
                       nextI)->linkTo[train->trainPathNodes.at(i)];

            // Add links to a set
            std::set<std::shared_ptr<NetLink>> temp;
            if (l1.size() > 0) { temp.insert(l1.begin(), l1.end()); }
            if (l2.size() > 0){ temp.insert(l2.begin(), l2.end()); }

            // Check if only one link exists
            if (temp.size() <= 1) { return true; }
        }

        // If more than one link exists between any pair of nodes, return false
        return false;
    }



    /**
     * Checks if the distance to the end of all leading trains on a link is
     * larger than a certain threshold (2 units in this case).
     * @param link A shared_ptr to a NetLink object.
     * @param train A shared_ptr to a Train object.
     * @return true if the distance to all trains is larger than 2 units,
     * false otherwise.
     *
     * @author Ahmed
     * @date 2/14/2023
     */
    bool DistanceToEndOfAllLinkTrainsIsLarge(
        const std::shared_ptr <NetLink> link,
        const std::shared_ptr <Train> train)
    {
        // Store distances to other trains on the same link
        std::set<double> distanceToOtherTrains;

        // Compute distances for each train on the link
        for (std::shared_ptr <Train>& t : link->currentTrains) {
            if (t != train) {
                distanceToOtherTrains.insert(
                    Utils::getDistanceByTwoCoordinates(
                        t->startEndPoints[1],
                        train->currentCoordinates));
            }
        }
        // Check if any distance is less than 2
        return std::find_if(distanceToOtherTrains.begin(),
                            distanceToOtherTrains.end(),
                            [](int v) {
                                return v < 2;
                            }) == distanceToOtherTrains.end();
    }


    /**
     * Computes the position of a train based on its travelled distance along a
     * path
     *
     * This function calculates the position of a train on its path given the
     * travelled distance.
     * The path is represented as a sequence of links, each with its own length.
     * The function iterates over these links to find the one on which the
     * train is currently located, and then calculates the train's position on
     * this link.
     *
     * If the travelled distance is 0 or less, the function returns the
     * coordinates of the first node on the path. If the travelled distance is
     * equal to or greater than the total path length,
     * it returns the coordinates of the last node. Otherwise, the function
     * determines on which link the train is located and calculates the precise
     * position.
     *
     * @param train A shared_ptr to the Train object for which to compute the
     *        position. This object contains the train's path, the total path
     *        length, and the cumulative lengths of the individual links.
     * @param travelledDistance The distance the train has travelled along its
     *        path. Must be non-negative and not exceed the total path length.
     * @return A std::pair<double, double> representing the train's position.
     *         The first element is the x-coordinate, and the second element is
     *         the y-coordinate. If the function is unable to determine the
     *         position (for example, if the travelled distance exceeds the
     *         total path length), it returns the coordinates (0.0, 0.0).
     * @author Ahmed Aredah
     * @date 2/14/2023
     */
    pair<double, double> getPositionbyTravelledDistance(
                std::shared_ptr <Train> train,
                double &travelledDistance) {

        // If the train has not moved, its position is the coordinates of the
        // first node in its path.
        if (travelledDistance <= 0.0) {
            return train->trainPathNodes[0]->coordinates();
        }

        // If the train has moved more than or equal to its total path length,
        // its position is the coordinates of the last node in its path.
        if (travelledDistance >= train->trainTotalPathLength) {
            return train->trainPathNodes.back()->coordinates();
        }

        // Traverse the train path
        for (int i = 0; i < train->trainPath.size() - 1; i++) {
            // Check if travelled distance is between the cumulative
            // lengths of consecutive links
            if (travelledDistance > train->linksCumLengths[i] &&
                (i == train->trainPath.size() - 2 ||
                      travelledDistance <= train->linksCumLengths[i+1])) {
                int nextI = i + 1;

                // Get the start node and the link on which the train is
                // currently on
                std::shared_ptr<NetNode> startNode = train->trainPathNodes[i];
                std::shared_ptr<NetLink> link =
                    getLinkByStartandEndNodeID(train, train->trainPath[i],
                                                train->trainPath[nextI], true);

                // Calculate the distance travelled on the current link
                double travelledDistanceOnLink =
                    travelledDistance - train->linksCumLengths[i];

                // Calculate and return the position on the current vector
                // based on the distance travelled on the link
                return link->findPositionOnLink(travelledDistanceOnLink,
                                                startNode);
            }
        }
        // If travelledDistance does not fall within any link's cumulative
        // lengths, return (0.0, 0.0)
        return std::make_pair(0.0, 0.0);
    }

    /**
     * Normalize a 2D vector
     *
     * This function takes a 2D vector represented as a std::pair of doubles
     * and normalizes it. Normalizing a vector adjusts its length to be 1,
     * but keeps its direction the same.
     * If the input vector has length 0, the function will return the same
     * vector.
     *
     * @param vec The vector to be normalized, represented as a std::pair of
     *            doubles.
     *            The first element of the pair represents the x component of
     *            the vector, and the second element represents the y component.
     * @return A std::pair representing the normalized vector.
     *         If the input vector had length 0, the same vector is returned.
     *
     * @author Ahmed
     * @date 6/8/2023
     */
    std::pair<double, double> normalize(std::pair<double, double> vec) {
        double length = sqrt(vec.first * vec.first + vec.second * vec.second);
        if(length == 0)
            return vec;
        return {vec.first / length, vec.second / length};
    }


    /**
     * Fetches the network signals associated with the list of given nodes.
     * Iterates over each node and checks the node's signals.
     * If a signal's current node matches the current iterating node,
     * the signal is added to a list that is returned after the completion of
     * the function.
     *
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	nodeList	Reference to a vector of shared pointers to NetNode
     *                      objects.
     * @returns	Vector of shared pointers to NetSignal objects associated with
     *          the nodes in the nodeList.
     */
    Vector< std::shared_ptr<NetSignal>> getSignalsByCurrentNodeList(
                        const std::vector<std::shared_ptr<NetNode>> nodeList) {
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
     * Fetches the network link by start and end node identifiers.
     * Checks all the links between the two nodes and returns the one that has
     * the train, or the link with the least cost if calcExact is false.
     * If calcExact is true, it will try to find a link where the train can
     * travel without colliding with other trains.
     *
     * @author	Ahmed
     * @date	2/14/2023
     * @param 	train	 	A shared pointer to the Train object for which the
     *                      link is to be found.
     * @param 	startID  	The start node identifier.
     * @param 	endID	 	The end node identifier.
     * @param 	calcExact	(Optional) Flag to calculate exact link, default
     *                      is true.
     * @returns	A shared pointer to the NetLink object representing the link
     *          between startID and endID.
     */
    std::shared_ptr<NetLink> getLinkByStartandEndNodeID(
                                const std::shared_ptr<Train> train,
                                int startID,
        int endID, bool calcExact = true) {
        Vector<std::shared_ptr<NetLink>> betweenNodesLinks =
            this->getNodeByID(startID)->linkTo.at(this->getNodeByID(endID));
        if (betweenNodesLinks.size() == 1) {
            return betweenNodesLinks.at(0);
        }

        // If there is more than one link, find the one that currently contains
        // the train
        for (std::shared_ptr<NetLink> l : betweenNodesLinks) {
            if (l->currentTrains.exist(train)) {
                return l;
            }
        }

        // If the train is not on any of the links, sort the links by cost and
        // potentially return the one with the lowest cost
        Vector<std::shared_ptr<NetLink>> betweenNLinks(
            betweenNodesLinks.begin(),
            betweenNodesLinks.end());
        std::sort(betweenNLinks.begin(), betweenNLinks.end(),
                  [](std::shared_ptr<NetLink> a, std::shared_ptr<NetLink> b) {
                        return a->cost < b->cost; });
        if (!calcExact) {
            return betweenNLinks[0];
        }

        // If calcExact is true, find a link where the train can travel without
        // colliding with other trains
        for (std::shared_ptr <NetLink> l : betweenNLinks) {
            if (l->currentTrains.empty() || (isSameDirection(l, train) &&
                           DistanceToEndOfAllLinkTrainsIsLarge(l, train))) {
                return l;
            }
        }
        return betweenNLinks[0];

    }

    /**
     * @brief Retrieves the network links between a specified start node and
     * end node.
     *
     * This function will search the linked nodes of the start node and return
     * all links that point to the provided end node.
     *
     * @param startNode The starting node of the links.
     * @param endNode The end node of the links.
     * @returns A vector of shared pointers to the NetLink objects.
     * @throws std::out_of_range If the endNode is not a neighbor of the
     *         startNode.
     */
    Vector<std::shared_ptr<NetLink>> getLinksByStartandEndNode(
                                            std::shared_ptr<NetNode> startNode,
                                            std::shared_ptr<NetNode> endNode) {
        return startNode->linkTo.at(endNode);
    }

    /**
     * @brief Calculates the total length of the train's path.
     *
     * This function iterates over the train's path, adding the length of each
     * link between nodes to compute the total path length.
     *
     * @param 	train	The train whose path length will be calculated.
     * @returns	The total length of the train's path.
     * @throws std::out_of_range If trainPath contains a node not connected to
     *         the previous node.
     */
    double getFullPathLength(std::shared_ptr <Train> train) {
        double l = 0.0;
        int prevI = 0;
        for (int i = 1; i < train->trainPath.size(); i++) {
            prevI = i - 1;
            l += getLinkByStartandEndNodeID(train, train->trainPath.at(prevI),
                                            train->trainPath.at(i),
                                            false)->length;
        }
        return l;
    }

    /**
     * @brief Retrieves a network link originating from a specified start node.
     *
     * This function finds the next node in the train's path after the specified
     * start node, then retrieves the link from the start node to the next node.
     *
     * @param 	train	   	The train that is being processed.
     * @param 	startNodeID	The identifier of the start node.
     * @returns	A shared pointer to the NetLink object from the start node to
     *          the next node.
     * @throws std::out_of_range If startNodeID is not in trainPath or it is
     *          the last node in the path.
     */
    std::shared_ptr <NetLink> getLinkByStartNodeID(
                const std::shared_ptr <Train> train,
                int startNodeID) {
        int i = train->trainPath.index(startNodeID);
        if (i < train->trainPath.size() - 1) {
            int indx = i + 1;
            int EndNodeID = train->trainPath.at(indx);
            return getLinkByStartandEndNodeID(train, startNodeID, EndNodeID);
        }
        return nullptr;
    }

    /**
     * @brief Retrieves the first link in a train's path.
     *
     * This function retrieves the first node from the train's path and finds
     * the link from that node to the next node in the path.
     *
     * @param 	train	The train whose first link is to be retrieved.
     * @returns	A shared pointer to the first NetLink object in the train's
     *          path.
     * @throws std::runtime_error If the train's path is empty.
     */
    std::shared_ptr<NetLink> getFirstTrainLink(
                                const std::shared_ptr <Train> train) {
        if (train->trainPath.size() == 0) {
            throw std::runtime_error("Error: " +
                                     std::to_string(
                                            static_cast<int>(
                                                Error::trainPathCannotBeNull)) +
                                     "\n Train" +
                                     std::to_string(train->id) +
                                     " path cannot be null!\n");
        }
        else {
            int strartID = train->trainPath.at(0);
            return getLinkByStartNodeID(train, strartID);
        }
    }

    /**
     * @brief Generates a vector containing the cumulative lengths of the
     * train's path.
     *
     * The function iterates over the train's path and for each node,
     * calculates the cumulative distance from the start node to this node.
     * The result is stored in a vector which is then returned.
     *
     * @param train A shared pointer to the train object.
     * @returns A vector of doubles containing the cumulative lengths of the
     *          links in the train's path.
     */
    Vector<double> generateCumLinksLengths(std::shared_ptr<Train> train) {
        int n = train->trainPath.size();
        Vector<double> linksCumLengths(n, 0);
        for (int i = 1; i < n; i++) {
            linksCumLengths[i] =
                getDistanceToSpecificNodeFromStart(train,
                                                   train->trainPath.at(i));
        }
        return linksCumLengths;
    }

    /**
     * @brief Retrieves the previous node in the train's path given a travelled
     * distance.
     *
     * This function iterates over the train's path until it finds the node
     * whose cumulative distance is greater than the travelled distance.
     * The function then retrieves the node prior to this one.
     *
     * @param train The train object.
     * @param travelledDistance The distance that has been travelled.
     * @param previousNodeID An input/output parameter that will store the ID
     *                       of the previous node.
     * @returns A shared pointer to the previous NetNode object in the train's
     *          path.
     */
    std::shared_ptr <NetNode> getPreviousNodeByDistance(
                                    std::shared_ptr<Train> train,
                                    double travelledDistance,
                                    int &previousNodeID) {
        int nextI = -1;
        for (int i = train->trainPath.index(previousNodeID);
                                    train->trainPath.size(); i++) {
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
     * @brief Retrieves the link in the train's path that includes a specified
     * travelled distance.
     *
     * This function iterates over the train's path until it finds the node
     * whose cumulative distance is greater than the travelled distance.
     * The function then retrieves the link from the previous node to this one.
     *
     * @param train The train object.
     * @param travelledDistance The distance that has been travelled.
     * @param previousNodeID An input/output parameter that will store the ID
     *                       of the previous node.
     * @returns A shared pointer to the NetLink object that includes the
     *          travelled distance.
     */
    std::shared_ptr <NetLink> getLinkFromDistance(std::shared_ptr <Train> train,
                                                 double &travelledDistance,
                                                 int &previousNodeID) {
        int nextI = -1;
        for (int i = train->trainPath.index(previousNodeID);
             i < train->trainPath.size(); i++) {
            if (train->linksCumLengths.at(i) > travelledDistance) {
                nextI = i;
                break;
            }
        }
        if (nextI == -1) { nextI = train->trainPath.size() - 1; }
        int prevI = nextI - 1;
        return this->getLinkByStartandEndNodeID(train,
                                                train->trainPath.at(prevI),
                                                train->trainPath.at(nextI),
                                                true);
    }

    /**
     * @brief Translates a user-provided node identifier to a simulator-specific
     *  node identifier.
     *
     * This function iterates over the network nodes until it finds a node with
     * a user ID that matches the provided oldID. The function then returns
     * the simulator-specific ID of this node.
     *
     * @param oldID The user-provided identifier.
     * @returns The simulator-specific identifier for the node.
     * @throws std::runtime_error If a node with the provided user ID cannot be
     *         found.
     */
    int getSimulatorNodeIDByUserID(int oldID) {
        for (const std::shared_ptr<NetNode>& n : this->theFileNodes) {
            if (n->userID == oldID) {
                return n->id;
            }
        }
        throw std::runtime_error(std::string("Error: ") +
                                     std::to_string(static_cast<int>(
                                                       Error::cannotFindNode)) +
                                     "\nCould not find the node ID: " +
                                 std::to_string(oldID) + "\n");
    }


    /**
     * @brief Calculates the distance to a specific node given a travelled
     * distance.
     *
     * This function retrieves the cumulative distance to the specified node
     * and subtracts the travelled distance. The result is the remaining
     * distance to the specified node.
     *
     * @param train The train object.
     * @param travelledDistance The distance that has been travelled.
     * @param nodeID The identifier of the node.
     * @returns The distance to the specified node from the current location.
     */
    double getDistanceToSpecificNodeByTravelledDistance(
                    std::shared_ptr<Train> train,
                    double& travelledDistance,
                    int& nodeID) {
        int endNodeIndex = train->trainPath.index(nodeID);
        return train->linksCumLengths[endNodeIndex] - travelledDistance;
    }

    /**
     * @brief Checks if three points make a counter-clockwise turn.
     *
     * Given three points A, B, C, it returns true if they are listed in
     * counterclockwise order and false if they are listed in clockwise
     * order or are colinear. The points are pairs of coordinates (x,y) in a
     * 2D plane.
     *
     * @param A The first point as a pair (x,y).
     * @param B The second point as a pair (x,y).
     * @param C The third point as a pair (x,y).
     * @returns True if the points A, B, C are in counter-clockwise order,
     *  false otherwise.
     */
    bool ccw(const std::pair<double, double>& A,
             const std::pair<double, double>& B,
        const std::pair<double, double>& C) {
        return (C.second - A.second) *
                   (B.first - A.first) > (B.second - A.second) *
                                                        (C.first - A.first);
    }

    /**
     * @brief Checks if two lines intersect.
     *
     * This function takes four points A, B, C, D representing two lines
     * (AB and CD). It determines whether these lines intersect by checking
     * if the two points of one line straddle the other line.
     *
     * @param A The first point of the first line as a pair (x,y).
     * @param B The second point of the first line as a pair (x,y).
     * @param C The first point of the second line as a pair (x,y).
     * @param D The second point of the second line as a pair (x,y).
     * @returns True if the lines AB and CD intersect, false otherwise.
     */
    bool twoLinesIntersect(const std::pair<double, double>& A,
                           const std::pair<double, double>& B,
        const std::pair<double, double>& C,
                           const std::pair<double, double>& D) {
        return (ccw(A, C, D) != ccw(B, C, D)) && (ccw(A, B, C) != ccw(A, B, D));
    }

    /**
     * @brief Performs a shortest path search between two nodes.
     *
     * This function implements a Dijkstra-like shortest path algorithm.
     * It takes the start node and
     * target node IDs, then calculates the shortest path between them by
     * traversing the connected nodes and minimizing the overall path length.
     * It returns a pair, where the first element is a vector of nodes
     * representing the shortest path, and the second element is the total
     * length of this path.
     *
     * @param startNodeID The identifier for the start node.
     * @param targetNodeID The identifier for the target node.
     * @returns A pair consisting of a vector of node pointers forming the
     *          shortest path (in order from start to target),
     *          and a double representing the total length of the path.
     */
    std::pair<Vector<std::shared_ptr<NetNode>>, double> shortestPathSearch(
                    int startNodeID, int targetNodeID) {
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
                    currentNode->linkTo.at(connectedNode).at(0)->length) <
                            connectedNode->graphSearchDistanceFromStart) {
                    // set the value
                    connectedNode->graphSearchDistanceFromStart =
                        currentNode->graphSearchDistanceFromStart +
                            currentNode->linkTo.at(connectedNode).at(0)->length;
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
            if (node.second->graphSearchDistanceFromStart < minValue &&
                node.second->graphSearchVisited == false) {
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
        for (const std::shared_ptr<NetLink> &l : this->links) {
            if (l->trafficSignalNo != 0 && l->trafficSignalNo != 10001) {
                if (l->direction == 1) {
                    // if the link should have a signal at end point
                    if (l->trafficSignalAtEnd.exist(l->toLoc->userID)) {
                        NetSignal networkSignal = NetSignal(l->trafficSignalNo,
                                                            l,
                                                            l->fromLoc,
                                                            l->toLoc);
                        std::shared_ptr<NetSignal> sharedSignal =
                            std::make_shared<NetSignal>(networkSignal);
                        l->toLoc->addSignal(sharedSignal);
                        networkSignals.push_back(sharedSignal);
                    }
                    // if the link should have a signal at start point
                    if (l->trafficSignalAtEnd.exist(l->fromLoc->userID)) {
                        NetSignal networkSignal = NetSignal(l->trafficSignalNo,
                                                            l,
                                                            l->toLoc,
                                                            l->fromLoc);
                        std::shared_ptr<NetSignal> sharedSignal =
                            std::make_shared<NetSignal>(networkSignal);
                        l->fromLoc->addSignal(sharedSignal);
                        networkSignals.push_back(sharedSignal);
                    }
                }
                else {
                    // if the link should have a signal at end point
                    if (l->trafficSignalAtEnd.exist(l->toLoc->userID)) {
                        NetSignal networkSignal1 = NetSignal(l->trafficSignalNo,
                                                             l,
                                                             l->fromLoc,
                                                             l->toLoc);
                        std::shared_ptr<NetSignal> sharedSignal1 =
                            std::make_shared<NetSignal>(networkSignal1);
                        l->toLoc->addSignal(sharedSignal1);
                        networkSignals.push_back(sharedSignal1);
                    }
                    // if the link should have a signal at start point
                    if (l->trafficSignalAtEnd.exist(l->fromLoc->userID)) {
                        NetSignal networkSignal2 = NetSignal(l->trafficSignalNo,
                                                             l,
                                                             l->toLoc,
                                                             l->fromLoc);
                        std::shared_ptr<NetSignal> sharedSignal2 =
                            std::make_shared<NetSignal>(networkSignal2);
                        l->fromLoc->addSignal(sharedSignal2);
                        networkSignals.push_back(sharedSignal2);
                        //networkSignals.push_back(sharedSignal2);
                    }
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
                throw std::runtime_error("Error: " +
                                         std::to_string(static_cast<int>(
                                             Error::wrongLinksLength)) +
                                         "\nHorizontal distance between nodes" +
                                         " should be greater than 0!" +
                                         "\nReview the nodes file!... " +
                                         std::to_string(l->id) +
                                         "'s length is equal to zero!\n");
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
    bool isSameDirection(std::shared_ptr<NetLink> link,
                         std::shared_ptr <Train> train) {
        std::set<bool> allTrainsAreInSameDirection;
        for (const std::shared_ptr<Train>& t : link->currentTrains) {
            if (t != train) {
                allTrainsAreInSameDirection.insert(
                    std::is_permutation(t->trainPath.begin(),
                                        t->trainPath.end(),
                                        train->trainPath.begin(),
                                        train->trainPath.end()));
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
    double getDistanceToSpecificNodeFromStart(std::shared_ptr<Train> train,
                                              int nodeID) {
        double l = 0;
        for (int i = 1; i < train->trainPath.size(); i++) {
            int prevI = i - 1;
            l += getLinkByStartandEndNodeID(train,
                                            train->trainPath.at(prevI),
                                            train->trainPath.at(i),
                                            true)->length;
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
        ostr << "the network has " << stud.nodes.size() << " nodes and " <<
            stud.links.size() << "links." << endl;
        return ostr;
    }

};

#endif // !NeTrainSim_Network_h
