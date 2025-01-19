#include "SimulationServer.h"
#include <QDataStream>
#include <QDebug>
#include <QMap>
#include <QTimer>
#include "./VersionConfig.h"
#include "qjsonarray.h"
#include "qobjectdefs.h"
#include <QNetworkInterface>
#include <thread>
#include "traindefinition/trainslist.h"
#include "simulatorapi.h"

static const int MAX_RECONNECT_ATTEMPTS = 5;
static const int RECONNECT_DELAY_SECONDS = 5;  // Delay between reconnection attempts

static const std::string EXCHANGE_NAME = "CargoNetSim.Exchange";
static const std::string COMMAND_QUEUE_NAME = "CargoNetSim.CommandQueue.NeTrainSim";
static const std::string RESPONSE_QUEUE_NAME = "CargoNetSim.ResponseQueue.NeTrainSim";
static const std::string RECEIVING_ROUTING_KEY = "CargoNetSim.Command.NeTrainSim";
static const std::string PUBLISHING_ROUTING_KEY = "CargoNetSim.Response.NeTrainSim";

SimulationServer::SimulationServer(QObject *parent)
    : QObject(parent), mWorkerBusy(false)
{

    setupServer();
}

void SimulationServer::setupServer() {
    // Register the typedef with the meta-object system
    qRegisterMetaType<TrainParamsMap>("TrainParamsMap");

    auto &simAPI = SimulatorAPI::InteractiveMode::getInstance();
    connect(&simAPI,
            &SimulatorAPI::simulationCreated, this,
            &SimulationServer::onSimulationCreated,
            Qt::DirectConnection);
    connect(&simAPI,
            &SimulatorAPI::simulationReachedReportingTime, this,
            &SimulationServer::onSimulationAdvanced,
            Qt::DirectConnection);
    // connect(&simAPI,
    //         &SimulatorAPI::simulationReachedReportingTime, this,
    //         &SimulationServer::onSimulationReachedReportingTime,
    //         Qt::DirectConnection);
    connect(&simAPI,
            &SimulatorAPI::trainsReachedDestination, this,
            [this](QMap<QString, QVector<QJsonObject>> trainNetworkPair) {
                QJsonObject nets;
                for (auto it = trainNetworkPair.begin();
                     it != trainNetworkPair.end(); ++it) {
                    const QString& networkKey = it.key();
                    const QVector<QJsonObject>& trainStateVector = it.value();

                    QJsonArray trains;
                    // Iterate through the QVector
                    for (const QJsonObject& trainState : trainStateVector) {
                        // Append each trainID to QJsonArray
                        trains.append(trainState);
                    }
                    nets[networkKey] = trains;
                }
                onTrainReachedDestination(nets);
            }, Qt::DirectConnection);
    connect(&simAPI,
            &SimulatorAPI::simulationResultsAvailable, this,
            &SimulationServer::onSimulationResultsAvailable,
            Qt::DirectConnection);
    connect(&simAPI,
            &SimulatorAPI::trainAddedToSimulation, this,
            &SimulationServer::onTrainsAddedToSimulator,
            Qt::DirectConnection);
    connect(&simAPI,
            &SimulatorAPI::containersAddedToTrain, this,
            &SimulationServer::onContainersAddedToTrain,
            Qt::DirectConnection);
    connect(&simAPI,
            &SimulatorAPI::trainReachedTerminal, this,
            &SimulationServer::onTrainReachedTerminal,
            Qt::DirectConnection);
    connect(&simAPI,
            &SimulatorAPI::errorOccurred, this,
            &SimulationServer::onErrorOccurred,
            Qt::DirectConnection);
}

SimulationServer::~SimulationServer() {
    stopRabbitMQServer();  // Ensure server stops cleanly before destroying
    if (mRabbitMQThread) {
        mRabbitMQThread->quit();
        mRabbitMQThread->wait();
    }
}

void SimulationServer::startRabbitMQServer(const std::string &hostname,
                                           int port) {
    mHostname = hostname;
    mPort = port;
    reconnectToRabbitMQ();
}

void SimulationServer::reconnectToRabbitMQ() {
    int retryCount = 0;

    // Show app details
    QString appDetails;
    QTextStream detailsStream(&appDetails);
    detailsStream << QString::fromStdString(NeTrainSim_NAME)
                  << " [Version: "
                  << QString::fromStdString(NeTrainSim_FULL_VERSION)
                  << "]"
                  << Qt::endl;
    detailsStream << NeTrainSim_VENDOR << Qt::endl;
    detailsStream << GithubLink << Qt::endl;

    qInfo().noquote() << appDetails;

    while (retryCount < MAX_RECONNECT_ATTEMPTS) {

        // Initialize RabbitMQ connection
        mRabbitMQConnection = amqp_new_connection();
        amqp_socket_t *socket = amqp_tcp_socket_new(mRabbitMQConnection);
        if (!socket) {
            qCritical() << "Error: Unable to create RabbitMQ socket. "
                           "Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(RECONNECT_DELAY_SECONDS));

            // Reset pointers before retry
            mRabbitMQConnection = nullptr;
            socket = nullptr;

            continue;  // Retry
        }

        int status = amqp_socket_open(socket, mHostname.c_str(), mPort);
        if (status != AMQP_STATUS_OK) {
            qCritical() << "Error: Failed to open RabbitMQ socket on"
                        << mHostname.c_str() << ":" << mPort << ". Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(RECONNECT_DELAY_SECONDS));
            continue;  // Retry
        }

        amqp_rpc_reply_t loginRes =
            amqp_login(mRabbitMQConnection, "/", 0, 131072, 0,
                       AMQP_SASL_METHOD_PLAIN, "guest", "guest");
        if (loginRes.reply_type != AMQP_RESPONSE_NORMAL) {
            qCritical() << "Error: RabbitMQ login failed. Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(RECONNECT_DELAY_SECONDS));
            continue;  // Retry
        }

        amqp_channel_open(mRabbitMQConnection, 1);
        amqp_rpc_reply_t channelRes = amqp_get_rpc_reply(mRabbitMQConnection);
        if (channelRes.reply_type != AMQP_RESPONSE_NORMAL) {
            qCritical() << "Error: Unable to open RabbitMQ channel. "
                           "Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(RECONNECT_DELAY_SECONDS));

            // Reset pointers before retry
            mRabbitMQConnection = nullptr;
            socket = nullptr;

            continue;  // Retry
        }

        // Declare the exchange for simulation
        amqp_exchange_declare_ok_t *exchange_declare_res =
            amqp_exchange_declare(
                mRabbitMQConnection,
                1,
                amqp_cstring_bytes(EXCHANGE_NAME.c_str()), // Exchange name
                amqp_cstring_bytes("topic"),         // Exchange type
                0,                                   // passive (false)
                1,                                   // durable (true)
                0,                                   // auto-delete (false)
                0,                                   // internal (false)
                amqp_empty_table                     // no additional arguments
                );

        if (!exchange_declare_res) {
            qCritical() << "Error: Unable to declare exchange "
                        << EXCHANGE_NAME.c_str() << ". Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(RECONNECT_DELAY_SECONDS));
            continue;
        }

        // Declare the command queue to listen to commands
        amqp_queue_declare(
            mRabbitMQConnection, 1,
            amqp_cstring_bytes(COMMAND_QUEUE_NAME.c_str()),
            0, 1, 0, 0, amqp_empty_table);

        if (amqp_get_rpc_reply(mRabbitMQConnection).reply_type
            != AMQP_RESPONSE_NORMAL)
        {
            qCritical() << "Error: Unable to declare RabbitMQ command queue. "
                           "Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(RECONNECT_DELAY_SECONDS));
            continue;  // Retry
        }

        // Bind the command queue to the exchange with a routing key
        amqp_queue_bind(
            mRabbitMQConnection,
            1,
            amqp_cstring_bytes(COMMAND_QUEUE_NAME.c_str()), // Queue name
            amqp_cstring_bytes(EXCHANGE_NAME.c_str()),        // Exchange name
            amqp_cstring_bytes(RECEIVING_ROUTING_KEY.c_str()),  // Routing key
            amqp_empty_table                                 // Arguments
            );

        amqp_rpc_reply_t comQueueRes = amqp_get_rpc_reply(mRabbitMQConnection);
        if (comQueueRes.reply_type != AMQP_RESPONSE_NORMAL) {
            qCritical() << "Error: Unable to bind queue to exchange.  "
                           "Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(RECONNECT_DELAY_SECONDS));
            continue;  // Retry
        }

        // Declare the response queue to send commands
        amqp_queue_declare(
            mRabbitMQConnection, 1,
            amqp_cstring_bytes(RESPONSE_QUEUE_NAME.c_str()),
            0, 1, 0, 0, amqp_empty_table);

        if (amqp_get_rpc_reply(mRabbitMQConnection).reply_type
            != AMQP_RESPONSE_NORMAL)
        {
            qCritical() << "Error: Unable to declare RabbitMQ response queue. "
                           "Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(RECONNECT_DELAY_SECONDS));
            continue;  // Retry
        }

        // Bind the response queue to the exchange with a routing key
        amqp_queue_bind(
            mRabbitMQConnection,
            1,
            amqp_cstring_bytes(RESPONSE_QUEUE_NAME.c_str()), // Queue name
            amqp_cstring_bytes(EXCHANGE_NAME.c_str()),        // Exchange name
            amqp_cstring_bytes(PUBLISHING_ROUTING_KEY.c_str()),  // Routing key
            amqp_empty_table                                 // Arguments
            );

        amqp_rpc_reply_t resQueueRes = amqp_get_rpc_reply(mRabbitMQConnection);
        if (resQueueRes.reply_type != AMQP_RESPONSE_NORMAL) {
            qCritical() << "Error: Unable to bind queue to exchange.  "
                           "Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(RECONNECT_DELAY_SECONDS));
            continue;  // Retry
        }

        // Listen for messages
        amqp_basic_consume(
            mRabbitMQConnection, 1,
            amqp_cstring_bytes(COMMAND_QUEUE_NAME.c_str()),
            amqp_empty_bytes, 0, 0, 0, amqp_empty_table);

        if (amqp_get_rpc_reply(mRabbitMQConnection).reply_type
            != AMQP_RESPONSE_NORMAL)
        {
            qCritical() << "Error: Failed to start consuming "
                           "from the queue. Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(RECONNECT_DELAY_SECONDS));
            continue;  // Retry
        }

        qInfo() << "Simulator initialized successfully. Awaiting commands from "
                << mHostname.c_str() << ":" << mPort
                << ". The system is now fully operational.";

        startConsumingMessages(); // Start consuming


        return;  // Successful connection

    }

    qCritical() << "Error: Failed to establish a connection to "
                   "RabbitMQ after"
                << MAX_RECONNECT_ATTEMPTS
                << "attempts. Server initialization aborted.";
}

void SimulationServer::stopRabbitMQServer() {
    emit stopConsuming();
    // If the connection is already closed, just return
    if (mRabbitMQConnection == nullptr) {
        qDebug() << "RabbitMQ connection already closed.";
        return;
    }

    // Gracefully close the RabbitMQ channel
    amqp_channel_close(mRabbitMQConnection, 1, AMQP_REPLY_SUCCESS);

    // Gracefully close the RabbitMQ connection
    amqp_connection_close(mRabbitMQConnection, AMQP_REPLY_SUCCESS);

    // Destroy the RabbitMQ connection
    amqp_destroy_connection(mRabbitMQConnection);

    mRabbitMQConnection = nullptr;  // Set the connection to null to indicate it's closed

    qDebug() << "RabbitMQ server stopped cleanly.";
}

void SimulationServer::startConsumingMessages() {
    if (mRabbitMQThread) {
        mRabbitMQThread->quit();
        mRabbitMQThread->wait();
        mRabbitMQThread->deleteLater();
    }

    // Move the consuming logic to a separate thread
    mRabbitMQThread = new QThread(this);
    connect(mRabbitMQThread , &QThread::started, this,
            &SimulationServer::consumeFromRabbitMQ);
    connect(this, &SimulationServer::stopConsuming,
            mRabbitMQThread , &QThread::quit);
    connect(mRabbitMQThread , &QThread::finished,
            mRabbitMQThread , &QThread::deleteLater);
    mRabbitMQThread->start();
    std::cout << "started!";
}

void SimulationServer::consumeFromRabbitMQ() {
    while (true) {
        // Ensure the RabbitMQ connection is valid
        if (!mRabbitMQConnection) {
            qCritical() << "RabbitMQ connection is not valid. "
                           "Exiting consume loop.";
            break;
        }

        {
            QMutexLocker locker(&mMutex);
            if (mWorkerBusy) {
                mWaitCondition.wait(&mMutex, 100);  // Wait for 100ms or until notified
                continue;
            }
        }

        amqp_rpc_reply_t res;
        amqp_envelope_t envelope;

        // Wait for a new message from RabbitMQ (with a short timeout
        //                                       to avoid blocking)
        amqp_maybe_release_buffers(mRabbitMQConnection);
        struct timeval timeout;
        timeout.tv_sec = 0;          // No blocking
        timeout.tv_usec = 100000;    // 100ms timeout

        res = amqp_consume_message(mRabbitMQConnection, &envelope, &timeout, 0);

        if (res.reply_type == AMQP_RESPONSE_NORMAL) {
            // Acknowledge the message regardless of validity
            amqp_basic_ack(mRabbitMQConnection, 1, envelope.delivery_tag, 0);

            QByteArray messageData(
                static_cast<char *>(envelope.message.body.bytes),
                envelope.message.body.len);
            QJsonDocument doc = QJsonDocument::fromJson(messageData);
            QJsonObject jsonMessage = doc.object();

            emit dataReceived(jsonMessage);
            onDataReceivedFromRabbitMQ(jsonMessage, envelope);

            // Clean up the envelope
            amqp_destroy_envelope(&envelope);
        } else if (res.reply_type == AMQP_RESPONSE_LIBRARY_EXCEPTION &&
                   res.library_error == AMQP_STATUS_TIMEOUT) {
            // Timeout reached but no message available, continue to next iteration
            // Sleep for 100ms before checking again to avoid a busy loop
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        } else {
            qCritical() << "Error receiving message from RabbitMQ. Type:"
                        << res.reply_type;
            stopRabbitMQServer();
            qDebug() << "Attempting to reconnect...";
            reconnectToRabbitMQ();
            break;
        }

        // Sleep for 100ms between each iteration to avoid busy-looping
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void SimulationServer::onDataReceivedFromRabbitMQ(
    const QJsonObject &message,
    const amqp_envelope_t &envelope) {

    {
        QMutexLocker locker(&mMutex);
        if (mWorkerBusy) {
            qInfo() << "Simulator is busy, not consuming new messages.";
            return;
        }
        mWorkerBusy = true;
    }

    processCommand(message);
}

void SimulationServer::processCommand(const QJsonObject &jsonMessage) {

    auto checkJsonField = [this](const QJsonObject &json,
                             const QString &fieldName,
                             const QString &command) -> bool {
        if (!json.contains(fieldName)) {
            qWarning() << "Missing parameter:" << fieldName
                       << "in command:" << command;
            mWorkerBusy = false;
            return false;
        }
        return true;
    };

    QString command = jsonMessage["command"].toString();

    if (command == "defineSimulator") {
        if (!checkJsonField(jsonMessage, "nodesFileContent", command) ||
            !checkJsonField(jsonMessage, "linksFileContent", command) ||
            !checkJsonField(jsonMessage, "networkName", command) ||
            !checkJsonField(jsonMessage, "trains", command) ||
            !checkJsonField(jsonMessage, "timeStep", command)) {
            return; // Skip processing this command
        }

        QString nodesContent = jsonMessage["nodesFileContent"].toString();
        QString linksContent = jsonMessage["linksFileContent"].toString();
        QString netName = jsonMessage["networkName"].toString();
        double timeStepValue = jsonMessage["timeStep"].toDouble();

        auto trainsI = TrainsList::readTrainsFromJSON(jsonMessage);
        auto trains = Utils::convertToQVector(trainsI);

        try {
            SimulatorAPI::InteractiveMode::
                createNewSimulationEnvironment(nodesContent, linksContent,
                                               netName, trains, timeStepValue);

        } catch (const std::exception &e) {
            qWarning() << "Error while creating the environment: " << e.what();
        }

        SimulatorAPI::InteractiveMode::initSimulation({netName});

    } else if (command == "runSimulator") {
        if (!checkJsonField(jsonMessage, "network", command) ||
            !checkJsonField(jsonMessage, "byTimeSteps", command)) {
            return; // Skip processing this command
        }

        // Extract the "network" array from jsonMessage
        QJsonArray networkArray = jsonMessage["network"].toArray();
        double byTimeSteps = jsonMessage["byTimeSteps"].toDouble(60);

        // Convert the QJsonArray to QVector<QString>
        QVector<QString> networkNamesVector;
        for (const QJsonValue &value : networkArray) {
            if (value.isString()) {
                networkNamesVector.append(value.toString());
            }
        }

        SimulatorAPI::InteractiveMode::runSimulation(networkNamesVector,
                                                     byTimeSteps);
        if (byTimeSteps < 0) {
            qInfo() << "Running Simulation till the end.";
        } else {
            qInfo() << "Running Simulation by" << byTimeSteps << "steps.";
        }

    } else if (command == "addTrainsToSimulator") {
        if (!checkJsonField(jsonMessage, "network", command) ||
            !checkJsonField(jsonMessage, "trains", command)) {
            return; // Skip processing this command
        }

        QString netName = jsonMessage["network"].toString();
        auto trains = TrainsList::ReadAndGenerateTrainsFromJSON(jsonMessage);

        QVector<std::shared_ptr<Train>> qTrains;
        for (auto& train: trains) {
            qTrains.push_back(std::move(train));
        }
        SimulatorAPI::InteractiveMode::addTrainToSimulation(netName, qTrains);

    } else if (command == "endSimulator") {
        if (!checkJsonField(jsonMessage, "network", command)) {
            return; // Skip processing this command
        }

        // Extract the "network" array from jsonMessage
        QJsonArray networkArray = jsonMessage["network"].toArray();

        // Convert the QJsonArray to QVector<QString>
        QVector<QString> networkNamesVector;
        for (const QJsonValue &value : networkArray) {
            if (value.isString()) {
                networkNamesVector.append(value.toString());
            }
        }

        SimulatorAPI::InteractiveMode::endSimulation(networkNamesVector);

    } else if (command == "addContainersToTrain") {
        if (!checkJsonField(jsonMessage, "networkName", command) ||
            !checkJsonField(jsonMessage, "trainID", command)) {
            return; // Skip processing this command
        }

        QString net =
            jsonMessage["networkName"].toString();
        QString trainID =
            jsonMessage["trainID"].toString();
        SimulatorAPI::InteractiveMode::addContainersToTrain(net, trainID,
                                                            jsonMessage);
    } else if (command == "restServer") {
        SimulatorAPI::InteractiveMode::resetAPI();
        onServerReset();
    } else {
        onWorkerReady();
        qWarning() << "Unrecognized command:" << command;
    }

}

void SimulationServer::onWorkerReady() {
    // Worker is now ready for the next command
    QMutexLocker locker(&mMutex);

    mWorkerBusy = false;

    mWaitCondition.wakeAll();  // Notify waiting consumers
}

void SimulationServer::sendRabbitMQMessage(const QString &routingKey,
                                           const QJsonObject &message) {
    QByteArray messageData = QJsonDocument(message).toJson();

    amqp_bytes_t messageBytes;
    messageBytes.len = messageData.size();
    messageBytes.bytes = messageData.data();

    int publishStatus =
        amqp_basic_publish(mRabbitMQConnection, 1,
                           amqp_cstring_bytes(EXCHANGE_NAME.c_str()),
                           amqp_cstring_bytes(routingKey.toUtf8().constData()),
                           0, 0, nullptr, messageBytes);

    if (publishStatus != AMQP_STATUS_OK) {
        qCritical() << "Failed to publish message to "
                       "RabbitMQ with routing key:"
                    << routingKey;
    }
}

// simulation events handling
void SimulationServer::onSimulationCreated(QString networkName) {
    QJsonObject jsonMessage;
    jsonMessage["event"] = "simulationCreated";
    jsonMessage["host"] = "NeTrainSim";
    jsonMessage["network"] = networkName;
    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();
    qInfo() << "Environemnt created successfully for network: " << networkName;
}

void SimulationServer::onSimulationsPaused(QVector<QString> networkNames) {
    QJsonObject jsonMessage;
    jsonMessage["event"] = "simulationPaused";
    jsonMessage["host"] = "NeTrainSim";

    // Convert QVector<QString> to QJsonArray
    QJsonArray jsonNetworkNames;
    for (const QString &name : networkNames) {
        jsonNetworkNames.append(name);
    }

    // Add the network names to the JSON message
    jsonMessage["networkNames"] = jsonNetworkNames;

    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();
}

void SimulationServer::onSimulationsResumed(QVector<QString> networkNames) {
    QJsonObject jsonMessage;
    jsonMessage["event"] = "simulationResumed";
    jsonMessage["host"] = "NeTrainSim";

    // Convert QVector<QString> to QJsonArray
    QJsonArray jsonNetworkNames;
    for (const QString &name : networkNames) {
        jsonNetworkNames.append(name);
    }

    // Add the network names to the JSON message
    jsonMessage["networkNames"] = jsonNetworkNames;

    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();
}

void SimulationServer::onSimulationsEnded(QVector<QString> networkNames) {
    QJsonObject jsonMessage;
    jsonMessage["event"] = "simulationEnded";
    jsonMessage["host"] = "NeTrainSim";

    // Convert QVector<QString> to QJsonArray
    QJsonArray jsonNetworkNames;
    for (const QString &name : networkNames) {
        jsonNetworkNames.append(name);
    }

    // Add the network names to the JSON message
    jsonMessage["networkNames"] = jsonNetworkNames;

    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();

    qInfo() << "Simulation ended successfully for networks: " << networkNames.join(", ");
}

void SimulationServer::onSimulationAdvanced(
    QMap<QString, QPair<double, double>> networkNamesSimulationTimePairs)
{
    QJsonObject jsonMessage;
    jsonMessage["event"] = "simulationAdvanced";
    jsonMessage["host"] = "NeTrainSim";

    // Convert QVector<QString> to QJsonArray
    QJsonObject jsonNetworkTimes;
    QJsonObject jsonNetworkProgress;
    for (auto it = networkNamesSimulationTimePairs.constBegin();
         it != networkNamesSimulationTimePairs.constEnd(); ++it) {
        // Add each network name (key) and its corresponding
        // simulation time (value) to the jsonNetworkTimes object
        jsonNetworkTimes[it.key()] = it.value().first;
        jsonNetworkProgress[it.key()] = it.value().second;
    }

    // Add the network names to the JSON message
    jsonMessage["networkNamesTimes"] = jsonNetworkTimes;
    jsonMessage["networkNamesProgress"] = jsonNetworkProgress;

    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();
}


void SimulationServer::onTrainsAddedToSimulator(const QString networkName,
                                                const QVector<QString> trainIDs)
{
    QJsonObject jsonMessage;
    jsonMessage["event"] = "trainAddedToSimulator";
    jsonMessage["networkNames"] = networkName;
    QJsonArray trainsJson;
    for (const auto& trainID : trainIDs) {
        trainsJson.append(trainID);
    }
    jsonMessage["trainIDs"] = trainsJson;
    jsonMessage["host"] = "NeTrainSim";
    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();

    qInfo() << "Train ID(s): " << trainIDs.join(", ") << " added successfully to network: " << networkName;

}

void SimulationServer::
    onTrainReachedDestination(const QJsonObject trainStatus)
{
    QJsonObject jsonMessage;
    jsonMessage["event"] = "trainReachedDestination";
    jsonMessage["state"] = trainStatus;
    jsonMessage["host"] = "NeTrainSim";
    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();

    // qInfo() << "Train reached destination";
}


void SimulationServer::
    onSimulationResultsAvailable(QMap<QString, TrainsResults>& results)
{
    QJsonObject jsonMessage;
    jsonMessage["event"] = "simulationResultsAvailable";

    // Add the results to the JSON
    QJsonObject resultData;   
    for (auto it = results.begin(); it != results.end(); ++it) {
        resultData[it.key()] = results[it.key()].toJson();
    }

    jsonMessage["results"] = resultData;
    jsonMessage["host"] = "NeTrainSim";
    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();

    qInfo() << "Simulation results sent to consumers!";
}

void SimulationServer::onContainersAddedToTrain(QString networkName,
                                                QString trainID)
{
    QJsonObject jsonMessage;
    jsonMessage["event"] = "containersAddedToTrain";
    jsonMessage["networkName"] = networkName;
    jsonMessage["trainID"] = trainID;
    jsonMessage["host"] = "NeTrainSim";

    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();

    qInfo() << "Container successfully added to Train ID: "
            << trainID << " of network: " << networkName << "!";
}

void SimulationServer::onTrainReachedTerminal(QString networkName,
                                              QString trainID,
                                              QString terminalID,
                                              QJsonArray containers)
{
    QJsonObject jsonMessage;
    jsonMessage["event"] = "trainReachedTerminal";
    jsonMessage["networkName"] = networkName;
    jsonMessage["trainID"] = trainID;
    jsonMessage["terminalID"] = terminalID;
    jsonMessage["containers"] = containers;
    jsonMessage["host"] = "NeTrainSim";

    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();

}

void SimulationServer::onErrorOccurred(const QString &errorMessage) {
    QJsonObject jsonMessage;
    jsonMessage["event"] = "errorOccurred";
    jsonMessage["errorMessage"] = errorMessage;
    jsonMessage["host"] = "NeTrainSim";
    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();
    qInfo() << "Error Occured: " << errorMessage;
}

void SimulationServer::onServerReset() {
    setupServer();
    QJsonObject jsonMessage;
    jsonMessage["event"] = "serverReset";
    jsonMessage["host"] = "NeTrainSim";
    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();
    qInfo() << "Server reset Successfully!";
}
