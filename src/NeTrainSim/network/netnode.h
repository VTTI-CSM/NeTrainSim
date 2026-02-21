/**
 * @file NetNode.h
 * @brief This file contains the declaration of the NetNode class.
 *        The NetNode class represents a network node in a simulation.
 *        It stores information about the node, such as its ID, coordinates, description, scales, etc.
 *        The NetNode class is used in conjunction with other classes, such as NetSignal and NetLink, to simulate network behavior.
 *        It also includes static variables and methods to keep track of the number of nodes in the simulator.
 *        The NetNode class is intended to be used as part of a network simulation system.
 *        It is designed to work with other classes and data structures in the simulation.
 *        The NetNode class can be used in a C++ application.
 *        Note: The implementation of some methods is not provided in this declaration file.
 *              They should be implemented separately in a corresponding source file.
 * @author Ahmed Aredah
 * @version 0.1
 * @date 6/7/2023
 */

#ifndef NeTrainSim_NetNode_h
#define NeTrainSim_NetNode_h

#include "../export.h"
#include <memory>
#include <string>
#include <iostream>
#include "../util/vector.h"
#include "../util/map.h"

class NetSignal; // Forward declaration of NetSignal class
class NetLink; // Forward declaration of NetLink class

using namespace std;

/**
 * @class NetNode
 * @brief The NetNode class represents a network node in a simulation.
 */
class NETRAINSIMCORE_EXPORT NetNode {
private:
    static unsigned int NumberOfNodesInSimulator; /**< The number of nodes in the simulator. */


public:
    int id = -1; /**< The identifier of the node. */
    int userID; /**< The user identifier of the node. */
    double x; /**< The x-coordinate of the node. */
    double y; /**< The y-coordinate of the node. */
    std::string alphaDesc; /**< The description of the node. */
    double xScale; /**< The x-value scale of the node. */
    double yScale; /**< The y-value scale of the node. */
    Vector<std::shared_ptr<NetSignal>> networkSignals; /**< The network signals associated with the node. */
    bool isTerminal; /**< Indicates whether the node is a stopping station or terminal for all trains. */
    double dwellTimeIfTerminal; /**< The dwell time at the terminal. */
    bool refillTanksAndBatteries; /**< Indicates whether trains refill tanks and batteries when passing through this node. */
    Map<std::shared_ptr<NetNode>, Vector<std::shared_ptr<NetLink>>> linkTo; /**< The neighbor nodes and their link connections. */

    // Graph search variables
    double graphSearchDistanceFromStart; /**< The graph search distance from the start node. */
    bool graphSearchVisited; /**< Indicates whether the node has been visited in graph search. */
    std::shared_ptr<NetNode> graphSearchPreviousNode; /**< The previous node in the path during graph search. */

    /**
     * @brief Constructor
     * @param simulatorID The simulator identifier of the node.
     * @param userID The user identifier of the node.
     * @param xCoord The x-coordinate of the node.
     * @param yCoord The y-coordinate of the node.
     * @param Desc The description of the node.
     * @param xScale The x-value scale of the node.
     * @param yScale The y-value scale of the node.
     */
    NetNode(int simulatorID, int userID, double xCoord, double yCoord, string Desc, double xScale, double yScale);

    /**
     * @brief Destructor
     */
    ~NetNode();

    /**
     * @brief Sets the simulator identifier of the node.
     * @param newID The new simulator identifier.
     */
    void setNodeSimulatorID(int newID);

    /**
     * @brief Gets the neighbors of the node.
     * @return A vector of shared pointers to the neighbor nodes.
     */
    Vector<std::shared_ptr<NetNode>> getNeighbors();

    /**
     * @brief Clears the graph search parameters.
     *        This should be called before running the graph search.
     */
    void clearGraphSearchParams();

    /**
     * @brief Gets the number of nodes in the simulator.
     * @return The number of nodes.
     */
    static unsigned int getNumberOfNodesInSimulator();

    /**
     * @brief Adds a network signal to the node.
     * @param networkSignal The network signal to add.
     */
    void addSignal(std::shared_ptr<NetSignal> networkSignal);

    /**
     * @brief Updates the x-coordinate scale of the node.
     * @param newScale The new scale value.
     */
    void updateXScale(const double& newScale);

    /**
     * @brief Updates the y-coordinate scale of the node.
     * @param newScale The new scale value.
     */
    void updateYScale(const double& newScale);

    /**
     * @brief Gets the coordinates of the node.
     * @return A pair containing the x and y coordinates.
     */
    pair<double, double> coordinates();

    // Other methods and friend function declarations...
};

/**
 * @brief Overloaded ostream operator for printing the NetNode object.
 * @param ostr The output stream object.
 * @param node The NetNode object to be printed.
 * @return The output stream object.
 */
ostream& operator<<(ostream& ostr, const NetNode& node);

#endif // NeTrainSim_NetNode_h
