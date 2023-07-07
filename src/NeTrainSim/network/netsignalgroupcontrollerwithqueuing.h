#ifndef NETSIGNALGROUPCONTROLLERWITHQUEUING_H
#define NETSIGNALGROUPCONTROLLERWITHQUEUING_H


#include "../util/map.h"
#include "../util/vector.h"
#include <deque>
#include <set>

class NetLink; // Forward declaration of NetLink class
class NetNode; // Forward declaration of NetNode class
class NetSignal; // Forward declaration of NetSignal class
class Train;    // Forward declaration of a train

/**
 * @class NetSignalGroupControllerWithQueuing
 * @brief Responsible for controlling and queuing network signals and
 * managing train movements.
 */
class NetSignalGroupControllerWithQueuing
{
private:
    // Timeout for network signal in seconds.
    double timeout;

    // Queue of waiting trains along with their associated timestamps.
    std::deque<std::pair<std::shared_ptr<Train>, double>> waitingTrains;

    // Collection of network signals associated with this controller.
    Vector<std::shared_ptr<NetSignal>> networkSignalsGroup;

    // Collection of nodes that this controller is associated with.
    Vector<std::shared_ptr<NetNode>> atNodes;

    // Map tracking the movement status of different network signals.
    Map<std::shared_ptr<NetSignal>, bool> movements;

    // The timestamp at which the controller last updated.
    double timeStamp;

public:

    /**
     * Constructor
     *
     * @author Ahmed Aredah
     * @date 7/5/2023
     *
     * @param nodes   Set of shared pointers to NetNode.
     * @param timeStep  Time step for the signal controller.
     */
    NetSignalGroupControllerWithQueuing(std::set<std::shared_ptr<NetNode>> nodes,
                                        double timeStep);

    /**
     * Get the signals this controller is managing.
     *
     * @author Ahmed Aredah
     * @date 7/5/2023
     *
     * @return networkSignals Vector of shared pointers to NetSignals.
     */
    Vector<std::shared_ptr<NetSignal>> getControllerSignals();

    /**
     * Clear Movements
     *
     * @author Ahmed Aredah
     * @date 7/5/2023
     */
    void addNode(std::shared_ptr<NetNode> node);

    /**
     * Add Train
     *
     * @author Ahmed Aredah
     * @date 7/5/2023
     *
     * @param train Shared pointer to a Train.
     * @param simulatorTime Simulation time.
     */
    void addTrain(std::shared_ptr<Train> train, double simulatorTime);

    /**
     * Send Pass Request to Control
     *
     * @author Ahmed Aredah
     * @date 7/5/2023
     *
     * @param train Shared pointer to a Train.
     * @param networkSignal Shared pointer to a NetSignal.
     * @param simulatorTime Reference to simulation time.
     * @param sameDirectionSignals Vector of shared pointers to NetSignal.
     */
    void sendPassRequestToControlTo(std::shared_ptr<Train> train,
                                    std::shared_ptr<NetSignal> networkSignal,
                                    double& simulatorTime,
                                    Vector<std::shared_ptr<NetSignal>> &sameDirectionSignals);

    /**
     * Update Time Step
     *
     * @author Ahmed Aredah
     * @date 7/5/2023
     *
     * @param train Shared pointer to a Train.
     * @param networkSignal Shared pointer to a NetSignal.
     * @param simulatorTime Reference to simulation time.
     * @param sameDirectionSignals Vector of shared pointers to NetSignal.
     */
    void updateTimeStep(std::shared_ptr<Train> train,
                        std::shared_ptr<NetSignal> networkSignal,
                        double& simulatorTime,
                        Vector<std::shared_ptr<NetSignal>> &sameDirectionSignals);

    /**
     * Get Feedback
     *
     * @author Ahmed Aredah
     * @date 7/5/2023
     *
     * @return pair of Vectors of shared pointers to NetSignal.
     */
    std::pair<Vector<std::shared_ptr<NetSignal> >,
              Vector<std::shared_ptr<NetSignal> > > getFeedback();

    /**
     * Turn off signal
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	networkSignals	The network signals to be turned off.
     */
    void turnOffSignals(Vector<std::shared_ptr<NetSignal>> turnOffSignals);


private:
    /**
     * Set Open Signals
     *
     * @author Ahmed Aredah
     * @date 7/5/2023
     *
     * @param sameDirectionSignals Vector of shared pointers to NetSignal.
     */
    void setOpenSignals(Vector<std::shared_ptr<NetSignal>>& sameDirectionSignals);

    /**
     * Clear Movements
     *
     * @author Ahmed Aredah
     * @date 7/5/2023
     */
    void clearMovements();
};

#endif // NETSIGNALGROUPCONTROLLERWITHQUEUING_H
