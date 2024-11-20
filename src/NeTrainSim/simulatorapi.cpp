#include "simulatorapi.h"
#include "./traindefinition/trainslist.h"
#include <QThread>

SimulatorAPI::Mode SimulatorAPI::mMode = Mode::Sync;
std::unique_ptr<SimulatorAPI> SimulatorAPI::instance(new SimulatorAPI());

SimulatorAPI& SimulatorAPI::getInstance() {
    if (!instance) {
        instance.reset(new SimulatorAPI());
    }
    return *instance;
}

void SimulatorAPI::resetInstance() {
    if (instance) {
        // Clean up existing resources
        for (auto& data : instance->mData) {
            if (data.simulator) {
                data.simulator->deleteLater();
            }
            if (data.network) {
                delete data.network;
            }
            if (data.workerThread) {
                data.workerThread->quit();
                data.workerThread->wait();
                delete data.workerThread;
            }
        }
        instance->mData.clear();

        // Destroy the current instance
        instance.reset();
    }

    // Create a new instance
    instance.reset(new SimulatorAPI());
}



void SimulatorAPI::
    createNewSimulationEnvironment(QString nodesFileContent,
                                   QString linksFileContent,
                                   QString networkName,
                                   QVector<std::shared_ptr<Train>> trainList,
                                   double timeStep,
                                   Mode mode)
{
    // Set locale to US format (comma for thousands separator, dot for decimals)
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    std::locale::global(std::locale("en_US.UTF-8"));
    std::cout.imbue(std::locale());

    if (mData.contains(networkName)) {
        emit errorOccurred("A network with name " + networkName + " exists!");
        return;
    }
    auto nodeRecords =
        ReadWriteNetwork::readNodesFileContent(nodesFileContent.toStdString());

    auto linkRecords =
        ReadWriteNetwork::readLinksFileContent(linksFileContent.toStdString());

    setupSimulator(networkName, nodeRecords, linkRecords,
                   trainList, timeStep, mode);

    emit simulationCreated(networkName);

}

void SimulatorAPI::
    setupSimulator(QString &networkName,
                   Vector<Map<std::string, std::string>> &nodeRecords,
                   Vector<Map<std::string, std::string>> &linkRecords,
                   QVector<std::shared_ptr<Train>> &trainList,
                   double &timeStep,
                   Mode mode) {

    auto nodes = ReadWriteNetwork::generateNodes(nodeRecords);
    auto links = ReadWriteNetwork::generateLinks(nodes, linkRecords);

    APIData apiData;

    std::cout << "Reading Network: " << networkName.toStdString()
              << "!                    \r";
    apiData.network = new Network(nodes, links, networkName.toStdString());
    std::cout << "Define Simulator Space for network: "
              << networkName.toStdString() << "!                          \r";
    Vector<std::shared_ptr<Train>> trains;
    for (const auto& train: trainList) {
        trains.push_back(train); // Share ownership
        apiData.trains[train->trainUserID] = train;
    }
    apiData.simulator = new Simulator(apiData.network, trains, timeStep);
    apiData.workerThread = new QThread(this);

    mData.insert(networkName, std::move(apiData));

    // Set up permanent connections for this network
    setupConnections(networkName, mode);

    mData[networkName].simulator->moveToThread(mData[networkName].workerThread);
    mData[networkName].workerThread->start();
}

void SimulatorAPI::setupConnections(const QString& networkName, Mode mode)
{
    auto& simulator = mData[networkName].simulator;

    connect(simulator, &Simulator::resultDataAvailable, this,
            [this, networkName, &mode](TrainsResults results) {
                handleResultsAvailable(networkName, results, mode);
            });

    connect(simulator, &Simulator::simulationTimeAdvanced, this,
            [this, networkName, &mode](double currentSimulatorTime) {
                handleOneTimeStepCompleted(networkName,
                                           currentSimulatorTime, mode);
            });

    connect(simulator, &Simulator::simulationReachedReportingTime, this,
            [this, networkName, &mode](double currentSimulatorTime) {
                handleOneTimeStepCompleted(networkName,
                                           currentSimulatorTime, mode);
            });

    connect(simulator, &Simulator::simulatorInitialized, this,
            [this, networkName, &mode]() {
                m_completedSimulatorsInitialization++;
                checkAndEmitSignal(m_completedSimulatorsInitialization,
                                   m_totalSimulatorsInitializationRequested.size(),
                                   m_totalSimulatorsInitializationRequested,
                                   &SimulatorAPI::simulationsInitialized,
                                   mode);
            });

    connect(simulator, &Simulator::simulatorPaused, this,
            [this, networkName, &mode]() {
                m_completedSimulatorsPaused++;
                checkAndEmitSignal(m_completedSimulatorsPaused,
                                   m_totalSimulatorPauseRequested.size(),
                                   m_totalSimulatorPauseRequested,
                                   &SimulatorAPI::simulationsPaused,
                                   mode);
            });

    connect(simulator, &Simulator::simulatorResumed, this,
            [this, networkName, &mode]() {
                m_completedSimulatorsResumed++;
                checkAndEmitSignal(m_completedSimulatorsResumed,
                                   m_totalSimulatorResumeRequested.size(),
                                   m_totalSimulatorResumeRequested,
                                   &SimulatorAPI::simulationsResumed,
                                   mode);
            });

    connect(simulator, &Simulator::simulatorEnded, this,
            [this, networkName, &mode]() {
                m_completedSimulatorsEnded++;
                checkAndEmitSignal(m_completedSimulatorsEnded,
                                   m_totalSimulatorEndRequested.size(),
                                   m_totalSimulatorEndRequested,
                                   &SimulatorAPI::simulationsEnded,
                                   mode);
            });

    for (const auto& train : mData[networkName].trains) {
        connect(train.get(), &Train::destinationReached, this,
                [this, networkName, train, &mode]() {
                    handleTrainReachedDestination(
                        networkName,
                        QString::fromStdString(train->trainUserID),
                        mode);
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
                                      "generateSummaryData", Qt::QueuedConnection);
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

    QVector<QString> trainIDs;
    for (auto train: trains) {
        connect(train.get(), &Train::destinationReached,
                [this, train, &networkName]() {
                    QString trainID = QString::fromStdString(train->trainUserID);

                    handleTrainReachedDestination(networkName, trainID, mMode);
                });

        mData[networkName].trains.insert(train->trainUserID, train);

        QMetaObject::invokeMethod(mData[networkName].simulator,
                                  "addTrainToSimulation", Qt::QueuedConnection,
                                  Q_ARG(std::shared_ptr<Train>, train));

        trainIDs.push_back(QString::fromStdString(train->trainUserID));
    }

    emit trainAddedToSimulation(networkName, trainIDs);
}

std::any SimulatorAPI::convertQVariantToAny(const QVariant& variant) {
    switch (variant.typeId()) {
    case QMetaType::Int:
        return std::any(variant.toInt());
    case QMetaType::Double:
        return std::any(variant.toDouble());
    case QMetaType::Bool:
        return std::any(variant.toBool());
    case QMetaType::QString:
        return std::any(variant.toString().toStdString());
    case QMetaType::QVariantList: {
        QVariantList qList = variant.toList();
        std::vector<std::any> anyList;
        for (const QVariant& qVar : qList) {
            anyList.push_back(convertQVariantToAny(qVar));
        }
        return std::any(anyList);
    }
    case QMetaType::QVariantMap: {
        QVariantMap qMap = variant.toMap();
        std::map<std::string, std::any> anyMap;
        for (auto it = qMap.begin(); it != qMap.end(); ++it) {
            anyMap[it.key().toStdString()] = convertQVariantToAny(it.value());
        }
        return std::any(anyMap);
    }
    default:
        return std::any();
    }
}

std::map<std::string, std::any>
SimulatorAPI::convertQMapToStdMap(const QMap<QString, QVariant>& qMap) {
    std::map<std::string, std::any> stdMap;

    for (auto it = qMap.begin(); it != qMap.end(); ++it) {
        std::string key = it.key().toStdString();
        std::any value = convertQVariantToAny(it.value());
        stdMap[key] = value;
    }

    return stdMap;
}

std::shared_ptr<Train> SimulatorAPI::getTrainByID(QString networkName,
                                                  QString& trainID)
{
    if (!mData.contains(networkName)) {
        emit errorOccurred("A network with name " + networkName +
                           " does not exist!");
        return nullptr;
    }
    auto trainIDStr = trainID.toStdString();
    if (mData[networkName].trains.contains(trainIDStr)) {
        return mData[networkName].trains.value(trainIDStr);
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
    getTrainByID(networkName, trainID)->addContainers(json);
    emit containersAddedToTrain(networkName, trainID);
}

void SimulatorAPI::handleTrainReachedDestination(QString networkName,
                                                 QString trainID,
                                                 Mode mode)
{
    m_trainsReachedBuffer[networkName].append(trainID);

    if (mode == Mode::Sync) {
        emit trainsReachedDestination(m_trainsReachedBuffer);
        m_trainsReachedBuffer.clear();
    }
}

void SimulatorAPI::handleOneTimeStepCompleted(QString networkName,
                                              double currentSimulatorTime,
                                              Mode mode)
{
    m_timeStepData.insert(networkName, currentSimulatorTime);

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

void SimulatorAPI::emitSimulationResults(QMap<QString, TrainsResults> results)
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
    QString networkName, QVector<std::shared_ptr<Train>> trainList,
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
                "runOneTimeStep", Qt::QueuedConnection);
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
                QMetaObject::invokeMethod(
                    getInstance().mData[networkName].simulator,
                    "runSimulation", Qt::QueuedConnection);
            }
        }
        else {
            if (getInstance().mData[networkName].simulator) {
                QMetaObject::invokeMethod(
                    getInstance().mData[networkName].simulator,
                    "runTillNextReportingTime", Qt::QueuedConnection,
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
                "initializeSimulator",
                Qt::QueuedConnection);
        }
    }
}

void SimulatorAPI::InteractiveMode::endSimulation(QVector<QString> networkNames)
{
    if (networkNames.contains("*")) {
        networkNames = getInstance().mData.keys();
    }

    getInstance().m_completedSimulatorsEnded = 0;
    getInstance().m_totalSimulatorEndRequested = networkNames;

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
                "finalizeSimulation",
                Qt::QueuedConnection);
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
    QVector<std::shared_ptr<Train>> trainList,
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
                "runSimulation", Qt::QueuedConnection);
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
            QMetaObject::invokeMethod(
                getInstance().mData[networkName].simulator,
                "pauseSimulation", Qt::QueuedConnection);
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
            QMetaObject::invokeMethod(
                getInstance().mData[networkName].simulator,
                "resumeSimulation", Qt::QueuedConnection);
        }
    }
}

void SimulatorAPI::ContinuousMode::endSimulation(QVector<QString> networkNames)
{
    if (networkNames.contains("*")) {
        networkNames = getInstance().mData.keys();
    }

    getInstance().m_completedSimulatorsEnded = 0;
    getInstance().m_totalSimulatorEndRequested = networkNames;

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
                "finalizeSimulation",
                Qt::QueuedConnection);
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
