/**
 * @file NetSignal.h
 * @brief This file contains the declaration of the NetSignal class.
 *        The NetSignal class represents a network signal in a simulation.
 *        It stores information about the signal, such as its ID, state, proximity, etc.
 *        The NetSignal class is used in conjunction with other classes, such as NetLink and NetNode, to simulate network behavior.
 *        It also includes static variables and methods to keep track of the number of signals in the simulator.
 *        The NetSignal class is intended to be used as part of a network simulation system.
 *        It is designed to work with other classes and data structures in the simulation.
 *        The NetSignal class can be used in a C++ application.
 *        Note: The implementation of some methods is not provided in this declaration file.
 *              They should be implemented separately in a corresponding source file.
 * @author Ahmed Aredah
 * @version 0.1
 * @date 2/14/2023
 */

#ifndef NeTrainSim_NetSignal_h
#define NeTrainSim_NetSignal_h

#include "../export.h"
#include <memory>
#include <string>
#include <iostream>
#include <map>

class NetLink; // Forward declaration of NetLink class
class NetNode; // Forward declaration of NetNode class

using namespace std;

/**
 * @class NetSignal
 * @brief The NetSignal class represents a network signal in a simulation.
 */
class NETRAINSIMCORE_EXPORT NetSignal : public std::enable_shared_from_this<NetSignal> {
private:
    static unsigned int NumberOfSignalsInSimulator; /**< The number of signals in the simulator. */

public:
    int userID; /**< The user identifier of the signal. */
    int id; /**< The identifier of the signal. */
    bool isGreen; /**< Indicates whether the signal is green. */
    double proximityToActivate; /**< The proximity at which the signal activates. */

    std::weak_ptr<NetLink> link; /**< The link associated with the signal. */
    std::weak_ptr<NetNode> previousNode; /**< The previous node connected to the signal. */
    std::weak_ptr<NetNode> currentNode; /**< The current node connected to the signal. */

    /**
     * @brief Constructor
     * @param signalID The identifier of the signal.
     * @param hostingLink The link hosting the signal.
     */
    NetSignal(int signalID, std::shared_ptr<NetLink> hostingLink);

    /**
     * @brief Constructor
     * @param signalID The identifier of the signal.
     * @param hostingLink The link hosting the signal.
     * @param previousLinkNode The previous node connected to the signal.
     * @param currentLinkNode The current node connected to the signal.
     */
    NetSignal(int signalID, std::shared_ptr<NetLink> hostingLink,
              std::shared_ptr<NetNode> previousLinkNode, std::shared_ptr<NetNode> currentLinkNode);

    /**
     * @brief Gets the number of signals in the simulator.
     * @return The number of signals.
     */
    static unsigned int getNumberOfSignalsInSimulator();

    /**
     * @brief Overloaded ostream operator for printing the NetSignal object.
     * @param ostr The output stream object.
     * @param signal The NetSignal object to be printed.
     * @return The output stream object.
     */
    friend ostream& operator<<(ostream& ostr, const NetSignal& signal);
};

#endif // NeTrainSim_NetSignal_h
