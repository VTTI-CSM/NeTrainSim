/**
 * @file	~\NeTrainSim\src\Simulator.cpp.
 *
 * Implements the simulator class
 */
#include "simulator.h"
#include "QtCore/qstandardpaths.h"    // Include for standard path access
#include "network/netsignalgroupcontroller.h" // Include for controlling network signal groups
#include "network/netsignalgroupcontrollerwithqueuing.h"
#include <filesystem> // Include for filesystem operations
#include <cstdio>
#include <thread>    // Include for multi-threading functionality
#include <chrono>    // Include for time-related operations
#include <ctime>
#include <locale>
#include "util/utils.h"
#include <filesystem>
#include <cmath>
#include <memory>    // Include for smart pointers
#include "util/error.h" // Include for error handling utilities

// Function to get the path to the home directory.
// If the path is not empty, it is returned, otherwise a runtime exception is thrown with an error message.
QString getHomeDirectory()
{
	QString homeDir;

// OS-dependent home directory retrieval
#if defined(Q_OS_WIN)
	homeDir = QDir::homePath();
#elif defined(Q_OS_LINUX) || defined(Q_OS_MAC)
	homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#endif

	if (!homeDir.isEmpty())
	{
		// Creating a path to NeTrainSim folder in the Documents directory
		const QString documentsDir = QDir(homeDir).filePath("Documents");
		const QString folder = QDir(documentsDir).filePath("NeTrainSim");

		QDir().mkpath(folder); // Create the directory if it doesn't exist

		return folder;
	}

	throw std::runtime_error("Error: Cannot retrieve home directory!");
}


// Getter for exportIndividualizedTrainsSummary flag
bool Simulator::getExportIndividualizedTrainsSummary() const {
	return exportIndividualizedTrainsSummary;
}

// Setter for exportIndividualizedTrainsSummary flag
void Simulator::setExportIndividualizedTrainsSummary(bool newExportIndividualizedTrainsSummary) {
	exportIndividualizedTrainsSummary = newExportIndividualizedTrainsSummary;
}

// Constructor for the Simulator class
Simulator::Simulator(Network* theNetwork, Vector<std::shared_ptr<Train>> networkTrains,
                     double simulatorTimeStep, QObject *parent) : QObject(parent), pauseFlag(false) {

	// Initialization of member variables
    this->network = theNetwork;
	this->trains = networkTrains;
	// Definition of train paths for the simulator
	this->setTrainSimulatorPath();
	this->setTrainsPathNodes();
	this->setTrainPathLength();
	// Setting simulation parameters
	this->simulationEndTime = DefaultEndTime;
    this->timeStep = simulatorTimeStep;
	this->simulationTime = 0.0;
	this->progress = 0.0;
	// Handling endless simulation setting
	if (this->simulationEndTime == 0.0) {
		this->runSimulationEndlessly = true;
	}
	else {
		this->runSimulationEndlessly = false;
	}

	// Set output location to home directory
	this->outputLocation = getHomeDirectory();

	// Get a high-resolution time point
	auto now = std::chrono::high_resolution_clock::now();

	// Convert the time point to a numerical value
	auto serial_number = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

	// Define default names for trajectory and summary files with current time
	this->trajectoryFilename = DefaultInstantaneousTrajectoryFilename + std::to_string(serial_number) + ".csv";
	this->summaryFileName = DefaultSummaryFilename + std::to_string(serial_number) + ".txt";

	this->exportTrajectory = false;

	// Define signals groups based on the length of the longest train
	auto max_train = std::max_element(this->trains.begin(), this->trains.end(),
										  [](const std::shared_ptr<Train> t1, const std::shared_ptr<Train> t2) {
											  return t1->totalLength < t2->totalLength;
										  });

	defineSignalsGroups((*max_train)->totalLength);
}

// Setter for the time step of the simulation
void Simulator::setTimeStep(double newTimeStep) {
	this->timeStep = newTimeStep;
}

// Setter for the end time of the simulation
void Simulator::setEndTime(double newEndTime) {
	this->simulationEndTime = newEndTime;
}

// Setter for the plot frequency
void Simulator::setPlotFrequency(int newPlotFrequency) {
    this->plotFrequency = newPlotFrequency;
}

// Setter for the output folder location
void Simulator::setOutputFolderLocation(string newOutputFolderLocation) {
	this->outputLocation = QString::fromStdString(newOutputFolderLocation);
}

// Setter for the summary file name
void Simulator::setSummaryFilename(string newfilename) {
	QString filename = QString::fromStdString(newfilename);
	if (!filename.isEmpty()){
		QFileInfo fileInfo(filename);
		// Check if the file name has an extension
		if (!fileInfo.completeSuffix().isEmpty()){
			this->summaryFileName = newfilename;
		}
		else{
            this->summaryFileName = newfilename + ".txt";
		}
	}
	else {
		this->summaryFileName = DefaultSummaryFilename;
	}
}

// Setter for the instantaneous trajectory export flag and filename
void Simulator::setExportInstantaneousTrajectory(bool exportInstaTraject, string newInstaTrajectFilename) {
	this->exportTrajectory = exportInstaTraject;
	if (newInstaTrajectFilename != ""){
		QString filename = QString::fromStdString(newInstaTrajectFilename);

		QFileInfo fileInfo(filename);
		// Check if the file name has an extension
		if (!fileInfo.completeSuffix().isEmpty())
		{
			// The new filename has an extension
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



// The openTrajectoryFile function tries to open a file to store the trajectory of trains.
// If it fails to open the file, it throws an exception.
void Simulator::openTrajectoryFile() {
	try {
		// Open the trajectory file to write the step trajectory data
		this->trajectoryFile.open(QDir(this->outputLocation).
								  filePath(QString::fromStdString(this->trajectoryFilename)).
								  toStdString(),
								  std::ios::out | std::ios::trunc);

		// if couldnt open, throw error
		if (!this->trajectoryFile.is_open()) {
			throw std::ios_base::failure(std::string("Error: ") +
										 std::to_string(static_cast<int>(Error::cannotOpenTrajectoryFile)) +
										 "\nError opening file: " + this->trajectoryFilename + "!\n");
		}
	}
	catch (const std::ios_base::failure& e) {
		throw std::ios_base::failure(std::string("Error: ") +
									 std::to_string(static_cast<int>(Error::cannotOpenTrajectoryFile)) +
									 "\nCould not create/open the trajectory file!\n");
	}
}

// The getOutputFolder function returns the path to the directory where the output files are stored.
std::string Simulator::getOutputFolder() {
	return this->outputLocation.toStdString();
}

// The openSummaryFile function attempts to open a file to store the summary of the simulation.
// If it fails to open the file, it throws an exception.
void Simulator::openSummaryFile() {
	try {
		// open the summary file to write the summary data
		this->summaryFile.open(QDir(this->outputLocation).
							   filePath(QString::fromStdString(this->summaryFileName)).toStdString(),
							   std::ios::out | std::ios::trunc);

		// throw error if couldnt open
        if (!this->summaryFile.is_open()) {
			throw std::ios_base::failure(std::string("Error: ") +
										 std::to_string(static_cast<int>(Error::cannotOpenSummaryFile)) +
										 "\nError opening file: " + this->summaryFileName + "!\n");
		}
	}
	catch (const std::ios_base::failure& e) {
        throw std::ios_base::failure(std::string("Error: ") +
                                     std::to_string(static_cast<int>(Error::cannotOpenSummaryFile)) +
                                     "\nCould not create/open the summary file!\n");
	}
}

// The checkAllTrainsReachedDestination function checks if all trains have reached their destinations.
// If a train has not reached its destination, it returns false; otherwise, it returns true.
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

// The loadTrain function sets up a train for simulation by setting its starting point,
// and connecting it to the network.
void Simulator::loadTrain(std::shared_ptr <Train> train) {
	train->loaded = true;
	train->currentCoordinates = train->trainPathNodes.at(0)->coordinates();
	train->setTrainsCurrentLinks(Vector<std::shared_ptr<NetLink>>(1, this->network->getFirstTrainLink(train)));
	train->linksCumLengths = this->network->generateCumLinksLengths(train);
	train->previousNodeID = train->trainPath.at(0);
	train->LastTrainPointpreviousNodeID = train->trainPath.at(0);
}

// The loadTrainLinksData function loads data about the links that a train will pass through during the simulation.
// This data includes information about curvature, grade, free-flow speed, and link pointers.
tuple<Vector<double>, Vector<double>, Vector<double>,
			Vector<std::shared_ptr<NetLink>>> Simulator::loadTrainLinksData(
					std::shared_ptr<Train> train, bool isVirtual)
{

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
    tuple<Vector<double>, Vector<double>, Vector<double>, Vector<std::shared_ptr<NetLink>>> myreturn;
	myreturn = std::make_tuple(curvatures, grades, freeFlowSpeeds, links);
	return myreturn;
}

// This function returns the free-flow speed of a given train by getting the network link
// from its current position and the previous node it was at.
double Simulator::loadTrainFreeSpeed(std::shared_ptr<Train> train) {
	// Fetch the network link that the train is currently on
	std::shared_ptr<NetLink> link = this->network->getLinkFromDistance(train, train->travelledDistance, train->previousNodeID);
	// Return the free-flow speed of the network link
	return link->freeFlowSpeed;
}

// This function returns the index of the network signal associated with a given network link.
// It returns -1 if no such signal is found.
int Simulator::getSignalFromLink(Vector<std::shared_ptr<NetSignal>> networkSignals, std::shared_ptr<NetLink> link) {
	// Iterate over the list of network signals
	for (int i = 0; i < networkSignals.size(); i++) {
		std::shared_ptr<NetSignal> networkSignal = networkSignals.at(i);
		// If a network signal is found for the given link, return its index
		if (auto &sharedSignal = networkSignal) {
			if (sharedSignal->link.lock() == link) { return i; }
		}
	}
	// Return -1 if no signal was found for the link
	return -1;
}


// This function returns the ID of the next node where the given train will stop and a boolean value
// indicating whether the train will have to stop due to a red signal.
pair<int, bool> Simulator::getNextStoppingNodeID(std::shared_ptr<Train> train, int &previousNodeID) {
	// Fetch the index of the previous node in the train's path
	int previousNodeIndex = train->trainPath.index(previousNodeID);
	// Iterate over the train's path
	for (int i = previousNodeIndex + 1; i < train->trainPath.size(); i++) {
		// If index exceeds the path size, return last node ID and false
		if (i >= train->trainPath.size()) { return std::make_pair(train->trainPath.back(), false); }
		// If current node is a depot and not the first node in path, return current node ID and false
		if (train->trainPathNodes[i]->isDepot && i > 0) {
			return std::make_pair(train->trainPath[i], false);
		}
		// If current node has network signals
		else if (!train->trainPathNodes[i]->networkSignals.empty()) {
            if (i == 0) { continue;}
			int prevI = i - 1;
			// Iterate over network signals at current node
            for (auto &s: train->trainPathNodes[i]->networkSignals) {
                // If the signal matches the path from the previous node to the current node
				if (s->currentNode.lock()->id == train->trainPathNodes[i]->id &&
					s->previousNode.lock()->id == train->trainPathNodes[prevI]->id) {
					// If signal is not green, return current node ID and true
					if (! s->isGreen) {
						return std::make_pair(train->trainPath[i], true);
					}
					// If signal is green, break from loop and continue
					else {
						break;
					}
				}
			}
			// this->getNextStoppingNodeID(train, train->trainPath[i]);
		}
	}
	// If no stopping node is found in remaining path, return last node ID and false
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
        double d1 = Utils::getDistanceByTwoCoordinates(train->currentCoordinates, otherTrain->startEndPoints[0]);
        double d2 = Utils::getDistanceByTwoCoordinates(train->currentCoordinates, otherTrain->startEndPoints[1]);
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

// This function simulates one time step for a given train in the simulation environment
void Simulator::playTrainOneTimeStep(std::shared_ptr <Train> train)
{
	// Indicator to skip loading the train
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

	// Continue if the train is loaded and its start time is past the current simulation time
	if ((train->trainStartTime <= this->simulationTime) && train->loaded) {

		// holds track data and speed
        tuple<Vector<double>, Vector<double>, Vector<double>, Vector<std::shared_ptr<NetLink>>> linksdata;
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
        tuple<Vector<double>, Vector<bool>, Vector<double>> criticalPointsDefinition;

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
            // calculate the reduction factor if the power source cannot supply the demand of energy
            // reset the restrictions every time step
            train->resetPowerRestriction();
            // check if a notch reduction is required
            // calculate the accelerations and speed
            double stepAcc = train->getStepAcceleration(this->timeStep, currentFreeFlowSpeed, std::get<0>(criticalPointsDefinition),
                                             std::get<1>(criticalPointsDefinition), std::get<2>(criticalPointsDefinition));
            double stepSpd = train->speedUpDown(train->previousSpeed, stepAcc, this->timeStep, currentFreeFlowSpeed);
            // calculate approximate power required
            pair<Vector<double>, double> out = train->getTractivePower(stepSpd, stepAcc, train->currentResistanceForces);
            // calculate approximate energy required
            double stepEC = train->getTotalEnergyConsumption(this->timeStep, out.first);
            // calculate approximate max energy supplied at this time step
            double maxEC = train->getMaxProvidedEnergy(this->timeStep).first;
            // If the stepEC is larger than what the train can consume in a time step,
            // reduce the locomotives power
            if (stepEC > maxEC) {
                double reductionFactor = maxEC / stepEC;
                train->reducePower(reductionFactor);
            }
			// move the train forward
			train->moveTrain(this->timeStep, currentFreeFlowSpeed, std::get<0>(criticalPointsDefinition),
				std::get<1>(criticalPointsDefinition), std::get<2>(criticalPointsDefinition));
		}
		// handle when the train reaches its destinations
		if ((std::round(train->trainTotalPathLength * 1000.0) / 1000.0) <= (std::round(train->travelledDistance * 1000.0) / 1000.0)) {
			train->travelledDistance = train->trainTotalPathLength;
			train->reachedDestination = true;
			train->calcTrainStats(freeFlowSpeed, currentFreeFlowSpeed, this->timeStep, train->currentFirstLink->region);
		}
		// handles when the train still has distance to travel
		else {
			train->currentCoordinates = this->network->getPositionbyTravelledDistance(train, train->travelledDistance);
			train->startEndPoints = this->getStartEndPoints(train, train->currentCoordinates);
			train->calcTrainStats(freeFlowSpeed, currentFreeFlowSpeed, this->timeStep, train->currentFirstLink->region);

			// holds track data and speed
            tuple<Vector<double>, Vector<double>, Vector<double>, Vector<std::shared_ptr<NetLink>>> linksdata;
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

			// Update the links that the train is spanning
			this->setOccupiedLinksByTrains(train);
		}
		// write the trajectory step data
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
                       << train->locomotives[0]->currentLocNotch << ","
                       << train->optimize
                       << std::endl;

			// write the step trajectory data to the file
			this->trajectoryFile << exportLine.str();

		}
	}

    // Minimize waiting when no trains are on network
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

void Simulator::PlayTrainVirtualStepsAStarOptimization(std::shared_ptr<Train> train, double timeStep){

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
        std::shared_ptr<NetSignalGroupControllerWithQueuing> s =
            std::make_shared< NetSignalGroupControllerWithQueuing>(NetSignalGroupControllerWithQueuing(group, this->timeStep));
        //s->confinedLinks = this->getLinksByNodes(nodesGroup.at(i));

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
                proximity.push_back(t->getSafeGap(t->getMinFollowingTrainGap(),
                                                  t->currentSpeed, s->link.lock()->freeFlowSpeed,
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
                if ((d < minSafeDistance) || (this->network->isConflictZone(this->trains.at(t1i),
                                                                            trainIntersections.at(i),
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
        exportLine << "TrainNo,TStep_s,TravelledDistance_m,Acceleration_mps2,"
                   << "Speed_mps,LinkMaxSpeed_mps,"
                   << "EnergyConsumption_KWH,DelayTimeToEach_s,DelayTime_s,"
                   << "Stoppings,tractiveForce_N,"
                   << "ResistanceForces_N,CurrentUsedTractivePower_kw,"
                   << "GradeAtTip_Perc,CurvatureAtTip_Perc,"
                   << "FirstLocoNotchPosition,optimizationEnabled\n";
		this->trajectoryFile << exportLine.str();
	}

    time_t init_time = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now());

    while (this->simulationTime <= this->simulationEndTime ||
           this->runSimulationEndlessly)
    {
        mutex.lock();
         // This will block the thread if pauseFlag is true
        if (pauseFlag) pauseCond.wait(&mutex);
        mutex.unlock();

		if (this->checkAllTrainsReachedDestination()) {
			break;
		}
		for (std::shared_ptr <Train>& t : (this->trains)) {
			if (t->reachedDestination) { continue;  }

			if (t->optimize){
				if (t->lookAheadCounterToUpdate <= 0) {
					t->resetTrainLookAhead();
					this->PlayTrainVirtualStepsAStarOptimization(t, this->timeStep);
				}
			}

			this->playTrainOneTimeStep(t);
		}

        if (plotFrequency > 0.0 && ((int(this->simulationTime) * 10) % (plotFrequency * 10)) == 0) {
            Vector<std::pair<std::string, Vector<std::pair<double,double>>>> trainsStartEndPoints;
            for (std::shared_ptr <Train>& t : (this->trains)) {
                if (! t->loaded) { continue; }
                trainsStartEndPoints.push_back(std::make_pair(t->trainUserID, t->startEndPoints));
            }
            emit this->plotTrainsUpdated(trainsStartEndPoints);
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
    trainsSummaryData.clear();
	std::stringstream exportLine;
    tuple<double, double, double, double, double> networkStats = this->network->getNetworkStats();

	exportLine << "~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~\n"
        << "NeTrainSim SIMULATION SUMMARY\n"
        << "Version: " << MYAPP_VERSION << "\n"
        << "Simulation Time: " << Utils::formatDuration(difTime) << " (dd:hh:mm:ss)\n"
        << "~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~.~\n\n"
        << "+ NETWORK STATISTICS:\n"
        << "  |_ Network Name                                                               \x1D : " << this->network->networkName << "\n"
        << "  |_ Nodes Count                                                                \x1D : " << Utils::thousandSeparator(this->network->nodes.size()) << "\n"
        << "  |_ Links Count                                                                \x1D : " << Utils::thousandSeparator(this->network->links.size()) << "\n"
        << "    |_ Total Lengths of All Links (meters)                                      \x1D : " << Utils::thousandSeparator(std::get<3>(networkStats)) << "\n"
        << "    |_ Total Lengths of All Links with Catenary (meters)                        \x1D : " << Utils::thousandSeparator(std::get<4>(networkStats)) << "\n"
        << "  |_ Total Signals                                                              \x1D : " << Utils::thousandSeparator(this->network->networkSignals.size()) << "\n"
        << "  |_ Total Number of Trains on Network                                          \x1D : " << Utils::thousandSeparator(this->trains.size()) << "\n"
        << "  |_ Percentage of Links with Catenaries to All Links (%)                       \x1D : " << Utils::thousandSeparator(std::get<0>(networkStats)) << "\n"
        << "  |_ Number of Trains with Enabled Trajectory Optimization                      \x1D : " << std::accumulate(this->trains.begin(), this->trains.end(), 0,
                                                                                                                               [](int total, const auto& train) {
                                                                                                                                   return total + (int)train->optimize;
                                                                                                                               }) << "\n"
        << "  |_ Catenary Total Energy Consumed (KW.h)                                      \x1D : " << Utils::thousandSeparator(std::get<1>(networkStats) - std::get<2>(networkStats)) << "\n"
        << "        |_ Average Catenary Energy Consumption per Net Weight (KW.h/ton)        \x1D : " << Utils::thousandSeparator( (std::get<1>(networkStats) - std::get<2>(networkStats)) /
                                                                                                                            std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                            [](double total, const auto& t) {
                                                                                                                                                                return total + (t->getCargoNetWeight());
                                                                                                                                                            })) << "\n"
        << "        |_ Average Catenary Energy Consumption per Net ton.km (KW.hx10^3/ton.km)\x1D : " << Utils::thousandSeparator(((std::get<1>(networkStats) - std::get<2>(networkStats)) * (double) 1000.0) /
                                                                                                                            std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                            [](double total, const auto& t) {
                                                                                                                                                return total + t->getTrainTotalTorque();
                                                                                                                                            })) << "\n"
        << "    |_ Catenary Energy Consumed (KW.h)                                          \x1D : " << Utils::thousandSeparator(std::get<1>(networkStats)) << "\n"
        << "    |_ Catenary Energy Regenerated (KW.h)                                       \x1D : " << Utils::thousandSeparator(std::get<2>(networkStats)) << "\n"

        << "....................................................\n\n"
        << "\n";

	exportLine
        << "+ AGGREGATED/ACCUMULATED TRAINS STATISTICS:\n"
        << "  |-> Train Information:\n"
        << "    |-> Locomotives Summary:\n"
        << "        |_ Number of Locomotives/Cars                                           \x1D : " << std::accumulate(this->trains.begin(), this->trains.end(), 0,
                                                                                                           [](int total, const auto& train) {
                                                                                                               return total + train->nlocs;
                                                                                                           }) << "/"
                                                                                                                << std::accumulate(this->trains.begin(), this->trains.end(), 0,
                                                                                                                [](int total, const auto& train) {
                                                                                                                    return total + train->nCars;
                                                                                                                }) << "\n"
        << "        |_ Locomotives (Technology, count)                                      \x1D : " << std::accumulate(
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
        << "        |_ Operating Locomotives to End of Trains Trip                          \x1D : " << std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                       [](int total, const auto& train) {
                                                                                                           return total + train->getActiveLocomotivesNumber();
                                                                                                       }) << "\n"
        << "    |-> Cars Summary:\n"
        << "        |_ Cars Count                                                           \x1D : " << std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                   [](int total, const auto& train) {
                                                                                                                       return total + train->cars.size();
                                                                                                                   }) << "\n"
        << "        |_ Cars (Types, count)                                                  \x1D : " << std::accumulate(
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
        << "        |_ Total Moved Cargo (ton)                                              \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                            [](double total, const auto& t) {
                                                                                                                                return total + (t->getCargoNetWeight());
                                                                                                                            })) << "\n"
        << "        |_ Total ton.km (ton.Km)                                                \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                            [](double total, const auto& t) {
                                                                                                                                return total + (t->getTrainTotalTorque());
                                                                                                                            })) << "\n\n"

        << "  |-> Route Information:\n"
        << "    |_ Trains Reached Destination                                               \x1D : " << std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                        [](int total, const auto& train) {
                                                                                                            return total + train->reachedDestination;
                                                                                                        }) << "\n"
        << "    |_ Trains Total Path Length (km)                                            \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                            [](double total, const auto& train) {
                                                                                                                                return total + (train->trainTotalPathLength / (double)1000.0);
                                                                                                                            })) << "\n\n"
        << "  |-> Train Performance:\n"
        << "    |_ Operating Time                                                           \x1D : " << Utils::formatDuration(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                         [](double total, const auto& train) {
                                                                                                                             return total + train->tripTime;
                                                                                                                         })) << "\n"
        << "    |_ Average Speed (meter/second)                                             \x1D : " << std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                   [](double total, const auto& train) {
                                                                                                                       return total + train->averageSpeed;
                                                                                                                   })/this->trains.size() << "\n"
        << "    |_ Average Acceleration (meter/square second)                               \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                   [](double total, const auto& train) {
                                                                                                                       return total + train->averageAcceleration;
                                                                                                                   })/this->trains.size(), 4) << "\n"
        << "    |_ Average Travelled Distance (km)                                          \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                            [](double total, const auto& train) {
                                                                                                                                                return total + (train->travelledDistance / (double)1000.0);
                                                                                                                                            })/this->trains.size()) << "\n"
        << "    |_ Consumed and Regenerated Energy:\n"
        << "        |_ Total Net Energy Consumed (KW.h)                                     \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
																																				[](double total, const auto& train) {
																																					return total + train->cumEnergyStat;
																																				})) << "\n"
        << "            |_ Total Energy Consumed (KW.h)                                     \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                            [](double total, const auto& train) {
                                                                                                                                                return total + train->totalEConsumed;
                                                                                                                                            })) << "\n"
        << "            |_ Total Energy Regenerated (KW.h)                                  \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                            [](double total, const auto& train) {
                                                                                                                                                return total + train->totalERegenerated;
                                                                                                                                            })) << "\n"
        << "            |_ Average Net Energy Consumption per Net Weight (KW.h/ton)         \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                               [](double total, const auto& t) {
                                                                                                                                                   return total + (t->cumEnergyStat);
                                                                                                                                               }) /
                                                                                                                                std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                                [](double total, const auto& t) {
                                                                                                                                                                    return total + (t->getCargoNetWeight());
                                                                                                                                                                })) << "\n"
        << "            |_ Average Net Energy Consumption per Net ton.km (KW.hx10^3/ton.km) \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                [](double total, const auto& t) {
                                                                                                                                    return total +
                                                                                                                                            (t->cumEnergyStat * (double) 1000.0);
                                                                                                                                }) /
                                                                                                                                std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                [](double total, const auto& t) {
                                                                                                                                                    return total + t->getTrainTotalTorque();
                                                                                                                                                })) << "\n"
        << "        |_ Tank Consumption:\n";
    auto tankComp = std::accumulate(
        this->trains.begin(),
        this->trains.end(),
        Map<std::string, double>{},
        [](Map<std::string, double> acc, const auto& train) {
            const auto& consumedTank = train->getTrainConsumedTank();
            for (const auto& kvp : consumedTank) {
                acc[kvp.first] += kvp.second;
            }
            return acc;
        }
        );

    exportLine
        << "            |_ Total Fuel Consumed (litters)                                    \x1D : " << (tankComp).toString() << "\n";
    if (tankComp.get_keys().size() == 1) {

        exportLine
        << "            |_ Average Fuel Consumed per Net Weight (litterx10^3/ton)           \x1D : " << Utils::thousandSeparator((tankComp.sumValues() * (double)1000.0) /
                                                                                                                            std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                            [](double total, const auto& train) {
                                                                                                                                                return total + train->getCargoNetWeight();
                                                                                                                                     })) << "\n"
        << "            |_ Average Fuel Consumed per Net ton.km (littersx10^3/ton.km)       \x1D : " << Utils::thousandSeparator((tankComp.sumValues()* (double)1000.0) /
                                                                                                                            std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                            [](double total, const auto& train) {
                                                                                                                                                return total + train->getTrainTotalTorque();
                                                                                                                                     })) << "\n";
    }
    exportLine
        << "        |_ Battery Consumption:\n"
        << "            |_ Total Energy Consumed (kW.h)                                     \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                      [](double total, const auto& train) {
                                                                                                                                                          return total + train->getBatteryEnergyConsumed();
                                                                                                                                                      })) << "\n"
        << "            |_ Total Energy Regenerated (kW.h)                                  \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                [](double total, const auto& train) {
                                                                                                                                                    return total + train->getBatteryEnergyRegenerated();
                                                                                                                                                })) << "\n"
        << "            |_ Total Net Energy Consumed (kW.h)                                 \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                [](double total, const auto& train) {
                                                                                                                                                    return total + train->getBatteryNetEnergyConsumed();
                                                                                                                                                })) << "\n"
        << "            |_ Average Net Energy Consumed per Net Weight (kW.h/ton)            \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                [](double total, const auto& train) {
                                                                                                                                                    return total + (train->getBatteryNetEnergyConsumed());
                                                                                                                                                }) /
                                                                                                                                std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                [](double total, const auto& train) {
                                                                                                                                                    return total + train->getCargoNetWeight();
                                                                                                                                                })) << "\n"
        << "            |_ Average Net Energy Consumed per Net ton.km (kW.hx10^3/ton.km)    \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                [](double total, const auto& train) {
                                                                                                                                                    return total +
                                                                                                                                                            (train->getBatteryNetEnergyConsumed() * (double)1000.0);
                                                                                                                                                }) /
                                                                                                                                std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                                [](double total, const auto& train) {
                                                                                                                                                    return total + train->getTrainTotalTorque();
                                                                                                                                                })) << "\n"
        << "        |_ Tank/Battery Status:\n"
        << "            |_ Average Locomotives Tank Status (%)                              \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                [](double total, const auto& t) {
                                                                                                                                    return total + (t->getAverageLocomotiveTankStatus() * (double)100.0);
                                                                                                                                })/ (double)this->trains.size()) << "\n"
        << "            |_ Average Locomotives Battery Status (%)                           \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                [](double total, const auto& t) {
                                                                                                                                    return total + (t->getAverageLocomotivesBatteryStatus() * (double)100.0);
                                                                                                                                }) / (double)this->trains.size()) << "\n"
        << "            |_ Average Tenders Tank Status (%)                                  \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                                [](double total, const auto& t) {
                                                                                                                                    return total + (t->getAverageTendersTankStatus() * (double)100.0);
                                                                                                                                }) / (double)this->trains.size()) << "\n"
        << "            |_ Average Tenders Battery Status (%)                               \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                            [](double total, const auto& t) {
                                                                                                                                return total + (t->getAverageTendersBatteryStatus() * (double)100.0);
                                                                                                                            }) / (double)this->trains.size()) << "\n"
        << "  |-> Statistics:\n"
        << "    |_ Average Delay Time To Each Link Speed                                    \x1D : " << Utils::formatDuration(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                         [](double total, const auto& t) {
                                                                                                                             return total + (t->cumDelayTimeStat);
                                                                                                                         }) / (double)this->trains.size()) << "\n"
        << "    |_ Average Delay Time To Max Links speed                                    \x1D : " << Utils::formatDuration(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                         [](double total, const auto& t) {
                                                                                                                             return total + (t->cumMaxDelayTimeStat);
                                                                                                                         }) / (double)this->trains.size()) << "\n"
        << "    |_ Average Stoppings                                                        \x1D : " << Utils::thousandSeparator(std::accumulate(this->trains.begin(), this->trains.end(), 0.0,
                                                                                                                            [](double total, const auto& t) {
                                                                                                                                return total + (t->cumStoppedStat);
                                                                                                                            }) / (double)this->trains.size()) << "\n"
        << "....................................................\n\n";

	if (this->exportIndividualizedTrainsSummary) {
        for (auto& t : this->trains) {
            std::stringstream trainStat;
            trainStat
            << "+ TRAIN STATISTICS:\n"
        << "  |-> Train Information:\n"
            << "    |-> Train ID                                                                \x1D : " << t->trainUserID << "\n"
            << "    |-> Locomotives Summary:\n"
            << "        |_ Number of Locomotives/Cars                                           \x1D : " << t->nlocs << "/" << t->nCars << "\n"
            << "        |_ Locomotives (Technology, count)                                      \x1D : " << t->LocTypeCount().toString() << "\n"
            << "        |_ Operating Locomotives to End of Trains Trip                          \x1D : " << t->getActiveLocomotivesNumber() << "\n"
            << "    |-> Cars Summary:\n"
            << "        |_ Cars Count                                                           \x1D : " << t->cars.size() << "\n"
            << "        |_ Cars (Types, count)                                                  \x1D : " << t->carTypeCount().toString() << "\n\n"
            << "    |-> Moved Commodity:\n"
            << "        |_ Total Moved Cargo (ton)                                              \x1D : " << Utils::thousandSeparator(t->getCargoNetWeight()) << "\n"
            << "        |_ Total ton.km (ton.Km)                                                \x1D : " << Utils::thousandSeparator(t->getTrainTotalTorque()) << "\n\n"

            << "  |-> Route Information:\n"
            << "    |_ Train Reached Destination                                                \x1D : " << (t->reachedDestination ? "true" : "false") << "\n"
            << "    |_ Start Node                                                               \x1D : " << t->trainPathNodes.at(0)->userID << "\n"
            << "    |_ Destination Node                                                         \x1D : " << t->trainPathNodes.back()->userID << "\n"
            << "    |_ Train Total Path Length (km)                                             \x1D : " << Utils::thousandSeparator(t->trainTotalPathLength/(double)1000.0) << "\n\n"

            << "  |-> Train Performance:\n"
            << "    |_ Operating Time                                                           \x1D : " << Utils::formatDuration(t->tripTime) << "\n"
            << "    |_ Average Speed (meter/second)                                             \x1D : " << t->averageSpeed << "\n"
            << "    |_ Average Acceleration (meter/square second)                               \x1D : " << t->averageAcceleration << "\n"
            << "    |_ Travelled Distance (km)                                                  \x1D : " << Utils::thousandSeparator(t->travelledDistance/ (double)1000.0) << "\n"
            << "    |_ Consumed and Regenerated Energy:\n"
            << "        |_ Single-Train Trajectory Optimization Enabled                         \x1D : " << (t->optimize? "true": "false") << "\n"
            << "        |_ Total Net Energy Consumed (KW.h)                                     \x1D : " << Utils::thousandSeparator(t->cumEnergyStat) << "\n"
            << "            |_ Total Energy Consumed (KW.h)                                     \x1D : " << Utils::thousandSeparator(t->totalEConsumed) << "\n"
            << "            |_ Total Energy Regenerated (KW.h)                                  \x1D : " << Utils::thousandSeparator(t->totalERegenerated) << "\n"
            << "            |_ Average Net Energy Consumption per Net Weight (KW.h/ton)         \x1D : " << Utils::thousandSeparator((t->cumEnergyStat) / (t->getCargoNetWeight())) << "\n"
            << "            |_ Average Net Energy Consumption per Net ton.km (KW.hx10^3/ton.km) \x1D : " << Utils::thousandSeparator((t->cumEnergyStat * (double) 1000.0) / (t->getTrainTotalTorque())) << "\n"
            << "        |_ Tank Consumption:\n"
            << "            |_ Total Fuel Consumed (litters)                                    \x1D : " << t->getTrainConsumedTank().toString() << "\n";
            if (t->getTrainConsumedTank().get_keys().size() == 1) {
                trainStat
                << "            |_ Average Fuel Consumed per Net Weight (litterx10^3/ton)           \x1D : " << Utils::thousandSeparator((t->getTrainConsumedTank().sumValues() * (double)1000.0) / t->getCargoNetWeight()) << "\n"
                << "            |_ Average Fuel Consumed per Net ton.km (littersx10^3/ton.km)       \x1D : " << Utils::thousandSeparator((t->getTrainConsumedTank().sumValues() * (double)1000.0) / t->getTrainTotalTorque()) << "\n";
            }
            trainStat
            << "        |_ Battery Consumption:\n"
            << "            |_ Total Energy Consumed (kW.h)                                     \x1D : " << Utils::thousandSeparator(t->getBatteryEnergyConsumed()) << "\n"
            << "            |_ Total Energy Regenerated (kW.h)                                  \x1D : " << Utils::thousandSeparator(t->getBatteryEnergyRegenerated()) << "\n"
            << "            |_ Total Net Energy Consumed (kW.h)                                 \x1D : " << Utils::thousandSeparator(t->getBatteryNetEnergyConsumed()) << "\n"
            << "            |_ Average Net Energy Consumed per Net Weight (kW.h/ton)            \x1D : " << Utils::thousandSeparator(t->getBatteryNetEnergyConsumed() / t->getCargoNetWeight()) << "\n"
            << "            |_ Average Net Energy Consumed per Net ton.km (kW.hx10^3/ton.km)    \x1D : " << Utils::thousandSeparator((t->getBatteryNetEnergyConsumed() * (double)1000.0) /t->getTrainTotalTorque()) << "\n"

            << "    |_ Total Energy Consumed (KW.h)                                             \x1D : " << Utils::thousandSeparator(t->cumEnergyStat) << "\n"
            << "        |_ Total Consumed (KW.h)                                                \x1D : " << Utils::thousandSeparator(t->totalEConsumed) << "\n"
            << "        |_ Total Regenerated (KW.h)                                             \x1D : " << Utils::thousandSeparator(t->totalERegenerated) << "\n"
            << "        |_ Total Fuel Consumed (litters)                                        \x1D : " << t->getTrainConsumedTank().toString() << "\n";
            if (t->getTrainConsumedTank().get_keys().size() == 1) {
                trainStat
                << "        |_ Total Fuel Consumed per Net Weight (litter/ton)                      \x1D : " << Utils::thousandSeparator(t->getTrainConsumedTank().sumValues()/t->getCargoNetWeight()) << "\n";
            }
            trainStat
            << "        |_ Total Energy Consumption per ton.km (KW.h/ton.km)                    \x1D : " << Utils::thousandSeparator(t->cumEnergyStat / (t->getTrainTotalTorque())) << "\n"
            << "        |_ Total Energy Consumed by Region (Region:KW.h)                        \x1D : " << t->cumRegionalConsumedEnergyStat.toString() << "\n"
            << "        |_ Average Locomotives Battery Status (%)                               \x1D : " << Utils::thousandSeparator(t->getAverageLocomotivesBatteryStatus() * 100.0) << "\n\n"

            << "  |-> Statistics:\n"
            << "        |_ Total Delay Time To Each Link Speed                                  \x1D : " << Utils::formatDuration(t->cumDelayTimeStat) << "\n"
            << "        |_ Total Delay Time To Max Links speed                                  \x1D : " << Utils::formatDuration(t->cumMaxDelayTimeStat) << "\n"
            << "        |_ Total Stoppings                                                      \x1D : " << Utils::thousandSeparator(t->cumStoppedStat) << "\n"
            << "  |-> Locomotives Details:\n";
				// print the locomotives summary
				int locI = 1;
				for (auto &loc: t->locomotives){
                    trainStat << "        |_ Locomotive Number                                                     \x1D : " << locI << "\n"
                               << "        |_ Is Locomotive On                                                      \x1D : " << ((loc->isLocOn)? "true": "false") << "\n"
                               << "        |_ Power Type                                                            \x1D : " << TrainTypes::PowerTypeToStr(loc->powerType) << "\n";
					if (TrainTypes::locomotiveBatteryOnly.exist(loc->powerType) ||
							TrainTypes::locomotiveHybrid.exist(loc->powerType)) {
                        trainStat << "        |_ Battery Initial Charge (KW.h)                                         \x1D : " << Utils::thousandSeparator(loc->getBatteryInitialCharge()) << "\n"
                                   << "        |_ Battery Current Charge  at End of Trip (KW.h)                         \x1D : " << Utils::thousandSeparator(loc->getBatteryCurrentCharge()) << "\n"
                                   << "        |_ Battery Initial State of Charge (%)                                   \x1D : " << Utils::thousandSeparator(loc->getBatteryInitialCharge() / loc->getBatteryMaxCharge() * 100.0) << "\n"
                                   << "        |_ Battery Current State of Charge  at End of Trip (%)                   \x1D : " << Utils::thousandSeparator(loc->getBatteryStateOfCharge() * 100.0) << "\n"
                                   << "        |_ Battery Cumulative Consumed Energy (kW.h)                             \x1D : " << Utils::thousandSeparator(loc->getBatteryCumEnergyConsumption()) << "\n"
                                   << "        |_ Battery Cumulative Regenerated Energy (kW.h)                          \x1D : " << Utils::thousandSeparator(loc->getBatteryCumEnergyRegenerated()) << "\n"
                                   << "        |_ Battery Cumulative Net Consumed Energy (kW.h)                         \x1D : " << Utils::thousandSeparator(loc->getBatteryCumNetEnergyConsumption()) << "\n";
					}
					else if (TrainTypes::locomotiveTankOnly.exist(loc->powerType) ||
							 TrainTypes::locomotiveHybrid.exist(loc->powerType)) {
                        trainStat << "        |_ Tank Initial Capacity (liters)                                        \x1D : " << Utils::thousandSeparator(loc->getTankInitialCapacity()) << "\n"
                                   << "        |_ Tank Current Capacity at End of Trip (liters)                         \x1D : " << Utils::thousandSeparator(loc->getTankCurrentCapacity()) << "\n"
                                   << "        |_ Tank Initial State of Capacity (%)                                    \x1D : " << Utils::thousandSeparator(loc->getTankInitialCapacity() / loc->getTankMaxCapacity() * 100.0) << "\n"
                                   << "        |_ Tank Current State of Capacity at End of Trip (%)                     \x1D : " << Utils::thousandSeparator(loc->getTankStateOfCapacity() * 100.0) << "\n"
                                   << "        |_ Tank Cumulative Consumed Fuel (liters)                                \x1D : " << Utils::thousandSeparator(loc->getTankCumConsumedFuel()) << "\n";
					}
					locI ++;
				}
				// if we have tenders, print their summary
				if (t->carsTypes[TrainTypes::CarType::cargo].size() != t->cars.size()) {
                    trainStat << "    |-> Tenders Details:\n";
					int tenderI = 1;
					for (auto &car: t->cars){
						if (car->carType != TrainTypes::CarType::cargo) {
                            trainStat << "      |_ Tender Number                                                           \x1D : " << tenderI << "\n"
                                       << "        |_ Tender Type                                                           \x1D : " << TrainTypes::carTypeToStr(car->carType) << "\n";
							if (car->carType == TrainTypes::CarType::batteryTender){
                                trainStat << "        |_ Battery Initial Charge                                                \x1D : " << Utils::thousandSeparator(car->getBatteryInitialCharge()) << "\n"
                                           << "        |_ Battery Current Charge                                                \x1D : " << Utils::thousandSeparator(car->getBatteryCurrentCharge()) << "\n"
                                           << "        |_ Battery Current State of Charge (%)                                   \x1D : " << Utils::thousandSeparator(car->getBatteryStateOfCharge()) << "\n";
							}
							else {
                                trainStat << "        |_ Tank Initial Capacity                                                 \x1D : " << Utils::thousandSeparator(car->getTankInitialCapacity()) << "\n"
                                           << "        |_ Tank Current Capacity                                                 \x1D : " << Utils::thousandSeparator(car->getTankCurrentCapacity()) << "\n"
                                           << "        |_ Tank Current State of Capacity (%)                                    \x1D : " << Utils::thousandSeparator(car->getTankStateOfCapacity()) << "\n";
							}
							tenderI ++;
						}
					}
				}
                exportLine << trainStat.str();
                exportLine << "..............\n";

        }
	}

	exportLine.imbue(locale());
	// setup the summary file
    this->openSummaryFile();
    this->summaryFile << Utils::replaceAll(exportLine.str(), "\x1D", " ");
// ##################################################################
// #                       end: summary file                      #
// ##################################################################
	this->summaryFile.close();
	this->trajectoryFile.close();

    trainsSummaryData = Utils::splitStringStream(exportLine, "\x1D :");

    std::string trajectoryFilePath = "";
    if (this->exportTrajectory) {
        trajectoryFilePath = QDir(this->outputLocation).filePath(QString::fromStdString(this->trajectoryFilename)).toStdString();
    }

    emit this->finishedSimulation(trainsSummaryData, trajectoryFilePath);

}

bool Simulator::checkTrainsCollision() {
	Vector<std::pair<std::shared_ptr<Train>, std::shared_ptr<Train>>> comb;
	for (int i = 0; i < this->trains.size() - 1; i++) {
		for (int j = i + 1; j < this->trains.size(); j++) {
			std::shared_ptr < Train> tt = this->trains.at(i);
            comb.push_back( std::make_pair(this->trains.at(i),
                                          this->trains.at(j)));
		}
	}
	for (auto &t : comb) {
        if (t.first->loaded && t.second->loaded && !t.first->offloaded &&
            !t.second->offloaded && !t.first->reachedDestination &&
            !t.second->reachedDestination)
        {
            if (this->network->twoLinesIntersect(
                    t.first->startEndPoints[0],
                    t.first->startEndPoints[1],
                    t.second->startEndPoints[0],
                    t.second->startEndPoints[1])
                && (t.second->currentLinks.hasCommonElement(
                    t.first->currentLinks))
                && (!t.first->reachedDestination
                    && !t.second->reachedDestination)
                && (this->timeStep > t.first->trainStartTime
                    && this->timeStep > t.second->trainStartTime))
            {
                    std::string msg = "Train " + t.first->trainUserID +
                                      "and " + t.second->trainUserID +
                                      "collided!";
                    emit trainsCollided(msg);
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
            auto nodes= this->network->getNodeByID(
                                          t->trainPath[0])->linkTo.get_keys();
			auto targetN = this->network->getNodeByID(t->trainPath[1]);
			for (auto & n: nodes){
				if (n == targetN) {
					continue;
				}
			}
            // if the path is more than 2 nodes but the user wants the
            // simulator to decide the path.
            Vector<std::shared_ptr<NetNode>> nPath =
                this->network->shortestPathSearch(t->trainPath[0],
                                                  t->trainPath[1]).first;
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


void Simulator::runSignalsforTrains() {
    // turn on temporarly all signals in the network
    // the signals that should be turned off will be processed based on
    // the trains locations
	this->turnOnAllSignals();

    // loop over all train in the simulator
	for (auto& train : this->trains) {
        // if the train is not yet loaded or reached destination already, or
        // it ran out of fuel, skip this train
        if (!train->loaded || train->reachedDestination || train->offloaded)
        {
            continue;
        }

        // try to retreive the next signal for that train
        // the next from both ends of the train
		std::shared_ptr<NetSignal> nextSignal = this->getClosestSignal(train);
        std::shared_ptr<NetSignal> nextBackSignal = this->getClosestSignalToTrainEnd(train);
        if (nextSignal == nullptr && nextBackSignal == nullptr) {
            continue; // if no signal is found, the train is not approaching any signal
        }


        // define a holder for the signal groups
        std::shared_ptr<NetSignalGroupControllerWithQueuing> sgfront = nullptr;
        std::shared_ptr<NetSignalGroupControllerWithQueuing> sgback = nullptr;

        // process the train's front side next signal
        if (nextSignal != nullptr) {
            // if a signal is found, calculate the distance to the signal
            double dFront = this->network->getDistanceToSpecificNodeByTravelledDistance(train,
                                                                                        train->travelledDistance,
                                                                                        nextSignal->currentNode.lock()->id);

            // get the signal group controller at the current node
            if (this->signalsGroups.is_key(std::shared_ptr<NetNode>(nextSignal->currentNode))) {
                sgfront = this->signalsGroups.at(std::shared_ptr<NetNode>(nextSignal->currentNode));
                sgfront->clearTimeoutTrains(this->simulationTime);
            }

            // if no group controller was found, skip it
            if (sgfront != nullptr) {
                // if the train is within the critical zone of the signal,
                // add the train to the controller queue to process its request
                if (dFront <= nextSignal->proximityToActivate) {
                    sgfront->addTrain(train, this->simulationTime);
                }

                // get the signals in the path of the train (signals of this group controller only)
                Vector<std::shared_ptr<NetSignal>> sameDirSignals =
                    this->getSignalsInSameDirection(train, sgfront->getControllerSignals());

                // send signal to let the train pass
                sgfront->sendPassRequestToControlTo(train, nextSignal, this->simulationTime, sameDirSignals);

                // get feedback from the controller
                Vector<std::shared_ptr<NetSignal>> otherDirSignals = sgfront->getFeedback().second;

                // turn off the signals that should be off
                sgfront->turnOffSignals(otherDirSignals);
            }

        }

        // process the train's end side next signal
        if (nextBackSignal != nullptr) {
            double endTravelledDistance = train->travelledDistance - train->totalLength;
            if (endTravelledDistance < 0) {
                continue;
            }
            // if a signal is found, calculate the distance to the signal
            double dBack = this->network->getDistanceToSpecificNodeByTravelledDistance(train,
                                                                                       endTravelledDistance,
                                                                                       nextBackSignal->currentNode.lock()->id);

            // get the signal group controller at the current node
            if (this->signalsGroups.is_key(std::shared_ptr<NetNode>(nextBackSignal->currentNode))) {
                sgback = this->signalsGroups.at(std::shared_ptr<NetNode>(nextBackSignal->currentNode));
                sgback->clearTimeoutTrains(this->simulationTime);
            }
            // skip if the same signal group
            // the train's call was already processed once
            if (sgback == sgfront) {
                continue;
            }
            // if the train is within the critical zone of the signal,
            // add the train to the controller queue to process its request
            if (dBack <= nextBackSignal->proximityToActivate) {
                sgback->addTrain(train, this->simulationTime);
            }

            // get the signals in the path of the train (signals of this group controller only)
            Vector<std::shared_ptr<NetSignal>> sameDirSignals =
                this->getSignalsInSameDirection(train, sgback->getControllerSignals());

            // send signal to let the train pass
            sgback->sendPassRequestToControlTo(train, nextBackSignal, this->simulationTime, sameDirSignals);

            // get feedback from the controller
            Vector<std::shared_ptr<NetSignal>> otherDirSignals = sgback->getFeedback().second;

            // turn off the signals that should be off
            sgback->turnOffSignals(otherDirSignals);
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

// Check if there is a train on the given link
bool Simulator::checkTrainOnLinks(std::shared_ptr<Train> &train, Vector<std::shared_ptr<NetLink>>& links) {
	if (links.empty()) { return false; }
	std::set<std::shared_ptr<NetLink>> currenttrainLinks = getAccurateTrainCurrentLink(train);
	if (currenttrainLinks.empty()) { return false; }
	for (auto& link : currenttrainLinks) {
		if (links.exist(link)) { return true; }
	}
	return false;
}

// get the current links the train is spanning on
std::set<std::shared_ptr<NetLink>> Simulator::getAccurateTrainCurrentLink(std::shared_ptr<Train>& train) {
	std::set<std::shared_ptr<NetLink>> links = std::set<std::shared_ptr<NetLink>>();
	links.insert(this->network->getLinkFromDistance(train, train->travelledDistance, train->previousNodeID));
	double lastPoint = train->travelledDistance - train->totalLength;
	if (lastPoint > 0) {
		links.insert(this->network->getLinkFromDistance(train, lastPoint, train->previousNodeID));
	}
	return links;
}

// check if the given links have no trains on them
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

std::shared_ptr<NetSignal> Simulator::getClosestSignalToTrainEnd(std::shared_ptr<Train>& train) {
    // get the index of the next node from the train end side
    double backD = train->travelledDistance - train->totalLength;
    if (backD <= 0) {
        return nullptr;
    }

    int indx = train->trainPath.index(
                   this->network->getPreviousNodeByDistance(train,
                                                            backD,
                                                            train->previousNodeID)->id) + 1;

    // loop over all nodes to check next signal availability
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
    int progressValue = fraction * bar_length - 1;
    int progressPercent = (int)(fraction * 100);


//#ifdef AS_CMD
    std::stringstream bar;
    for (int i = 0; i < progressValue; i++) { bar << '-'; }
    bar << '>';
    bar << std::string(bar_length - progressValue, ' ');

    char ending = (current == total) ? '\n' : '\r';

    std::cout << "Progress: [" << bar.str() << "] " << progressPercent << "%" << ending;
//#endif

    if (progressPercent != this->progress) {
        this->progress = progressPercent;
        emit this->progressUpdated(this->progress);
    }
}

void Simulator::pauseSimulation() {
    mutex.lock();
    pauseFlag = true;
    mutex.unlock();
}

void Simulator::resumeSimulation() {
    mutex.lock();
    pauseFlag = false;
    mutex.unlock();
    pauseCond.wakeAll(); // This will wake up the thread
}
