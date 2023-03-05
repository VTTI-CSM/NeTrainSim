#include "NetSignalGroupController.h"
#include "NetNode.h"
#include "NetSignal.h"
#include "NetLink.h"
#include "../util/Vector.h"

NetSignalGroupController::NetSignalGroupController(std::set<std::shared_ptr<NetNode>> nodes) {
	
	for (std::shared_ptr<NetNode> node : nodes) {
		this->atNodes.push_back(node);
		for (int i = 0; i < node->networkSignals.size(); i++) {
			if (!this->networkSignalsGroup.exist(std::shared_ptr<NetSignal>(node->networkSignals.at(i)))) {
				this->networkSignalsGroup.push_back(std::shared_ptr<NetSignal>(node->networkSignals.at(i)));
			}
			this->movements[node->networkSignals.at(i)] = false;
		}
	}
	timeStamp = -10.0;
	this->otherDirectionSignals = Vector<std::shared_ptr<NetSignal>>();
	this->lockedOnSignal = nullptr;
}

void NetSignalGroupController::addNode(std::shared_ptr<NetNode> node) {
	if (node->networkSignals.size() > 0) {
		if (! this->atNodes.exist(node)) {
			this->atNodes.push_back(node);
			for (int i = 0; i < node->networkSignals.size(); i++) {
				if (! this->networkSignalsGroup.exist(std::shared_ptr<NetSignal> (node->networkSignals.at(i)))) {
					this->networkSignalsGroup.push_back(std::shared_ptr<NetSignal>(node->networkSignals.at(i)));
				}
			}
		}
	}
}

void NetSignalGroupController::sendPassRequestToControlTo(std::shared_ptr<NetSignal> networkSignal, double& simulationTime) {
	if (this->lockedOnSignal == networkSignal) {
		this->timeStamp = simulationTime;
	}
	else {
		if (simulationTime - this->timeStamp > 5) {
			this->timeStamp = simulationTime;
			this->clear();
            this->movements[networkSignal] = true;
        }
    }
}

void NetSignalGroupController::setSignalsInSameDirection(Vector<std::shared_ptr<NetSignal>> sameDirectionSignals) {
    // if none is provided, make all signals in the group as the other direction except the LockSignal
    if (sameDirectionSignals.empty()) {
        // all signals are should be in the opposite direction except the LockSignal
        this->otherDirectionSignals = this->networkSignalsGroup;
        // remove the lock signal from the other direction signals
        if (this->lockedOnSignal != nullptr) {
            if (this->otherDirectionSignals.exist(this->lockedOnSignal)) {
                this->otherDirectionSignals.removeValue(this->lockedOnSignal);
            }
        }
    }
    // if other direction signals is None, add all signals in the controller to the other direction
    // signals if they are not in the same direction
    if (this->otherDirectionSignals.empty()) {
        for (auto& netSignal : this->networkSignalsGroup) {
            if (! sameDirectionSignals.exist(netSignal)) {
                this->otherDirectionSignals.push_back(netSignal);
            }
        }

    }
}

void NetSignalGroupController::updateTimeStep(double& timeStep) {
    if (this->lockedOnSignal != nullptr && this->atNodes.size() > 1) {
		this->timeStamp = timeStep;
	}
}

std::pair<std::shared_ptr<NetSignal>, Vector<std::shared_ptr<NetSignal>>> NetSignalGroupController::getFeedback() {

	std::shared_ptr<NetSignal> signalOn = std::shared_ptr<NetSignal>();
	if (this->lockedOnSignal != nullptr) {
        signalOn = this->lockedOnSignal;
	}
    else {
        for (auto& netSignal : this->movements.get_keys()) {
            if (this->movements[netSignal] == true) {
                this->lockedOnSignal = netSignal;
                signalOn = this->lockedOnSignal;
                break;
            }
        }
    }
    if (signalOn != nullptr) {
        return std::make_pair(signalOn, this->otherDirectionSignals);
    }
    return std::make_pair(signalOn, Vector<std::shared_ptr<NetSignal>>());
}



void NetSignalGroupController::clear() {
	for (std::shared_ptr<NetSignal>& netSignal : this->movements.get_keys()) {
		this->movements[netSignal] = false;
	}
	this->lockedOnSignal = nullptr;
    this->otherDirectionSignals.clear();
}
