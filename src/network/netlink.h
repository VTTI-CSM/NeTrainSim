/**
 * @file NetLink.h
 * @brief This file contains the declaration of the NetLink class.
 *        The NetLink class represents a network link in a simulation.
 *        It stores information about the link, such as its ID, starting and ending nodes, length, speed, grade, etc.
 *        The NetLink class is used in conjunction with other classes, such as NetNode and Train, to simulate train movement on a network.
 *        It also includes static variables and methods to keep track of the number of links in the simulator.
 *        The NetLink class is intended to be used as part of a network simulation system.
 *        It is designed to work with other classes and data structures in the simulation.
 *        The NetLink class can be used in a C++ application.
 *        Note: The implementation of some methods is not provided in this declaration file.
 *              They should be implemented separately in a corresponding source file.
 * @author Ahmed Aredah
 * @version 0.1
 * @date 6/7/2023
 */

#ifndef NeTrainSim_NetLink_h
#define NeTrainSim_NetLink_h

#include <string>
#include <iostream>
#include <map>
#include "../util/vector.h"
#include <memory>

class NetNode; // Forward declaration of NetNode class
class Train; // Forward declaration of Train class

using namespace std;

/**
 * @class NetLink
 * @brief The NetLink class represents a network link in a simulation.
 */
class NetLink {
private:
    static unsigned int NumberOfLinksInSimulator; /**< The number of links in the simulator. */

public:
    int id; /**< The identifier of the link. */
    int userID; /**< The user identifier of the link. */
    std::shared_ptr<NetNode> fromLoc; /**< The starting node of the link. */
    std::shared_ptr<NetNode> toLoc; /**< The end node of the link. */
    double length; /**< The length of the link. */
    double freeFlowSpeed; /**< The free flow speed of the link. */
    int trafficSignalNo; /**< The traffic network signal number of the link. */
    map<int, double> grade; /**< The grade of the link. */
    double curvature; /**< The curvature of the link. */
    int direction; /**< The direction of the link. */
    double speedVariation; /**< The speed variation of the link. */
    std::string region; /**< The region in which the link is located. */
    double linksScaleLength; /**< The length scale of the links. */
    double linksScaleFreeSpeed; /**< The free speed scale of the links. */
    Vector<std::shared_ptr<Train>> currentTrains; /**< A vector of pointers to trains currently on this link. */
    double cost; /**< The cost associated with this link. */
    bool hasCatenary; /**< Indicates whether the link has catenary. */
    double catenaryCumRegeneratedEnergy; /**< The amount of energy regenerated by all trains on the network. */
    double catenaryCumConsumedEnergy; /**< The amount of energy consumed by all trains on the network. */

    /**
     * @brief Constructor
     * @param simulatorID The simulator identifier of the link.
     * @param linkID The identifier of the link.
     * @param fromNode The starting node of the link.
     * @param toNode The end node of the link.
     * @param linkLength The length of the link.
     * @param maxSpeed The maximum speed of the link.
     * @param trafficSignalID The traffic signal identifier of the link.
     * @param linkGrade The grade of the link.
     * @param linkCurvature The curvature of the link.
     * @param linkNoOfDirections The number of directions of the link.
     * @param speedVariationfactor The speed variation factor of the link.
     * @param isCatenaryAvailable Indicates whether catenary is available on the link.
     * @param linkInRegion The region in which the link is located.
     * @param lengthScale The length scale of the link.
     * @param maxSpeedScale The maximum speed scale of the link.
     */
    NetLink(int simulatorID, int linkID, std::shared_ptr<NetNode> fromNode, std::shared_ptr<NetNode> toNode,
            double linkLength, double maxSpeed, int trafficSignalID, double linkGrade,
            double linkCurvature, int linkNoOfDirections, double speedVariationfactor,
            bool isCatenaryAvailable, string linkInRegion, double lengthScale, double maxSpeedScale);

    /**
     * @brief Destructor
     */
    ~NetLink();

    /**
     * @brief Sets the simulator identifier of the link.
     * @param newID The new simulator identifier.
     */
    void setLinkSimulatorID(int newID);

    /**
     * @brief Updates the links scale length.
     * @param newScale The new scale value.
     */
    void updateLinksScaleLength(double newScale);

    /**
     * @brief Gets the number of links in the simulator.
     * @return The number of links.
     */
    static unsigned int getNumberOfLinks();

    /**
     * @brief Updates the links scale free speed.
     * @param newScale The new scale value.
     */
    void updateLinksScaleFreeSpeed(double newScale);

    /**
     * @brief Sets the grade of the link.
     * @param grade The grade map.
     * @return The grade map.
     */
    map<int, double> setGrade(double grade);

    /**
     * @brief Gets the cost associated with the link.
     * @return The cost value.
     */
    double getCost();

};

/**
 * @brief Overloaded ostream operator for printing the NetLink object.
 * @param ostr The output stream object.
 * @param link The NetLink object to be printed.
 * @return The output stream object.
 */
ostream& operator<<(ostream& ostr, const NetLink& link);

#endif // !NeTrainSim_NetLink_h