#include "simulatorapi.h"
#include "./traindefinition/trainslist.h"
#include <QThread>
#include <QMetaObject> // Required for QMetaObject::invokeMethod and Q_ARG
#include <QVariant>    // Required for QVariant types
#include <QThread>     // Required for QThread
#include <QDebug>      // For debugging

SimulatorAPI::Mode SimulatorAPI::mMode = Mode::Sync;
std::unique_ptr<SimulatorAPI> SimulatorAPI::instance(new SimulatorAPI());

void SimulatorAPI::registerQMeta() {
    // Register enums
    qRegisterMetaType<SimulatorAPI::Mode>("SimulatorAPI::Mode");

    // Register Train and its components
    qRegisterMetaType<Train>("Train");
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
    if (!instance) {
        registerQMeta();
        instance.reset(new SimulatorAPI());
    }
    return *instance;
}

void SimulatorAPI::resetInstance() {
    if (instance) {
        for (auto& data : instance->mData) {
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

            // Clear ships
            data.trains.clear();
        }

        instance->mData.clear(); // Clear all APIData
        instance.reset();        // Destroy the singleton instance
    }

    registerQMeta();
    instance.reset(new SimulatorAPI()); // Create a new instance
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


void SimulatorAPI::
    createNewSimulationEnvironment(
        QString nodesFileContent,
        QString linksFileContent,
        QString networkName,
        QVector<QMap<QString, std::any>> trainsData,
        double timeStep,
        Mode mode)
{
    // Set locale to US format (no thousands separator, dot for decimals)
    setLocale();

    if (mData.contains(networkName)) {
        emit errorOccurred("A network with name " + networkName + " exists!");
        return;
    }
    auto nodeRecords =
        ReadWriteNetwork::readNodesFileContent(nodesFileContent.toStdString());

    auto linkRecords =
        ReadWriteNetwork::readLinksFileContent(linksFileContent.toStdString());

    auto nodeRecordsf = Utils::convertToQVectorString(nodeRecords);
    auto linkRecordsf = Utils::convertToQVectorString(linkRecords);


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

    if (mData.contains(networkName)) {
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

    if (mData.contains(networkName)) {
        emit errorOccurred("A network with name " + networkName + " exists!");
        return;
    }

    setupSimulator(networkName, nodeRecords, linkRecords,
                   trainList, timeStep, mode);

    emit simulationCreated(networkName);
}

void SimulatorAPI::
    setupSimulator(QString &networkName,
                   QVector<QMap<QString, QString>> &nodeRecords,
                   QVector<QMap<QString, QString>> &linkRecords,
                   QVector<QMap<QString, std::any>> &trainList,
                   double &timeStep,
                   Mode mode) {

    // Create a new thread for simulation
    QThread* workerThread = new QThread(this);

    // Initialize APIData and store the working thread
    APIData apiData;
    apiData.workerThread = workerThread;
    mData.insert(networkName, std::move(apiData)); // Use move semantics for efficiency

    // Create the worker
    SimulatorWorker* simulatorWorker = new SimulatorWorker();

    // Move the worker to the thread
    simulatorWorker->moveToThread(workerThread);

    // Store the worker in APIData
    mData[networkName].simulatorWorker = simulatorWorker;

    // Event loop to block until setupSimulator is complete
    QEventLoop loop;

    // Connect signals and slots
    QObject::connect(simulatorWorker, &SimulatorWorker::simulatorLoaded, [&]() {
        qInfo() << "Simulator loaded successfully for network:" << networkName;

        // Update APIData safely
        QMetaObject::invokeMethod(qApp, [&]() {
            mData[networkName].simulatorWorker = simulatorWorker;
            mData[networkName].workerThread = workerThread;
        });

        // Exit the event loop
        loop.quit();
    });

    QObject::connect(simulatorWorker, &SimulatorWorker::errorOccured, [&](const QString& error) {
        qWarning() << "Error occurred while setting up simulator for network:" << networkName << " - " << error;

        // Exit the event loop in case of error
        loop.quit();

        // Stop and clean up the worker thread
        workerThread->quit();
    });

    // Start the thread
    workerThread->start();

    // Use QMetaObject::invokeMethod to call setupSimulator in the worker thread
    QMetaObject::invokeMethod(simulatorWorker, [=, this]() {
        simulatorWorker->setupSimulator(mData[networkName], networkName, nodeRecords, linkRecords, trainList, timeStep);
    });

    // Block until setupSimulator is complete
    loop.exec();

    // Set up permanent connections for this network
    setupConnections(networkName, mode);

    qInfo() << "Simulator setup complete for network:" << networkName;
}


void SimulatorAPI::setupConnections(const QString& networkName, Mode mode)
{
    auto& simulator = mData[networkName].simulator;

    connect(simulator, &Simulator::resultDataAvailable, this,
            [this, networkName, mode](TrainsResults results) {
                handleResultsAvailable(networkName, results, mode);
            }, Qt::QueuedConnection);

    connect(simulator, &Simulator::simulationTimeAdvanced, this,
            [this, networkName, mode](double currentSimulatorTime,
                                      double simulatorProgress) {
                handleOneTimeStepCompleted(networkName,
                                           currentSimulatorTime,
                                           simulatorProgress, mode);
            }, Qt::QueuedConnection);

    connect(simulator, &Simulator::simulationReachedReportingTime, this,
            [this, networkName, mode](double currentSimulatorTime,
                                      double simulatorProgress) {
                handleReportingTimeReached(networkName,
                                           currentSimulatorTime,
                                           simulatorProgress, mode);
            }, Qt::QueuedConnection);

    connect(simulator, &Simulator::simulatorInitialized, this,
            [this, networkName, mode]() {
                m_completedSimulatorsInitialization++;
                checkAndEmitSignal(m_completedSimulatorsInitialization,
                                   m_totalSimulatorsInitializationRequested.size(),
                                   m_totalSimulatorsInitializationRequested,
                                   &SimulatorAPI::simulationsInitialized,
                                   mode);
            }, Qt::QueuedConnection);

    connect(simulator, &Simulator::progressUpdated, this,
            [this, networkName, mode](double simulationTime, int progress) {
                emit simulationProgressUpdated({networkName,
                                                {simulationTime, progress}});
            }, Qt::QueuedConnection);

    connect(simulator, &Simulator::simulatorPaused, this,
            [this, networkName, mode]() {
                m_completedSimulatorsPaused++;
                checkAndEmitSignal(m_completedSimulatorsPaused,
                                   m_totalSimulatorPauseRequested.size(),
                                   m_totalSimulatorPauseRequested,
                                   &SimulatorAPI::simulationsPaused,
                                   mode);
            }, Qt::QueuedConnection);

    connect(simulator, &Simulator::simulatorResumed, this,
            [this, networkName, mode]() {
                m_completedSimulatorsResumed++;
                checkAndEmitSignal(m_completedSimulatorsResumed,
                                   m_totalSimulatorResumeRequested.size(),
                                   m_totalSimulatorResumeRequested,
                                   &SimulatorAPI::simulationsResumed,
                                   mode);
            }, Qt::QueuedConnection);

    connect(simulator, &Simulator::simulatorTerminated, this,
            [this, networkName, mode]() {
                m_completedSimulatorsTerminated++;
                checkAndEmitSignal(m_completedSimulatorsTerminated,
                                   m_totalSimulatorTerminatedRequested.size(),
                                   m_totalSimulatorTerminatedRequested,
                                   &SimulatorAPI::simulationsTerminated,
                                   mode);
            }, Qt::QueuedConnection);

    connect(simulator, &Simulator::simulationFinished, this,
            [this, networkName, mode]() {
                m_completedSimulatorsFinished++;
                checkAndEmitSignal(m_completedSimulatorsFinished,
                                   m_totalSimulatorFinishedRequested.size(),
                                   m_totalSimulatorFinishedRequested,
                                   &SimulatorAPI::simulationsFinished,
                                   mode);
            }, Qt::QueuedConnection);

    connect(simulator, &Simulator::plotTrainsUpdated,
            this, [this, networkName, mode]
            (Vector<std::pair<std::string,
                              Vector<std::pair<double,
                                               double>>>> trainsStartEndPoints) {
                handleTrainCoordsUpdate(networkName, trainsStartEndPoints, mode);
            }, Qt::QueuedConnection);

    connect(simulator, &Simulator::errorOccurred, this,
            [this, networkName](QString error) {
                emit errorOccurred(QString("Error in Network ") + networkName +
                                   QString(": ") + error);
            }, Qt::QueuedConnection);

    connect(simulator, &Simulator::trainsAddedToSimulation, this,
            [this, networkName](QVector<QString> trainIDs) {
                emit trainAddedToSimulation(networkName, trainIDs);
            }, Qt::QueuedConnection);

    connect(simulator, &Simulator::trainReachedTerminal, this,
            [this, networkName](QString trainID, QString terminalID,
                                QJsonArray containers) {
        emit trainReachedTerminal(networkName, trainID,
                                  terminalID, containers);
    });

    for (const auto& train : mData[networkName].trains) {
        connect(train.get(), &Train::destinationReached, this,
                [this, networkName, train, mode](QJsonObject state) {
                    handleTrainReachedDestination(
                        networkName,
                        state,
                        mode);
                }, Qt::QueuedConnection);
        connect(train.get(), &Train::containersAdded, this,
                [this, networkName, train]() {
                    emit containersAddedToTrain(
                        networkName,
                        QString::fromStdString(train->trainUserID));
                });
    }
}

SimulatorAPI::~SimulatorAPI() {
    for (auto& data : mData) {
        data.workerThread->quit();
        data.workerThread->wait();
        delete data.workerThread;

        if (data.simulator) {
            delete data.simulator;
        }

        if (data.network) {
            delete data.network;
        }
    }
}

Simulator* SimulatorAPI::getSimulator(QString networkName)
{
    if (!mData.contains(networkName)) {
        emit errorOccurred("A network with name " + networkName +
                           " does not exist!");
        return nullptr;
    }
    return mData[networkName].simulator;
}

Network* SimulatorAPI::getNetwork(QString networkName)
{
    if (!mData.contains(networkName)) {
        emit errorOccurred("A network with name " + networkName +
                           " does not exist!");
        return nullptr;
    }
    return mData[networkName].network;
}

void SimulatorAPI::requestSimulationCurrentResults(QVector<QString> networkNames)
{
    if (networkNames.contains("*")) {
        networkNames = mData.keys();
    }
    for (const auto &networkName: networkNames) {
        if (!mData.contains(networkName)) {
            emit errorOccurred("A network with name " + networkName +
                               " does not exist!");
            return;
        }
        if (mData[networkName].simulator) {
            QMetaObject::invokeMethod(mData[networkName].simulator,
                                      "generateSummaryData");
        }
    }
}

void SimulatorAPI::addTrainToSimulation(QString networkName,
                                        QVector<std::shared_ptr<Train>> trains)
{
    if (!mData.contains(networkName)) {
        emit errorOccurred("A network with name " + networkName +
                           " does not exist!");
        return;
    }

    for (auto train: trains) {
        connect(train.get(), &Train::destinationReached,
                [this, train, networkName](QJsonObject state) {
                    handleTrainReachedDestination(networkName, state, mMode);
                });
        connect(train.get(), &Train::containersAdded, this,
                [this, networkName, train]() {
                    emit containersAddedToTrain(
                        networkName,
                        QString::fromStdString(train->trainUserID));
                });

        mData[networkName].trains.insert(
            QString::fromStdString(train->trainUserID), train);

        QMetaObject::invokeMethod(mData[networkName].simulator,
                                  "addTrainToSimulation",
                                  Q_ARG(std::shared_ptr<Train>, train));

    }

}

std::shared_ptr<Train> SimulatorAPI::getTrainByID(QString networkName,
                                                  QString& trainID)
{
    if (!mData.contains(networkName)) {
        emit errorOccurred("A network with name " + networkName +
                           " does not exist!");
        return nullptr;
    }
    if (mData[networkName].trains.contains(trainID)) {
        return mData[networkName].trains.value(trainID);
    }
    return nullptr;
}

QVector<std::shared_ptr<Train>> SimulatorAPI::getAllTrains(QString networkName)
{
    if (!mData.contains(networkName)) {
        emit errorOccurred("A network with name " + networkName +
                           " does not exist!");
        return QVector<std::shared_ptr<Train>>();
    }
    return QVector<std::shared_ptr<Train>>(
        mData[networkName].trains.values().begin(),
        mData[networkName].trains.values().end());
}

void SimulatorAPI::addContainersToTrain(QString networkName,
                                        QString trainID,
                                        QJsonObject json)
{
    QMetaObject::invokeMethod(getTrainByID(networkName, trainID).get(),
                              "addContainers",
                              Qt::QueuedConnection,
                              Q_ARG(QJsonObject, json));

}

QVector<std::shared_ptr<Train>> SimulatorAPI::getTrains(QString networkName)
{
    return mData[networkName].trains.values().toVector();
}

void SimulatorAPI::handleTrainReachedDestination(QString networkName,
                                                 QJsonObject trainState,
                                                 Mode mode)
{
    m_trainsReachedBuffer[networkName].append(trainState);

    if (mode == Mode::Sync) {
        emit trainsReachedDestination(m_trainsReachedBuffer);
        m_trainsReachedBuffer.clear();
    }
}

void SimulatorAPI::handleTrainCoordsUpdate(
    QString networkName,
    Vector<std::pair<string,
                     Vector<std::pair<double,
                                      double>>>> simulatorTrainsCoords,
    Mode mode)
{
    auto output = Utils::convertToQtTrainsCoords(simulatorTrainsCoords);
    m_trainsCoordsUpdated[networkName].append(output);
    m_processedTrainCoordsUpdate++;

    if (mode == Mode::Async) {
        emit trainsCoordinatesUpdated(m_trainsCoordsUpdated);
        m_trainsCoordsUpdated.clear();
        m_processedTrainCoordsUpdate = 0;
    } else {
        if (m_trainsCoordsUpdated.keys().size() == m_processedTrainCoordsUpdate) {
            emit trainsCoordinatesUpdated(m_trainsCoordsUpdated);
            m_trainsCoordsUpdated.clear();
            m_processedTrainCoordsUpdate = 0;
        }
    }

}

void SimulatorAPI::handleOneTimeStepCompleted(QString networkName,
                                              double currentSimulatorTime,
                                              double currentSimulatorProgress,
                                              Mode mode)
{
    m_timeStepData.insert(networkName, {currentSimulatorTime, currentSimulatorProgress});

    switch (mode) {
    case Mode::Async:
        // Increment the number of completed simulators
        m_completedSimulatorsTimeStep++;

        // Check if all simulators have completed their time step
        if (m_completedSimulatorsTimeStep == mData.size()) {
            // Emit the aggregated signal
            emit simulationAdvanced(m_timeStepData);

            // Emit the aggregated signal
            emitTrainsReachedDestination();

            m_completedSimulatorsTimeStep = 0;  // Reset the counter for the next time step
        }
        break;

    case Mode::Sync:
        // In sync mode, emit results immediately for each network
        emit simulationAdvanced(m_timeStepData);
        break;

    default:
        // Handle unexpected mode
        qWarning() << "Unexpected mode in handleResultsAvailable";
        break;
    }
}

void SimulatorAPI::handleReportingTimeReached(QString networkName,
                                              double currentSimulatorTime,
                                              double currentSimulatorProgress,
                                              Mode mode)
{
    m_ReportingTimeData.insert(networkName, {currentSimulatorTime, currentSimulatorProgress});

    switch (mode) {
    case Mode::Async:
        // Increment the number of completed simulators
        m_completedSimulatorsTimeStep++;

        // Check if all simulators have completed their time step
        if (m_completedSimulatorsTimeStep == mData.size()) {
            // Emit the aggregated signal
            emit simulationReachedReportingTime(m_ReportingTimeData);

            // Emit the aggregated signal
            emitTrainsReachedDestination();

            m_completedSimulatorsTimeStep = 0;  // Reset the counter for the next time step
        }
        break;

    case Mode::Sync:
        // In sync mode, emit results immediately for each network
        emit simulationReachedReportingTime(m_ReportingTimeData);
        break;

    default:
        // Handle unexpected mode
        qWarning() << "Unexpected mode in handleResultsAvailable";
        break;
    }
}

void SimulatorAPI::handleResultsAvailable(QString networkName,
                                          TrainsResults result,
                                          Mode mode)
{

    // Always store the result in the buffer
    m_simulationResultBuffer.insert(networkName, result);

    switch (mode) {
    case Mode::Async:
        // Increment the number of completed simulators
        m_completedSummaryResults++;

        // Check if all simulators have completed their time step
        if (m_completedSummaryResults == mData.size()) {
            // Emit the aggregated signal
            emitSimulationResults(m_simulationResultBuffer);
            m_completedSummaryResults = 0;  // Reset the counter for the next time step
        }
        break;

    case Mode::Sync:
        // In sync mode, emit results immediately for each network
        emitSimulationResults(m_simulationResultBuffer);
        break;

    default:
        // Handle unexpected mode
        qWarning() << "Unexpected mode in handleResultsAvailable";
        break;
    }

}

void SimulatorAPI::emitTrainsReachedDestination()
{
    if (!m_trainsReachedBuffer.isEmpty()) {
        emit trainsReachedDestination(m_trainsReachedBuffer);
        m_trainsReachedBuffer.clear();
    }
}

void SimulatorAPI::emitSimulationResults(QMap<QString, TrainsResults> &results)
{
    if (!results.isEmpty()) {
        emit simulationResultsAvailable(results);
        results.clear();
    }
}

void SimulatorAPI::checkAndEmitSignal(
    int& counter, int total,
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
    QString nodesFileContent, QString linksFileContent,
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
    runOneTimeStep(QVector<QString> networkNames)
{
    if (networkNames.contains("*")) {
        networkNames = getInstance().mData.keys();
    }
    for (const auto &networkName: networkNames) {
        if (!getInstance().mData.contains(networkName)) {
            emit getInstance().errorOccurred("A network with name "
                                             + networkName +
                                             " does not exist!");
            return;
        }
        if (getInstance().mData[networkName].simulator) {
            QMetaObject::invokeMethod(
                getInstance().mData[networkName].simulator,
                "runOneTimeStep");
        }
    }
}

void SimulatorAPI::InteractiveMode::
    runSimulation(QVector<QString> networkNames, double timeSteps) {
    if (networkNames.contains("*")) {
        networkNames = getInstance().mData.keys();
    }
    for (const auto &networkName: networkNames) {
        if (!getInstance().mData.contains(networkName)) {
            emit getInstance().errorOccurred("A network with name "
                                             + networkName +
                                             " does not exist!");
            return;
        }
        if (timeSteps < 0) {
            if (getInstance().mData[networkName].simulator) {
                if (getInstance().mData[networkName].workerThread->isRunning()){
                    qDebug() << "RUNNIG";
                }
                else {
                    qDebug() << "NOT RUNNIG";
                }
                QMetaObject::invokeMethod(
                    getInstance().mData[networkName].simulator,
                    "runSimulation");
            }
        }
        else {
            if (getInstance().mData[networkName].simulator) {
                QMetaObject::invokeMethod(
                    getInstance().mData[networkName].simulator,
                    "runBy",
                    Q_ARG(double, timeSteps));
            }
        }

    }
}

void SimulatorAPI::InteractiveMode::
    initSimulation(QVector<QString> networkNames)
{
    if (networkNames.contains("*")) {
        networkNames = getInstance().mData.keys();
        getInstance().m_totalSimulatorsInitializationRequested = networkNames;
    }

    getInstance().m_completedSimulatorsInitialization = 0;

    for (const auto &networkName: networkNames) {
        if (!getInstance().mData.contains(networkName)) {
            emit getInstance().errorOccurred("A network with name "
                                             + networkName +
                                             " does not exist!");
            return;
        }
        if (getInstance().mData[networkName].simulator) {
            QMetaObject::invokeMethod(
                getInstance().mData[networkName].simulator,
                "initializeSimulator");
        }
    }
}

void SimulatorAPI::InteractiveMode::endSimulation(QVector<QString> networkNames)
{
    if (networkNames.contains("*")) {
        networkNames = getInstance().mData.keys();
    }

    getInstance().m_completedSimulatorsTerminated = 0;
    getInstance().m_totalSimulatorTerminatedRequested = networkNames;

    for (const auto &networkName: networkNames) {
        if (!getInstance().mData.contains(networkName)) {
            emit getInstance().errorOccurred("A network with name "
                                             + networkName +
                                             " does not exist!");
            return;
        }
        if (getInstance().mData[networkName].simulator) {
            QMetaObject::invokeMethod(
                getInstance().mData[networkName].simulator,
                "finalizeSimulation");
        }
    }
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
    return getInstance().getTrains(networkName);
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
    QString nodesFileContent,
    QString linksFileContent,
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
    QVector<QString> networkNames)
{
    if (networkNames.contains("*")) {
        networkNames = getInstance().mData.keys();
    }

    getInstance().m_completedSimulatorsRun = 0;
    getInstance().m_totalSimulatorsRunRequested = networkNames;

    for (const auto &networkName: networkNames) {
        if (!getInstance().mData.contains(networkName)) {
            emit getInstance().errorOccurred("A network with name "
                                             + networkName +
                                             " does not exist!");
            return;
        }
        if (getInstance().mData[networkName].simulator) {
            QMetaObject::invokeMethod(
                getInstance().mData[networkName].simulator,
                "runSimulation");
        }
    }

}

void SimulatorAPI::ContinuousMode::
    pauseSimulation(QVector<QString> networkNames)
{
    if (networkNames.contains("*")) {
        networkNames = getInstance().mData.keys();
    }

    getInstance().m_completedSimulatorsPaused = 0;
    getInstance().m_totalSimulatorPauseRequested = networkNames;

    for (const auto &networkName: networkNames) {
        if (!getInstance().mData.contains(networkName)) {
            emit getInstance().errorOccurred("A network with name "
                                             + networkName +
                                             " does not exist!");
            return;
        }
        if (getInstance().mData[networkName].simulator) {
            getInstance().mData[networkName].simulator->pauseSimulation(true);
        }
    }
}

void SimulatorAPI::ContinuousMode::
    resumeSimulation(QVector<QString> networkNames)
{
    if (networkNames.contains("*")) {
        networkNames = getInstance().mData.keys();
    }

    getInstance().m_completedSimulatorsResumed = 0;
    getInstance().m_totalSimulatorResumeRequested = networkNames;

    for (const auto &networkName: networkNames) {
        if (!getInstance().mData.contains(networkName)) {
            emit getInstance().errorOccurred("A network with name "
                                             + networkName +
                                             " does not exist!");
            return;
        }
        if (getInstance().mData[networkName].simulator) {
            getInstance().mData[networkName].simulator->resumeSimulation(true);
        }
    }
}

void SimulatorAPI::ContinuousMode::endSimulation(QVector<QString> networkNames)
{
    if (networkNames.contains("*")) {
        networkNames = getInstance().mData.keys();
    }

    getInstance().m_completedSimulatorsTerminated = 0;
    getInstance().m_totalSimulatorTerminatedRequested = networkNames;

    for (const auto &networkName: networkNames) {
        if (!getInstance().mData.contains(networkName)) {
            emit getInstance().errorOccurred("A network with name "
                                             + networkName +
                                             " does not exist!");
            return;
        }
        if (getInstance().mData[networkName].simulator) {
            QMetaObject::invokeMethod(
                getInstance().mData[networkName].simulator,
                "finalizeSimulation");
        }
    }
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
    return getInstance().getTrains(networkName);
}
