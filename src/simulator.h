//
// Created by Ahmed Aredah
// Version 0.1
//

#ifndef NeTrainSim_Simulator_h
#define NeTrainSim_Simulator_h

#include <QObject>
#include "src/traindefinition/train.h"
#include "src/network/network.h"
#include "network/netsignalgroupcontroller.h"
#include "src/util/vector.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <memory>
#include <QDir>


/**
 * @class Simulator
 * @brief The Simulator class represents a simulation engine for train simulations.
 *        It manages the simulation process, including the trains, time step,
 *        output files, network, and progress. The Simulator class handles the
 *        execution of the simulation and provides functionality to export trajectory
 *        and summary files. It also allows for customization of simulation parameters
 *        such as the simulation end time and plot frequency. The Simulator class is
 *        derived from QObject and can be used with the Qt framework.
 *
 * @author Ahmed Aredah
 * @date 2/28/2023
 */
class Simulator : public QObject {
    Q_OBJECT
private:
	/** (Immutable) the default time step */
	static constexpr double DefaultTimeStep = 1.0;
	/** (Immutable) the default end time */
	static constexpr double DefaultEndTime = 0.0;
	/** (Immutable) true to default export instantaneous trajectory */
	static constexpr bool DefaultExportInstantaneousTrajectory = true;
	/** (Immutable) the default instantaneous trajectory empty filename */
	inline static const std::string DefaultInstantaneousTrajectoryEmptyFilename = "";
	/** (Immutable) the default summary empty filename */
	inline static const std::string DefaultSummaryEmptyFilename = "";
	/** (Immutable) the default instantaneous trajectory filename */
	inline static const std::string DefaultInstantaneousTrajectoryFilename = "trainTrajectory_";
	/** (Immutable) the default summary filename */
	inline static const std::string DefaultSummaryFilename =  "trainSummary_";
	/** (Immutable) true to optimize each train trajectory */
	static constexpr bool DefaultOptimizeSingleTrains = false;

private:
	/** The trains */
	Vector<std::shared_ptr<Train>> trains;
	/** The simulation time */
	double simulationTime;
	/** The simulation end time */
	double simulationEndTime;
	/** The time step */
	double timeStep;
    /** The frequency of plotting the trains */
    int plotFrequency;
	/** The output location */
    QString outputLocation;
	/** Filename of the summary file */
	std::string summaryFileName;
	/** Filename of the trajectory file */
	std::string trajectoryFilename;
	/** The network */
	Network* network;
	/** The progress */
	double progress;
	/** True to run simulation endlessly */
	bool runSimulationEndlessly;
	/** True to export trajectory */
	bool exportTrajectory;
	/** The trajectory file */
	std::ofstream trajectoryFile;
	/** The summary file */
	std::ofstream summaryFile;
	//Vector<Vector<Vector < std::shared_ptr<NetNode>>>> conflictTrainsIntersections;
	/** Groups the signals belongs to */
	Map<std::shared_ptr<NetNode>, std::shared_ptr<NetSignalGroupController>> signalsGroups;
	/** export individualized trains summary in the summary file*/
	bool exportIndividualizedTrainsSummary = false;

public:

    Vector<std::pair<std::string, std::string>> trainsSummaryData;

	/**
	 * @brief Simulator constructor
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param  [in,out]	theNetwork        The network.
     * @param  [in] 	networkTrains     The network trains.
     * @param  [in] 	simulatorTimeStep The simulator time step. Default value is 1.0
	 */
    explicit Simulator(Network *theNetwork, Vector<std::shared_ptr<Train>> networkTrains,
                       double simulatorTimeStep = DefaultTimeStep, QObject *parent = nullptr);

	/**
	 * @brief set simulator time step
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param newTimeStep
	 */
	void setTimeStep(double newTimeStep);

	/**
	 * @brief Get the output folder directory.
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @return the directory.
	 */
	std::string getOutputFolder();
	/**
	 * @brief set simulator end time.
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param newEndTime    the new end time of the simulator in seconds.
	 *                      Zero means do not stop untill all trains reach destination.
	 */
	void setEndTime(double newEndTime);

    /**
     * @brief set the plot frequency of trains, this only works in the gui
     * @param newPlotFrequency
     */
    void setPlotFrequency(int newPlotFrequency);

	/**
	 * @brief setOutputFolderLocation.
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param newOutputFolderLocation   the new output folder location on the disk.
	 */
	void setOutputFolderLocation(string newOutputFolderLocation);

	/**
	 * @brief set summary filename.
	 * @param newfilename           the new file name of the summary file.
	 */
	void setSummaryFilename(string newfilename = DefaultSummaryEmptyFilename);

	/**
	 * @brief setExportInstantaneousTrajectory
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param exportInstaTraject
	 * @param newInstaTrajectFilename
	 */
	void setExportInstantaneousTrajectory(bool exportInstaTraject, string newInstaTrajectFilename = DefaultInstantaneousTrajectoryEmptyFilename);


	/**
	 * Defines the signals grouping based on the min allowable distance between them.
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param [in,out]	minSafeDistance	- A reference to the variable holding the minimum safe
	 * 									distance value.
	 * 									</summary>
	 */
	void defineSignalsGroups(double& minSafeDistance);

	/**
	 * Gets links by nodes
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	nodes	The nodes.
	 *
	 * @returns	The links by nodes.
	 */
	Vector<std::shared_ptr<NetLink>> getLinksByNodes(std::set<std::shared_ptr<NetNode>> nodes);

	/**
	 * Gets closest signal
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param [in,out]	train	The train.
	 *
	 * @returns	The closest signal.
	 */
	std::shared_ptr<NetSignal> getClosestSignal(std::shared_ptr<Train>& train);

	/**
	 * Opens trajectory file
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 */
	void openTrajectoryFile();

	/**
	 * Opens summary file
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 */
	void openSummaryFile();

	/**
	 * Determines if we can check all trains reached destination
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @returns	True if it succeeds, false if it fails.
	 */
	bool checkAllTrainsReachedDestination();

	/**
	 * Executes the 'threaded one time step' operation
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param [in,out]	obj  	If non-null, the object.
	 * @param [in,out]	train	If non-null, the train.
	 */
	void runThreadedOneTimeStep(Simulator* obj, Train* train);

	/**
	 * Determines if we can check trains collision
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @returns	True if it succeeds, false if it fails.
	 */
	bool checkTrainsCollision();

	/**
	 * Play train one time step
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	train	The train.
	 */
	void playTrainOneTimeStep(std::shared_ptr <Train> train);

	/**
	 * Loads a train
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	train	The train.
	 */
	void loadTrain(std::shared_ptr<Train> train);

	/**
	 * Gets signal from link
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	networkSignals	The network signals.
	 * @param 	link		  	The link.
	 *
	 * @returns	The signal from link.
	 */
	int getSignalFromLink(Vector<std::shared_ptr<NetSignal>> networkSignals, std::shared_ptr<NetLink> link);

	/**
	 * Sets occupied links by trains
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	train	The train.
	 */
	void setOccupiedLinksByTrains(std::shared_ptr<Train> train);

	/**
	 * Gets all lower speeds i ds
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 		  	train			  	The train.
	 * @param [in,out]	previousNodeID	  	Identifier for the previous node.
	 * @param [in,out]	nextStoppingNodeID	Identifier for the next stopping node.
	 *
	 * @returns	all lower speeds i ds.
	 */
	Map<int, double> getAllLowerSpeedsIDs(std::shared_ptr<Train> train, int& previousNodeID, int& nextStoppingNodeID);

	/**
	 * Gets the ahead train and the gap between the current train and the ahead train.
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	train	The current train.
	 *
	 * @returns	a pointer to the ahead train and the gap between the current train and
	 *          the ahead train.
	 */
	std::pair<std::shared_ptr<Train>, double> getAheadTrainAndGap(std::shared_ptr <Train> train);

	/**
	 * Gets start end points
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	train			  	The train.
	 * @param 	currentCoordinates	The current coordinates.
	 *
	 * @returns	The start end points.
	 */
	Vector<std::pair<double, double>> getStartEndPoints(std::shared_ptr<Train> train, std::pair<double, double> currentCoordinates);

	/**
	 * Gets the next stopping node identifier
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 		  	train			 	The train.
	 * @param [in,out]	previousNodeIndex	Zero-based index of the previous node.
	 *
	 * @returns	The next stopping node identifier.
	 */
	std::pair<int, bool> getNextStoppingNodeID(std::shared_ptr<Train> train, int& previousNodeIndex);

	/**
	 * !
	 *  * \brief loadTrainLinksData
	 *  * \param train
	 *  * \return curvatures, grades, freeFlowSpeeds, links
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	train	 	The train.
	 * @param 	isVirtual	True if is virtual, false if not.
	 *
	 * @returns	The train links data.
	 */
	tuple<Vector<double>, Vector<double>, Vector<double>,
			Vector<std::shared_ptr<NetLink>>> loadTrainLinksData(std::shared_ptr <Train> train, bool isVirtual);

	/**
	 * Progress bar
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	current   	The current.
	 * @param 	total	  	Number of.
	 * @param 	bar_length	(Optional) Length of the bar.
	 */
	void ProgressBar(double current, double total, int bar_length = 100);

	/**
	 * Loads train free speed
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	train	The train.
	 *
	 * @returns	The train free speed.
	 */
	double loadTrainFreeSpeed(std::shared_ptr<Train> train);

	/**
	 * Sets train simulator path
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 */
	void setTrainSimulatorPath();

	/**
	 * Sets trains nodes from their path.
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 */
	void setTrainsPathNodes();

	/**
	 * Sets train path length
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 */
	void setTrainPathLength();

	/**
	 * Executes the 'signalsfor trains' operation
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 */
	void runSignalsforTrains();

    /**
     * @brief checkNoTrainIsOnNetwork
     * @return
     */
    bool checkNoTrainIsOnNetwork();

    /**
     * @brief getNotLoadedTrainsMinStartTime
     * @return
     */
    double getNotLoadedTrainsMinStartTime();

	/**
	 * Play train virtual steps a star optimization
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	train   	The train.
	 * @param 	timeStep	The time step.
	 */
	void PlayTrainVirtualStepsAStarOptimization(std::shared_ptr<Train> train, double timeStep);

	/**
	 * Gets export individualized trains summary
	 *
	 * @author	Ahmed Aredah
	 * @date	3/20/2023
	 *
	 * @returns	True if it succeeds, false if it fails.
	 */
	bool getExportIndividualizedTrainsSummary() const;

	/**
	 * Sets export individualized trains summary
	 *
	 * @author	Ahmed Aredah
	 * @date	3/20/2023
	 *
	 * @param 	newExportIndividualizedTrainsSummary	True to new export individualized trains
	 * 													summary.
	 */
	void setExportIndividualizedTrainsSummary(bool newExportIndividualizedTrainsSummary);

private:

	/**
	 * Turn on all signals
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 */
	void turnOnAllSignals();

	/**
	 * Turn off signal
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	networkSignals	The network signals.
	 */
	void turnOffSignal(Vector<std::shared_ptr<NetSignal>> networkSignals);

	/**
	 * Check links are free
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param [in,out]	links	The links.
	 *
	 * @returns	True if it succeeds, false if it fails.
	 */
	bool checkLinksAreFree(Vector<std::shared_ptr<NetLink>> &links);

	/**
	 * Check train on links
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param [in,out]	train	The train.
	 * @param [in,out]	links	The links.
	 *
	 * @returns	True if it succeeds, false if it fails.
	 */
	bool checkTrainOnLinks(std::shared_ptr<Train>& train, Vector<std::shared_ptr<NetLink>>& links);

	/**
	 * Gets accurate train current link
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param [in,out]	train	The train.
	 *
	 * @returns	The accurate train current link.
	 */
	std::set<std::shared_ptr<NetLink>> getAccurateTrainCurrentLink(std::shared_ptr<Train>& train);

	/**
	 * Gets signals in same direction
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param [in,out]	train				The train.
	 * @param 		  	SignalsGroupList	List of signals groups.
	 *
	 * @returns	The signals in same direction.
	 */
	Vector<std::shared_ptr<NetSignal>> getSignalsInSameDirection(std::shared_ptr<Train>& train,
		Vector<std::shared_ptr<NetSignal>> SignalsGroupList);

	/**
	 * Gets nodes intersections for signals
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param [in,out]	minSafeDistance	The minimum safe distance.
	 *
	 * @returns	The nodes intersections for signals.
	 */
	Vector<std::set<std::shared_ptr<NetNode>>> getNodesIntersectionsForSignals(double& minSafeDistance);

	/**
	 * Gets trains intersections
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	train1	The first train.
	 * @param 	train2	The second train.
	 *
	 * @returns	The trains intersections.
	 */
	Vector<std::shared_ptr<NetNode>> getTrainsIntersections(std::shared_ptr<Train> train1, std::shared_ptr<Train> train2);

	/**
	 * Gets conflict trains intersections
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @returns	The conflict trains intersections.
	 */
	Vector<Vector<Vector < std::shared_ptr<NetNode>>>> getConflictTrainsIntersections();

	/**
	 * Gets node groups for signals
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 		  	ConflictTrainsIntersections	The conflict trains intersections.
	 * @param [in,out]	minSafeDistance			   	The minimum safe distance.
	 *
	 * @returns	The node groups for signals.
	 */
	Vector<Vector< Map<int, std::set<std::shared_ptr<NetNode>>>>> getNodeGroupsForSignals(Vector<Vector<Vector < std::shared_ptr<NetNode>>>> ConflictTrainsIntersections, double& minSafeDistance);

	/**
	 * Union nodes intersections for signals
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	nodesGroups	Groups the nodes belongs to.
	 *
	 * @returns	A Vector&lt;std::set&lt;std::shared_ptr&lt;NetNode&gt;&gt;&gt;
	 */
	Vector<std::set<std::shared_ptr<NetNode>>> unionNodesIntersectionsForSignals(Vector<Vector< Map<int, std::set<std::shared_ptr<NetNode>>>>> nodesGroups);

	/**
	 * Removes the redundancy described by unified
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	unified	The unified.
	 *
	 * @returns	A Vector&lt;std::set&lt;std::shared_ptr&lt;NetNode&gt;&gt;&gt;
	 */
	Vector<std::set<std::shared_ptr<NetNode>>> removeRedundancy(Vector<std::set<std::shared_ptr<NetNode>>> unified);

	/**
	 * Calculates the signals proximities
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 */
	void calculateSignalsProximities();

public: signals:
    /**
     * @brief Updates the progress of the simulation.
     *
     * @param progressPercentage The progress of the simulation as a percentage.
     */
    void progressUpdated(int progressPercentage);

    /**
     * @brief Updates the plot of trains with their start and end points.
     *
     * @param trainsStartEndPoints A vector containing the names of the trains
     *                            along with their start and end points.
     */
    void plotTrainsUpdated(Vector<std::pair<std::string, Vector<std::pair<double, double>>>> trainsStartEndPoints);

    /**
     * @brief Signals that the simulation has finished.
     *
     * @param summaryData   A vector containing the summary data of the simulation.
     * @param trajectoryFile The file path of the generated trajectory file.
     */
    void finishedSimulation(const Vector<std::pair<std::string, std::string>>& summaryData, const std::string& trajectoryFile);

public slots:
    /**
     * Executes the 'simulator' operation
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     */
    void runSimulation();

};
#endif // !NeTrainSim_Simulator_h
