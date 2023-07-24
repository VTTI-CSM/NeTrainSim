/**
 * @file NetLink.h
 * @brief This file contains the declaration of the NetLink class.
 *        The NetLink class represents a network link in a simulation.
 *        It stores information about the link, such as its ID, starting and
 *        ending nodes, length, speed, grade, etc.
 *        The NetLink class is used in conjunction with other classes,
 *        such as NetNode and Train, to simulate train movement on a network.
 *        It also includes static variables and methods to keep track of the
 *        number of links in the simulator.
 *        The NetLink class is intended to be used as part of
 *        a network simulation system.
 *        It is designed to work with other classes and data
 *        structures in the simulation.
 *        The NetLink class can be used in a C++ application.
 *        Note: The implementation of some methods is not provided
 *        in this declaration file.
 *        They should be implemented separately in a corresponding source file.
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
    /// The number of links in the simulator.
    static unsigned int NumberOfLinksInSimulator;

public:
    /// The identifier of the link.
    int id;
    /// The user identifier of the link.
    int userID;
    /// The starting node of the link.
    std::shared_ptr<NetNode> fromLoc;
    /// The end node of the link.
    std::shared_ptr<NetNode> toLoc;
    /// This extends the line to a polyline link.
    Vector<std::shared_ptr<NetNode>> intermediatePoints;
    /// The user-defined length of the link.
    double length;
    /// The simulator length. This length is used for visualization only.
    double simulatorLength;
    /// The free flow speed of the link.
    double freeFlowSpeed;
    /// The traffic network signal number of the link.
    int trafficSignalNo;
    /// The grade of the link.
    map<int, double> grade;
    /// The curvature of the link.
    double curvature;
    /// The direction of the link.
    int direction;
    /// The speed variation of the link.
    double speedVariation;
    /// The region in which the link is located.
    std::string region;
    /// the scale factor of the user defined length
    double linksScaleLength;
    /// The free speed scale of the links.
    double linksScaleFreeSpeed;
    /// A vector of pointers to trains currently on this link.
    Vector<std::shared_ptr<Train>> currentTrains;
    /// The cost associated with this link.
    double cost;
    /// Indicates whether the link has catenary.
    bool hasCatenary;
    /// The amount of energy regenerated by all trains on the network.
    double catenaryCumRegeneratedEnergy;
    /// The amount of energy consumed by all trains on the network.
    double catenaryCumConsumedEnergy;
    /// The Node ID's where the signals are placed at.
    Vector<int> trafficSignalAtEnd;

    /**
     * @brief Constructor
     * @param simulatorID The simulator identifier of the link.
     * @param linkID The identifier of the link.
     * @param fromNode The starting node of the link.
     * @param toNode The end node of the link.
     * @param linkLength The length of the link.
     * @param maxSpeed The maximum speed of the link.
     * @param trafficSignalID The traffic signal identifier of the link.
     * @param signalAtEnd Which End of the link, the signal is placed at
     * @param linkGrade The grade of the link.
     * @param linkCurvature The curvature of the link.
     * @param linkNoOfDirections The number of directions of the link.
     * @param speedVariationfactor The speed variation factor of the link.
     * @param isCatenaryAvailable Indicates whether catenary
     *        is available on the link.
     * @param linkInRegion The region in which the link is located.
     * @param lengthScale The length scale of the link.
     * @param maxSpeedScale The maximum speed scale of the link.
     */
    NetLink(int simulatorID, int linkID, std::shared_ptr<NetNode> fromNode,
            std::shared_ptr<NetNode> toNode, double linkLength,
            double maxSpeed, int trafficSignalID,
            string signalAtEnd, double linkGrade,
            double linkCurvature, int linkNoOfDirections,
            double speedVariationfactor, bool isCatenaryAvailable,
            string linkInRegion, double lengthScale, double maxSpeedScale);

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

    /**
     * Creates a vector representation of a link with respect
     * to a given start node.
     * @param startNode A shared_ptr to a NetNode object
     *        representing the start node.
     * @return A pair of pairs of doubles, where the first
     *         pair represents the coordinates of the start
     *         node, and the second pair represents the vector
     *         from the start node to the end node.
     * @author Ahmed
     * @date 2/14/2023
     */
    std::pair<std::pair<double, double>,
              std::pair<double, double>> vectorizeLinkOfOneSegment(
        std::shared_ptr <NetNode> startNode);

    pair<double, double> findPositionOnLink(
        double travelledLength,
        std::shared_ptr<NetNode> startNode);


private:
    /**
     * Computes the position on a given vector after travelling a
     * specified distance along the x-axis
     *
     * This function calculates the position reached on a 2D vector
     * after moving a specified distance along the x-axis.
     * The vector is specified by two points (a pair of doubles),
     * where the first point is the origin and the second point
     * represents the direction of the vector. The function
     * uses the projectLengthonPathVector function to determine
     * the displacement along the vector.
     *
     * Note: This function assumes that the vector lies on
     * a plane with the x-axis and that the travelled
     * distance is along the x-axis.
     *
     * @param vector The 2D vector on which to compute the position.
     *        This is represented as a pair of
     *        std::pair<double, double> elements.
     *        The first pair represents the origin of the vector
     *        and the second pair represents the direction of the vector.
     * @param travelledDistanceOnXAxis The distance travelled along
     *        the x-axis. This value must be non-negative.
     * @return A std::pair<double, double> representing the
     *         position on the vector after travelling the specified
     *         distance along the x-axis. The first element of the
     *         pair is the x-coordinate, and the second
     *         element is the y-coordinate.
     * @throws std::invalid_argument If travelledDistanceOnXAxis
     *         is less than 0.
     * @author Ahmed
     * @date 2/14/2023
     */
    std::pair<double, double> getPositionOnVector(
        std::pair<std::pair<double, double>,
                  std::pair<double, double>>vector,
        double travelledDistanceOnXAxis);

    /**
     * @brief update the simulator length. this length is only used in
     * visualization. The function gets the length by the nodes coordinates.
     */
    void updateSimulatorLength();

    /**
     * Project a specified length along a given vector direction
     *
     * This function projects a given length along the direction
     * of a specified 2D vector,
     * represented as a std::pair of doubles. It creates a vector
     * of the specified length
     * that shares the same direction as the input vector.
     *
     * The method used to achieve this is by creating a unit vector
     * in the direction of the input vector, multiplying it by the
     * desired length, and then rotating this new vector
     * to the same direction as the input vector.
     *
     * The function throws an std::invalid_argument exception if
     * the length is negative.
     *
     * @param vectorEndpoints The 2D vector along which to project
     *        the length, represented
     *        as a std::pair of doubles. The first element represents
     *        the x component, and the second element represents
     *        the y component. This vector does not need to
     *        be normalized; it simply defines a direction.
     * @param length The length to be projected along the direction
     *        of the input vector.
     * @return A std::pair representing the vector resulting from
     *         the projection. The vector
     *         has the same direction as the input vector and a
     *         length specified by the 'length' parameter.
     * @throws std::invalid_argument If the length is less than 0.
     * @author Ahmed
     * @date 2/14/2023
     */
    std::pair<double, double> projectLengthonPathVector(
        std::pair<double, double>& vectorEndpoints,
        double length);
};



/**
 * @brief Overloaded ostream operator for printing the NetLink object.
 * @param ostr The output stream object.
 * @param link The NetLink object to be printed.
 * @return The output stream object.
 */
ostream& operator<<(ostream& ostr, const NetLink& link);

#endif // !NeTrainSim_NetLink_h
