#include "simulatorapi.h"
#include "./traindefinition/trainslist.h"
#include <QThread>
#include <QMetaObject> // Required for QMetaObject::invokeMethod and Q_ARG
#include <QVariant>    // Required for QVariant types
#include <QThread>     // Required for QThread
#include <QDebug>      // For debugging


#ifndef QT_NO_DEBUG
#define CHECK_TRUE(instruction) Q_ASSERT(instruction)
#else
#define CHECK_TRUE(instruction) (instruction)
#endif

// Initialize static members

QBasicMutex SimulatorAPI::s_instanceMutex;
SimulatorAPI::Mode SimulatorAPI::mMode = Mode::Sync;
std::unique_ptr<SimulatorAPI> SimulatorAPI::instance(new SimulatorAPI());

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//                                BASIC FUNCTIONS
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------


void SimulatorAPI::registerQMeta() {
    // Register enums
    qRegisterMetaType<SimulatorAPI::Mode>("SimulatorAPI::Mode");

    // Register Train and its components
    qRegisterMetaType<Train>("Train");
    qRegisterMetaType<Train*>("Train*");
    qRegisterMetaType<std::shared_ptr<Train>>("std::shared_ptr<Train>");
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<Vector<double>>("Vector<double>");
    qRegisterMetaType<Vector<std::shared_ptr<NetNode>>>("Vector<std::shared_ptr<NetNode>>");
    qRegisterMetaType<Vector<std::shared_ptr<NetLink>>>("Vector<std::shared_ptr<NetLink>>");
    qRegisterMetaType<Vector<std::shared_ptr<Car>>>("Vector<std::shared_ptr<Car>>");
    qRegisterMetaType<Vector<std::shared_ptr<Locomotive>>>("Vector<std::shared_ptr<Locomotive>>");
    qRegisterMetaType<Vector<Map<std::string, std::string>>>("Vector<Map<std::string, std::string>>");
    qRegisterMetaType<Map<std::string, double>>("Map<std::string, double>");
    qRegisterMetaType<std::map<std::string, std::any>>("std::map<std::string, std::any>");
    qRegisterMetaType<Map<std::shared_ptr<TrainComponent>, double>>("Map<std::shared_ptr<TrainComponent>, double>");

    // Register Network and its components
    qRegisterMetaType<Network*>("Network*");
    qRegisterMetaType<Vector<std::shared_ptr<NetNode>>>("Vector<std::shared_ptr<NetNode>>");
    qRegisterMetaType<Vector<std::shared_ptr<NetLink>>>("Vector<std::shared_ptr<NetLink>>");
    qRegisterMetaType<Vector<std::shared_ptr<NetSignal>>>("Vector<std::shared_ptr<NetSignal>>");
    qRegisterMetaType<std::pair<double, double>>("std::pair<double, double>");
    qRegisterMetaType<std::shared_ptr<NetNode>>("std::shared_ptr<NetNode>");
    qRegisterMetaType<std::shared_ptr<NetLink>>("std::shared_ptr<NetLink>");
    qRegisterMetaType<std::shared_ptr<NetSignal>>("std::shared_ptr<NetSignal>");

    qRegisterMetaType<APIData>("APIData");

    qRegisterMetaType<TrainsResults>("TrainsResults");
}

SimulatorAPI& SimulatorAPI::getInstance() {
    QMutexLocker locker(&s_instanceMutex);
    if (!instance) {
        registerQMeta();
        instance.reset(new SimulatorAPI());
    }
    return *instance;
}

void SimulatorAPI::resetInstance() {
    if (instance) {
        // Get all network names from the thread-safe data map
        QList<QString> networkNames = instance->apiDataMap.getNetworkNames();
        for (const QString& networkName : networkNames) {
            // Retrieve the APIData for the current network
            APIData data = instance->apiDataMap.get(networkName);

            // Clean up network
            if (data.network) {
                delete data.network;
                data.network = nullptr;
            }

            // Clean up simulatorWorker
            if (data.simulatorWorker) {
                if (data.simulatorWorker->thread() == data.workerThread) {
                    QMetaObject::invokeMethod(
                        data.simulatorWorker, "deleteLater",
                        Qt::BlockingQueuedConnection);
                } else {
                    delete data.simulatorWorker;
                }
                data.simulatorWorker = nullptr;
            }

            // Clean up simulator
            if (data.simulator) {
                if (data.simulator->thread() == data.workerThread) {
                    QMetaObject::invokeMethod(
                        data.simulator, "deleteLater",
                        Qt::BlockingQueuedConnection);
                } else {
                    delete data.simulator;
                }
                data.simulator = nullptr;
            }

            // Clean up workerThread
            if (data.workerThread) {
                data.workerThread->quit(); // Request thread to stop
                data.workerThread->wait(); // Wait for thread to finish
                delete data.workerThread;  // Delete the thread
                data.workerThread = nullptr;
            }

            // Clear trains
            data.trains.clear();

            // Update the APIData in the thread-safe map
            instance->apiDataMap.addOrUpdate(networkName, data);
        }

        // Clear all APIData from the thread-safe map
        instance->apiDataMap.clear();

        // Destroy the singleton instance
        instance.reset();
    }

    // Re-register meta types and create a new instance
    instance.reset();
    registerQMeta();
    instance.reset(new SimulatorAPI());
}

void SimulatorAPI::setLocale() {
    try {
        // Set locale to UTF-8 with no thousands separator
        std::locale customLocale(
#ifdef _WIN32
            std::locale("English_United States.1252"), // Windows
#else
            std::locale("en_US.UTF-8"),               // Linux/macOS
#endif
            new NoThousandsSeparator()
            );

        // Apply custom locale globally and to std::cout
        std::locale::global(customLocale);
        std::cout.imbue(customLocale);
    } catch (const std::exception& e) {
        qWarning() << "Failed to set std::locale: " << e.what();

        // Fallback to "C" locale
        std::locale fallbackLocale("C");
        std::locale::global(fallbackLocale);
        std::cout.imbue(fallbackLocale);
    }

    // Configure Qt's QLocale for UTF-8 without thousands separators
    QLocale qtLocale(QLocale::English, QLocale::UnitedStates);
    qtLocale.setNumberOptions(QLocale::OmitGroupSeparator); // Disable thousands separator
    QLocale::setDefault(qtLocale);
}


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//                     CREATION, LOADING & SETTING UPFUNCTIONS
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------



void SimulatorAPI::
    createNewSimulationEnvironment(
        QJsonObject nodesFileContent,
        QJsonObject linksFileContent,
        QString networkName,
        QVector<QMap<QString, std::any>> trainsData,
        double timeStep,
        Mode mode)
{
    // Set locale to US format (no thousands separator, dot for decimals)
    setLocale();

    if (apiDataMap.contains(networkName)) {
        emit errorOccurred("A network with name " + networkName + " exists!");
        return;
    }

    auto nodeRec = ReadWriteNetwork::readNodesFromJson(nodesFileContent);
    auto linkRec = ReadWriteNetwork::readLinksFromJson(linksFileContent);

    auto nodeRecordsf = Utils::convertToQVectorString(nodeRec);
    auto linkRecordsf = Utils::convertToQVectorString(linkRec);


    setupSimulator(networkName, nodeRecordsf, linkRecordsf,
                   trainsData, timeStep, mode);

    emit simulationCreated(networkName);

}

void SimulatorAPI::
    createNewSimulationEnvironmentFromFiles(
        QString nodesFile,
        QString linksFile,
        QString networkName,
        QString trainsFile,
        double timeStep,
        Mode mode)
{
    // Set locale to US format (no thousands separator, dot for decimals)
    setLocale();

    if (apiDataMap.contains(networkName)) {
        emit errorOccurred("A network with name " + networkName + " exists!");
        return;
    }
    auto nodeRecords =
        ReadWriteNetwork::readNodesFile(nodesFile.toStdString());

    auto linkRecords =
        ReadWriteNetwork::readLinksFile(linksFile.toStdString());

    auto trainsRecords = TrainsList::readTrainsFile(trainsFile.toStdString());

    auto nodeRecordsf = Utils::convertToQVectorString(nodeRecords);
    auto linkRecordsf = Utils::convertToQVectorString(linkRecords);




    auto trainsRecordsf = Utils::convertToQVector(trainsRecords);

    setupSimulator(networkName, nodeRecordsf, linkRecordsf,
                   trainsRecordsf, timeStep, mode);

    emit simulationCreated(networkName);

}

void SimulatorAPI::createNewSimulationEnvironment(
    QString networkName,
    QVector<QMap<QString, QString>> &nodeRecords,
    QVector<QMap<QString, QString>> &linkRecords,
    QVector<QMap<QString, std::any>> &trainList,
    double timeStep,
    Mode mode)
{
    // Set locale to US format (no thousands separator, dot for decimals)
    setLocale();

    if (apiDataMap.contains(networkName)) {
        emit errorOccurred("A network with name " + networkName + " exists!");
        return;
    }

    setupSimulator(networkName, nodeRecords, linkRecords,
                   trainList, timeStep, mode);

    emit simulationCreated(networkName);
}

void SimulatorAPI::setupSimulator(
    QString& networkName,
    QVector<QMap<QString, QString>>& nodeRecords,
    QVector<QMap<QString, QString>>& linkRecords,
    QVector<QMap<QString, std::any>>& trainList,
    double timeStep,
    Mode mode)
{
    // Capture networkName by value to ensure stability
    QString localNetworkName = networkName;

    // Create a new thread for simulation
    QThread* workerThread = new QThread(this);

    // Initialize APIData and store the working thread
    APIData apiData;
    apiData.workerThread = workerThread;
    apiDataMap.addOrUpdate(localNetworkName, apiData);

    // Create the worker and move it to the thread
    SimulatorWorker* simulatorWorker = new SimulatorWorker();
    simulatorWorker->moveToThread(workerThread);

    // Update APIData with worker and store
    apiData.simulatorWorker = simulatorWorker;
    apiDataMap.addOrUpdate(localNetworkName, apiData);

    QEventLoop loop;
    bool setupSuccess = false;  // Track success/failure

    // Connect signals with networkName captured by value
    CHECK_TRUE(QObject::connect(simulatorWorker, &SimulatorWorker::simulatorLoaded,
                     [this, localNetworkName, &loop, &setupSuccess]
                     (APIData& loadedData)
                     {
                         qInfo() << "Simulator loaded for network:" << localNetworkName;
                         apiDataMap.addOrUpdate(localNetworkName, loadedData);
                         setupSuccess = true;  // Mark as successful
                         loop.quit();
                     }));

    CHECK_TRUE(QObject::connect(simulatorWorker, &SimulatorWorker::errorOccured,
                     [this, localNetworkName, workerThread, &loop, &setupSuccess]
                     (const QString& error) {
                         qWarning() << "Error setting up"
                                    << localNetworkName << ":" << error;
                         workerThread->quit();
                         setupSuccess = false;  // Mark as failed
                         loop.quit();
                     }));

    workerThread->start();

    // Invoke setup in worker thread with correct networkName
    QMetaObject::invokeMethod(simulatorWorker, [=, this]() {
        APIData workerData = apiDataMap.get(localNetworkName);
        simulatorWorker->setupSimulator(workerData, localNetworkName, nodeRecords,
                                        linkRecords, trainList, timeStep);
    });

    loop.exec();

    // Only continue with connections if setup was successful
    if (setupSuccess) {

        // Setup connections with the updated APIData
        setupConnections(localNetworkName, mode);

        CHECK_TRUE(connect(workerThread, &QThread::finished, this,
                [this, localNetworkName]() {
                    handleWorkersReady(localNetworkName);
                }, Qt::DirectConnection));

        APIData latestApiData = apiDataMap.get(localNetworkName);
        if (latestApiData.workerThread) {
            latestApiData.workerThread->setPriority(QThread::LowPriority);
            apiDataMap.addOrUpdate(localNetworkName, latestApiData);
        }

        qInfo() << "Simulator setup complete for" << localNetworkName;

    } else {
        // Clean up resources on failure
        if (workerThread) {
            workerThread->quit();
            workerThread->wait();
            delete workerThread;
        }

        if (simulatorWorker) {
            delete simulatorWorker;
        }

        // Remove the incomplete network data
        apiDataMap.remove(localNetworkName);

        qWarning() << "Failed to set up simulator for" << localNetworkName;
    }
}


void SimulatorAPI::setupConnections(const QString& networkName, Mode mode)
{
    // Retrieve the APIData for the specified network
    APIData apiData = apiDataMap.get(networkName);

    // Ensure the simulator instance is valid
    if (!apiData.simulator) {
        emit errorOccurred(QString("Simulator instance is nullptr! "
                                   "Initialization failed for network: %1")
                               .arg(networkName));
        qFatal("Simulator instance is nullptr! "
               "Initialization failed for network: %s",
               qPrintable(networkName));
    }

    auto& simulator = apiData.simulator;

    CHECK_TRUE(connect(simulator, &Simulator::resultDataAvailable, this,
            [this, networkName, mode](TrainsResults results) {
                handleResultsAvailable(networkName, results, mode);
            }, Qt::QueuedConnection));

    // Connect simulation finished signal
    CHECK_TRUE(connect(simulator, &Simulator::simulationFinished, this,
            [this, networkName]() {
                qDebug() << "Current thread 3:" << QThread::currentThread();
                handleSimulationFinished(networkName);
            }, mConnectionType));

    // Connect simulation reached reporting time signal
    CHECK_TRUE(connect(simulator,
            &Simulator::simulationReachedReportingTime, this,
            [this, networkName, mode]
            (double currentSimulatorTime,
             double progressPercent) {
                handleOneTimeStepCompleted(networkName, currentSimulatorTime,
                                           progressPercent, mode);
            }, mConnectionType));

    // Connect progress updated signal
    CHECK_TRUE(connect(simulator,
            &Simulator::progressUpdated, this,
            [this, networkName](double simulationTime, int progressPercentage) {
                handleProgressUpdate(networkName,
                                     simulationTime,
                                     progressPercentage);
            }, mConnectionType));

    // Connect simulation paused signal
    CHECK_TRUE(connect(simulator, &Simulator::simulationPaused, this,
            [this, networkName, mode]() {
                mPauseTracker.incrementCompletedRequests();
                checkAndEmitSignal(
                    mPauseTracker.getCompletedRequests(),
                    mPauseTracker.getRequestedNetworks().size(),
                    mPauseTracker.getRequestedNetworks(),
                    &SimulatorAPI::simulationsPaused,
                    mode);
            }, mConnectionType));

    // Connect simulation resumed signal
    CHECK_TRUE(connect(simulator, &Simulator::simulationResumed, this,
            [this, networkName, mode]() {
                mResumeTracker.incrementCompletedRequests();
                checkAndEmitSignal(
                    mResumeTracker.getCompletedRequests(),
                    mResumeTracker.getRequestedNetworks().size(),
                    mResumeTracker.getRequestedNetworks(),
                    &SimulatorAPI::simulationsResumed,
                    mode);
            }, mConnectionType));

    // Connect simulation terminated signal
    CHECK_TRUE(connect(simulator, &::Simulator::simulationTerminated, this,
            [this, networkName, mode]() {
                mTerminateTracker.incrementCompletedRequests();
                checkAndEmitSignal(
                    mTerminateTracker.getCompletedRequests(),
                    mTerminateTracker.getRequestedNetworks().size(),
                    mTerminateTracker.getRequestedNetworks(),
                    &SimulatorAPI::simulationsTerminated,
                    mode);
            }, mConnectionType));

    // Connect simulation restarted signal
    CHECK_TRUE(connect(simulator, &Simulator::simulationRestarted, this,
            [this, networkName, mode]() {
                mRestartTracker.incrementCompletedRequests();
                checkAndEmitSignal(
                    mRestartTracker.getCompletedRequests(),
                    mRestartTracker.getRequestedNetworks().size(),
                    mRestartTracker.getRequestedNetworks(),
                    &SimulatorAPI::simulationsRestarted,
                    mode);
            }, mConnectionType));


    CHECK_TRUE(connect(simulator, &Simulator::plotTrainsUpdated,
            this, [this, networkName, mode]
            (Vector<std::pair<std::string,
                              Vector<std::pair<double,
                                               double>>>> trainsStartEndPoints) {
                handleTrainCoordsUpdate(networkName, trainsStartEndPoints, mode);
            }, Qt::QueuedConnection));

    CHECK_TRUE(connect(simulator, &Simulator::errorOccurred, this,
            [this, networkName](QString error) {
                emit errorOccurred(QString("Error in Network ") + networkName +
                                   QString(": ") + error);
            }, Qt::QueuedConnection));

    CHECK_TRUE(connect(simulator, &Simulator::trainReachedTerminal, this,
            [this, networkName](QString trainID, QString terminalID,
                                int containersCount) {
        emit trainReachedTerminal(networkName, trainID,
                                  terminalID, containersCount);
    }));

    setupTrainsConnection(apiData.trains.values(), networkName, mode);

    // Update the APIData in the thread-safe map (if any changes were made)
    apiDataMap.addOrUpdate(networkName, apiData);
}

void SimulatorAPI::setupTrainsConnection(
    QVector<std::shared_ptr<Train>> trains, QString networkName, Mode mode)
{
    for (const auto& train : trains) {
        CHECK_TRUE(connect(train.get(), &Train::destinationReached, this,
                [this, networkName, train, mode](QJsonObject state) {
                    handleTrainReachedDestination(
                        networkName,
                        state,
                        mode);
                }, Qt::QueuedConnection));
        CHECK_TRUE(connect(train.get(), &Train::containersLoaded, this,
                [this, networkName, train]() {
                    emit containersAddedToTrain(
                        networkName,
                        QString::fromStdString(train->trainUserID));
                }, Qt::QueuedConnection));

        CHECK_TRUE(connect(train.get(), &Train::containersUnloaded, this,
                [this, train, networkName]
                (QString trainID, QString seaPortCode, QJsonArray containers)
                {
                    emit SimulatorAPI::ContainersUnloaded(networkName,
                                                          trainID,
                                                          seaPortCode,
                                                          containers);
                }, Qt::QueuedConnection));

        CHECK_TRUE(connect(train.get(), &Train::trainStateAvailable, this,
                [this, train, networkName] (QJsonObject state) {
                    emit SimulatorAPI::trainStateAvailable(
                        networkName,
                        QString::fromStdString(train->trainUserID),
                        state);
                }, Qt::QueuedConnection));
    }
}

SimulatorAPI::~SimulatorAPI() {
    // Get all network names from the thread-safe data map
    QList<QString> networkNames = apiDataMap.getNetworkNames();

    // Iterate over each network and clean up its resources
    for (const QString& networkName : networkNames) {
        // Retrieve the APIData for the current network
        APIData apiData = apiDataMap.get(networkName);

        // Clean up the worker thread
        if (apiData.workerThread) {
            apiData.workerThread->quit(); // Request thread to stop
            apiData.workerThread->wait(); // Wait for thread to finish
            delete apiData.workerThread;  // Delete the thread
            apiData.workerThread = nullptr;
        }

        // Clean up the simulator
        if (apiData.simulator) {
            apiData.simulator->terminateSimulation(false);
            delete apiData.simulator;
            apiData.simulator = nullptr;
        }

        // Clean up the network
        if (apiData.network) {
            delete apiData.network;
            apiData.network = nullptr;
        }

        // Update the APIData in the thread-safe map
        apiDataMap.addOrUpdate(networkName, apiData);

    }

    // Clear all APIData from the thread-safe map
    apiDataMap.clear();
}



// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//                            RECEIVED SIGNALS HANDLERS
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------


void SimulatorAPI::handleTrainReachedDestination(QString networkName,
                                                 QJsonObject trainState,
                                                 Mode mode)
{
    QJsonObject response;
    QJsonObject networkData;
    QJsonArray trainsArray;

    trainsArray.append(trainState);
    networkData["trainStates"] = trainsArray;
    response[networkName] = networkData;

    emit trainsReachedDestination(networkName, response);
}

void SimulatorAPI::handleResultsAvailable(QString networkName,
                                          TrainsResults result,
                                          Mode mode)
{
    // Emit single result immediately
    emit simulationResultsAvailable(networkName, result);

}

void SimulatorAPI::handleProgressUpdate(QString networkName,
                                        double simulationTime,
                                        int progressPercentage)
{
    // Create immediate response for single network
    QPair<double, int> immediateProgress =
        {simulationTime, progressPercentage};

    // Emit and clean up
    emit simulationProgressUpdated(networkName, immediateProgress);
}

void SimulatorAPI::handleTrainCoordsUpdate(
    QString networkName,
    Vector<std::pair<string,
                     Vector<std::pair<double,
                                      double>>>> simulatorTrainsCoords,
    Mode mode)
{
    auto output = Utils::convertToQtTrainsCoords(simulatorTrainsCoords);
    emit trainsCoordinatesUpdated(networkName, output);
}

void SimulatorAPI::handleOneTimeStepCompleted(QString networkName,
                                              double currentSimulatorTime,
                                              double currentSimulatorProgress,
                                              Mode mode)
{
    // 1. Store time step data atomically
    mTimeStepTracker.addUpdateData(networkName,
                                   {currentSimulatorTime,
                                    currentSimulatorProgress});

    // 2. Mark network as available
    apiDataMap.setBusy(networkName, false);

    switch (mode) {
    case Mode::Async: {
        // 3. Atomic increment and check
        const int completed = mTimeStepTracker.incrementAndGetCompleted();
        const int total = mTimeStepTracker.getRequestedCount();

        // Check if all simulators have completed their time step
        if (completed == total) {
            // 4. Get copy of time step data
            QMap<QString, QPair<double, double>> timeData =
                mTimeStepTracker.getDataBuffer();

            // 5. Emit aggregated time steps
            emit simulationAdvanced(timeData);

            // 6. Reset tracking
            mTimeStepTracker.clearAll();
        }
        break;
    }

    case Mode::Sync: {
        // 8. Create immediate response for single network
        QMap<QString, QPair<double, double>> immediateTime;
        immediateTime[networkName] = {currentSimulatorTime,
                                      currentSimulatorProgress};

        // 9. Emit and clean up
        emit simulationAdvanced(immediateTime);
        mTimeStepTracker.removeData(networkName);
        break;
    }

    default:
        // Handle unexpected mode
        emit errorOccurred("Unexpected mode in handleOneTimeStepCompleted");
        qWarning() << "Unexpected mode in handleOneTimeStepCompleted";
        break;
    }
}

void SimulatorAPI::handleSimulationFinished(QString networkName)
{

    // Immediate emission for single network
    emit simulationFinished(networkName);

    apiDataMap.setBusy(networkName, false);

}

void SimulatorAPI::handleWorkersReady(QString networkName)
{
    switch (mMode) {
    case Mode::Async: {
        // Atomic increment and check
        const int completed = mWorkerTracker.incrementAndGetCompleted();
        const int total = mWorkerTracker.getRequestedCount();

        if (completed == total) {
            // Get copy of ready networks
            QVector<QString> readyNetworks =
                mWorkerTracker.getRequestedNetworks();

            // Emit signal with prepared list
            emit workersReady(readyNetworks);

            // Reset tracking state
            mWorkerTracker.clearAll();
        }
        break;
    }

    case Mode::Sync:
        // Immediate emission for single network
        emit workersReady({networkName});
        mWorkerTracker.removeData(networkName);
        break;

    default:
        emit errorOccurred("Unexpected mode in handleWorkersReady");
        qWarning() << "Unexpected mode in handleWorkersReady";
        break;
    }
}


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
//                               GETTERS AND REQUESTS
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

Simulator* SimulatorAPI::getSimulator(QString networkName)
{
    // Check if the network exists in the thread-safe map
    if (!apiDataMap.contains(networkName)) {
        emit errorOccurred("A network with name " +
                           networkName +
                           " does not exist!");
        return nullptr;
    }

    // Retrieve the APIData for the specified network
    APIData apiData = apiDataMap.get(networkName);

    // Return the simulator instance
    return apiData.simulator;
}

Network* SimulatorAPI::getNetwork(QString networkName)
{
    // Check if the network exists in the thread-safe map
    if (!apiDataMap.contains(networkName)) {
        emit errorOccurred("A network with name " +
                           networkName +
                           " does not exist!");
        return nullptr;
    }

    // Retrieve the APIData for the specified network
    APIData apiData = apiDataMap.get(networkName);

    // Return the network instance
    return apiData.network;
}

void SimulatorAPI::requestSimulationCurrentResults(QVector<QString> networkNames)
{
    // If "*" is specified, process all networks
    if (networkNames.contains("*")) {
        networkNames = apiDataMap.getNetworkNames();
    }

    for (const auto &networkName: networkNames) {
        // Check if the network exists in the thread-safe map
        if (!apiDataMap.contains(networkName)) {
            emit errorOccurred("A network with name " +
                               networkName +
                               " does not exist!");
            return;
        }

        // Retrieve the APIData for the specified network
        APIData apiData = apiDataMap.get(networkName);

        // Check if the simulator exists
        if (apiData.simulator) {
            // Invoke the generateSummaryData method on the simulator
            bool success = QMetaObject::invokeMethod(apiData.simulator,
                                                     "generateSummaryData",
                                                     mConnectionType);
            if (!success) {
                emit errorOccurred("Failed to invoke generateSummaryData");
                qWarning() << "Failed to invoke generateSummaryData";
            }
        }
    }
}

void SimulatorAPI::requestPauseSimulation(QVector<QString> networkNames)
{
    // If "*" is specified, process all networks
    if (networkNames.contains("*")) {
        networkNames = apiDataMap.getNetworkNames();
    }

    // Reset the pause tracker
    mPauseTracker.resetCompletedRequests();
    mPauseTracker.setRequestedNetworks(networkNames);

    // Iterate over each network name
    for (const auto& networkName : networkNames) {
        // Check if the network exists in the thread-safe map
        if (!apiDataMap.contains(networkName)) {
            emit errorOccurred("A network with name " +
                               networkName +
                               " does not exist!");
            return;
        }

        // Retrieve the APIData for the specified network
        APIData apiData = apiDataMap.get(networkName);

        // Check if the simulator exists
        if (apiData.simulator) {
            // Pause the simulation
            apiData.simulator->pauseSimulation(true);
        }
    }
}

void SimulatorAPI::requestResumeSimulation(QVector<QString> networkNames)
{
    // If "*" is specified, process all networks
    if (networkNames.contains("*")) {
        networkNames = apiDataMap.getNetworkNames();
    }

    // Reset the resume tracker
    mResumeTracker.resetCompletedRequests();
    mResumeTracker.setRequestedNetworks(networkNames);

    // Iterate over each network name
    for (const auto& networkName : networkNames) {
        // Check if the network exists in the thread-safe map
        if (!apiDataMap.contains(networkName)) {
            emit errorOccurred("A network with name " +
                               networkName +
                               " does not exist!");
            return;
        }

        // Retrieve the APIData for the specified network
        APIData apiData = apiDataMap.get(networkName);

        // Check if the simulator exists
        if (apiData.simulator) {
            // Resume the simulation
            apiData.simulator->resumeSimulation(true);
        }
    }
}

void SimulatorAPI::requestTerminateSimulation(QVector<QString> networkNames)
{

    if (networkNames.contains("*")) {
        networkNames = apiDataMap.getNetworkNames();
    }

    mTerminateTracker.resetCompletedRequests();
    mTerminateTracker.setRequestedNetworks(networkNames);

    // Iterate over each network name
    for (const auto &networkName: networkNames) {
        // Check if the network exists in the thread-safe map
        if (!apiDataMap.contains(networkName)) {
            emit errorOccurred("A network with name " +
                               networkName +
                               " does not exist!");
            return;
        }

        // Retrieve the APIData for the specified network
        APIData apiData = apiDataMap.get(networkName);

        // Check if the simulator exists
        if (apiData.simulator)
        {
            apiData.simulator->terminateSimulation();
        }
    }
}

void SimulatorAPI::addTrainToSimulation(QString networkName,
                                        QVector<std::shared_ptr<Train>> trains)
{
    // Check if the network exists in the thread-safe map
    if (!apiDataMap.contains(networkName)) {
        emit errorOccurred("A network with name " +
                           networkName +
                           " does not exist!");
        return;
    }

    // Set up connections for the trains
    setupTrainsConnection(trains, networkName, mMode);

    // Retrieve the APIData for the specified network
    APIData apiData = apiDataMap.get(networkName);

    QVector<QString> IDs;
    for (auto& train : trains) {
        // Add the train to the trains map in APIData
        apiData.trains.insert(QString::fromStdString(train->trainUserID),
                              train);

        // Invoke the addTrainToSimulation method on the simulator
        bool success = QMetaObject::invokeMethod(
            apiData.simulator,
            "addTrainToSimulation",
            mConnectionType,
            Q_ARG(std::shared_ptr<Train>, train)
            );

        if (!success) {
            emit errorOccurred("Failed to invoke addTraomToSimulation");
            qWarning() << "Failed to invoke addTrainToSimulation";
        }

        // Store the train ID
        IDs.push_back(QString::fromStdString(train->trainUserID));
    }

    // Update the APIData in the thread-safe map
    apiDataMap.addOrUpdate(networkName, apiData);

    // Emit the trainsAddedToSimulation signal
    emit trainsAddedToSimulation(networkName, IDs);
}

std::shared_ptr<Train> SimulatorAPI::getTrainByID(QString networkName,
                                                  QString& trainID)
{
    // Check if the network exists in the thread-safe map
    if (!apiDataMap.contains(networkName)) {
        emit errorOccurred("A network with name " +
                           networkName +
                           " does not exist!");
        return nullptr;
    }

    // Retrieve the APIData for the specified network
    APIData apiData = apiDataMap.get(networkName);

    // Check if the train exists in the trains map
    if (apiData.trains.contains(trainID)) {
        return apiData.trains.value(trainID);
    }

    return nullptr;
}

QVector<std::shared_ptr<Train>> SimulatorAPI::getAllTrains(QString networkName)
{
    // Check if the network exists in the thread-safe map
    if (!apiDataMap.contains(networkName)) {
        emit errorOccurred("A network with name " +
                           networkName +
                           " does not exist!");
        return QVector<std::shared_ptr<Train>>();
    }

    // Retrieve the APIData for the specified network
    APIData apiData = apiDataMap.get(networkName);


    // Return all trains as a QVector
    return apiData.trains.values().toVector();
}

void SimulatorAPI::addContainersToTrain(QString networkName,
                                        QString trainID,
                                        QJsonObject json)
{
    getTrainByID(networkName, trainID)->addContainers(json);
}

void SimulatorAPI::requestRunSimulation(QVector<QString> networkNames,
                                        double timeSteps,
                                        bool endSimulationAfterRun,
                                        bool getStepEndSignal)
{
    // If "*" is specified, process all networks
    if (networkNames.contains("*")) {
        networkNames = apiDataMap.getNetworkNames();
    }
    for (const auto &networkName: networkNames) {
        // Check if the network exists in the thread-safe map
        if (!apiDataMap.contains(networkName)) {
            emit errorOccurred("A network with name " +
                               networkName +
                               " does not exist!");
            return;
        }

        mReachedDesTracker.setRequestedNetworks(networkNames);

        // Retrieve the APIData for the specified network
        APIData apiData = apiDataMap.get(networkName);

        // Check if the simulator exists
        if (apiData.simulator) {
            // Flag the simulator as busy
            apiDataMap.setBusy(networkName, true);

            // Invoke the runSimulation method on the simulator
            bool success = QMetaObject::invokeMethod(
                apiData.simulator,
                "runSimulation",
                mConnectionType,
                Q_ARG(double, timeSteps),
                Q_ARG(bool, endSimulationAfterRun),
                Q_ARG(bool, getStepEndSignal)
                );

            if (!success) {
                qWarning() << "Failed to invoke runSimulation";
            }
        }
    }

    mReachedDesTracker.resetCompletedRequests();
    mReachedDesTracker.setRequestedNetworks(networkNames);
}

void SimulatorAPI::requestFinalizeSimulation(QVector<QString> networkNames)
{
    // If "*" is specified, process all networks
    if (networkNames.contains("*")) {
        networkNames = apiDataMap.getNetworkNames();
    }

    // Iterate over each network name
    for (const auto& networkName : networkNames) {
        // Check if the network exists in the thread-safe map
        if (!apiDataMap.contains(networkName)) {
            emit errorOccurred("A network with name " +
                               networkName +
                               " does not exist!");
            return;
        }

        // Retrieve the APIData for the specified network
        APIData apiData = apiDataMap.get(networkName);

        // Check if the simulator exists
        if (apiData.simulator) {
            // Invoke the finalizeSimulation method on the simulator
            bool success = QMetaObject::invokeMethod(
                apiData.simulator,
                "finalizeSimulation",
                mConnectionType
                );

            if (!success) {
                qWarning() << "Failed to invoke finalizeSimulation";
            }
        }
    }
}

void SimulatorAPI::requestUnloadContainersAtTerminal(
    QString networkName,
    QString trainID,
    QVector<QString> portNames)
{
    // Check if the network exists in the thread-safe map
    if (!apiDataMap.contains(networkName)) {
        emit errorOccurred("A network with name " +
                           networkName +
                           " does not exist!");
        return;
    }

    // Retrieve the APIData for the specified network
    APIData apiData = apiDataMap.get(networkName);

    // Check if the train exists in the trains map
    if (apiData.trains.contains(trainID)) {
        // Request train to unload containers
        apiData.trains.value(trainID)->
            requestUnloadContainersAtTerminal(portNames);
    }

    // Emit an error if the train does not exist
    emit errorOccurred("A train with ID " + trainID + " does not exist!");
}


void SimulatorAPI::checkAndEmitSignal(
    const int& counter, const int total,
    const QVector<QString>& networkNames,
    void(SimulatorAPI::*signal)(QVector<QString>),
    Mode mode) {

    switch (mode) {
    case Mode::Async:
        // In async mode, emit signal when counter reaches total
        if (counter == total) {
            (this->*signal)(networkNames);
        }
        break;

    case Mode::Sync:
        // In sync mode, emit signal immediately
        (this->*signal)(networkNames);
        break;

    default:
        // Handle unexpected mode
        qWarning() << "Unexpected mode in checkAndEmitSignal";
        break;
    }
}

// -----------------------------------------------------------------------------
// ---------------------------- Interactive Mode -------------------------------
// -----------------------------------------------------------------------------

SimulatorAPI& SimulatorAPI::InteractiveMode::getInstance() {
    return SimulatorAPI::getInstance();
}

void SimulatorAPI::InteractiveMode::resetAPI()
{
    SimulatorAPI::resetInstance();
}

void SimulatorAPI::InteractiveMode::createNewSimulationEnvironment(
    QJsonObject nodesFileContent, QJsonObject linksFileContent,
    QString networkName, QVector<QMap<QString, std::any>> trainList,
    double timeStep, Mode mode)
{
    mMode = mode;

    getInstance().createNewSimulationEnvironment(nodesFileContent,
                                                 linksFileContent,
                                                 networkName,
                                                 trainList,
                                                 timeStep,
                                                 mode);
}

void SimulatorAPI::InteractiveMode::createNewSimulationEnvironment(
    QString networkName,
    QVector<QMap<QString, QString> > &nodeRecords,
    QVector<QMap<QString, QString> > &linkRecords,
    QVector<QMap<QString, std::any>> &trainList,
    double timeStep,
    Mode mode)
{
    mMode = mode;

    getInstance().createNewSimulationEnvironment(networkName,
                                                 nodeRecords,
                                                 linkRecords,
                                                 trainList,
                                                 timeStep,
                                                 mode);
}

void SimulatorAPI::InteractiveMode::createNewSimulationEnvironmentFromFiles(
    QString nodesFile, QString linksFile,
    QString networkName, QString trainsFile,
    double timeStep, Mode mode)
{
    mMode = mode;

    getInstance().createNewSimulationEnvironmentFromFiles(
        nodesFile, linksFile, networkName,
        trainsFile, timeStep, mode);
}


void SimulatorAPI::InteractiveMode::addTrainToSimulation(
    QString networkName, QVector<std::shared_ptr<Train>> trains)
{
    getInstance().addTrainToSimulation(networkName, trains);
}

void SimulatorAPI::InteractiveMode::
    runSimulation(QVector<QString> networkNames,
                  double timeSteps,
                  bool getProgressSignal)
{
    bool endSimulationAfterRun = false;

    if (timeSteps < 0) {
        timeSteps = std::numeric_limits<double>::infinity();
    }

    getInstance().requestRunSimulation(networkNames, timeSteps,
                                       endSimulationAfterRun,
                                       getProgressSignal);
}

void SimulatorAPI::InteractiveMode::
    finalizeSimulation(QVector<QString> networkNames)
{
    getInstance().requestFinalizeSimulation(networkNames);
}

void SimulatorAPI::InteractiveMode::
    terminateSimulation(QVector<QString> networkNames)
{
    getInstance().requestTerminateSimulation(networkNames);
}

Simulator* SimulatorAPI::InteractiveMode::getSimulator(QString networkName) {
    return getInstance().getSimulator(networkName);
}

Network* SimulatorAPI::InteractiveMode::getNetwork(QString networkName) {
    return getInstance().getNetwork(networkName);
}

std::shared_ptr<Train>
SimulatorAPI::InteractiveMode::getTrainByID(
    QString networkName, QString& trainID)
{
    return getInstance().getTrainByID(networkName, trainID);
}

QVector<std::shared_ptr<Train>>
SimulatorAPI::InteractiveMode::getAllTrains(QString networkName)
{
    return getInstance().getAllTrains(networkName);
}

void SimulatorAPI::InteractiveMode::addContainersToTrain(QString networkName,
                                                         QString trainID,
                                                         QJsonObject json)
{
    getInstance().addContainersToTrain(networkName, trainID, json);
}

QVector<std::shared_ptr<Train> >
SimulatorAPI::InteractiveMode::getTrains(QString networkName) {
    return getInstance().getAllTrains(networkName);
}


void SimulatorAPI::InteractiveMode::
    requestUnloadContainersAtTerminal(QString networkName,
                                  QString trainID, QVector<QString> portNames)
{
    getInstance().requestUnloadContainersAtTerminal(
        networkName, trainID, portNames);
}

// -----------------------------------------------------------------------------
// ----------------------------- Continuous Mode -------------------------------
// -----------------------------------------------------------------------------

SimulatorAPI& SimulatorAPI::ContinuousMode::getInstance() {
    return SimulatorAPI::getInstance();
}

void SimulatorAPI::ContinuousMode::resetAPI()
{
    SimulatorAPI::resetInstance();
}

void SimulatorAPI::ContinuousMode::createNewSimulationEnvironment(
    QJsonObject nodesFileContent,
    QJsonObject linksFileContent,
    QString networkName,
    QVector<QMap<QString, std::any>> trainList,
    double timeStep,
    Mode mode)
{
    mMode = mode;
    getInstance().createNewSimulationEnvironment(nodesFileContent,
                                                 linksFileContent,
                                                 networkName,
                                                 trainList,
                                                 timeStep,
                                                 mode);
}

void SimulatorAPI::ContinuousMode::createNewSimulationEnvironment(
    QString networkName,
    QVector<QMap<QString, QString> > &nodeRecords,
    QVector<QMap<QString, QString> > &linkRecords,
    QVector<QMap<QString, std::any>> &trainList,
    double timeStep, Mode mode)
{
    mMode = mode;

    getInstance().createNewSimulationEnvironment(networkName,
                                                 nodeRecords,
                                                 linkRecords,
                                                 trainList,
                                                 timeStep,
                                                 mode);
}

void SimulatorAPI::ContinuousMode::createNewSimulationEnvironmentFromFiles(
    QString nodesFile,
    QString linksFile,
    QString networkName,
    QString trainsFile,
    double timeStep,
    Mode mode)
{
    mMode = mode;
    getInstance().createNewSimulationEnvironmentFromFiles(
        nodesFile, linksFile, networkName,
        trainsFile, timeStep, mode);
}

void SimulatorAPI::ContinuousMode::runSimulation(
    QVector<QString> networkNames, bool getProgressSignal)
{
    double timeSteps = std::numeric_limits<double>::infinity();
    bool endSimulationAfterRun = true;

    getInstance().requestRunSimulation(networkNames, timeSteps,
                                       endSimulationAfterRun,
                                       getProgressSignal);

}

void SimulatorAPI::ContinuousMode::
    pauseSimulation(QVector<QString> networkNames)
{
    getInstance().requestPauseSimulation(networkNames);
}

void SimulatorAPI::ContinuousMode::
    resumeSimulation(QVector<QString> networkNames)
{
    getInstance().requestResumeSimulation(networkNames);
}

void SimulatorAPI::ContinuousMode::
    terminateSimulation(QVector<QString> networkNames)
{
    getInstance().requestTerminateSimulation(networkNames);
}

Simulator* SimulatorAPI::ContinuousMode::getSimulator(QString networkName) {
    return getInstance().getSimulator(networkName);
}

Network* SimulatorAPI::ContinuousMode::getNetwork(QString networkName) {
    return getInstance().getNetwork(networkName);
}

void SimulatorAPI::ContinuousMode::addContainersToTrain(QString networkName,
                                                         QString trainID,
                                                         QJsonObject json)
{
    getInstance().addContainersToTrain(networkName, trainID, json);
}

QVector<std::shared_ptr<Train> >
SimulatorAPI::ContinuousMode::getTrains(QString networkName) {
    return getInstance().getAllTrains(networkName);
}

void SimulatorAPI::ContinuousMode::
    requestUnloadContainersAtTerminal(QString networkName,
                                  QString shipID,
                                  QVector<QString> portNames)
{
    getInstance().requestUnloadContainersAtTerminal(networkName,
                                                    shipID,
                                                    portNames);
}
