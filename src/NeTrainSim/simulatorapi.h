/**
 * @mainpage SimulatorAPI Documentation
 * @brief A comprehensive API for train simulation with
 * support for interactive and continuous simulation modes.
 *
 * @section overview Overview
 * SimulatorAPI provides a thread-safe, event-driven
 * interface for train simulation with support for multiple
 * networks, trains, and simulation modes.
 *
 * Key Features:
 * - Multi-threaded simulation support
 * - Interactive and Continuous simulation modes
 * - Network and train management
 * - Container tracking (when BUILD_SERVER_ENABLED is
 * defined)
 *
 * @section architecture Architecture
 * The API uses a singleton pattern with two primary
 * operation modes:
 * - Interactive Mode: Step-by-step simulation control
 * - Continuous Mode: Continuous simulation with
 * pause/resume capabilities
 *
 * @section examples Usage Examples
 *
 * Interactive Mode Example:
 * @code{.cpp}
 * // Load network and create simulation
 * auto* network =
 *      SimulatorAPI::InteractiveMode::
 *          createNewSimulationEnvironmentFromFiles(
 *                   "nodes.json", "links.json",
 * "myNetwork", "trains.json");
 *
 * // Run step by step
 * SimulatorAPI::InteractiveMode::runSimulation({"myNetwork"},
 * 1.0, true);
 * @endcode
 *
 * Continuous Mode Example:
 * @code{.cpp}
 * // Continuous execution
 * SimulatorAPI::ContinuousMode::createNewSimulationEnvironment(
 *     "nodes.json", "links.json", "myNetwork", {}, 1.0,
 *     SimulatorAPI::Mode::Async);
 * SimulatorAPI::ContinuousMode::runSimulation({"myNetwork"},
 * true);
 * @endcode
 *
 * @section error_handling Error Handling
 * The API uses signal-based error reporting:
 * - Connect to `errorOccurred` signal for error
 * notifications
 * - All methods that can fail emit detailed error messages
 * - Async operations report completion via specific signals
 */

#ifndef SIMULATORAPI_H
#define SIMULATORAPI_H

// Necessary headers
#include "export.h"
#include "network/network.h"
#include "requestdata.h"
#include "simulator.h"
#include "simulatorworker.h"
#include "threadsafeapidatamap.h"
#include "traindefinition/trainscommon.h"
#include "util/vector.h"
#include <QObject>
#include <any>
#include <memory>

/**
 * @class NoThousandsSeparator
 * @brief Custom locale facet to remove thousand separators
 * from numeric output
 *
 * @details This class customizes number formatting by
 * removing thousand separators (e.g., converts "1,000" to
 * "1000"). Used internally for consistent numeric string
 * representation across different locales.
 *
 * @note Inherits from std::numpunct<char> to modify numeric
 * punctuation behavior
 */
class NoThousandsSeparator : public std::numpunct<char>
{
protected:
    /**
     * @brief Defines the thousand separator character
     * @return '\0' to indicate no separator
     */
    char do_thousands_sep() const override
    {
        return '\0'; // No separator
    }

    /**
     * @brief Defines the grouping behavior for digits
     * @return Empty string to disable digit grouping
     */
    std::string do_grouping() const override
    {
        return ""; // No grouping
    }
};

/**
 * @class SimulatorAPI
 * @brief Main API class providing simulation control and
 * management
 *
 * The SimulatorAPI class is implemented as a singleton and
 * provides two operation modes through
 * SimulatorAPI::InteractiveMode and
 * SimulatorAPI::ContinuousMode.
 */
class NETRAINSIMCORE_EXPORT SimulatorAPI : public QObject
{
    Q_OBJECT
    friend struct std::default_delete<
        SimulatorAPI>; // Allow unique_ptr to delete
public:
    /**
     * @enum Mode
     * @brief Defines the operation mode for signal emission
     *
     * @var Mode::Async
     * Signals are emitted only when all simulators/networks
     * reach the same point
     *
     * @var Mode::Sync
     * Signals are emitted immediately for each
     * simulator/network
     */
    enum class Mode
    {
        Async,
        Sync
    };

signals:
    /**
     * @brief Emitted when a new simulation environment is
     * successfully created.
     * @param networkName The name of the network for which
     * the simulation was created.
     * @details This signal indicates that the simulation
     * setup has been completed and is ready for execution.
     */
    void simulationCreated(QString networkName);

    /**
     * @brief Emitted when simulations are paused
     * (Continuous Mode only).
     * @param networkNames List of network names whose
     * simulations were paused.
     * @details In Async mode, this signal is emitted only
     * when all specified networks are paused. In Sync mode,
     * it is emitted as soon as each individual network is
     * paused.
     */
    void simulationsPaused(QVector<QString> networkNames);

    /**
     * @brief Emitted when paused simulations are resumed
     * (Continuous Mode only).
     * @param networkNames List of network names whose
     * simulations were resumed.
     * @details In Async mode, this signal is emitted only
     * when all specified networks are resumed. In Sync
     * mode, it is emitted as soon as each individual
     * network is resumed.
     */
    void simulationsResumed(QVector<QString> networkNames);

    /**
     * @brief Emitted when simulations are restarted from
     * their initial state.
     * @param networkNames List of network names whose
     * simulations were restarted.
     * @details This signal is emitted after all resources
     * are reset, and the simulations are restarted from
     * their initial configurations.
     */
    void
    simulationsRestarted(QVector<QString> networkNames);

    /**
     * @brief Emitted when simulations are forcefully
     * terminated.
     * @param networkNames List of network names whose
     * simulations were terminated.
     * @details In Async mode, this signal is emitted only
     * when all specified networks are terminated. In Sync
     * mode, it is emitted as soon as each individual
     * network is terminated.
     */
    void
    simulationsTerminated(QVector<QString> networkNames);

    /**
     * @brief Emitted when simulations complete their
     * execution naturally.
     * @param networkName The name of the network whose
     * simulation finished.
     * @details This signal differs from
     * `simulationsTerminated`, which indicates forced
     * termination. It signifies the normal completion of a
     * simulation.
     */
    void simulationFinished(QString networkName);

    /**
     * @brief Emitted to update the progress of a
     * simulation.
     * @param networkName The name of the network whose
     * progress is being updated.
     * @param simulatorProgress A pair consisting of the
     * current simulation time and the progress percentage
     * (0-100).
     * @details This signal provides periodic updates on
     * simulation progress.
     */
    void simulationProgressUpdated(
        QString            networkName,
        QPair<double, int> simulatorProgress);

    /**
     * @brief Emitted when the simulation advances by one
     * step.
     * @param currentSimulorTimePairs Map of network names
     * to their current simulation time and progress
     * percentage.
     * @details This signal reports the simulation time
     * advancement for each network.
     */
    void
    simulationAdvanced(QMap<QString, QPair<double, double>>
                           currentSimulorTimePairs);

    /**
     * @brief Emitted when simulations reach a specific
     * reporting time.
     * @param currentSimulorTimeProgressPairs Map of network
     * names to their current simulation time and progress
     * percentage.
     * @details This signal is emitted when the simulation
     * reaches pre-defined reporting milestones.
     */
    void simulationReachedReportingTime(
        QMap<QString, QPair<double, double>>
            currentSimulorTimeProgressPairs);

    /**
     * @brief Emitted when all trains in the network reach
     * their destinations
     * @param networkName The name of the network containing
     * the trains.
     */
    void allTrainsReachedDestination(QString networkName);

    /**
     * @brief Emitted when trains reach their destinations.
     * @param networkName The name of the network containing
     * the trains.
     * @param trainState JSON object containing the states
     * of trains that reached their destinations.
     * @details This signal provides the final states of
     * trains upon arrival at their intended destinations.
     */
    void trainsReachedDestination(QString     networkName,
                                  QJsonObject trainState);

    /**
     * @brief Emitted when a train's position is updated
     * during simulation.
     * @param networkName The name of the network containing
     * the train.
     * @param trainsCoord A vector containing pairs of train
     * IDs and their updated coordinates.
     * @details This signal provides real-time updates on
     * the positions of trains during the simulation.
     */
    void trainsCoordinatesUpdated(
        QString networkName,
        QVector<
            QPair<QString, QVector<QPair<double, double>>>>
            trainsCoord);

    /**
     * @brief Emitted when new trains are successfully added
     * to a simulation.
     * @param networkName The name of the network where
     * trains were added.
     * @param trainIDs List of IDs of the newly added
     * trains.
     * @details This signal indicates the successful
     * addition of trains to a specific simulation network.
     */
    void trainsAddedToSimulation(
        const QString          networkName,
        const QVector<QString> trainIDs);

    /**
     * @brief Emitted when simulation results are ready for
     * retrieval.
     * @param networkName The name of the network providing
     * the results.
     * @param results Object containing the simulation
     * results.
     * @details This signal is emitted when the simulation
     * has completed or reached a reporting milestone,
     * making results available for analysis.
     */
    void simulationResultsAvailable(QString networkName,
                                    TrainsResults &results);

    /**
     * @brief Emitted when an error occurs during any
     * operation.
     * @param error Description of the error that occurred.
     * @details This signal provides detailed error messages
     * for handling issues during simulation setup,
     * execution, or termination.
     */
    void errorOccurred(QString error);

    /**
     * @brief Emitted when containers are successfully added
     * to a train.
     * @param networkName The name of the network containing
     * the train.
     * @param trainID The unique identifier of the train to
     * which containers were added.
     * @details This signal indicates the successful
     * addition of containers to a train in the simulation.
     */
    void containersAddedToTrain(QString networkName,
                                QString trainID);

    /**
     * @brief Emitted when a train reaches a terminal.
     * @param networkName The name of the network containing
     * the train.
     * @param trainID The unique identifier of the train.
     * @param terminalID The unique identifier of the
     * terminal reached.
     * @param containersCount The number of containers
     * carried by the train.
     * @details This signal indicates the arrival of a train
     * at a specific terminal and provides details about its
     * cargo.
     */
    void trainReachedTerminal(QString networkName,
                              QString trainID,
                              QString terminalID,
                              int     containersCount);

    /**
     * @brief Emitted when a train unloads containers at a
     * terminal.
     * @param networkName The name of the network containing
     * the train.
     * @param trainID The unique identifier of the train.
     * @param terminalID The unique identifier of the
     * terminal.
     * @param containers JSON array containing details of
     * the containers being unloaded.
     * @details This signal provides information about the
     * unloading of containers at a terminal. It is only
     * available when BUILD_SERVER_ENABLED is defined.
     */
    void ContainersUnloaded(QString    networkName,
                            QString    trainID,
                            QString    terminalID,
                            QJsonArray containers);

    /**
     * @brief Emitted when the state of a train becomes
     * available.
     * @param networkName The name of the network containing
     * the train.
     * @param trainID The unique identifier of the train.
     * @param trainState JSON object containing the current
     * state of the train.
     * @details This signal provides detailed state
     * information about a train during the simulation.
     */
    void trainStateAvailable(QString           networkName,
                             QString           trainID,
                             const QJsonObject trainState);

    /**
     * @brief Emitted when all worker threads complete their
     * assigned tasks.
     * @param networkNames List of networks whose workers
     * have completed.
     * @details In Async mode, this signal is emitted only
     * when all workers are done. In Sync mode, it is
     * emitted for each worker completion.
     */
    void workersReady(QVector<QString> networkNames);

protected:
    static Mode mMode;

    /** @brief Map of network names to their simulation data
     */
    ThreadSafeAPIDataMap apiDataMap;

    /** @brief Connection type for Qt signals/slots */
    Qt::ConnectionType mConnectionType =
        Qt::QueuedConnection;

    /** @brief Tracks simulation time steps and progress */
    RequestData<QPair<double, double>> mTimeStepTracker;

    /** @brief Track trains reached destination */
    RequestData<QJsonObject> mReachedDesTracker;

    /** @brief Tracks pause operations */
    RequestData<QString> mPauseTracker;

    /** @brief Tracks resume operations */
    RequestData<QString> mResumeTracker;

    /** @brief Tracks termination operations */
    RequestData<QString> mTerminateTracker;

    /** @brief Tracks simulation restart operations */
    RequestData<QString> mRestartTracker;

    /** @brief Tracks worker thread status */
    RequestData<QString> mWorkerTracker;

    /**
     * @brief Get the singleton instance of SimulatorAPI.
     * @return Reference to the singleton instance.
     * @details Provides a single global instance of the
     * API, ensuring no multiple instances exist. This is
     * implemented in a thread-safe manner.
     */
    static SimulatorAPI &getInstance();

    /**
     * @brief Reset the API instance to its initial state.
     * @details Cleans up all allocated resources, resets
     * all networks, simulators, and workers, and creates a
     * new instance. This is useful for complete
     * reinitialization.
     */
    static void resetInstance();

    /**
     * @brief Destructor for SimulatorAPI.
     * @details Cleans up resources such as worker threads,
     * simulators, networks, and other allocated memory used
     * during the simulation lifecycle.
     */
    ~SimulatorAPI();

    /**
     * @brief Default constructor for the SimulatorAPI
     * class.
     * @details Initializes the SimulatorAPI instance. This
     * constructor is protected to enforce the singleton
     * pattern, ensuring that the instance is created and
     * accessed only through the `getInstance` method.
     */
    SimulatorAPI() = default;

    /**
     * @brief Deleted copy constructor to prevent copying of
     * the SimulatorAPI instance.
     * @details This ensures that the singleton pattern is
     * enforced, and no additional instances can be created
     * by copying the existing instance.
     */
    SimulatorAPI(const SimulatorAPI &) = delete;

    /**
     * @brief Deleted assignment operator to prevent
     * assignment of the SimulatorAPI instance.
     * @details This ensures that the singleton instance
     * cannot be reassigned or duplicated through
     * assignment.
     */
    SimulatorAPI &operator=(const SimulatorAPI &) = delete;

    /**
     * @brief Set up connections for the specified network.
     * @param networkName Name of the network for which
     * connections are to be established.
     * @param mode Operation mode (Async or Sync).
     * @details Establishes signal-slot connections between
     * the simulator, trains, and API components for the
     * specified network. These connections are essential
     * for handling events such as progress updates, train
     * state changes, and simulation completion.
     *
     * - In Async mode, connections are set up to wait for
     * all networks to synchronize.
     * - In Sync mode, connections allow immediate
     * processing of events as they occur.
     */
    void setupConnections(const QString &networkName,
                          Mode           mode);

    /**
     * @brief Set up signal-slot connections for trains in
     * the simulation.
     * @param trains Vector of shared pointers to Train
     * objects.
     * @param networkName Name of the network associated
     * with these trains.
     * @param mode Operation mode, either Async or Sync.
     * @details This function establishes connections
     * between the trains and the API for simulation
     * updates, ensuring proper communication for events
     * like position updates or state changes.
     */
    void setupTrainsConnection(
        QVector<std::shared_ptr<Train>> trains,
        QString networkName, Mode mode);

    /**
     * @brief Initialize and configure the simulator for the
     * specified network.
     * @param networkName Name of the network for which the
     * simulator is being set up.
     * @param nodeRecords Vector of node data in the form of
     * QMap.
     * @param linkRecords Vector of link data in the form of
     * QMap.
     * @param trainList Vector of train configurations, with
     * each train represented as QMap.
     * @param timeStep Time step size for the simulation,
     * specified in seconds.
     * @param mode Operation mode, either Async or Sync.
     * @details This function sets up the simulator
     * environment by loading the network nodes, links, and
     * trains, and configuring them for simulation according
     * to the provided parameters.
     */
    void setupSimulator(
        QString                          &networkName,
        QVector<QMap<QString, QString>>  &nodeRecords,
        QVector<QMap<QString, QString>>  &linkRecords,
        QVector<QMap<QString, std::any>> &trainList,
        double timeStep, Mode mode);

    /**
     * @brief Emit the specified signal if conditions are
     * met.
     * @param counter The current count of completed
     * operations.
     * @param total The total number of expected operations.
     * @param networkNames List of network names associated
     * with the operation.
     * @param signal Pointer to the member signal that
     * should be emitted.
     * @param mode Operation mode, either Async or Sync.
     * @return true if the signal was emitted; false
     * otherwise.
     * @details Ensures the specified signal is emitted only
     * when conditions are satisfied. In Async mode, waits
     * for all networks to reach the required state. In Sync
     * mode, emits the signal as soon as the state is
     * reached.
     */
    void checkAndEmitSignal(
        const int &counter, const int total,
        const QVector<QString> &networkNames,
        void (SimulatorAPI::*signal)(QVector<QString>),
        Mode mode);

    // Simulator control methods

    /**
     * @brief Create a new simulation environment using
     * network content.
     * @param nodesFileContent Content of the file defining
     * the network nodes.
     * @param linksFileContent Content of the file defining
     * the network links.
     * @param networkName Name of the network to create.
     * Defaults to "*".
     * @param trainList List of train configurations
     * (optional).
     * @param timeStep Duration of each simulation time step
     * (in seconds). Defaults to 1.0.
     * @param mode Synchronization mode (Async or Sync).
     * Defaults to Async.
     * @details This method initializes a new simulation
     * environment by parsing the provided network content
     * and train data. The simulation is prepared for
     * execution but does not begin until explicitly
     * started.
     */
    void createNewSimulationEnvironment(
        QJsonObject                      nodesFileContent,
        QJsonObject                      linksFileContent,
        QString                          networkName = "*",
        QVector<QMap<QString, std::any>> trainList =
            QVector<QMap<QString, std::any>>(),
        double timeStep = 1.0, Mode mode = Mode::Async);

    /**
     * @brief Create a new simulation environment using
     * network and train files.
     * @param nodesFile Path to the file defining network
     * nodes.
     * @param linksFile Path to the file defining network
     * links.
     * @param networkName Name of the network to create.
     * @param trainsFile Path to the file defining train
     * configurations.
     * @param timeStep Duration of each simulation time step
     * (in seconds). Defaults to 1.0.
     * @param mode Synchronization mode (Async or Sync).
     * Defaults to Async.
     * @details Reads network and train data from the
     * specified files, initializes the network, and
     * prepares the simulation environment for execution.
     */
    void createNewSimulationEnvironmentFromFiles(
        QString nodesFile, QString linksFile,
        QString networkName, QString trainsFile,
        double timeStep = 1.0, Mode mode = Mode::Async);

    /**
     * @brief Create a new simulation environment using
     * in-memory network and train data.
     * @param networkName Name of the network to create.
     * @param nodeRecords List of nodes with their
     * attributes as key-value pairs.
     * @param linkRecords List of links with their
     * attributes as key-value pairs.
     * @param trainList List of train configurations with
     * attributes as key-value pairs.
     * @param timeStep Duration of each simulation time step
     * (in seconds). Defaults to 1.0.
     * @param mode Synchronization mode (Async or Sync).
     * Defaults to Async.
     * @details Creates and initializes a network and train
     * simulation environment directly from in-memory data
     * structures, bypassing the need for file I/O.
     */
    void createNewSimulationEnvironment(
        QString                          networkName,
        QVector<QMap<QString, QString>> &nodeRecords,
        QVector<QMap<QString, QString>> &linkRecords,
        QVector<QMap<QString, any>>     &trainList,
        double timeStep = 1.0, Mode mode = Mode::Async);

    /**
     * @brief Request current results for active
     * simulations.
     * @param networkNames List of networks for which
     * results are requested.
     * @details This method triggers the retrieval of
     * current simulation results for the specified
     * networks. Results are provided asynchronously via the
     * `simulationResultsAvailable` signal.
     */
    void requestSimulationCurrentResults(
        QVector<QString> networkNames);

    // Train control methods

    /**
     * @brief Add trains to an existing simulation.
     * @param networkName Name of the network where the
     * trains will be added.
     * @param trains List of train objects to add to the
     * simulation.
     * @details This method allows dynamic addition of
     * trains to an active simulation, updating the
     * simulation environment accordingly.
     * @throws Emits `errorOccurred` if the network does not
     * exist or if train addition fails.
     */
    void addTrainToSimulation(
        QString                         networkName,
        QVector<std::shared_ptr<Train>> trains);

    // GETTERS

    /**
     * @brief Get the simulator instance for a specific
     * network.
     * @param networkName Name of the network whose
     * simulator is requested.
     * @return Pointer to the Simulator object, or nullptr
     * if not found.
     * @details Provides access to the simulator for the
     * specified network, enabling direct interaction with
     * its functionality.
     */
    Simulator *getSimulator(QString networkName);

    /**
     * @brief Get the network instance for a specific name.
     * @param networkName Name of the network to retrieve.
     * @return Pointer to the Network object, or nullptr if
     * not found.
     * @details Returns the initialized network associated
     * with the given name.
     */
    Network *getNetwork(QString networkName);

    /**
     * @brief Get a train by its unique identifier.
     * @param networkName Name of the network containing the
     * train.
     * @param trainID Unique identifier of the train.
     * @return Shared pointer to the Train object, or
     * nullptr if not found.
     * @details Allows retrieval of a specific train for
     * inspection or modification.
     */
    std::shared_ptr<Train> getTrainByID(QString networkName,
                                        QString &trainID);

    /**
     * @brief Get all trains in a specified network.
     * @param networkName Name of the network containing the
     * trains.
     * @return Vector of shared pointers to all Train
     * objects in the network.
     * @details Provides access to the list of all trains
     * within a given network.
     */
    QVector<std::shared_ptr<Train>>
    getAllTrains(QString networkName);

    /**
     * @brief Add containers to a specific train.
     * @param networkName Name of the network containing the
     * train.
     * @param trainID Unique identifier of the train.
     * @param json JSON object describing the container
     * configurations.
     * @details This method allows adding containers to a
     * train for freight simulation.
     * @note Available only when `BUILD_SERVER_ENABLED` is
     * defined.
     *
     * @return true if containers were successfully added;
     * false otherwise.
     */
    bool addContainersToTrain(QString     networkName,
                              QString     trainID,
                              QJsonObject json);

    /**
     * @brief Request a train to unload containers at a
     * specific terminal.
     * @param networkName Name of the network containing the
     * train.
     * @param trainID Unique identifier of the train.
     * @param portNames List of potential names for the
     * target terminal.
     * @details This method triggers the unloading process
     * for the specified train at the target terminal. The
     * terminal is identified by matching one of the
     * provided names.
     */
    void requestUnloadContainersAtTerminal(
        QString networkName, QString trainID,
        QVector<QString> portNames);

protected slots:

    /**
     * @brief Handle the event when all trains reach their
     * destinations.
     * @param networkName Name of the network where the
     * event occurred.
     *
     * @details This function processes the event triggered
     * when all train reach their designated destinations.
     */
    void
    handleAllTrainsReachedDestination(QString networkName);

    /**
     * @brief Handle the event when a train reaches its
     * destination.
     * @param networkName Name of the network where the
     * event occurred.
     * @param trainState JSON object containing the state of
     * the train at its destination.
     * @param mode Synchronization mode (Async or Sync).
     * @details This function processes the event triggered
     * when a train reaches its designated destination. In
     * Async mode, results are accumulated and emitted once
     * all trains reach their destinations. In Sync mode,
     * results are emitted immediately.
     * @note Emits the `trainsReachedDestination` signal
     * with the processed data.
     */
    void
    handleTrainReachedDestination(QString     networkName,
                                  QJsonObject trainState,
                                  Mode        mode);

    /**
     * @brief Handle updates to train coordinates during
     * simulation.
     * @param networkName Name of the network where the
     * trains are running.
     * @param simulatorTrainsCoords A vector containing
     * train IDs and their updated coordinates.
     * @param mode Synchronization mode (Async or Sync).
     * @details This function processes coordinate updates
     * for trains in the specified network. In Async mode,
     * updates are collected for all trains before emitting
     * signals. In Sync mode, updates are processed
     * immediately.
     * @note Emits the `trainsCoordinatesUpdated` signal
     * with updated data.
     */
    void handleTrainCoordsUpdate(
        QString networkName,
        Vector<std::pair<std::string,
                         Vector<std::pair<double, double>>>>
             simulatorTrainsCoords,
        Mode mode);

    /**
     * @brief Handle the completion of a single simulation
     * time step.
     * @param networkName Name of the network where the time
     * step was completed.
     * @param currentSimulatorTime Current simulation time
     * after the step.
     * @param currentSimulatorProgress Current progress
     * percentage of the simulation.
     * @param mode Synchronization mode (Async or Sync).
     * @details This function is triggered when a simulation
     * time step is completed. Updates progress tracking and
     * emits the `simulationAdvanced` signal based on the
     * specified mode.
     */
    void handleOneTimeStepCompleted(
        QString networkName, double currentSimulatorTime,
        double currentSimulatorProgress, Mode mode);

    /**
     * @brief Handle the event when a simulation finishes
     * naturally.
     * @param networkName Name of the network whose
     * simulation finished.
     * @details This function is triggered when a simulation
     * completes without manual termination. It updates
     * completion trackers and emits the
     * `simulationFinished` signal.
     */
    void handleSimulationFinished(QString networkName);

    /**
     * @brief Handle the availability of simulation results.
     * @param networkName Name of the network whose results
     * are available.
     * @param result Object containing the simulation
     * results.
     * @param mode Synchronization mode (Async or Sync).
     * @details This function processes the results
     * generated during the simulation. In Async mode,
     * results for all networks are collected and emitted
     * together. In Sync mode, results are emitted as they
     * become available.
     * @note Emits the `simulationResultsAvailable` signal
     * with the processed results.
     */
    void handleResultsAvailable(QString       networkName,
                                TrainsResults result,
                                Mode          mode);

    /**
     * @brief Handler for worker thread readiness
     * @param networkName Network where the worker thread is
     * ready
     *
     * @details This method is called when a worker thread
     * completes its assigned tasks. It updates the worker
     * readiness tracker and emits the `workersReady` signal
     * when all worker threads are ready.
     */
    void handleWorkersReady(QString networkName);

    /**
     * @brief Handler for simulation progress updates
     * @param networkName Network reporting progress
     * @param progress Progress percentage (0-100)
     *
     * @details Updates progress tracking and emits
     * simulationProgressUpdated
     */
    void handleProgressUpdate(QString networkName,
                              double  simulationTime,
                              int     progressPercentage);

    /**
     * @brief Finalize the simulation for the specified
     * networks.
     * @param networkNames List of networks to finalize the
     * simulation for.
     * @details This function concludes the simulation for
     * the specified networks, performing necessary cleanup
     * and generating summary data. The simulation
     * environment remains intact but is marked as complete.
     *
     * - Emits the `simulationFinished` signal upon
     * successful completion.
     * - Use this function when the simulation has naturally
     * reached its endpoint and you want to produce a
     * summary report or finalize results.
     */
    void requestFinalizeSimulation(
        QVector<QString> networkNames);

    /**
     * @brief Pauses the simulation for the specified
     * networks
     * @param networkNames List of networks to pause the
     * simulation for
     *
     * @details This method pauses the simulation for the
     * specified networks. The simulation state is preserved
     * and can be resumed later using `resumeSimulation`.
     */
    void
    requestPauseSimulation(QVector<QString> networkNames);

    /**
     * @brief Resumes the simulation for the specified
     * networks
     * @param networkNames List of networks to resume the
     * simulation for
     *
     * @details This method resumes the simulation for the
     * specified networks from the last paused state. The
     * simulation continues from where it was paused.
     */
    void
    requestResumeSimulation(QVector<QString> networkNames);

    /**
     * @brief Terminates the simulation for the specified
     * networks
     * @param networkNames List of networks to terminate the
     * simulation for
     *
     * @details This method forcefully terminates the
     * simulation for the specified networks. The simulation
     * cannot be resumed after termination.
     */
    void requestTerminateSimulation(
        QVector<QString> networkNames);

    /**
     * @brief Runs the simulation for the specified networks
     * @param networkNames List of networks to run the
     * simulation for
     * @param timeSteps Duration of the simulation step (in
     * seconds)
     * @param endSimulationAfterRun If true, the simulation
     * will end after the current run
     * @param getStepEndSignal If true, a signal will be
     * emitted at the end of each step
     *
     * @details This method starts or continues the
     * simulation for the specified networks. It supports
     * both finite and infinite time steps, depending on the
     * value of `timeSteps`.
     * If `endSimulationAfterRun` is true, the simulation
     * will terminate after the current run. If
     * `getStepEndSignal` is true, a signal will be emitted
     * at the end of each simulation step.
     */
    void requestRunSimulation(QVector<QString> networkNames,
                              double           timeSteps,
                              bool endSimulationAfterRun,
                              bool getStepEndSignal);

public:
    /**
     * @class SimulatorAPI::InteractiveMode
     * @brief Provides step-by-step control over train
     * simulations.
     *
     * @details The `InteractiveMode` class offers a static
     * interface for controlling train simulations in a
     * step-by-step manner, allowing fine-grained control
     * over simulation execution. This mode is ideal for:
     * - Debugging and testing.
     * - Visualizing simulation steps.
     * - Implementing custom simulation control flows.
     */
    class NETRAINSIMCORE_EXPORT InteractiveMode
    {
    public:
        /**
         * @brief Get the singleton instance of the
         * SimulatorAPI.
         * @return Reference to the SimulatorAPI singleton.
         * @details Provides access to the global instance
         * of the SimulatorAPI, ensuring that there is only
         * one active instance throughout the application.
         */
        static SimulatorAPI &getInstance();

        /**
         * @brief Reset the API to its initial state.
         * @details Cleans up all resources and resets the
         * state of the API, allowing it to be reinitialized
         * for a fresh simulation.
         */
        static void resetAPI();

        // Simulator control methods

        /**
         * @brief Create a new simulation environment using
         * network content.
         * @param nodesFileContent Content of the file
         * defining the network nodes.
         * @param linksFileContent Content of the file
         * defining the network links.
         * @param networkName Name of the network to create.
         * Defaults to "*".
         * @param trainsData List of train configurations
         * (optional).
         * @param timeStep Duration of each simulation time
         * step (in seconds). Defaults to 1.0.
         * @param mode Synchronization mode (Async or Sync).
         * Defaults to Async.
         * @details This method initializes a new simulation
         * environment using the provided network and train
         * data, but does not start the simulation.
         */
        static void createNewSimulationEnvironment(
            QJsonObject nodesFileContent,
            QJsonObject linksFileContent,
            QString     networkName = "*",
            QVector<QMap<QString, std::any>> trainsData =
                QVector<QMap<QString, std::any>>(),
            double timeStep = 1.0, Mode mode = Mode::Async);

        /**
         * @brief Create a new simulation environment using
         * in-memory network and train data.
         * @param networkName Name of the network to create.
         * @param nodeRecords List of nodes with their
         * attributes as key-value pairs.
         * @param linkRecords List of links with their
         * attributes as key-value pairs.
         * @param trainList List of train configurations
         * with attributes as key-value pairs.
         * @param timeStep Duration of each simulation time
         * step (in seconds). Defaults to 1.0.
         * @param mode Synchronization mode (Async or Sync).
         * Defaults to Async.
         * @details Initializes the simulation environment
         * using in-memory data structures, bypassing
         * file-based input.
         */
        static void createNewSimulationEnvironment(
            QString                          networkName,
            QVector<QMap<QString, QString>> &nodeRecords,
            QVector<QMap<QString, QString>> &linkRecords,
            QVector<QMap<QString, any>>     &trainList,
            double timeStep = 1.0, Mode mode = Mode::Async);

        /**
         * @brief Create a new simulation environment using
         * network and train files.
         * @param nodesFile Path to the file defining
         * network nodes.
         * @param linksFile Path to the file defining
         * network links.
         * @param networkName Name of the network to create.
         * @param trainsFile Path to the file defining train
         * configurations.
         * @param timeStep Duration of each simulation time
         * step (in seconds). Defaults to 1.0.
         * @param mode Synchronization mode (Async or Sync).
         * Defaults to Async.
         * @details Reads network and train data from the
         * specified files, initializes the network, and
         * prepares the simulation environment for
         * execution.
         */
        static void createNewSimulationEnvironmentFromFiles(
            QString nodesFile, QString linksFile,
            QString networkName, QString trainsFile,
            double timeStep = 1.0, Mode mode = Mode::Async);

        /**
         * @brief Add trains to an existing simulation.
         * @param networkName Name of the network where the
         * trains will be added.
         * @param trains List of train objects to add to the
         * simulation.
         * @details Dynamically adds trains to an active
         * simulation, updating the simulation environment.
         */
        static void addTrainToSimulation(
            QString                         networkName,
            QVector<std::shared_ptr<Train>> trains);

        /**
         * @brief Run the simulation for a specified
         * duration.
         * @param networkNames List of networks to run.
         * @param timeSteps Duration to run the simulation
         * (in seconds).
         * @param getProgressSignal If true, emits progress
         * signals during execution.
         * @details Executes the simulation step-by-step for
         * the specified duration. Progress signals are
         * emitted if requested.
         */
        static void
        runSimulation(QVector<QString> networkNames,
                      double           timeSteps,
                      bool             getProgressSignal);

        /**
         * @brief Finalize the simulation for the specified
         * networks.
         * @param networkNames List of networks to finalize
         * the simulation for.
         * @details Concludes the simulation for the given
         * networks and generates any summary data or
         * reports.
         */
        static void
        finalizeSimulation(QVector<QString> networkNames);

        /**
         * @brief Terminate the simulation for the specified
         * networks.
         * @param networkNames List of networks to
         * terminate.
         * @details Forcefully stops the simulation for the
         * specified networks. The simulation cannot be
         * resumed after termination.
         */
        static void
        terminateSimulation(QVector<QString> networkNames);

        // Getters

        /**
         * @brief Get the simulator instance for a specific
         * network.
         * @param networkName Name of the network whose
         * simulator is requested.
         * @return Pointer to the Simulator object, or
         * nullptr if not found.
         * @details Provides access to the simulator for the
         * specified network.
         */
        static Simulator *getSimulator(QString networkName);

        /**
         * @brief Get the network instance for a specific
         * name.
         * @param networkName Name of the network to
         * retrieve.
         * @return Pointer to the Network object, or nullptr
         * if not found.
         * @details Returns the initialized network
         * associated with the given name.
         */
        static Network *getNetwork(QString networkName);

        /**
         * @brief Get a train by its unique identifier.
         * @param networkName Name of the network containing
         * the train.
         * @param trainID Unique identifier of the train.
         * @return Shared pointer to the Train object, or
         * nullptr if not found.
         * @details Retrieves the train object for
         * inspection or modification.
         */
        static std::shared_ptr<Train>
        getTrainByID(QString networkName, QString &trainID);

        /**
         * @brief Get all trains in a specified network.
         * @param networkName Name of the network containing
         * the trains.
         * @return Vector of shared pointers to all Train
         * objects in the network.
         * @details Provides a list of all trains in the
         * specified network.
         */
        static QVector<std::shared_ptr<Train>>
        getAllTrains(QString networkName);

        /**
         * @brief Add containers to a specific train.
         * @param networkName Name of the network containing
         * the train.
         * @param trainID Unique identifier of the train.
         * @param json JSON object describing the container
         * configurations.
         * @details Adds containers to a train in the
         * simulation.
         * @note Available only when `BUILD_SERVER_ENABLED`
         * is defined.
         *
         * @return true if containers were successfully
         */
        static bool
        addContainersToTrain(QString     networkName,
                             QString     trainID,
                             QJsonObject json);

        /**
         * @brief Get all trains in the specified network.
         * @param networkName Name of the network to query.
         * @return Vector of shared pointers to Train
         * objects.
         * @details Returns all the trains in the specified
         * network.
         */
        static QVector<std::shared_ptr<Train>>
        getTrains(QString networkName);

        /**
         * @brief Request a train to unload containers at a
         * specific terminal.
         * @param networkName Name of the network containing
         * the train.
         * @param trainID Unique identifier of the train.
         * @param portNames List of potential terminal names
         * for unloading.
         * @details Requests the train to unload its
         * containers at the target terminal.
         */
        static void requestUnloadContainersAtTerminal(
            QString networkName, QString trainID,
            QVector<QString> portNames);
    };

    /**
     * @class SimulatorAPI::ContinuousMode
     * @brief Provides continuous control over train
     * simulations with pause and resume capabilities.
     *
     * @details The `ContinuousMode` class offers a static
     * interface for running train simulations continuously
     * until completion or manual intervention. This mode is
     * ideal for:
     * - Production-grade simulations.
     * - Long-running scenarios.
     * - Automated simulation execution.
     */
    class NETRAINSIMCORE_EXPORT ContinuousMode
    {
    public:
        /**
         * @brief Get the singleton instance of the
         * SimulatorAPI.
         * @return Reference to the SimulatorAPI singleton.
         * @details Ensures that there is only one active
         * instance of the API and provides global access to
         * it.
         */
        static SimulatorAPI &getInstance();

        /**
         * @brief Reset the API to its initial state.
         * @details Cleans up all resources and resets the
         * state of the API, preparing it for a new
         * simulation run.
         */
        static void resetAPI();

        // Simulator control methods

        /**
         * @brief Create a new simulation environment using
         * network content.
         * @param nodesFileContent Content of the file
         * defining the network nodes.
         * @param linksFileContent Content of the file
         * defining the network links.
         * @param networkName Name of the network to create.
         * Defaults to "*".
         * @param trainList List of train configurations
         * (optional).
         * @param timeStep Duration of each simulation time
         * step (in seconds). Defaults to 1.0.
         * @param mode Synchronization mode (Async or Sync).
         * Defaults to Async.
         * @details This method initializes a new simulation
         * environment using the provided network and train
         * data for continuous execution.
         */
        static void createNewSimulationEnvironment(
            QJsonObject nodesFileContent,
            QJsonObject linksFileContent,
            QString     networkName = "*",
            QVector<QMap<QString, std::any>> trainList =
                QVector<QMap<QString, std::any>>(),
            double timeStep = 1.0, Mode mode = Mode::Async);

        /**
         * @brief Create a new simulation environment using
         * in-memory network and train data.
         * @param networkName Name of the network to create.
         * @param nodeRecords List of nodes with their
         * attributes as key-value pairs.
         * @param linkRecords List of links with their
         * attributes as key-value pairs.
         * @param trainList List of train configurations
         * with attributes as key-value pairs.
         * @param timeStep Duration of each simulation time
         * step (in seconds). Defaults to 1.0.
         * @param mode Synchronization mode (Async or Sync).
         * Defaults to Async.
         * @details Initializes the simulation environment
         * using in-memory data structures, bypassing
         * file-based input.
         */
        static void createNewSimulationEnvironment(
            QString                          networkName,
            QVector<QMap<QString, QString>> &nodeRecords,
            QVector<QMap<QString, QString>> &linkRecords,
            QVector<QMap<QString, any>>     &trainList,
            double timeStep = 1.0, Mode mode = Mode::Async);

        /**
         * @brief Create a new simulation environment using
         * network and train files.
         * @param nodesFile Path to the file defining
         * network nodes.
         * @param linksFile Path to the file defining
         * network links.
         * @param networkName Name of the network to create.
         * @param trainsFile Path to the file defining train
         * configurations.
         * @param timeStep Duration of each simulation time
         * step (in seconds). Defaults to 1.0.
         * @param mode Synchronization mode (Async or Sync).
         * Defaults to Async.
         * @details Reads network and train data from the
         * specified files and initializes the network for
         * continuous simulation execution.
         */
        static void createNewSimulationEnvironmentFromFiles(
            QString nodesFile, QString linksFile,
            QString networkName, QString trainsFile,
            double timeStep = 1.0, Mode mode = Mode::Async);

        /**
         * @brief Start continuous simulation execution.
         * @param networkNames List of networks to run.
         * @param getProgressSignal If true, emits progress
         * signals during execution.
         * @details Executes the simulation continuously for
         * the specified networks. The simulation runs until
         * completion or manual intervention.
         */
        static void
        runSimulation(QVector<QString> networkNames,
                      bool             getProgressSignal);

        /**
         * @brief Pause running simulations.
         * @param networkNames List of networks to pause.
         * @details Halts the simulation for the specified
         * networks, preserving the current state for
         * resumption.
         */
        static void
        pauseSimulation(QVector<QString> networkNames);

        /**
         * @brief Resume paused simulations.
         * @param networkNames List of networks to resume.
         * @details Resumes the simulation for the specified
         * networks from the last paused state.
         */
        static void
        resumeSimulation(QVector<QString> networkNames);

        /**
         * @brief Terminate running simulations.
         * @param networkNames List of networks to
         * terminate.
         * @details Forcefully stops the simulation for the
         * specified networks. The simulation cannot be
         * resumed after termination.
         */
        static void
        terminateSimulation(QVector<QString> networkNames);

        // Getters

        /**
         * @brief Get the simulator instance for a specific
         * network.
         * @param networkName Name of the network whose
         * simulator is requested.
         * @return Pointer to the Simulator object, or
         * nullptr if not found.
         * @details Provides access to the simulator for the
         * specified network.
         */
        static Simulator *getSimulator(QString networkName);

        /**
         * @brief Get the network instance for a specific
         * name.
         * @param networkName Name of the network to
         * retrieve.
         * @return Pointer to the Network object, or nullptr
         * if not found.
         * @details Returns the initialized network
         * associated with the given name.
         */
        static Network *getNetwork(QString networkName);

        /**
         * @brief Add containers to a specific train.
         * @param networkName Name of the network containing
         * the train.
         * @param trainID Unique identifier of the train.
         * @param json JSON object describing the container
         * configurations.
         * @details Adds containers to a train in the
         * simulation.
         * @note Available only when `BUILD_SERVER_ENABLED`
         * is defined.
         *
         * @return true if containers were successfully
         */
        static bool
        addContainersToTrain(QString     networkName,
                             QString     trainID,
                             QJsonObject json);

        /**
         * @brief Get all trains in the specified network.
         * @param networkName Name of the network to query.
         * @return Vector of shared pointers to Train
         * objects.
         * @details Returns all the trains in the specified
         * network.
         */
        static QVector<std::shared_ptr<Train>>
        getTrains(QString networkName);

        /**
         * @brief Request a train to unload containers at a
         * specific terminal.
         * @param networkName Name of the network containing
         * the train.
         * @param trainID Unique identifier of the train.
         * @param terminalNames List of potential terminal
         * names for unloading.
         * @details Requests the train to unload its
         * containers at the target terminal.
         */
        static void requestUnloadContainersAtTerminal(
            QString networkName, QString trainID,
            QVector<QString> terminalNames);
    };

private:
    /**
     * @brief Singleton instance of the SimulatorAPI class.
     * @details This static member holds the single instance
     * of the SimulatorAPI. It is initialized and accessed
     * through the `getInstance` method to ensure
     * thread-safe creation and management.
     *
     * - Uses `std::unique_ptr` for efficient and safe
     * memory management.
     * - Ensures that the instance is destroyed
     * automatically when the program ends.
     */
    static std::unique_ptr<SimulatorAPI> instance;

    /**
     * @brief Mutex for thread-safe access to the singleton
     * instance.
     * @details This static `QBasicMutex` ensures that the
     * creation of the singleton instance is thread-safe,
     * preventing race conditions when accessed from
     * multiple threads.
     *
     * - Used in the `getInstance` method to lock access
     * during instance initialization.
     * - Ensures that only one thread can initialize the
     * instance at any time.
     */
    static QBasicMutex s_instanceMutex;

    /**
     * @brief Register custom meta-types for signal-slot
     * communication.
     * @details This function ensures that custom data types
     * like Train and TrainsResults can be used seamlessly
     * in Qt signals and slots.
     */
    static void registerQMeta();

    /**
     * @brief Configure the locale settings for numeric
     * formatting.
     * @details Disables thousand separators in numeric
     * strings to ensure consistent formatting across
     * different locales. This is critical for parsing and
     * representing numbers in a standardized way during
     * simulation.
     */
    static void setLocale();
};

Q_DECLARE_METATYPE(APIData)
Q_DECLARE_METATYPE(APIData *)

#endif // SIMULATORAPI_H
