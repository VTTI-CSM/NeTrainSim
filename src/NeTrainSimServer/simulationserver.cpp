#include "SimulationServer.h"
#include <QDataStream>
#include <QDebug>
#include <QMap>
#include <QTimer>
#include "./VersionConfig.h"
#include "qjsonarray.h"
#include "qobjectdefs.h"
#include <QNetworkInterface>
#include "traindefinition/trainslist.h"

static const int MAX_RECONNECT_ATTEMPTS = 5;
static const int RECONNECT_DELAY_SECONDS = 5;  // Delay between reconnection attempts


SimulationServer::SimulationServer(QObject *parent)
    : QObject(parent), mWorkerBusy(false) {

    // Register the typedef with the meta-object system
    qRegisterMetaType<TrainParamsMap>("TrainParamsMap");

    connect(&SimulatorAPI::ContinuousMode::getInstance(),
            &SimulatorAPI::simulationCreated, this,
            &SimulationServer::onSimulationCreated);
    connect(&SimulatorAPI::ContinuousMode::getInstance(),
            &SimulatorAPI::simulationsPaused, this,
            &SimulationServer::onSimulationsPaused);
    connect(&SimulatorAPI::ContinuousMode::getInstance(),
            &SimulatorAPI::simulationsResumed, this,
            &SimulationServer::onSimulationsResumed);
    connect(&SimulatorAPI::ContinuousMode::getInstance(),
            &SimulatorAPI::simulationsEnded, this,
            &SimulationServer::onSimulationsEnded);
    connect(&SimulatorAPI::ContinuousMode::getInstance(),
            &SimulatorAPI::trainsReachedDestination, this,
            [this](QMap<QString, QVector<QString>> trainNetworkPair) {
                QJsonObject nets;
                for (auto it = trainNetworkPair.begin();
                     it != trainNetworkPair.end(); ++it) {
                    const QString& networkKey = it.key();
                    const QVector<QString>& trainsVector = it.value();

                    QJsonArray trains;
                    // Iterate through the QVector
                    for (const QString& trainID : trainsVector) {
                        // Append each trainID to QJsonArray
                        trains.append(trainID);
                    }
                    nets[networkKey] = trains;
                }
                onTrainReachedDestination(nets);
            });
    connect(&SimulatorAPI::ContinuousMode::getInstance(),
            &SimulatorAPI::simulationResultsAvailable, this,
            &SimulationServer::onSimulationResultsAvailable);
}

SimulationServer::~SimulationServer() {
    stopRabbitMQServer();  // Ensure server stops cleanly before destroying
}

void SimulationServer::startRabbitMQServer(const std::string &hostname,
                                           int port) {
    mHostname = hostname;
    mPort = port;


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
            continue;  // Retry
        }

        int status = amqp_socket_open(socket, hostname.c_str(), port);
        if (status != AMQP_STATUS_OK) {
            qCritical() << "Error: Failed to open RabbitMQ socket on"
                        << hostname.c_str() << ":" << port << ". Retrying...";
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
            continue;  // Retry
        }

        // Declare the queue to listen to commands
        amqp_queue_declare(mRabbitMQConnection, 1,
                           amqp_cstring_bytes("netrainsim_commands"), 0, 0, 0,
                           1, amqp_empty_table);
        amqp_rpc_reply_t queueRes = amqp_get_rpc_reply(mRabbitMQConnection);
        if (queueRes.reply_type != AMQP_RESPONSE_NORMAL) {
            qCritical() << "Error: Unable to declare RabbitMQ queue. "
                           "Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(RECONNECT_DELAY_SECONDS));
            continue;  // Retry
        }

        // Listen for messages
        amqp_basic_consume(mRabbitMQConnection, 1,
                           amqp_cstring_bytes("netrainsim_commands"),
                           amqp_empty_bytes, 0, 0, 0, amqp_empty_table);

        qInfo() << "Simulator initialized successfully. Awaiting commands from "
                << hostname.c_str() << ":" << port
                << ". The system is now fully operational.";
        break;  // Successful connection

    }

    if (retryCount == MAX_RECONNECT_ATTEMPTS) {
        qCritical() << "Error: Failed to establish a connection to "
                       "RabbitMQ after"
                    << MAX_RECONNECT_ATTEMPTS
                    << "attempts. Server initialization aborted.";
    }
}

void SimulationServer::stopRabbitMQServer() {
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

void SimulationServer::onDataReceivedFromRabbitMQ(
    const QJsonObject &message,
    const amqp_envelope_t &envelope) {

    QString command = message["command"].toString();

    if (command == "pauseSimulator") {
        // Immediately process the pause command
        qDebug() << "Received 'pauseSimulator' command, executing immediately.";

        // Acknowledge the message only after it is
        // successfully processed or queued
        amqp_basic_ack(mRabbitMQConnection, 1, envelope.delivery_tag, 0);

        processCommand(message);  // Prioritize pauseSimulator
    }

    if (mWorkerBusy) {
        if (mCommandQueue.size() < 1) {
            // Only queue one extra command while processing the current one
            mCommandQueue.enqueue(message);

            // Acknowledge the message only after it is
            // successfully processed or queued
            amqp_basic_ack(mRabbitMQConnection, 1, envelope.delivery_tag, 0);
        } else {
            qDebug() << "Command queue is full. Ignoring additional commands.";
            return;
        }
    } else {
        // Acknowledge the message only after it is
        // successfully processed or queued
        amqp_basic_ack(mRabbitMQConnection, 1, envelope.delivery_tag, 0);

        // If the worker is not busy, process the command directly
        processCommand(message);
        mWorkerBusy = true;  // Mark worker as busy
    }
}

void SimulationServer::processCommand(const QJsonObject &jsonMessage) {
    QString command = jsonMessage["command"].toString();

    if (command == "defineSimulator") {
        QString nodesContent = jsonMessage["nodesFileContent"].toString();
        QString linksContent = jsonMessage["linksFileContent"].toString();
        QString netName = jsonMessage["networkName"].toString();

        double timeStepValue = jsonMessage["timeStep"].toDouble();
        auto trains = TrainsList::ReadAndGenerateTrainsFromJSON(jsonMessage);

        QVector<std::shared_ptr<Train>> qTrains;
        for (auto& train: trains) {
            qTrains.push_back(std::move(train));
        }

        SimulatorAPI::ContinuousMode::
            createNewSimulationEnvironment(nodesContent, linksContent,
                                           netName, qTrains, timeStepValue);

    } else if (command == "runSimulator") {
        // Extract the "network" array from jsonMessage
        QJsonArray networkArray = jsonMessage["network"].toArray();

        // Convert the QJsonArray to QVector<QString>
        QVector<QString> networkNamesVector;
        for (const QJsonValue &value : networkArray) {
            if (value.isString()) {
                networkNamesVector.append(value.toString());
            }
        }

        SimulatorAPI::ContinuousMode::runSimulation(networkNamesVector);

    } else if (command == "pauseSimulator") {

        // Extract the "network" array from jsonMessage
        QJsonArray networkArray = jsonMessage["network"].toArray();

        // Convert the QJsonArray to QVector<QString>
        QVector<QString> networkNamesVector;
        for (const QJsonValue &value : networkArray) {
            if (value.isString()) {
                networkNamesVector.append(value.toString());
            }
        }

        SimulatorAPI::ContinuousMode::pauseSimulation(networkNamesVector);

    } else if (command == "resumeSimulator") {

        // Extract the "network" array from jsonMessage
        QJsonArray networkArray = jsonMessage["network"].toArray();

        // Convert the QJsonArray to QVector<QString>
        QVector<QString> networkNamesVector;
        for (const QJsonValue &value : networkArray) {
            if (value.isString()) {
                networkNamesVector.append(value.toString());
            }
        }

        SimulatorAPI::ContinuousMode::resumeSimulation(networkNamesVector);

    } else if (command == "endSimulator") {

        // Extract the "network" array from jsonMessage
        QJsonArray networkArray = jsonMessage["network"].toArray();

        // Convert the QJsonArray to QVector<QString>
        QVector<QString> networkNamesVector;
        for (const QJsonValue &value : networkArray) {
            if (value.isString()) {
                networkNamesVector.append(value.toString());
            }
        }

        SimulatorAPI::ContinuousMode::endSimulation(networkNamesVector);

    } else {
        qWarning() << "Unrecognized command:" << command;
    }

}

void SimulationServer::onWorkerReady() {
    // Worker is now ready for the next command
    mWorkerBusy = false;

    if (!mCommandQueue.isEmpty()) {
        // If there's a command in the queue, process it
        QJsonObject nextCommand = mCommandQueue.dequeue();
        processCommand(nextCommand);
        mWorkerBusy = true;  // Mark worker as busy again
    } else {
        // Continue consuming messages from RabbitMQ
        amqp_rpc_reply_t res;
        amqp_envelope_t envelope;

        // Wait for a new message from RabbitMQ
        amqp_maybe_release_buffers(mRabbitMQConnection);
        res = amqp_consume_message(mRabbitMQConnection, &envelope, nullptr, 0);

        if (res.reply_type == AMQP_RESPONSE_NORMAL) {
            QByteArray messageData(
                static_cast<char *>(envelope.message.body.bytes),
                envelope.message.body.len);
            QJsonDocument doc = QJsonDocument::fromJson(messageData);
            QJsonObject jsonMessage = doc.object();

            emit dataReceived(jsonMessage);
            onDataReceivedFromRabbitMQ(jsonMessage, envelope);  // Process the new message
        }
        else {
            qCritical() << "Error receiving message from RabbitMQ. Type:"
                        << res.reply_type;
            stopRabbitMQServer();  // Close the connection first
            qDebug() << "Attempting to reconnect...";
            startRabbitMQServer(mHostname, mPort);  // Retry connection
        }

        amqp_destroy_envelope(&envelope);
    }
}

void SimulationServer::sendRabbitMQMessage(const QString &queue,
                                           const QJsonObject &message) {
    QByteArray messageData = QJsonDocument(message).toJson();

    amqp_bytes_t messageBytes;
    messageBytes.len = messageData.size();
    messageBytes.bytes = messageData.data();

    int publishStatus =
        amqp_basic_publish(mRabbitMQConnection, 1, amqp_empty_bytes,
                           amqp_cstring_bytes(queue.toUtf8().constData()),
                           0, 0, nullptr, messageBytes);

    if (publishStatus != AMQP_STATUS_OK) {
        qCritical() << "Failed to publish message to RabbitMQ queue:" << queue;
    }
}

// simulation events handling
void SimulationServer::onSimulationCreated(QString networkName) {
    QJsonObject jsonMessage;
    jsonMessage["event"] = "simulationCreated";
    jsonMessage["host"] = "NeTrainSim";
    jsonMessage["network"] = networkName;
    sendRabbitMQMessage("client_queue", jsonMessage);
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

    sendRabbitMQMessage("client_queue", jsonMessage);
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

    sendRabbitMQMessage("client_queue", jsonMessage);
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

    sendRabbitMQMessage("client_queue", jsonMessage);
}

// void SimulationServer::onTrainAddedToSimulator(const QString trainID) {
//     QJsonObject jsonMessage;
//     jsonMessage["event"] = "trainAddedToSimulator";
//     jsonMessage["trainID"] = trainID;
//     jsonMessage["host"] = "NeTrainSim";
//     sendRabbitMQMessage("client_queue", jsonMessage);
// }

void SimulationServer::onTrainReachedDestination(const QJsonObject trainStatus) {
    QJsonObject jsonMessage;
    jsonMessage["event"] = "trainReachedDestination";
    jsonMessage["state"] = trainStatus;
    jsonMessage["host"] = "NeTrainSim";
    sendRabbitMQMessage("client_queue", jsonMessage);
}


void SimulationServer::onSimulationResultsAvailable(QMap<QString, TrainsResults>& results) {
    QJsonObject jsonMessage;
    jsonMessage["event"] = "simulationResultsAvailable";

    // Add the results to the JSON
    QJsonObject resultData;   
    for (auto it = results.begin(); it != results.end(); ++it) {
        resultData[it.key()] = results[it.key()].toJson();
    }

    jsonMessage["results"] = resultData;
    jsonMessage["host"] = "NeTrainSim";
    sendRabbitMQMessage("client_queue", jsonMessage);
}

void SimulationServer::onErrorOccurred(const QString &errorMessage) {
    QJsonObject jsonMessage;
    jsonMessage["event"] = "errorOccurred";
    jsonMessage["errorMessage"] = errorMessage;
    jsonMessage["host"] = "NeTrainSim";
    sendRabbitMQMessage("client_queue", jsonMessage);
}
