/**
 * @file	~\NeTrainSim\src\Simulator.cpp.
 *
 * Implements the simulator class
 */
#include "Simulator.h"
#include "network/NetSignalGroupController.h"
#include <filesystem>
#include <cstdio>
#include <thread>
#include <chrono>
#include <ctime>
#include <locale>
#include "util/Utils.h"
#include "util/Error.h"
#include <filesystem>
#include <cmath>
#include <memory>

// get the path to the home directory. 
// If the path is not empty, it is returned, otherwise a runtime exception is thrown with an error message.
std::filesystem::path getHomeDirectory() {
#ifdef _WIN32
	const char* home = std::getenv("USERPROFILE");
#elif linux or __APPLE__
	const char* home = std::getenv("HOME");
#endif // linux

	if (home)
	{
		const std::filesystem::path documents = std::filesystem::path(home) / "Documents";
		const std::filesystem::path folder = documents / "NeTrainSim";

		if (!std::filesystem::exists(folder))
		{
			try
			{
				std::filesystem::create_directory(folder);
				return folder;
			}
			catch (const std::filesystem::filesystem_error& ex)
			{
				throw std::runtime_error(std::string("Error: ") +
										 std::to_string(static_cast<int>(Error::cannotRetrieveHomeDir)) +
										 "\nHome directory cannot be retreived!" +
										 folder.string() + ex.what() + "\n");
			}
		}
		return folder;
	}
	throw std::runtime_error(std::string("Error: ") +
								 std::to_string(static_cast<int>(Error::cannotRetrieveHomeDir)) +
								 "\nHome directory cannot be retreived!");
}


bool Simulator::getExportIndividualizedTrainsSummary() const {
	return exportIndividualizedTrainsSummary;
}

void Simulator::setExportIndividualizedTrainsSummary(bool newExportIndividualizedTrainsSummary) {
	exportIndividualizedTrainsSummary = newExportIndividualizedTrainsSummary;
}

Simulator::Simulator(Network& theNetwork, Vector<std::shared_ptr<Train>> networkTrains, double simulatorTimeStep) {
	// variables initialization
	this->network = &theNetwork;
	this->trains = networkTrains;
	// define train path as per simulator
	this->setTrainSimulatorPath();
	this->setTrainsPathNodes();
	this->setTrainPathLength();
	this->simulationEndTime = DefaultEndTime;
    this->timeStep = simulatorTimeStep;
	this->simulationTime = 0.0;
	this->progress = 0.0;
	if (this->simulationEndTime == 0.0) {
		this->runSimulationEndlessly = true;
	}
	else {
		this->runSimulationEndlessly = false;
	}

	this->outputLocation = getHomeDirectory();

	// Get a high-resolution time point
	auto now = std::chrono::high_resolution_clock::now();

	// Convert the time point to a numerical value
	auto serial_number = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

	this->trajectoryFilename = DefaultInstantaneousTrajectoryFilename + std::to_string(serial_number) + ".csv";
	this->summaryFileName = DefaultSummaryFilename + std::to_string(serial_number) + ".txt";

	this->exportTrajectory = false;

	// Define signals groups
	auto max_train = std::max_element(this->trains.begin(), this->trains.end(),
										  [](const std::shared_ptr<Train> t1, const std::shared_ptr<Train> t2) {
											  return t1->totalLength < t2->totalLength;
										  });

	defineSignalsGroups((*max_train)->totalLength);
}

void Simulator::setTimeStep(double newTimeStep) {
	this->timeStep = newTimeStep;
}


void Simulator::setEndTime(double newEndTime) {
	this->simulationEndTime = newEndTime;
}


void Simulator::setOutputFolderLocation(string newOutputFolderLocation) {
	this->outputLocation = newOutputFolderLocation;
}

void Simulator::setSummaryFilename(string newfilename) {
	if (newfilename != ""){
		if (std::filesystem::path(newfilename).has_extension()){
			this->summaryFileName = newfilename;
		}
		else{
			this->summaryFileName = newfilename + ".csv";
		}
	}
	else {
		this->summaryFileName = DefaultSummaryFilename;
	}
}

void Simulator::setExportInstantaneousTrajectory(bool exportInstaTraject, string newInstaTrajectFilename) {
	this->exportTrajectory = exportInstaTraject;
	if (newInstaTrajectFilename != ""){
		if (std::filesystem::path(newInstaTrajectFilename).has_extension()){
			this->trajectoryFilename = newInstaTrajectFilename;
		}
		else{
			this->trajectoryFilename = newInstaTrajectFilename + ".csv";
		}
	}
	else {
		// Get a high-resolution time point
		auto now = std::chrono::high_resolution_clock::now();

		// Convert the time point to a numerical value
		auto serial_number = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

		this->trajectoryFilename = DefaultInstantaneousTrajectoryFilename + std::to_string(serial_number) + ".csv";
	}
}



void Simulator::openTrajectoryFile() {
	try {
		this->trajectoryFile.open(this->outputLocation / this->trajectoryFilename, std::ios::out | std::ios::trunc);
		if (!this->trajectoryFile.is_open()) {
			throw std::ios_base::failure(std::string("Error: ") +
										 std::to_string(static_cast<int>(Error::cannotOpenTrajectoryFile)) +
										 "\nError opening file: " + this->trajectoryFilename + "!\n");
		}
	}
	catch (const std::ios_base::failure& e) {
		std::cerr << e.what() << std::endl;
		throw std::ios_base::failure(std::string("Error: ") +
									 std::to_string(static_cast<int>(Error::cannotOpenTrajectoryFile)) +
									 "\nCould not create/open the trajectory file!\n");
	}
}

std::string Simulator::getOutputFolder() {
	return this->outputLocation.string();
}

void Simulator::openSummaryFile() {
	try {
		this->summaryFile.open(this->outputLocation / this->summaryFileName, std::ios::out | std::ios::trunc);
		if (!this->summaryFile.is_open()) {
			throw std::ios_base::failure(std::string("Error: ") +
										 std::to_string(static_cast<int>(Error::cannotOpenSummaryFile)) +
										 "\nError opening file: " + this->summaryFileName + "!\n");
		}
	}
	catch (const std::ios_base::failure& e) {
		std::cerr << e.what() << std::endl;
		throw std::ios_base::failure(std::string("Error: ") +
									 std::to_string(static_cast<int>(Error::cannotOpenSummaryFile)) +
									 "\nCould not create/open the trajectory file!\n");
	}
}

bool Simulator::checkAllTrainsReachedDestination() {
	for (std::shared_ptr<Train>& t : (this->trains)) {
		if (t->outOfEnergy) {
			continue;
		}
		if (! t->reachedDestination) {
			return false;
		}
	}
	return true;
}

void Simulator::loadTrain(std::shared_ptr <Train> train) {
	train->loaded = true;
	train->currentCoordinates = train->trainPathNodes.at(0)->coordinates();
	train->setTrainsCurrentLinks(Vector<std::shared_ptr<NetLink>>(1, this->network->getFirstTrainLink(train)));
	train->linksCumLengths = this->network->generateCumLinksLengths(train);
	train->previousNodeID = train->trainPath.at(0);
	train->LastTrainPointpreviousNodeID = train->trainPath.at(0);
}

std::tuple<Vector<double>, Vector<double>, Vector<double>, 
			Vector<std::shared_ptr<NetLink>>> Simulator::loadTrainLinksData(std::shared_ptr<Train> train, bool isVirtual) {

	Vector<double> curvatures;
	Vector<double> grades;
	Vector<double> freeFlowSpeeds;
	Vector<std::shared_ptr<NetLink>> links;

	double distance = 0;
	// loop over all train's vehicles
	for (const auto& vehicle : train->trainVehicles) {
		// get centroid distance from start of the train
		if (isVirtual){
			distance = train->virtualTravelledDistance - train->WeightCentroids.at(vehicle);
		}
		else{
			distance = train->travelledDistance - train->WeightCentroids.at(vehicle);
		}

		// set the boundries
		if (distance < 0.0) { distance = 0.0; }
		if (distance > train->trainTotalPathLength) { distance = train->trainTotalPathLength; }
		// get the vehicle-spanning link
		std::shared_ptr<NetLink>  link = this->network->getLinkFromDistance(train, distance, train->previousNodeID);
		// push link geometric data to their corresponding vectors
		curvatures.push_back(link->curvature);
		if (train->LinkGradeDirection.count(link->id) == 0) {
			int indx;
			if (train->trainPath.index(link->fromLoc->id) < train->trainPath.index(link->toLoc->id)) {
				indx = link->fromLoc->id;
			}
			else {
				indx = link->toLoc->id;
			}
			train->LinkGradeDirection[link->id] = link->grade.at(indx);
		}
		grades.push_back(train->LinkGradeDirection[link->id]);
		freeFlowSpeeds.push_back(link->freeFlowSpeed);
		links.push_back(link);
	}
	std::tuple<Vector<double>, Vector<double>, Vector<double>, Vector<std::shared_ptr<NetLink>>> myreturn;
	myreturn = std::make_tuple(curvatures, grades, freeFlowSpeeds, links);
	return myreturn;
}

double Simulator::loadTrainFreeSpeed(std::shared_ptr<Train> train) {
	std::shared_ptr<NetLink> link = this->network->getLinkFromDistance(train, train->travelledDistance, train->previousNodeID);
	return link->freeFlowSpeed;
}

int Simulator::getSignalFromLink(Vector<std::shared_ptr<NetSignal>> networkSignals, std::shared_ptr<NetLink> link) {
	for (int i = 0; i < networkSignals.size(); i++) {
		std::shared_ptr<NetSignal> networkSignal = networkSignals.at(i);
		if (auto &sharedSignal = networkSignal) {
			if (sharedSignal->link.lock() == link) { return i; }
		}
	}
	return -1;
}

pair<int, bool> Simulator::getNextStoppingNodeID(std::shared_ptr<Train> train, int &previousNodeID) {
	int previousNodeIndex = train->trainPath.index(previousNodeID);
	for (int i = previousNodeIndex + 1; i < train->trainPath.size(); i++) {
		if (i >= train->trainPath.size()) { return std::make_pair(train->trainPath.back(), false); }
		if (train->trainPathNodes[i]->isDepot && i > 0) {
			return std::make_pair(train->trainPath[i], false);
		}
		else if (!train->trainPathNodes[i]->networkSignals.empty()) {
			int prevI = i - 1;
			for (auto &s: train->trainPathNodes[i]->networkSignals) {
				if (s->currentNode.lock()->id == train->trainPathNodes[i]->id &&
						s->previousNode.lock()->id == train->trainPathNodes[prevI]->id) {
					if (! s->isGreen) {
						return std::make_pair(train->trainPath[i], true);
					}
					else {
						break;
					}
				}

			}
			// this->getNextStoppingNodeID(train, train->trainPath[i]);
		}
	}
	return std::make_pair(train->trainPath.back(), false);
}

/**
 * @brief Simulator::get all lower speeds node IDs on the path of the train
 *
 * @param train
 * @param previousNodeID        is the previous node the front tip of the train passed
 * @param nextStoppingNodeID    is the next stop node ID the train should stop completely at
 * @return Map<int, double>     a map of all node IDs which have lower speeds compared to
 *                              the current link speed of the train. Each node ID will have
 *                              a double value of its corresponding lower speed
 */
Map<int, double> Simulator::getAllLowerSpeedsIDs(std::shared_ptr<Train> train, int& previousNodeID, int& nextStoppingNodeID) {
	int prevI = train->trainPath.index(previousNodeID);
	int nextSI = train->trainPath.index(nextStoppingNodeID);

	// check if the values have been already memorized
	// if not, get them
	if (train->LowerSpeedNodeIDs[prevI][nextSI].empty()) {
		Map<int, double> lowerSpeedMapping;
		for (int i = prevI + 1; i < train->trainPath.size(); i++) {
			if (nextStoppingNodeID == train->trainPath[i]) { break; }
			else {
				int prevIndx = i - 1;
				std::shared_ptr<NetLink> l1 = this->network->getLinkByStartNodeID(train, train->trainPath[prevIndx]);
				std::shared_ptr<NetLink> l2 = this->network->getLinkByStartNodeID(train, train->trainPath[i]);
				if (l2 == nullptr) { continue; }
				if (l2->freeFlowSpeed < l1->freeFlowSpeed) { lowerSpeedMapping[train->trainPath[i]] = l2->freeFlowSpeed; }
			}
		}
		train->LowerSpeedNodeIDs[prevI][nextSI] = lowerSpeedMapping;
		return lowerSpeedMapping;
	}
	// else, return the memorized value
	else {
		return train->LowerSpeedNodeIDs[prevI][nextSI];
	}
}

std::pair<std::shared_ptr<Train>, double> Simulator::getAheadTrainAndGap(std::shared_ptr <Train> train) {
	std::pair<std::shared_ptr<Train>, double> toTrainsDistance = { nullptr, 0.0 };
	for (std::shared_ptr<Train>& otherTrain : this->trains) {
		// check if the train is loaded and not reached destination
		if (otherTrain.get() == train.get() || !otherTrain->loaded || otherTrain->reachedDestination) { continue; }
		double d1 = this->network->getDistanceByTwoCoordinates(train->currentCoordinates, otherTrain->startEndPoints[0]);
		double d2 = this->network->getDistanceByTwoCoordinates(train->currentCoordinates, otherTrain->startEndPoints[1]);
		double d = (d1 < d2) ? d1 : d2;

		if (toTrainsDistance.first == nullptr) {
			if (toTrainsDistance.second > d) {
				toTrainsDistance = { train, d };
			}
		}
		else {
			
			toTrainsDistance = { train, d };
		}
	}
	return toTrainsDistance;
}

void Simulator::setOccupiedLinksByTrains(std::shared_ptr <Train> train) {
	for (std::shared_ptr<NetLink>& l : train->previousLinks) {
		if (l->currentTrains.exist(train)) {
			l->currentTrains.removeValue(train);
		}
		//else { break; }
	}
	for (std::shared_ptr <NetLink>& l : train->currentLinks) {
		if (! l->currentTrains.exist(train)) {
			l->currentTrains.push_back(train);
		}
	}

}


void Simulator::playTrainOneTimeStep(std::shared_ptr <Train> train)
{
	bool skipTrainMove = false;

	// Check if the train start time is passed
	// if such, load train first and then run the simulation
	if (this->simulationTime >= train->trainStartTime && !train->loaded) {
		bool skipLoadingTrain = false;

		// check if there is a train that is already loaded in the same node and still on the same starting node
		for (std::shared_ptr <Train>& otherTrain : this->trains) {
			// check if the train is loaded and not reached destination
			if (otherTrain == train || !otherTrain->loaded || otherTrain->reachedDestination) { continue; }
			// check if the train has the same starting node and the otherTrain still on the same starting node
			if (otherTrain->trainPath.at(0) == train->trainPath.at(0) && 
				otherTrain->travelledDistance <= otherTrain->totalLength) {
				skipLoadingTrain = true;
				break;
			}
		}
		// skip loading the train if there is any train at the same point
		if (!skipLoadingTrain) {
			// load train
			this->loadTrain(train);
		}
	}

	if ((train->trainStartTime <= this->simulationTime) && train->loaded) {

		// holds track data and speed
		std::tuple<Vector<double>, Vector<double>, Vector<double>, Vector<std::shared_ptr<NetLink>>> linksdata;
		// Load path geometric data for each vehicle in the train (at mass centroid of each)
		linksdata = this->loadTrainLinksData(train, false);
		// train spanned links curvatures
		Vector<double> curvatures = std::get<0>(linksdata);
		// train spanned links grades
		Vector<double> grades = std::get<1>(linksdata);
		// train spanned links free flow speed
		Vector<double> freeFlowSpeed = std::get<2>(linksdata);
		// train spanned links
		Vector<std::shared_ptr<NetLink>> links = std::get<3>(linksdata);

		// the free flow speed of the tip of the train
		double currentLinkFreeSpeed = this->loadTrainFreeSpeed(train);
		// the spanned links of the train
		train->setTrainsCurrentLinks(links);
		// all previous links the train passed on
		for (const std::shared_ptr<NetLink> &link : train->currentLinks) {
			if (!train->previousLinks.exist(link)) {
				train->previousLinks.push_back(link);
			}
		}
		// the max speed the train cannot go higher than
		double currentFreeFlowSpeed = std::min(currentLinkFreeSpeed, freeFlowSpeed.min());

		// set the train memorization parameters to speed up the calculations later
		train->previousNodeID = this->network->getPreviousNodeByDistance(train, train->travelledDistance, train->previousNodeID)->id;
		double lastTrainTipTravelledDistance = train->travelledDistance - train->totalLength;
		if (lastTrainTipTravelledDistance < 0.0) { lastTrainTipTravelledDistance = 0.0; }
		int LastTrainTipPreviousNodeID;
		LastTrainTipPreviousNodeID = (train->LastTrainPointpreviousNodeID <= 0.0) ? train->trainPath[0] : train->LastTrainPointpreviousNodeID;
		train->LastTrainPointpreviousNodeID = this->network->getPreviousNodeByDistance(train, 
			lastTrainTipTravelledDistance, LastTrainTipPreviousNodeID)->id;
// ##################################################################
// #                      start: critical points                    #
// ##################################################################
		pair<int, bool> nextStop = this->getNextStoppingNodeID(train, train->previousNodeID);
		int nextStoppingNodeID = nextStop.first;
		bool isSignal = nextStop.second;

		// the map defines all lower nodes/points in its path.
		// the map has the train point ids as keys and its speed as its values
		Map<int, double> lowerSpeedsNs = this->getAllLowerSpeedsIDs(train, train->previousNodeID, nextStoppingNodeID);

		// this tuple defines the critical points in the train path. the critical points include 
		// 1. lower speed links (critical point is the start of the link),
		// 2. leading trains (critical point is the end of the train),
		// 3. stopping station or depot.
		// The tuple takes 3 vectors: 
		// 1. vector 0 is for distances to critical point, 
		// 2. vector 1 is a bool indicating the critical point is a train,
		// 3. vector 2 is for speed of the critical point.
		std::tuple<Vector<double>, Vector<bool>, Vector<double>> criticalPointsDefinition;

		// add all lower speed points to their corresponding lists
		for (pair<int, double> lwrSpeedNS: lowerSpeedsNs) {
			std::get<0>(criticalPointsDefinition).push_back(this->network->getDistanceToSpecificNodeByTravelledDistance(
			train, train->travelledDistance, lwrSpeedNS.first));
			std::get<1>(criticalPointsDefinition).push_back(false);
			std::get<2>(criticalPointsDefinition).push_back(lwrSpeedNS.second);
		}
		// add the leading train to the list
		std::pair<std::shared_ptr<Train>, double> trainAheadWithDistance = this->getAheadTrainAndGap(train);
		if (trainAheadWithDistance.first != nullptr) {
			std::get<0>(criticalPointsDefinition).push_back(trainAheadWithDistance.second);
			std::get<1>(criticalPointsDefinition).push_back(true);
			std::get<2>(criticalPointsDefinition).push_back(trainAheadWithDistance.first->currentSpeed);
		}
		// add the stopping station to the list
		double distanceToStop = this->network->getDistanceToSpecificNodeByTravelledDistance(train, 
			train->travelledDistance, nextStoppingNodeID);
		std::get<0>(criticalPointsDefinition).push_back(distanceToStop);
		std::get<1>(criticalPointsDefinition).push_back(false);
		std::get<2>(criticalPointsDefinition).push_back(0.0);
// ##################################################################
// #                      end: critical points                    #
// ##################################################################

		// check if the next stop is a network signal, if yes and distance is very small, stop the train
		if (isSignal) {
			if ((train->currentAcceleration < 0 &&
				std::get<0>(criticalPointsDefinition).back() <= train->currentSpeed * this->timeStep) ||
				(train->currentSpeed == 0.0 && std::get<0>(criticalPointsDefinition).back() <= 1.0)) {
				train->immediateStop(this->timeStep);
				skipTrainMove = true;
			}
		}
		else {
			if ((std::get<0>(criticalPointsDefinition).size() == 1) && (train->currentAcceleration < 0.0) &&
				((std::round(train->previousSpeed * 1000.0) / 1000.0) == 0.0) &&
				((std::round(train->currentSpeed * 1000.0) / 1000.0) == 0.0)) {
				train->kickForwardADistance(std::get<0>(criticalPointsDefinition).back());
			}
		}

		// set memorization parameters for the train
		train->nextNodeID = train->trainPath.at(train->trainPath.index(train->previousNodeID) + 1);

		if (!skipTrainMove) {
            train->updateGradesCurvatures(grades, curvatures);
            train->resetPowerRestriction();

            // check if a notch reduction is required
            // calculate the accelerations and speed
            auto as = train->getStepDynamics(this->timeStep, currentFreeFlowSpeed, std::get<0>(criticalPointsDefinition),
                                             std::get<1>(criticalPointsDefinition), std::get<2>(criticalPointsDefinition));
            // calculate the total resistance
            double res = train->getTotalResistance(as.second);
            // calculate approximate power required
            pair<Vector<double>, double> out = train->getTractivePower(as.second, as.first, res);
            // calculate approximate energy required
            double stepEC = train->getTotalEnergyConsumption(this->timeStep, out.first);
            // check if the train can provide this required energy
            if (! train->canProvideEnergy(stepEC, this->timeStep)) {
                train->reducePower(); // reduce notch by 1 notch
            }


			train->moveTrain(this->timeStep, currentFreeFlowSpeed, std::get<0>(criticalPointsDefinition),
				std::get<1>(criticalPointsDefinition), std::get<2>(criticalPointsDefinition));
		}
		if ((std::round(train->trainTotalPathLength * 1000.0) / 1000.0) <= (std::round(train->travelledDistance * 1000.0) / 1000.0)) {
			train->travelledDistance = train->trainTotalPathLength;
			train->reachedDestination = true;
			train->calcTrainStats(freeFlowSpeed, currentFreeFlowSpeed, this->timeStep, train->currentFirstLink->region);
		}
		else {
			train->currentCoordinates = this->network->getPositionbyTravelledDistance(train, train->travelledDistance);
			train->startEndPoints = this->getStartEndPoints(train, train->currentCoordinates);
			train->calcTrainStats(freeFlowSpeed, currentFreeFlowSpeed, this->timeStep, train->currentFirstLink->region);

			// holds track data and speed
			std::tuple<Vector<double>, Vector<double>, Vector<double>, Vector<std::shared_ptr<NetLink>>> linksdata;
			// Load path geometric data for each vehicle in the train (at mass centroid of each)
			linksdata = this->loadTrainLinksData(train, false);
			links = std::get<3>(linksdata);
			// the spanned links of the train
			train->setTrainsCurrentLinks(links);
			// the first link the train is on
			train->currentFirstLink = links.at(0);
			// all previous links the train passed on
			for (const std::shared_ptr<NetLink> &link : train->currentLinks) {
				if (!train->previousLinks.exist(link)) {
					train->previousLinks.push_back(link);
				}
			}
			this->setOccupiedLinksByTrains(train);
		}
		if (this->exportTrajectory) {
			std::stringstream exportLine;
			exportLine << train->trainUserID << ","
				<< this->simulationTime << ","
				<< train->travelledDistance << ","
				<< train->currentAcceleration << ","
				<< train->currentSpeed << ","
				<< currentFreeFlowSpeed << ","
				<< train->energyStat << ","
				<< train->maxDelayTimeStat << ","
				<< train->delayTimeStat << ","
				<< train->stoppedStat << ","
				<< train->currentTractiveForce << ","
				<< train->currentResistanceForces << ","
				<< train->currentUsedTractivePower << ","
				<< grades[0] << ","
				<< curvatures[0] << ","
                << train->locomotives[0]->currentLocNotch
				<< std::endl;

			this->trajectoryFile << exportLine.str();

		}
	}

    if (this->checkNoTrainIsOnNetwork()) {
        double shiftTime = this->getNotLoadedTrainsMinStartTime();
        if (shiftTime > this->simulationTime) {
            this->simulationTime = shiftTime;
        }
    }
}

bool Simulator::checkNoTrainIsOnNetwork() {
    for (std::shared_ptr<Train>& t : (this->trains)) {
        if (t->loaded && ! t->reachedDestination) {
            return false;
        }
    }
    return true;
}

double Simulator::getNotLoadedTrainsMinStartTime() {
    Vector<double> st;
    for (std::shared_ptr<Train>& t : (this->trains)) {
        if (!t->loaded) {
            st.push_back(t->trainStartTime);
        }
    }
    if (st.empty()) { return -1.0; }
    return st.min();
}

void Simulator::playTrainVirtualStepsAStarOptimization(std::shared_ptr<Train> train, double timeStep){

	if (train->trainStartTime <= this->simulationTime){

		train->virtualTravelledDistance = train->travelledDistance;
		double speed = train->currentSpeed;
		double prevSpeed = train->previousSpeed;
		double accel = train->currentAcceleration;
		double throttleLevel = -1;
		Vector<double> throttleLevelVec = Vector<double>();

		for (int i = 0; i < train->lookAheadStepCounter; i++){
			train->virtualTravelledDistance += speed *timeStep;
			auto linksData = this->loadTrainLinksData(train, true);
			auto CurrentFreeSpeed_ms = std::get<2>(linksData).min();

			unsigned int pindx = 0;
			for (int i = 0; i < train->linksCumLengths.size() ; i++) {
				if (train->linksCumLengths[i] >= train->virtualTravelledDistance) {
					pindx = i-1;
					break;
				}
			}

			int pNodeID = train->trainPath[pindx];
			auto nextStop = this->getNextStoppingNodeID(train, pNodeID);
			Vector<double> oDistanceToNextStationTrain;
			Vector<double> oAheadSpeed;

			// get all lower speed IDs
			for ( auto N : this->getAllLowerSpeedsIDs(train, pNodeID, nextStop.first)){
				int theNodeID = N.first;
				oDistanceToNextStationTrain.push_back(
							this->network->getDistanceToSpecificNodeByTravelledDistance(train,
																						train->virtualTravelledDistance,
																						theNodeID));
				oAheadSpeed.push_back(N.second);
			}

			oDistanceToNextStationTrain.push_back(this->network->getDistanceToSpecificNodeByTravelledDistance(train,
																						train->virtualTravelledDistance,
																						nextStop.first));
			oAheadSpeed.push_back(0.0);

			auto out = train->AStarOptimization( prevSpeed, speed, accel, throttleLevel,
									  std::get<1>(linksData), std::get<0>(linksData),
									  CurrentFreeSpeed_ms, timeStep, oAheadSpeed, oDistanceToNextStationTrain);
			speed = std::get<0>(out);
			accel = std::get<1>(out);
			throttleLevel = std::get<2>(out);
			throttleLevelVec.push_back(throttleLevel);
		}

		train->pickOptimalThrottleLevelAStar(throttleLevelVec, train->lookAheadCounterToUpdate);
	}
}


Vector<Vector<Vector < std::shared_ptr<NetNode>>>> Simulator::getConflictTrainsIntersections() {
	Vector<Vector<Vector < std::shared_ptr<NetNode>>>> trainSignalsGroups(this->trains.size(), Vector<Vector < std::shared_ptr<NetNode>>>(this->trains.size()));
	if (this->trains.size() < 2) {return trainSignalsGroups;}
	vector<pair<std::shared_ptr<Train>, std::shared_ptr<Train>>> ts;
	for (int i = 0; i < this->trains.size(); ++i) {
		// if the train has the same nodes as the previous train, skip it
		if (i > 0){
			int prevI = i - 1;
			if (this->trains.at(i)->trainPath.isSubsetOf(this->trains.at(prevI)->trainPath) ||
					this->trains.at(prevI)->trainPath.isSubsetOf(this->trains.at(i)->trainPath)) {
				continue;
			}
		}

		for (int j = i + 1; j < this->trains.size(); ++j) {
			if (i == j) { continue; }
			ts.emplace_back(this->trains.at(i), this->trains.at(j));
		}
	}
	if (ts.empty()) {
		return trainSignalsGroups;
	}
	else {
		for (auto& t : ts) {
			Vector<std::shared_ptr<NetNode>> intersections = this->getTrainsIntersections(t.first, t.second);
			trainSignalsGroups[t.first->id][t.second->id] = intersections;
		}
		return trainSignalsGroups;
	}
}
Vector<std::shared_ptr<NetNode>> Simulator::getTrainsIntersections(std::shared_ptr<Train> train1, std::shared_ptr<Train> train2) {
	Vector<std::shared_ptr<NetNode>> intersections;
	for (std::shared_ptr<NetNode>& value : train1->trainPathNodes) {
		if (find(train2->trainPathNodes.begin(), train2->trainPathNodes.end(), value) != train2->trainPathNodes.end()) {
			// check if the node has signals, if yes, add it to control
			if (value->networkSignals.size() > 0){
				intersections.push_back(value);
			}
		}
	}
	return intersections;
}

Vector<std::set<std::shared_ptr<NetNode>>> Simulator::getNodesIntersectionsForSignals(double& minSafeDistance) {
	Vector<Vector<Vector < std::shared_ptr<NetNode>>>> ConflictTrainsIntersections = this->getConflictTrainsIntersections();
	Vector<Vector< Map<int, std::set<std::shared_ptr<NetNode>>>>> GS =
		this->getNodeGroupsForSignals(ConflictTrainsIntersections, minSafeDistance);
	auto r = this->unionNodesIntersectionsForSignals(GS);
	return r;
}

void Simulator::defineSignalsGroups(double& minSafeDistance) {
	this->calculateSignalsProximities();
	Vector<std::set<std::shared_ptr<NetNode>>> nodesGroup = this->getNodesIntersectionsForSignals(minSafeDistance);
	for (int i = 0; i < nodesGroup.size(); i++) {
		std::set<std::shared_ptr<NetNode>> group = nodesGroup.at(i);
		std::shared_ptr<NetSignalGroupController> s = 
			std::make_shared< NetSignalGroupController>(NetSignalGroupController(group));
		s->confinedLinks = this->getLinksByNodes(nodesGroup.at(i));

		for (std::shared_ptr<NetNode> n : group) {
			this->signalsGroups[n] = s;
		}
	}
	
}

void Simulator::calculateSignalsProximities() {
	for (auto &s : this->network->networkSignals) {
		Vector<double> proximity;
		for (auto &t : this->trains) {
			if (t->trainPathNodes.exist(std::shared_ptr<NetNode>(s->currentNode)) && 
				t->trainPathNodes.exist(std::shared_ptr<NetNode>(s->previousNode))) {
				proximity.push_back(t->getSafeGap(t->getMinFollowingTrainGap(), t->currentSpeed, s->link.lock()->freeFlowSpeed,
					t->T_s, true));
			}
		}
		if (proximity.size() > 0) {
			s->proximityToActivate = std::max(s->proximityToActivate, proximity.min());
		}
	}
}

Vector<Vector< Map<int, std::set<std::shared_ptr<NetNode>>>>> Simulator::getNodeGroupsForSignals(
		Vector<Vector<Vector < std::shared_ptr<NetNode>>>> ConflictTrainsIntersections, double& minSafeDistance) {

	Vector<Vector< Map<int, std::set<std::shared_ptr<NetNode>>>>> nodesGroups(this->trains.size(),
		Vector<Map<int, std::set<std::shared_ptr<NetNode>>>>(this->trains.size()));

	if (this->trains.size() < 2) { return nodesGroups; }
	for (int t1i = 0; t1i < ConflictTrainsIntersections.size(); t1i++) {
		for (int t2i = 0; t2i < ConflictTrainsIntersections.at(t1i).size(); t2i++) {
			// if it is the same train, skip
			if (t1i == t2i) { continue; }
			// get the conflicts of train t1i and t2i
			Vector<std::shared_ptr<NetNode>> trainIntersections = ConflictTrainsIntersections[t1i][t2i];
			// if the intersections is empty, skip
			if (trainIntersections.size() < 1) { continue; }
			// map of group number and nodes of intersections
			Map<int, std::set<std::shared_ptr<NetNode>>> G;
			int gn = 0;
			// loop over all intersecting nodes
			for (int i = 0; i < trainIntersections.size() - 1; i++) {
				int nextI = i + 1;
				double d = this->network->getDistanceBetweenTwoNodes(this->trains.at(t1i), 
					trainIntersections.at(i), trainIntersections.at(nextI));

				// if the distance is short or the section has only 1 link, it is a conflict
				// and shoud be controlled by one controller
				if ((d < minSafeDistance) || (this->network->isConflictZone(this->trains.at(t1i), trainIntersections.at(i),
																			trainIntersections.at(nextI)))) {
					G[gn].insert(trainIntersections.at(i)); 
					G[gn].insert(trainIntersections.at(nextI)); 
				}
				else {
					G[gn].insert(trainIntersections.at(i));
					gn++;
					G[gn].insert(trainIntersections.at(nextI));
				}
			}
			nodesGroups[t1i][t2i] = G;
		}
	}
	return nodesGroups;

}

Vector<std::set<std::shared_ptr<NetNode>>> Simulator::unionNodesIntersectionsForSignals(
	Vector<Vector< Map<int, std::set<std::shared_ptr<NetNode>>>>> nodesGroups) {

	Vector<std::set<std::shared_ptr<NetNode>>> unified;
	for (int t1i = 0; t1i < nodesGroups.size(); t1i++) {
		for (int t2i = 0; t2i < nodesGroups.at(t1i).size(); t2i++) {
			Vector<std::set<std::shared_ptr<NetNode>>> g = nodesGroups.at(t1i).at(t2i).get_values();
			if (g.size() < 1) { continue; }

			bool isUnified = false;
			for (int i = 0; i < g.size(); i++) {
				for (int j = 0; j < unified.size(); j++){
					isUnified = false;
					std::set<std::shared_ptr<NetNode>> intersections;
					std::set_intersection(g.at(i).begin(), g.at(i).end(), 
											unified.at(j).begin(), unified.at(j).end(),
											std::inserter(intersections, intersections.begin()));
					if (!intersections.empty()) {
						std::set<std::shared_ptr<NetNode>> union_set;
						std::set_union(g.at(i).begin(), g.at(i).end(),
										unified.at(j).begin(), unified.at(j).end(),
										std::inserter(union_set, union_set.begin()));
						unified.at(j) = union_set;
						isUnified = true;
					}
				}
				if (!isUnified) {
					unified.push_back(g.at(i));
				}
			}
		}
	}
	return removeRedundancy(unified);

}

Vector<std::set<std::shared_ptr<NetNode>>> Simulator::removeRedundancy(Vector<std::set<std::shared_ptr<NetNode>>> unified) {
	bool needsAnotherRun = true;
	while (needsAnotherRun) {
		needsAnotherRun = false;
		for (int i = 0; i < unified.size(); i++) {
			if (unified.at(i).empty()) { continue; }
			for (int j = 0; j < unified.size(); j++) {
				if (i == j) { continue; }
				if (unified.at(j).empty()) { continue; }

				std::set<std::shared_ptr<NetNode>> intersections;
				std::set_intersection(unified.at(i).begin(), unified.at(i).end(),
					unified.at(j).begin(), unified.at(j).end(),
					std::inserter(intersections, intersections.begin()));
				if (!intersections.empty()) {
					std::set<std::shared_ptr<NetNode>> union_set;
					std::set_union(unified.at(i).begin(), unified.at(i).end(),
						unified.at(j).begin(), unified.at(j).end(),
						std::inserter(union_set, union_set.begin()));
					unified.at(i) = union_set;
					unified.at(j).clear();
					needsAnotherRun = true;
				}
			}
		}
	}
	unified.erase(std::remove_if(unified.begin(), unified.end(), [](const std::set<std::shared_ptr<NetNode>>& set) { return set.empty(); }), unified.end());
	return unified;

}

/**
 * Gets links by nodes
 * @author	Ahmed
 * @date	2/14/2023
 * @param 	nodes	The nodes.
 * @returns	The links by nodes.
 */
Vector<std::shared_ptr<NetLink>> Simulator::getLinksByNodes(std::set<std::shared_ptr<NetNode>> nodes) {
	Vector<std::shared_ptr<NetLink>> links;
	if (nodes.size() > 1) {
		// permutation of all links, this is in case nodes do not line up
		std::set<std::shared_ptr<NetNode>>::iterator it = nodes.begin();
		while (it != nodes.end()) {
			std::set<std::shared_ptr<NetNode>>::iterator jt = it;
			++jt;
			while (jt != nodes.end()) {
				std::vector<std::shared_ptr<NetNode>> permutation = { *it, *jt };
				// process permutation
				Vector<std::shared_ptr<NetNode>> path = this->network->shortestPathSearch(it->get()->id, jt->get()->id).first;
				if (!path.empty()) {
					for (int i = 0; i < path.size() - 1; i++) {
						int nextI = i + 1;
						Vector<std::shared_ptr<NetLink>> betweenLinks =
														this->network->getLinksByStartandEndNode(path.at(i),
																								 path.at(nextI));
						for (auto& link : betweenLinks) {
							links.push_back(link);
						}
					}
				}

				++jt;
			}
			++it;
		}
	}
	else {
		Vector<std::shared_ptr<NetNode>> endNodes = (*nodes.begin())->linkTo.get_keys();
		for (auto& n : endNodes) {
			Vector<std::shared_ptr<NetLink>> betweenLinks =
				this->network->getLinksByStartandEndNode((*nodes.begin()), n);
			for (auto& link : betweenLinks) {
				links.push_back(link);
			}
		}
	}
	return links;
}



Vector<std::pair<double, double>> Simulator::getStartEndPoints(std::shared_ptr<Train> train, std::pair<double, double> currentCoordinates) {
	Vector<pair<double, double>> startEndPoints;
	startEndPoints.push_back(currentCoordinates);
	double endDistance = train->travelledDistance - train->totalLength;
	startEndPoints.push_back(this->network->getPositionbyTravelledDistance(train, endDistance));
	return startEndPoints;
}

void Simulator::runSimulation() {
	// define trajectory file and set it up
	if (this->exportTrajectory) {
		this->openTrajectoryFile();
		std::stringstream exportLine;
        exportLine << "TrainNo,TStep_s,TravelledDistance_m,Acceleration_mps2,Speed_mps,LinkMaxSpeed_mps,"
                   << "EnergyConsumption_KWH,DelayTimeToEach_s,DelayTime_s,Stoppings,tractiveForce_N,"
                   << "ResistanceForces_N,CurrentUsedTractivePower_kw,GradeAtTip_Perc,CurvatureAtTip_Perc,"
                   << "FirstLocoNotchPosition\n";
		this->trajectoryFile << exportLine.str();
	}


	time_t init_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	while (this->simulationTime <= this->simulationEndTime || this->runSimulationEndlessly) {
		if (this->checkAllTrainsReachedDestination()) {
			break;
		}
		for (std::shared_ptr <Train>& t : (this->trains)) {
			if (t->reachedDestination) { continue;  }

			if (t->optimize){
				if (t->lookAheadCounterToUpdate <= 0) {
					t->resetTrainLookAhead();
                    this->playTrainVirtualStepsAStarOptimization(t, this->timeStep);
				}
			}

			this->playTrainOneTimeStep(t);
		}
// ##################################################################
// #             start: show progress on console                    #
// ##################################################################
		Vector<double> trainPathLengths;
		Vector<double> travelledDistances;
		for (std::shared_ptr <Train>& t : this->trains) {
			trainPathLengths.push_back(t->trainTotalPathLength);
			travelledDistances.push_back(t->travelledDistance);
		}
		this->runSignalsforTrains();
		double trainsAv = accumulate(travelledDistances.begin(), travelledDistances.end(), 0.0) / travelledDistances.size();
		double totalTrainAv = accumulate(trainPathLengths.begin(), trainPathLengths.end(), 0.0) / trainPathLengths.size();

		this->ProgressBar(trainsAv, totalTrainAv);

		this->checkTrainsCollision();

		this->simulationTime += this->timeStep;

	}
// ##################################################################
// #                       start: summary file                      #
// ##################################################################
	time_t fin_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	double difTime = difftime(fin_time, init_time);
	std::stringstream exportLine;
	std::tuple<double, double, double, double, double> networkStats = this->network->getNetworkStats();

	exportLine << "~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~\n"
        << MYAPP_TARGET << " SIMULATION SUMMARY\n"
        << "Version: " << MYAPP_VERSION << "\n"
        << "Simulation Time: " << Utils::formatDuration(difTime) << " (dd:hh:mm:ss)\n"
        << "~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~\n\n"
        << "+ NETWORK STATISTICS:\n"
        << "  |_ Network Name                                                               : " << this->network->networkName << "\n"
        << "  |_ Nodes Count                                                                : " << Utils::thousandSeparator(this->network->nodes.size()) << "\n"
        << "  |_ Links Count                                                                : " << Utils::thousandSeparator(this->network->links.size()) << "\n"
        << "    |_ Total Lengths of All Links (meters)                                      : " << Utils::thousandSeparator(std::get<3>(networkStats)) << "\n"
        << "    |_ Total Lengths of All Links with Catenary (meters)                        : " << Utils::thousandSeparator(std::get<4>(networkStats)) << "\n"
        << "  |_ Total Signals                                                              : " << Utils::thousandSeparator(this->network->networkSignals.size()) << "\n"
        << "  |_ Total Number of Trains on Network                                          : " << Utils::thousandSeparator(this->trains.size()) << "\n"
        << "  |_ Percentage of Links with Catenaries to All Links (%)                       : " << Utils::thousandSeparator(std::get<0>(networkStats)) << "\n"
        << "  |_ Catenary Total Energy Consumed (KW.h)                                      : " << Utils::thousandSeparator(std::get<1>(networkStats) - std::get<2>(networkStats)) << "\n"
        << "        |_ Average Catenary Energy Consumption per Net Weight (KW.h/ton)        : " << Utils::thousandSeparator( (std::get<1>(networkStats) - std::get<2>(networkStats)) /
                                                                                                                            std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                            [](double total, const auto& t) {
                                                                                                                                                                return total + (t->getCargoNetWeight());
                                                                                                                                                            })) << "\n"
        << "        |_ Average Catenary Energy Consumption per Net ton.km (KW.hx10^3/ton.km): " << Utils::thousandSeparator(((std::get<1>(networkStats) - std::get<2>(networkStats)) * (double) 1000.0) /
                                                                                                                            std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                            [](double total, const auto& t) {
                                                                                                                                                return total + t->getTrainTotalTorque();
                                                                                                                                            })) << "\n"
        << "    |_ Catenary Energy Consumed (KW.h)                                          : " << Utils::thousandSeparator(std::get<1>(networkStats)) << "\n"
        << "    |_ Catenary Energy Regenerated (KW.h)                                       : " << Utils::thousandSeparator(std::get<2>(networkStats)) << "\n"

        << "....................................................\n\n"
        << "\n";

	exportLine
        << "+ AGGREGATED/ACCUMULATED TRAINS STATISTICS:\n"
        << "  |-> Train Information:\n"
        << "    |-> Locomotives Summary:\n"
        << "        |_ Number of Locomotives/Cars                                           : " << std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                           [](int total, const auto& train) {
                                                                                                               return total + train->nlocs;
                                                                                                           }) << "/"
                                                                                                                << std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                [](int total, const auto& train) {
                                                                                                                    return total + train->nCars;
                                                                                                                }) << "\n"
        << "        |_ Locomotives (Technology, count)                                      : " << std::accumulate(
                                                                                                        this->trains.begin(), this->trains.end(), Map<TrainTypes::PowerType, int>(),
                                                                                                            [](Map<TrainTypes::PowerType, int> a, const auto& train) {
                                                                                                                      auto c = train->LocTypeCount();
                                                                                                                      for (const auto& [key, value] : c) {
                                                                                                                         a[key] += value;
                                                                                                                      }
                                                                                                                      a.insert(c.begin(), c.end());
                                                                                                                      return a;
                                                                                                                    }
                                                                                                                    ).toString() << "\n"
        << "        |_ Operating Locomotives to End of Trains Trip                          : " << std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                       [](int total, const auto& train) {
                                                                                                           return total + train->getActiveLocomotivesNumber();
                                                                                                       }) << "\n"
        << "    |-> Cars Summary:\n"
        << "        |_ Cars Count                                                           : " << std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                   [](int total, const auto& train) {
                                                                                                                       return total + train->cars.size();
                                                                                                                   }) << "\n"
        << "        |_ Cars (Types, count)                                                  : " << std::accumulate(
                                                                                                       this->trains.begin(), this->trains.end(), Map<TrainTypes::CarType, int>(),
                                                                                                           [](Map<TrainTypes::CarType, int> a, const auto& train) {
                                                                                                                     auto c = train->carTypeCount();
                                                                                                                     for (const auto& [key, value] : c) {
                                                                                                                        a[key] += value;
                                                                                                                     }
                                                                                                                     a.insert(c.begin(), c.end());
                                                                                                                     return a;
                                                                                                                   }
                                                                                                                   ).toString() << "\n"
        << "    |-> Moved Commodity:\n"
        << "        |_ Total Moved Cargo (ton)                                              : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                            [](double total, const auto& t) {
                                                                                                                                return total + (t->getCargoNetWeight());
                                                                                                                            })) << "\n"
        << "        |_ Total ton.km (ton.Km)                                                : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                            [](double total, const auto& t) {
                                                                                                                                return total + (t->getTrainTotalTorque());
                                                                                                                            })) << "\n\n"

        << "  |-> Route Information:\n"
        << "    |_ Trains Reached Destination                                               : " << std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                        [](int total, const auto& train) {
                                                                                                            return total + train->reachedDestination;
                                                                                                        }) << "\n"
        << "    |_ Trains Total Path Length (km)                                            : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                            [](double total, const auto& train) {
                                                                                                                                return total + (train->trainTotalPathLength / (double)1000.0);
                                                                                                                            })) << "\n\n"
        << "  |-> Train Performance:\n"
        << "    |_ Operating Time                                                           : " << Utils::formatDuration(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                         [](double total, const auto& train) {
                                                                                                                             return total + train->tripTime;
                                                                                                                         })) << "\n"
        << "    |_ Average Speed (meter/second)                                             : " << std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                   [](double total, const auto& train) {
                                                                                                                       return total + train->averageSpeed;
                                                                                                                   })/this->trains.size() << "\n"
        << "    |_ Average Acceleration (meter/square second)                               : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                   [](double total, const auto& train) {
                                                                                                                       return total + train->averageAcceleration;
                                                                                                                   })/this->trains.size(), 4) << "\n"
        << "    |_ Average Travelled Distance (km)                                          : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                            [](double total, const auto& train) {
                                                                                                                                                return total + (train->travelledDistance / (double)1000.0);
                                                                                                                                            })/this->trains.size()) << "\n"
        << "    |_ Consumed and Regenerated Energy:\n"
        << "        |_ Total Net Energy Consumed (KW.h)                                     : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
																																				[](double total, const auto& train) {
																																					return total + train->cumEnergyStat;
																																				})) << "\n"
        << "            |_ Total Energy Consumed (KW.h)                                     : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                            [](double total, const auto& train) {
                                                                                                                                                return total + train->totalEConsumed;
                                                                                                                                            })) << "\n"
        << "            |_ Total Energy Regenerated (KW.h)                                  : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                            [](double total, const auto& train) {
                                                                                                                                                return total + train->totalERegenerated;
                                                                                                                                            })) << "\n"
        << "            |_ Average Net Energy Consumption per Net Weight (KW.h/ton)         : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                               [](double total, const auto& t) {
                                                                                                                                                   return total + (t->cumEnergyStat);
                                                                                                                                               }) /
                                                                                                                                std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                                [](double total, const auto& t) {
                                                                                                                                                                    return total + (t->getCargoNetWeight());
                                                                                                                                                                })) << "\n"
        << "            |_ Average Net Energy Consumption per Net ton.km (KW.hx10^3/ton.km) : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                [](double total, const auto& t) {
                                                                                                                                    return total +
                                                                                                                                            (t->cumEnergyStat * (double) 1000.0);
                                                                                                                                }) /
                                                                                                                                std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                [](double total, const auto& t) {
                                                                                                                                                    return total + t->getTrainTotalTorque();
                                                                                                                                                })) << "\n"
        << "        |_ Tank Consumption:\n"
        << "            |_ Total Fuel Consumed (litters)                                    : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                            [](double total, const auto& train) {
                                                                                                                                                return total + train->getTrainConsumedTank();
                                                                                                                                            })) << "\n"
        << "            |_ Average Fuel Consumed per Net Weight (litterx10^3/ton)           : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                            [](double total, const auto& train) {
                                                                                                                                                return total + (train->getTrainConsumedTank() * (double)1000.0);
                                                                                                                                            }) /
                                                                                                                            std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                            [](double total, const auto& train) {
                                                                                                                                                return total + train->getCargoNetWeight();
                                                                                                                                            })) << "\n"
        << "            |_ Average Fuel Consumed per Net ton.km (littersx10^3/ton.km)       : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                            [](double total, const auto& train) {
                                                                                                                                                return total +
                                                                                                                                                        (train->getTrainConsumedTank() * (double)1000.0);
                                                                                                                                            }) /
                                                                                                                            std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                            [](double total, const auto& train) {
                                                                                                                                                return total + train->getTrainTotalTorque();
                                                                                                                                            })) << "\n"
        << "        |_ Battery Consumption:\n"
        << "            |_ Total Energy Consumed (kW.h)                                     : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                      [](double total, const auto& train) {
                                                                                                                                                          return total + train->getBatteryEnergyConsumed();
                                                                                                                                                      })) << "\n"
        << "            |_ Total Energy Regenerated (kW.h)                                  : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                [](double total, const auto& train) {
                                                                                                                                                    return total + train->getBatteryEnergyRegenerated();
                                                                                                                                                })) << "\n"
        << "            |_ Total Net Energy Consumed (kW.h)                                 : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                [](double total, const auto& train) {
                                                                                                                                                    return total + train->getBatteryNetEnergyConsumed();
                                                                                                                                                })) << "\n"
        << "            |_ Average Net Energy Consumed per Net Weight (kW.h/ton)            : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                [](double total, const auto& train) {
                                                                                                                                                    return total + (train->getBatteryNetEnergyConsumed());
                                                                                                                                                }) /
                                                                                                                                std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                [](double total, const auto& train) {
                                                                                                                                                    return total + train->getCargoNetWeight();
                                                                                                                                                })) << "\n"
        << "            |_ Average Net Energy Consumed per Net ton.km (kW.hx10^3/ton.km)    : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                [](double total, const auto& train) {
                                                                                                                                                    return total +
                                                                                                                                                            (train->getBatteryNetEnergyConsumed() * (double)1000.0);
                                                                                                                                                }) /
                                                                                                                                std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                [](double total, const auto& train) {
                                                                                                                                                    return total + train->getTrainTotalTorque();
                                                                                                                                                })) << "\n"
        << "        |_ Tank/Battery Status:\n"
        << "            |_ Average Locomotives Tank Status (%)                              : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                [](double total, const auto& t) {
                                                                                                                                    return total + (t->getAverageLocomotiveTankStatus() * (double)100.0);
                                                                                                                                })/ (double)this->trains.size()) << "\n"
        << "            |_ Average Locomotives Battery Status (%)                           : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                [](double total, const auto& t) {
                                                                                                                                    return total + (t->getAverageLocomotivesBatteryStatus() * (double)100.0);
                                                                                                                                }) / (double)this->trains.size()) << "\n"
        << "            |_ Average Tenders Tank Status (%)                                  : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                [](double total, const auto& t) {
                                                                                                                                    return total + (t->getAverageTendersTankStatus() * (double)100.0);
                                                                                                                                }) / (double)this->trains.size()) << "\n"
        << "            |_ Average Tenders Battery Status (%)                               : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                            [](double total, const auto& t) {
                                                                                                                                return total + (t->getAverageTendersBatteryStatus() * (double)100.0);
                                                                                                                            }) / (double)this->trains.size()) << "\n"
        << "  |-> Statistics:\n"
        << "    |_ Average Delay Time To Each Link Speed                                    : " << Utils::formatDuration(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                         [](double total, const auto& t) {
                                                                                                                             return total + (t->cumDelayTimeStat);
                                                                                                                         }) / (double)this->trains.size()) << "\n"
        << "    |_ Average Delay Time To Max Links speed                                    : " << Utils::formatDuration(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                         [](double total, const auto& t) {
                                                                                                                             return total + (t->cumMaxDelayTimeStat);
                                                                                                                         }) / (double)this->trains.size()) << "\n"
        << "    |_ Average Stoppings                                                        : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                            [](double total, const auto& t) {
                                                                                                                                return total + (t->cumStoppedStat);
                                                                                                                            }) / (double)this->trains.size()) << "\n"
        << "....................................................\n\n";

	if (this->exportIndividualizedTrainsSummary) {
		for (auto& t : this->trains) {
			exportLine
			<< "+ TRAIN STATISTICS:\n"
        << "  |-> Train Information:\n"
            << "    |-> Train ID                                                                : " << t->trainUserID << "\n"
            << "    |-> Locomotives Summary:\n"
            << "        |_ Number of Locomotives/Cars                                           : " << t->nlocs << "/" << t->nCars << "\n"
            << "        |_ Locomotives (Technology, count)                                      : " << t->LocTypeCount().toString() << "\n"
            << "        |_ Operating Locomotives to End of Trains Trip                          : " << t->getActiveLocomotivesNumber() << "\n"
            << "    |-> Cars Summary:\n"
            << "        |_ Cars Count                                                           : " << t->cars.size() << "\n"
            << "        |_ Cars (Types, count)                                                  : " << t->carTypeCount().toString() << "\n\n"
            << "    |-> Moved Commodity:\n"
            << "        |_ Total Moved Cargo (ton)                                              : " << Utils::thousandSeparator(t->getCargoNetWeight()) << "\n"
            << "        |_ Total ton.km (ton.Km)                                                : " << Utils::thousandSeparator(t->getTrainTotalTorque()) << "\n\n"

            << "  |-> Route Information:\n"
            << "    |_ Train Reached Destination                                                : " << (t->reachedDestination ? "true" : "false") << "\n"
			<< "    |_ Start Node                                                               : " << t->trainPathNodes.at(0)->userID << "\n"
			<< "    |_ Destination Node                                                         : " << t->trainPathNodes.back()->userID << "\n"
            << "    |_ Train Total Path Length (km)                                             : " << Utils::thousandSeparator(t->trainTotalPathLength/(double)1000.0) << "\n\n"

			<< "  |-> Train Performance:\n"
			<< "    |_ Operating Time                                                           : " << Utils::formatDuration(t->tripTime) << "\n"
			<< "    |_ Average Speed (meter/second)                                             : " << t->averageSpeed << "\n"
			<< "    |_ Average Acceleration (meter/square second)                               : " << t->averageAcceleration << "\n"
            << "    |_ Travelled Distance (km)                                                  : " << Utils::thousandSeparator(t->travelledDistance/ (double)1000.0) << "\n"
            << "    |_ Consumed and Regenerated Energy:\n"
            << "        |_ Single-Train Trajectory Optimization Enabled                         : " << (t->optimize? "true": "false") << "\n"
            << "        |_ Total Net Energy Consumed (KW.h)                                     : " << Utils::thousandSeparator(t->cumEnergyStat) << "\n"
            << "            |_ Total Energy Consumed (KW.h)                                     : " << Utils::thousandSeparator(t->totalEConsumed) << "\n"
            << "            |_ Total Energy Regenerated (KW.h)                                  : " << Utils::thousandSeparator(t->totalERegenerated) << "\n"
            << "            |_ Average Net Energy Consumption per Net Weight (KW.h/ton)         : " << Utils::thousandSeparator((t->cumEnergyStat) / (t->getCargoNetWeight())) << "\n"
            << "            |_ Average Net Energy Consumption per Net ton.km (KW.hx10^3/ton.km) : " << Utils::thousandSeparator((t->cumEnergyStat * (double) 1000.0) / (t->getTrainTotalTorque())) << "\n"
            << "        |_ Tank Consumption:\n"
            << "            |_ Total Fuel Consumed (litters)                                    : " << Utils::thousandSeparator(t->getTrainConsumedTank()) << "\n"
            << "            |_ Average Fuel Consumed per Net Weight (litterx10^3/ton)           : " << Utils::thousandSeparator((t->getTrainConsumedTank() * (double)1000.0) / t->getCargoNetWeight()) << "\n"
            << "            |_ Average Fuel Consumed per Net ton.km (littersx10^3/ton.km)       : " << Utils::thousandSeparator((t->getTrainConsumedTank() * (double)1000.0) / t->getTrainTotalTorque()) << "\n"
            << "        |_ Battery Consumption:\n"
            << "            |_ Total Energy Consumed (kW.h)                                     : " << Utils::thousandSeparator(t->getBatteryEnergyConsumed()) << "\n"
            << "            |_ Total Energy Regenerated (kW.h)                                  : " << Utils::thousandSeparator(t->getBatteryEnergyRegenerated()) << "\n"
            << "            |_ Total Net Energy Consumed (kW.h)                                 : " << Utils::thousandSeparator(t->getBatteryNetEnergyConsumed()) << "\n"
            << "            |_ Average Net Energy Consumed per Net Weight (kW.h/ton)            : " << Utils::thousandSeparator(t->getBatteryNetEnergyConsumed() / t->getCargoNetWeight()) << "\n"
            << "            |_ Average Net Energy Consumed per Net ton.km (kW.hx10^3/ton.km)    : " << Utils::thousandSeparator((t->getBatteryNetEnergyConsumed() * (double)1000.0) /t->getTrainTotalTorque()) << "\n"

			<< "    |_ Total Energy Consumed (KW.h)                                             : " << Utils::thousandSeparator(t->cumEnergyStat) << "\n"
			<< "        |_ Total Consumed (KW.h)                                                : " << Utils::thousandSeparator(t->totalEConsumed) << "\n"
			<< "        |_ Total Regenerated (KW.h)                                             : " << Utils::thousandSeparator(t->totalERegenerated) << "\n"
			<< "        |_ Total Fuel Consumed (litters)                                        : " << Utils::thousandSeparator(t->getTrainConsumedTank()) << "\n"
			<< "        |_ Total Fuel Consumed per Net Weight (litter/ton)                      : " << Utils::thousandSeparator(t->getTrainConsumedTank()/t->getCargoNetWeight()) << "\n"
            << "        |_ Total Energy Consumption per ton.km (KW.h/ton.km)                    : " << Utils::thousandSeparator(t->cumEnergyStat / (t->getTrainTotalTorque())) << "\n"
			<< "        |_ Total Energy Consumed by Region (Region:KW.h)                        : " << t->cumRegionalConsumedEnergyStat.toString() << "\n"
            << "        |_ Average Locomotives Battery Status (%)                               : " << Utils::thousandSeparator(t->getAverageLocomotivesBatteryStatus() * 100.0) << "\n\n"

			<< "  |-> Statistics:\n"
            << "        |_ Total Delay Time To Each Link Speed                                  : " << Utils::formatDuration(t->cumDelayTimeStat) << "\n"
            << "        |_ Total Delay Time To Max Links speed                                  : " << Utils::formatDuration(t->cumMaxDelayTimeStat) << "\n"
            << "        |_ Total Stoppings                                                      : " << Utils::thousandSeparator(t->cumStoppedStat) << "\n"
			<< "  |-> Locomotives Details:\n";
				// print the locomotives summary
				int locI = 1;
				for (auto &loc: t->locomotives){
                    exportLine << "        |_ Locomotive Number                                                     : " << locI << "\n"
                               << "        |_ Is Locomotive On                                                      : " << ((loc->isLocOn)? "true": "false") << "\n"
							   << "        |_ Power Type                                                            : " << TrainTypes::PowerTypeToStr(loc->powerType) << "\n";
					if (TrainTypes::locomotiveBatteryOnly.exist(loc->powerType) ||
							TrainTypes::locomotiveHybrid.exist(loc->powerType)) {
						exportLine << "        |_ Battery Initial Charge (KW.h)                                         : " << Utils::thousandSeparator(loc->getBatteryInitialCharge()) << "\n"
                                   << "        |_ Battery Current Charge  at End of Trip (KW.h)                         : " << Utils::thousandSeparator(loc->getBatteryCurrentCharge()) << "\n"
								   << "        |_ Battery Initial State of Charge (%)                                   : " << Utils::thousandSeparator(loc->getBatteryInitialCharge() / loc->getBatteryMaxCharge() * 100.0) << "\n"
                                   << "        |_ Battery Current State of Charge  at End of Trip (%)                   : " << Utils::thousandSeparator(loc->getBatteryStateOfCharge() * 100.0) << "\n"
                                   << "        |_ Battery Cumulative Consumed Energy (kW.h)                             : " << Utils::thousandSeparator(loc->getBatteryCumEnergyConsumption()) << "\n"
                                   << "        |_ Battery Cumulative Regenerated Energy (kW.h)                          : " << Utils::thousandSeparator(loc->getBatteryCumEnergyRegenerated()) << "\n"
                                   << "        |_ Battery Cumulative Net Consumed Energy (kW.h)                         : " << Utils::thousandSeparator(loc->getBatteryCumNetEnergyConsumption()) << "\n";
					}
					else if (TrainTypes::locomotiveTankOnly.exist(loc->powerType) ||
							 TrainTypes::locomotiveHybrid.exist(loc->powerType)) {
                        exportLine << "        |_ Tank Initial Capacity (liters)                                        : " << Utils::thousandSeparator(loc->getTankInitialCapacity()) << "\n"
                                   << "        |_ Tank Current Capacity at End of Trip (liters)                         : " << Utils::thousandSeparator(loc->getTankCurrentCapacity()) << "\n"
								   << "        |_ Tank Initial State of Capacity (%)                                    : " << Utils::thousandSeparator(loc->getTankInitialCapacity() / loc->getTankMaxCapacity() * 100.0) << "\n"
                                   << "        |_ Tank Current State of Capacity at End of Trip (%)                     : " << Utils::thousandSeparator(loc->getTankStateOfCapacity() * 100.0) << "\n"
                                   << "        |_ Tank Cumulative Consumed Fuel (liters)                                : " << Utils::thousandSeparator(loc->getTankCumConsumedFuel()) << "\n";
					}
					locI ++;
				}
				// if we have tenders, print their summary
				if (t->carsTypes[TrainTypes::CarType::cargo].size() != t->cars.size()) {
					exportLine << "    |-> Tenders Details:\n";
					int tenderI = 1;
					for (auto &car: t->cars){
						if (car->carType != TrainTypes::CarType::cargo) {
							exportLine << "      |_ Tender Number                                                           : " << tenderI << "\n"
									   << "        |_ Tender Type                                                           : " << TrainTypes::carTypeToStr(car->carType) << "\n";
							if (car->carType == TrainTypes::CarType::batteryTender){
								exportLine << "        |_ Battery Initial Charge                                                : " << Utils::thousandSeparator(car->getBatteryInitialCharge()) << "\n"
										   << "        |_ Battery Current Charge                                                : " << Utils::thousandSeparator(car->getBatteryCurrentCharge()) << "\n"
										   << "        |_ Battery Current State of Charge (%)                                   : " << Utils::thousandSeparator(car->getBatteryStateOfCharge()) << "\n";
							}
							else {
								exportLine << "        |_ Tank Initial Capacity                                                 : " << Utils::thousandSeparator(car->getTankInitialCapacity()) << "\n"
										   << "        |_ Tank Current Capacity                                                 : " << Utils::thousandSeparator(car->getTankCurrentCapacity()) << "\n"
										   << "        |_ Tank Current State of Capacity (%)                                    : " << Utils::thousandSeparator(car->getTankStateOfCapacity()) << "\n";
							}
							tenderI ++;
						}
					}
				}
				exportLine << "..............\n";
		}
	}

	exportLine.imbue(locale());
	// setup the summary file
	this->openSummaryFile();
	this->summaryFile << exportLine.str();
// ##################################################################
// #                       end: summary file                      #
// ##################################################################
	this->summaryFile.close();
	this->trajectoryFile.close();


}

bool Simulator::checkTrainsCollision() {
	Vector<std::pair<std::shared_ptr<Train>, std::shared_ptr<Train>>> comb;
	for (int i = 0; i < this->trains.size() - 1; i++) {
		for (int j = i + 1; j < this->trains.size(); j++) {
			std::shared_ptr < Train> tt = this->trains.at(i);
			comb.push_back( std::make_pair(this->trains.at(i), this->trains.at(j)));
		}
	}
	for (auto &t : comb) {
		if (t.first->loaded && t.second->loaded) {
			if (this->network->twoLinesIntersect(t.first->startEndPoints[0], t.first->startEndPoints[1],
												 t.second->startEndPoints[0], t.second->startEndPoints[1])
					&& (t.second->currentLinks.hasCommonElement(t.first->currentLinks))
					&& (!t.first->reachedDestination && !t.second->reachedDestination)
					&& (this->timeStep > t.first->trainStartTime && this->timeStep > t.second->trainStartTime)) {
				return true;
			}
		}

	}
	return false;
}


void Simulator::setTrainSimulatorPath() {
	for (std::shared_ptr <Train>& t : this->trains) {
		// set the train path ids to the simulator ids instead of the users ids
		t->trainPath = this->network->getSimulatorTrainPath(t->trainPath);

		if (t->trainPath.size() == 2) {
			// check if the path is only 2 nodes
			auto nodes= this->network->getNodeByID(t->trainPath[0])->linkTo.get_keys();
			auto targetN = this->network->getNodeByID(t->trainPath[1]);
			for (auto & n: nodes){
				if (n == targetN) {
					continue;
				}
			}
			// if the path is more than 2 nodes but the user wants the simulator to decide the path.
			Vector<std::shared_ptr<NetNode>> nPath = this->network->shortestPathSearch(t->trainPath[0], t->trainPath[1]).first;
			Vector<int> path;
			for (auto &n: nPath) {
				path.push_back(n->id);
			}
			t->setTrainPath(path);
		}
	}

}



void Simulator::setTrainsPathNodes() {
	for (std::shared_ptr<Train>& t : this->trains) {
		for (int tpn : t->trainPath) {
			t->trainPathNodes.push_back(this->network->getNodeByID(tpn));
		}
	}
}
void Simulator::setTrainPathLength() {
	for (std::shared_ptr <Train>& t : this->trains) {
		t->trainTotalPathLength = this->network->getFullPathLength(t);
	}
}

void Simulator::turnOnAllSignals() {
	for (auto& s : this->network->networkSignals) {
		s->isGreen = true;
	}
}

void Simulator::turnOffSignal(Vector<std::shared_ptr<NetSignal>> networkSignals) {
	if (networkSignals.empty()) { return; }
	for (auto& networkSignal : networkSignals) {
		networkSignal->isGreen = false;
	}
}

void Simulator::runSignalsforTrains() {
	this->turnOnAllSignals();

	for (auto& train : this->trains) {
		if (!train->loaded || train->reachedDestination) { continue; }

		std::shared_ptr<NetSignal> nextSignal = this->getClosestSignal(train);
		if (nextSignal == nullptr) { continue; }

		double d = this->network->getDistanceToSpecificNodeByTravelledDistance(train,
			train->travelledDistance, nextSignal->currentNode.lock()->id);

		std::shared_ptr<NetSignalGroupController> sg = nullptr;
		if (this->signalsGroups.is_key(std::shared_ptr<NetNode>(nextSignal->currentNode))) {
			sg = this->signalsGroups.at(std::shared_ptr<NetNode>(nextSignal->currentNode));
		}
		if (sg == nullptr) { continue; }

		Vector<std::shared_ptr<NetLink>> lockedLinks = sg->confinedLinks;
		if (lockedLinks.size() == 0 || ! this->checkLinksAreFree(lockedLinks)) {
			sg->updateTimeStep(this->simulationTime);
		}

		if ((d <= nextSignal->proximityToActivate) || (this->checkTrainOnLinks(train, lockedLinks))) {

			Vector<std::shared_ptr<NetSignal>> sameDirSignals =
					this->getSignalsInSameDirection(train, sg->networkSignalsGroup);

			sg->sendPassRequestToControlTo(nextSignal, this->simulationTime, sameDirSignals);

			Vector<std::shared_ptr<NetSignal>> otherDirSignals =
					sg->getFeedback().second;
			this->turnOffSignal(otherDirSignals);

		}
	}
}

Vector<std::shared_ptr<NetSignal>> Simulator::getSignalsInSameDirection(std::shared_ptr<Train>& train, 
	Vector<std::shared_ptr<NetSignal>> signalsGroupList) {

	Vector<std::shared_ptr<NetSignal>> signalsList;
	for (auto& netSignal : signalsGroupList) {
		if ((train->trainPath.exist(netSignal->currentNode.lock()->id)) &&
			(train->trainPath.exist(netSignal->previousNode.lock()->id)) ) {
			if ((train->trainPath.index(netSignal->currentNode.lock()->id)) >
				(train->trainPath.index(netSignal->previousNode.lock()->id))) {
				signalsList.push_back(netSignal);
			}
		}
	}
	return signalsList;
}


bool Simulator::checkTrainOnLinks(std::shared_ptr<Train> &train, Vector<std::shared_ptr<NetLink>>& links) {
	if (links.empty()) { return false; }
	std::set<std::shared_ptr<NetLink>> currenttrainLinks = getAccurateTrainCurrentLink(train);
	if (currenttrainLinks.empty()) { return false; }
	for (auto& link : currenttrainLinks) {
		if (links.exist(link)) { return true; }
	}
	return false;


}
std::set<std::shared_ptr<NetLink>> Simulator::getAccurateTrainCurrentLink(std::shared_ptr<Train>& train) {
	std::set<std::shared_ptr<NetLink>> links = std::set<std::shared_ptr<NetLink>>();
	links.insert(this->network->getLinkFromDistance(train, train->travelledDistance, train->previousNodeID));
	double lastPoint = train->travelledDistance - train->totalLength;
	if (lastPoint > 0) {
		links.insert(this->network->getLinkFromDistance(train, lastPoint, train->previousNodeID));
	}
	return links;
}

bool Simulator::checkLinksAreFree(Vector<std::shared_ptr<NetLink>> &links) {
	if (links.empty()) { return true; }
	for (auto& link : links) {
		if (!link->currentTrains.empty()) { return false; }
	}
	return true;
}

std::shared_ptr<NetSignal> Simulator::getClosestSignal(std::shared_ptr<Train>& train) {
	int indx = train->trainPath.index(this->network->getPreviousNodeByDistance(train, 
		train->travelledDistance, train->previousNodeID)->id) + 1;

	for (int i = indx; i < train->trainPath.size(); i++) {
		auto & networkSignals = train->trainPathNodes.at(i)->networkSignals;
		if (networkSignals.size() > 0) {
			for (int j = 0; j < networkSignals.size(); j++) {
				if (train->trainPathNodes.exist(std::shared_ptr<NetNode>(networkSignals.at(j)->previousNode))) {

					if (train->trainPath.index(networkSignals.at(j)->currentNode.lock()->id) >
						(train->trainPath.index(networkSignals.at(j)->previousNode.lock()->id))) {
						return networkSignals.at(j);
					}
				}
			}

		}
	}
	return nullptr;
}

void Simulator::ProgressBar(double current, double total, int bar_length) {
	double fraction = current / total;
	int progress = fraction * bar_length - 1;
	std::stringstream bar;
	for (int i = 0; i < progress; i++) { bar << '-'; }
	bar << '>';
	bar << std::string(bar_length - progress, ' ');
	//for (int i = progress; i < bar_length; i++) { bar << ' '; }

	char ending = (current == total) ? '\n' : '\r';

	std::cout << "Progress: [" << bar.str() << "] " << (int)(fraction * 100) << "%" << ending;
}
