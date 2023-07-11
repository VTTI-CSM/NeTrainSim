#include "netsignalgroupcontrollerwithqueuing.h"

#include "netnode.h"
#include "netsignal.h"
#include "../traindefinition/train.h"
#include "../util/vector.h"

// Constructor for the signal controller
NetSignalGroupControllerWithQueuing::NetSignalGroupControllerWithQueuing(
    std::set<std::shared_ptr<NetNode>> nodes,
    double timeStep) {

    // Iterate through each node and for each node's signal, add it to
    // the signal group if it doesn't already exist and mark its movement
    // as false
    for (const std::shared_ptr<NetNode> &node : nodes) {
        this->atNodes.push_back(node);
        for (int i = 0; i < node->networkSignals.size(); i++) {
            if (!this->networkSignalsGroup.exist(
                    std::shared_ptr<NetSignal>(node->networkSignals.at(i))))
            {
                this->networkSignalsGroup.push_back(
                    std::shared_ptr<NetSignal>(node->networkSignals.at(i)));
            }
            this->movements[node->networkSignals.at(i)] = false;
        }
    }
    timeStamp = -10.0;

    // Setting the timeout to be 5 times the provided time step
    timeout = 5 * timeStep;

    clearTrainsAt = 0.0;
}

// Method to add a new node to the network
void NetSignalGroupControllerWithQueuing::addNode(
    std::shared_ptr<NetNode> node) {
    // Check if the node has any signals. If it does and it doesn't
    // already exist in the network, add it to the network and its
    // signals to the signal group
    if (node->networkSignals.size() > 0) {
        if (! this->atNodes.exist(node)) {
            this->atNodes.push_back(node);
            for (int i = 0; i < node->networkSignals.size(); i++) {
                if (! this->networkSignalsGroup.exist(
                        std::shared_ptr<NetSignal> (
                            node->networkSignals.at(i)))) {
                    this->networkSignalsGroup.push_back(
                        std::shared_ptr<NetSignal>(
                            node->networkSignals.at(i)));
                }
            }
        }
    }
}

// Method to update the time step for a specific train
void NetSignalGroupControllerWithQueuing::updateTimeStep(
    std::shared_ptr<Train> train,
    std::shared_ptr<NetSignal> networkSignal,
    double& simulatorTime,
    Vector<std::shared_ptr<NetSignal>>& sameDirectionSignals) {
    // If the network signal is in the same direction and the train is at
    // the front of the queue, update the timestamp,
    // clear the movements and set the open signals
    if (sameDirectionSignals.exist(networkSignal) &&
        waitingTrains.front().first == train)
    {
        this->timeStamp = simulatorTime;
        this->clearMovements();
        this->setOpenSignals(sameDirectionSignals);
    }
}

// Method to clear all movements in the network
void NetSignalGroupControllerWithQueuing::clearMovements()
{
    // Set all movements in the network to false
    for (std::shared_ptr<NetSignal>& netSignal : this->movements.get_keys())
    {
        this->movements[netSignal] = false;
    }
}

void NetSignalGroupControllerWithQueuing::addTrain(
    std::shared_ptr<Train> train, double simulatorTime)
{
    // Add the current train to the queue only if it's not already present
    if (std::find_if(waitingTrains.begin(),
                     waitingTrains.end(),
                     [&](const auto& pair)
                     { return pair.first == train; }) == waitingTrains.end())
    {
        waitingTrains.push_back(std::make_pair(train, simulatorTime));
    }
}

// Method to add a train to the queue
void NetSignalGroupControllerWithQueuing::sendPassRequestToControlTo(
    std::shared_ptr<Train> train,
    std::shared_ptr<NetSignal> networkSignal,
    double& simulatorTime,
    Vector<std::shared_ptr<NetSignal>>& sameDirectionSignals)
{
    // if the deque is empty, return
    if (waitingTrains.empty()) { return; }

    // If the train is not in the queue, ignore the request
    if (std::find_if(waitingTrains.begin(),
                     waitingTrains.end(),
                     [&](const auto& pair)
                     { return pair.first == train; }) == waitingTrains.end())
    {
        return;
    }

    // If the train is the highest priority train in the queue and
    // its signal is in the same direction, update the timestamp and
    // clear movements and set open signals If timeout has passed,
    // remove the highest priority train from the queue and repeat
    // the above steps for the next train in the queue
    if ( waitingTrains.front().first->id == train->id ) {
        this->timeStamp = simulatorTime;
        // update that all controlled trains are synced
        for (auto& pair : waitingTrains) {
            pair.second = simulatorTime;
        }
        this->clearMovements();
        this->setOpenSignals(sameDirectionSignals);
    }
    // Check if timeout has passed
    else if (simulatorTime - this->timeStamp > timeout) {
        // remove only the first high priority train
        // and give an opportunity for the rest to be checked
//        if (simulatorTime - waitingTrains.front().second > timeout) {
//            // Processed last waiting train
//            waitingTrains.pop_front();

            // Reset the timestamp
            this->timeStamp = simulatorTime;
//            // update that all controlled trains are synced
//            for (auto& pair : waitingTrains) {
//                pair.second = simulatorTime;
//            }
            this->clearMovements();
//            this->setOpenSignals(sameDirectionSignals);
//        }

    }
}

void NetSignalGroupControllerWithQueuing::clearTimeoutTrains(
    double simulatorTime) {
    if (clearTrainsAt != simulatorTime)
    {
        // remove all timeout trains
        std::erase_if(waitingTrains, [this, simulatorTime](const auto& pair) {
            return simulatorTime - pair.second > timeout;
        });

        clearTrainsAt = simulatorTime;
    }
}


// Method to set open signals
void NetSignalGroupControllerWithQueuing::setOpenSignals(
    Vector<std::shared_ptr<NetSignal>>& sameDirectionSignals) {
    // Mark all signals in the same direction as true
    for (std::shared_ptr<NetSignal>& netSignal : sameDirectionSignals) {
        this->movements[netSignal] = true;
    }
}

// Method to get feedback from the network
std::pair<Vector<std::shared_ptr<NetSignal> >,
          Vector<std::shared_ptr<NetSignal> >
         > NetSignalGroupControllerWithQueuing::getFeedback() {

    Vector<std::shared_ptr<NetSignal>> sameDirection;
    Vector<std::shared_ptr<NetSignal>> otherDirection;

    // check if there is any waiting trains first
    // if non, return all signals are green
    if (waitingTrains.size() == 0) {
        return std::make_pair(this->movements.get_keys(), otherDirection);
    }

    // For each signal, if it's moving in the same direction,
    // add it to the sameDirection vector.
    // If it's moving in the other direction, add it to the
    // otherDirection vector.
    for (std::shared_ptr<NetSignal>& netSignal : this->movements.get_keys()) {
        if (this->movements[netSignal] == true) {
            sameDirection.push_back(netSignal);
        }
        else {
            otherDirection.push_back(netSignal);
        }
    }
    // Return a pair of vectors: one with signals moving in the
    // same direction and one with signals moving in the other direction
    return std::make_pair(sameDirection, otherDirection);
}

// Method to turn off signals in the network
void NetSignalGroupControllerWithQueuing::turnOffSignals(
    Vector<std::shared_ptr<NetSignal>> networkSignals) {

    // If the vector of signals is not empty, set the
    // isGreen property of each signal to false
    if (networkSignals.empty()) { return; }
    for (auto& networkSignal : networkSignals) {
        networkSignal->isGreen = false;
    }
}

// Method to get all signals controlled by the controller
Vector<std::shared_ptr<NetSignal>> NetSignalGroupControllerWithQueuing::
                                                        getControllerSignals() {
    // Return the vector of signals controlled by the controller
    return this->networkSignalsGroup;
}
