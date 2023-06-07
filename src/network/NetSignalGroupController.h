/**
 * @file NetSignalGroupController.h
 * @brief This file contains the declaration of the NetSignalGroupController class.
 *        The NetSignalGroupController class represents a controller for handling network signal groups in a simulation.
 *        It manages the network signals, nodes, and confined links within the group.
 *        The controller is responsible for processing pass requests from trains, maintaining the controller state, and providing feedback on signal activation.
 *        The NetSignalGroupController class is intended to be used as part of a network simulation system.
 *        It is designed to work with other classes and data structures in the simulation.
 *        The NetSignalGroupController class can be used in a C++ application.
 *        Note: The implementation of some methods is not provided in this declaration file.
 *              They should be implemented separately in a corresponding source file.
 * @author Ahmed Aredah
 * @version 0.1
 * @date 6/7/2023
 */

#ifndef NeTrainSim_NetSignalGroupController_h
#define NeTrainSim_NetSignalGroupController_h

#include <memory>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include "../util/Vector.h"
#include "../util/Map.h"

class NetLink; // Forward declaration of NetLink class
class NetNode; // Forward declaration of NetNode class
class NetSignal; // Forward declaration of NetSignal class

using namespace std;

/**
 * @class NetSignalGroupController
 * @brief The NetSignalGroupController class represents a controller for handling network signal groups in a simulation.
 */
class NetSignalGroupController {
private:
    /** The duration between the time stamp of the controller and the simulator is
     *  referred to as the timeout threshold. This threshold is the amount of time the
     *  controller maintains the same lockedOnSignal and associated data. Once this
     *  duration has elapsed, any new requests will be processed, and the old data
     *  will be discarded. A higher value for this threshold means that more time
     *  is needed to process requests from trains traveling in the opposite direction. */
    static constexpr double timeout = 5;

    static unsigned int NumberOfSignalsInSimulator; /**< The number of signals in the simulator. */

public:
    Vector<std::shared_ptr<NetSignal>> networkSignalsGroup; /**< The network signals in the group. */
    Vector<std::shared_ptr<NetNode>> atNodes; /**< The nodes confined by this group controller. */
    Vector<std::shared_ptr<NetLink>> confinedLinks; /**< The confined links in the group. */
    Map<std::shared_ptr<NetSignal>, bool> movements; /**< The movement status of the signals. */
    double timeStamp; /**< The timestamp of the controller. */
    std::shared_ptr<NetSignal> lockedOnSignal; /**< The locked network signal. */
    Vector<std::shared_ptr<NetSignal>> otherDirectionSignals; /**< The signals in the opposite direction. */

    /**
     * @brief Constructor
     * @param nodes The set of nodes confined by this group controller.
     */
    NetSignalGroupController(std::set<std::shared_ptr<NetNode>> nodes);

    /**
     * @brief Clears the controller to its initial state.
     */
    void clear();

    /**
     * @brief Adds a node to the group controller.
     * @param node The node to add.
     */
    void addNode(std::shared_ptr<NetNode> node);

    /**
     * @brief Sends a pass request to control a specific network signal.
     * @param networkSignal The network signal to control.
     * @param simulationTime The current simulation time.
     * @param sameDirectionSignals The signals in the same direction as the train.
     */
    void sendPassRequestToControlTo(std::shared_ptr<NetSignal> networkSignal,
                                    double& simulationTime,
                                    Vector<std::shared_ptr<NetSignal>>& sameDirectionSignals);

    /**
     * @brief Updates the controller state based on the time step.
     * @param timeStep The time step.
     */
    void updateTimeStep(double& timeStep);

    /**
     * @brief Gets the feedback on which signals should be turned on/off.
     * @return A pair of the locked network signal and the signals to be turned off.
     */
    std::pair<std::shared_ptr<NetSignal>, Vector<std::shared_ptr<NetSignal>>> getFeedback();

    /**
     * @brief Sets the signals in the same direction as the train.
     * @param sameDirectionSignals The signals in the same direction.
     */
    void setSignalsInSameDirection(Vector<std::shared_ptr<NetSignal>> sameDirectionSignals);

    /**
     * @brief Overloaded ostream operator for printing the NetSignalGroupController object.
     * @param ostr The output stream object.
     * @param group The NetSignalGroupController object to be printed.
     * @return The output stream object.
     */
    friend ostream& operator<<(ostream& ostr, const NetSignalGroupController& group);

private:
    /**
     * @brief Turns off the specified signals.
     * @param turnOffSignals The signals to turn off.
     */
    void turnOffSignals(Vector<std::shared_ptr<NetSignal>> turnOffSignals);
};

#endif // NeTrainSim_NetSignalGroupController_h
