//
// Created by Ahmed Aredah
// Version 0.1
//

#ifndef NeTrainSim_Simulator_h
#define NeTrainSim_Simulator_h

#include "TrainDefintion/Train.h"
#include "TrainDefintion/TrainsList.h"
#include "network/Network.h"
#include "network/NetSignalGroupController.h"
#include "./util/Vector.h"
#include <string>
#include <iostream>
#include <filesystem>

class Simulator {
private:
	static constexpr double DefaultTimeStep = 1.0;
	static constexpr double DefaultEndTime = 0.0;
	static constexpr bool DefaultExportInstantaneousTrajectory = true;
	inline static const std::string DefaultOutputFolder = "";
	inline static const std::string DefaultInstantaneousTrajectoryEmptyFilename = "";
	inline static const std::string DefaultSummaryEmptyFilename = "";
	inline static const std::string DefaultInstantaneousTrajectoryFilename = "NeTrainSim_TrainTrajectory.csv";
	inline static const std::string DefaultSummaryFilename =  "NeTrainSim_TrainSummary.txt";

private:
	Vector<std::shared_ptr<Train>> trains;
	double simulationTime;
	double simulationEndTime;
	double timeStep;
	std::filesystem::path outputLocation;
	std::string summaryFileName;
	std::string trajectoryFilename;
	Network* network;
	double progress;
	bool runSimulationEndlessly;
	bool exportTrajectory;
	std::ofstream trajectoryFile;
	std::ofstream summaryFile;
	//Vector<Vector<Vector < std::shared_ptr<NetNode>>>> conflictTrainsIntersections;
	Map<std::shared_ptr<NetNode>, std::shared_ptr<NetSignalGroupController>> signalsGroups;

public:
	Simulator(Network& theNetwork, Vector<std::shared_ptr<Train>> networkTrains, double simulationEndTime_ = DefaultEndTime,
		double simulationTimeStep_ = DefaultTimeStep,
		bool exportInstantaneousTrajectory = DefaultExportInstantaneousTrajectory,
		string outputFolderLocation = DefaultOutputFolder,
		string instantaneousTrajectoryFilename = DefaultInstantaneousTrajectoryEmptyFilename,
		string summaryFilename = DefaultSummaryEmptyFilename);

	/// <summary>
	/// Defines the signals grouping based on the min allowable distance between them. 
	/// @param minSafeDistance - A reference to the variable holding the minimum safe distance value.
	/// </summary>
	void defineSignalsGroups(double& minSafeDistance);
	Vector<std::shared_ptr<NetLink>> getLinksByNodes(std::set<std::shared_ptr<NetNode>> nodes);

	std::shared_ptr<NetSignal> getClosestSignal(std::shared_ptr<Train>& train);
	void openTrajectoryFile();
	void openSummaryFile();
	bool checkAllTrainsReachedDestination();
	void runThreadedOneTimeStep(Simulator* obj, Train* train);
	bool checkTrainsCollision();
	void playTrainOneTimeStep(std::shared_ptr <Train> train);
	void loadTrain(std::shared_ptr<Train> train);
	int getSignalFromLink(Vector<std::shared_ptr<NetSignal>> networkSignals, std::shared_ptr<NetLink> link);
	void setOccupiedLinksByTrains(std::shared_ptr<Train> train);
	Map<int, double> getAllLowerSpeedsIDs(std::shared_ptr<Train> train, int& previousNodeID, int& nextStoppingNodeID);
	std::pair<std::shared_ptr<Train>, double> getTrainAndDistanceToTrainAhead(std::shared_ptr <Train> train);
	Vector<std::pair<double, double>> getStartEndPoints(std::shared_ptr<Train> train, std::pair<double, double> currentCoordinates);
	std::pair<int, bool> getNextStoppingNodeID(std::shared_ptr<Train> train, int& previousNodeIndex);
	std::tuple<Vector<double>, Vector<double>, Vector<double>, Vector<std::shared_ptr<NetLink>>> loadTrainLinksData(std::shared_ptr <Train> train);
	void ProgressBar(double current, double total, int bar_length = 100);
	double loadTrainFreeSpeed(std::shared_ptr<Train> train);
	void runSimulator();
	void setTrainSimulatorPath();
	void setTrainsStoppingStations();
	void setTrainPathLength();
	void runSignalsforTrains();

private:
	void turnOnAllSignals();
	void turnOffSignal(Vector<std::shared_ptr<NetSignal>> networkSignals);
	bool checkLinksAreFree(Vector<std::shared_ptr<NetLink>> &links);
	bool checkTrainOnLinks(std::shared_ptr<Train>& train, Vector<std::shared_ptr<NetLink>>& links);
	std::set<std::shared_ptr<NetLink>> getAccurateTrainCurrentLink(std::shared_ptr<Train>& train);
	Vector<std::shared_ptr<NetSignal>> getSignalsInSameDirection(std::shared_ptr<Train>& train,
		Vector<std::shared_ptr<NetSignal>> SignalsGroupList);
	Vector<std::set<std::shared_ptr<NetNode>>> getNodesIntersectionsForSignals(double& minSafeDistance);
	Vector<std::shared_ptr<NetNode>> getTrainsIntersections(std::shared_ptr<Train> train1, std::shared_ptr<Train> train2);
	Vector<Vector<Vector < std::shared_ptr<NetNode>>>> getConflictTrainsIntersections();
	Vector<Vector< Map<int, std::set<std::shared_ptr<NetNode>>>>> getNodeGroupsForSignals(Vector<Vector<Vector < std::shared_ptr<NetNode>>>> ConflictTrainsIntersections, double& minSafeDistance);
	Vector<std::set<std::shared_ptr<NetNode>>> unionNodesIntersectionsForSignals(Vector<Vector< Map<int, std::set<std::shared_ptr<NetNode>>>>> nodesGroups);
	Vector<std::set<std::shared_ptr<NetNode>>> removeRedundancy(Vector<std::set<std::shared_ptr<NetNode>>> unified);
	void calculateSignalsProximities();
};
#endif // !NeTrainSim_Simulator_h