#include "SimulationServer.h"
#include "./VersionConfig.h"
#include "qjsonarray.h"
#include "qobjectdefs.h"
#include "simulatorapi.h"
#include "traindefinition/trainslist.h"
#include <QCoreApplication>
#include <QDataStream>
#include <QDebug>
#include <QMap>
#include <QNetworkInterface>
#include <QTimer>
#include <thread>

static const int MAX_RECONNECT_ATTEMPTS = 5;
static const int RECONNECT_DELAY_SECONDS =
    5; // Delay between reconnection attempts

static const std::string EXCHANGE_NAME =
    "CargoNetSim.Exchange";
static const std::string COMMAND_QUEUE_NAME =
    "CargoNetSim.CommandQueue.NeTrainSim";
static const std::string RESPONSE_QUEUE_NAME =
    "CargoNetSim.ResponseQueue.NeTrainSim";
static const std::string RECEIVING_ROUTING_KEY =
    "CargoNetSim.Command.NeTrainSim";
static const std::string PUBLISHING_ROUTING_KEY =
    "CargoNetSim.Response.NeTrainSim";
static const int MAX_SEND_COMMAND_RETRIES = 3;

SimulationServer::SimulationServer(QObject *parent)
    : QObject(parent)
    , mWorkerBusy(false)
{
    setupServer();
}

void SimulationServer::setupServer()
{
    // Register the typedef with the meta-object system
    qRegisterMetaType<TrainParamsMap>("TrainParamsMap");

    auto &simAPI =
        SimulatorAPI::InteractiveMode::getInstance();
    connect(&simAPI, &SimulatorAPI::simulationCreated, this,
            &SimulationServer::onSimulationCreated);
    connect(&simAPI,
            &SimulatorAPI::simulationReachedReportingTime,
            this, &SimulationServer::onSimulationAdvanced);

    connect(
        &simAPI, &SimulatorAPI::allTrainsReachedDestination,
        this,
        &SimulationServer::onAllTrainsReachedDestination);
    connect(&simAPI,
            &SimulatorAPI::trainsReachedDestination, this,
            &SimulationServer::onTrainReachedDestination);
    connect(
        &simAPI, &SimulatorAPI::simulationResultsAvailable,
        this,
        &SimulationServer::onSimulationResultsAvailable);
    connect(&simAPI, &SimulatorAPI::trainsAddedToSimulation,
            this,
            &SimulationServer::onTrainsAddedToSimulator);
    connect(&simAPI, &SimulatorAPI::containersAddedToTrain,
            this,
            &SimulationServer::onContainersAddedToTrain);
    connect(&simAPI, &SimulatorAPI::trainReachedTerminal,
            this,
            &SimulationServer::onTrainReachedTerminal);
    connect(&simAPI, &SimulatorAPI::ContainersUnloaded,
            this, &SimulationServer::onContainersUnloaded);
    connect(&simAPI, &SimulatorAPI::errorOccurred, this,
            &SimulationServer::onErrorOccurred);
}

SimulationServer::~SimulationServer()
{
    stopRabbitMQServer(); // Ensure server stops cleanly
                          // before destroying
    if (mRabbitMQThread)
    {
        mRabbitMQThread->quit();
        mRabbitMQThread->wait();
    }
}

void SimulationServer::startRabbitMQServer(
    const std::string &hostname, int port)
{
    mHostname = hostname;
    mPort     = port;
    reconnectToRabbitMQ();
}

void SimulationServer::reconnectToRabbitMQ()
{
    int retryCount = 0;

    // Show app details
    QString     appDetails;
    QTextStream detailsStream(&appDetails);
    detailsStream << QString::fromStdString(NeTrainSim_NAME)
                  << " [Version: "
                  << QString::fromStdString(
                         NeTrainSim_FULL_VERSION)
                  << "]" << Qt::endl;
    detailsStream << NeTrainSim_VENDOR << Qt::endl;
    detailsStream << GithubLink << Qt::endl;

    qInfo().noquote() << appDetails;

    while (retryCount < MAX_RECONNECT_ATTEMPTS)
    {
        // Initialize RabbitMQ connection
        mRabbitMQConnection = amqp_new_connection();
        amqp_socket_t *socket =
            amqp_tcp_socket_new(mRabbitMQConnection);
        if (!socket)
        {
            qCritical() << "Error: Unable to create "
                           "RabbitMQ socket. "
                           "Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(
                    RECONNECT_DELAY_SECONDS));

            // Reset pointers before retry
            mRabbitMQConnection = nullptr;
            socket              = nullptr;

            continue; // Retry
        }

        int status = amqp_socket_open(
            socket, mHostname.c_str(), mPort);
        if (status != AMQP_STATUS_OK)
        {
            qCritical() << "Error: Failed to open RabbitMQ "
                           "socket on"
                        << mHostname.c_str() << ":" << mPort
                        << ". Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(
                    RECONNECT_DELAY_SECONDS));
            continue; // Retry
        }

        amqp_rpc_reply_t loginRes = amqp_login(
            mRabbitMQConnection, "/", 0, 131072, 0,
            AMQP_SASL_METHOD_PLAIN, "guest", "guest");
        if (loginRes.reply_type != AMQP_RESPONSE_NORMAL)
        {
            qCritical() << "Error: RabbitMQ login failed. "
                           "Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(
                    RECONNECT_DELAY_SECONDS));
            continue; // Retry
        }

        amqp_channel_open(mRabbitMQConnection, 1);
        amqp_rpc_reply_t channelRes =
            amqp_get_rpc_reply(mRabbitMQConnection);
        if (channelRes.reply_type != AMQP_RESPONSE_NORMAL)
        {
            qCritical() << "Error: Unable to open RabbitMQ "
                           "channel. "
                           "Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(
                    RECONNECT_DELAY_SECONDS));

            // Reset pointers before retry
            mRabbitMQConnection = nullptr;
            socket              = nullptr;

            continue; // Retry
        }

        // Declare the exchange for simulation
        amqp_exchange_declare_ok_t *exchange_declare_res =
            amqp_exchange_declare(
                mRabbitMQConnection, 1,
                amqp_cstring_bytes(
                    EXCHANGE_NAME.c_str()), // Exchange name
                amqp_cstring_bytes(
                    "topic"),    // Exchange type
                0,               // passive (false)
                1,               // durable (true)
                0,               // auto-delete (false)
                0,               // internal (false)
                amqp_empty_table // no additional arguments
            );

        if (!exchange_declare_res)
        {
            qCritical()
                << "Error: Unable to declare exchange "
                << EXCHANGE_NAME.c_str() << ". Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(
                    RECONNECT_DELAY_SECONDS));
            continue;
        }

        // Declare the command queue to listen to commands
        amqp_queue_declare(
            mRabbitMQConnection, 1,
            amqp_cstring_bytes(COMMAND_QUEUE_NAME.c_str()),
            0, 1, 0, 0, amqp_empty_table);

        if (amqp_get_rpc_reply(mRabbitMQConnection)
                .reply_type
            != AMQP_RESPONSE_NORMAL)
        {
            qCritical() << "Error: Unable to declare "
                           "RabbitMQ command queue. "
                           "Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(
                    RECONNECT_DELAY_SECONDS));
            continue; // Retry
        }

        // Bind the command queue to the exchange with a
        // routing key
        amqp_queue_bind(
            mRabbitMQConnection, 1,
            amqp_cstring_bytes(
                COMMAND_QUEUE_NAME.c_str()), // Queue name
            amqp_cstring_bytes(
                EXCHANGE_NAME.c_str()), // Exchange name
            amqp_cstring_bytes(RECEIVING_ROUTING_KEY
                                   .c_str()), // Routing key
            amqp_empty_table                  // Arguments
        );

        amqp_rpc_reply_t comQueueRes =
            amqp_get_rpc_reply(mRabbitMQConnection);
        if (comQueueRes.reply_type != AMQP_RESPONSE_NORMAL)
        {
            qCritical() << "Error: Unable to bind queue to "
                           "exchange.  "
                           "Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(
                    RECONNECT_DELAY_SECONDS));
            continue; // Retry
        }

        // Declare the response queue to send commands
        amqp_queue_declare(
            mRabbitMQConnection, 1,
            amqp_cstring_bytes(RESPONSE_QUEUE_NAME.c_str()),
            0, 1, 0, 0, amqp_empty_table);

        if (amqp_get_rpc_reply(mRabbitMQConnection)
                .reply_type
            != AMQP_RESPONSE_NORMAL)
        {
            qCritical() << "Error: Unable to declare "
                           "RabbitMQ response queue. "
                           "Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(
                    RECONNECT_DELAY_SECONDS));
            continue; // Retry
        }

        // Bind the response queue to the exchange with a
        // routing key
        amqp_queue_bind(
            mRabbitMQConnection, 1,
            amqp_cstring_bytes(
                RESPONSE_QUEUE_NAME.c_str()), // Queue name
            amqp_cstring_bytes(
                EXCHANGE_NAME.c_str()), // Exchange name
            amqp_cstring_bytes(PUBLISHING_ROUTING_KEY
                                   .c_str()), // Routing key
            amqp_empty_table                  // Arguments
        );

        amqp_rpc_reply_t resQueueRes =
            amqp_get_rpc_reply(mRabbitMQConnection);
        if (resQueueRes.reply_type != AMQP_RESPONSE_NORMAL)
        {
            qCritical() << "Error: Unable to bind queue to "
                           "exchange.  "
                           "Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(
                    RECONNECT_DELAY_SECONDS));
            continue; // Retry
        }

        // Listen for messages
        amqp_basic_consume(
            mRabbitMQConnection, 1,
            amqp_cstring_bytes(COMMAND_QUEUE_NAME.c_str()),
            amqp_empty_bytes, 0, 0, 0, amqp_empty_table);

        if (amqp_get_rpc_reply(mRabbitMQConnection)
                .reply_type
            != AMQP_RESPONSE_NORMAL)
        {
            qCritical()
                << "Error: Failed to start consuming "
                   "from the queue. Retrying...";
            retryCount++;
            std::this_thread::sleep_for(
                std::chrono::seconds(
                    RECONNECT_DELAY_SECONDS));
            continue; // Retry
        }

        qInfo() << "Simulator initialized successfully. "
                   "Awaiting commands from "
                << mHostname.c_str() << ":" << mPort
                << ". The system is now fully operational.";

        startConsumingMessages(); // Start consuming

        return; // Successful connection
    }

    qCritical()
        << "Error: Failed to establish a connection to "
           "RabbitMQ after"
        << MAX_RECONNECT_ATTEMPTS
        << "attempts. Server initialization aborted.";
}

void SimulationServer::stopRabbitMQServer()
{
    emit stopConsuming();
    // If the connection is already closed, just return
    if (mRabbitMQConnection == nullptr)
    {
        qDebug() << "RabbitMQ connection already closed.";
        return;
    }

    // Gracefully close the RabbitMQ channel
    amqp_channel_close(mRabbitMQConnection, 1,
                       AMQP_REPLY_SUCCESS);

    // Gracefully close the RabbitMQ connection
    amqp_connection_close(mRabbitMQConnection,
                          AMQP_REPLY_SUCCESS);

    // Destroy the RabbitMQ connection
    amqp_destroy_connection(mRabbitMQConnection);

    mRabbitMQConnection =
        nullptr; // Set the connection to null to indicate
                 // it's closed

    qDebug() << "RabbitMQ server stopped cleanly.";
}

void SimulationServer::startConsumingMessages()
{
    if (mRabbitMQThread)
    {
        mRabbitMQThread->quit();
        mRabbitMQThread->wait();
        mRabbitMQThread->deleteLater();
    }

    // Move the consuming logic to a separate thread
    mRabbitMQThread = new QThread(this);
    connect(mRabbitMQThread, &QThread::started, this,
            &SimulationServer::consumeFromRabbitMQ);
    connect(this, &SimulationServer::stopConsuming,
            mRabbitMQThread, &QThread::quit);
    connect(mRabbitMQThread, &QThread::finished,
            mRabbitMQThread, &QThread::deleteLater);
    mRabbitMQThread->start();
    std::cout << "started!";
}

void SimulationServer::consumeFromRabbitMQ()
{
    while (true)
    {
        // Ensure the RabbitMQ connection is valid
        if (!mRabbitMQConnection)
        {
            qCritical()
                << "RabbitMQ connection is not valid. "
                   "Exiting consume loop.";
            break;
        }

        // Check if worker is busy before attempting to
        // consume messages
        bool workerIsBusy = false;
        {
            QMutexLocker locker(&mMutex);
            workerIsBusy = mWorkerBusy;
        }

        if (workerIsBusy)
        {
            // Process events while waiting for worker to be
            // ready
            QCoreApplication::processEvents();
            QThread::msleep(
                100); // Small sleep to prevent CPU hogging
            continue; // Skip message consumption and check
                      // again
        }

        // Worker is not busy, proceed with message
        // consumption
        amqp_rpc_reply_t res;
        amqp_envelope_t  envelope;

        // Wait for a new message from RabbitMQ (with a
        // short timeout to avoid blocking)
        amqp_maybe_release_buffers(mRabbitMQConnection);
        struct timeval timeout;
        timeout.tv_sec  = 0;      // No blocking
        timeout.tv_usec = 100000; // 100ms timeout

        res = amqp_consume_message(mRabbitMQConnection,
                                   &envelope, &timeout, 0);

        if (res.reply_type == AMQP_RESPONSE_NORMAL)
        {
            // Acknowledge the message regardless of
            // validity
            amqp_basic_ack(mRabbitMQConnection, 1,
                           envelope.delivery_tag, 0);

            QByteArray messageData(
                static_cast<char *>(
                    envelope.message.body.bytes),
                envelope.message.body.len);
            QJsonDocument doc =
                QJsonDocument::fromJson(messageData);
            QJsonObject jsonMessage = doc.object();

            emit dataReceived(jsonMessage);
            onDataReceivedFromRabbitMQ(jsonMessage,
                                       envelope);

            // Clean up the envelope
            amqp_destroy_envelope(&envelope);
        }
        else if (res.reply_type
                     == AMQP_RESPONSE_LIBRARY_EXCEPTION
                 && res.library_error
                        == AMQP_STATUS_TIMEOUT)
        {
            // Timeout reached but no message available,
            // continue to next iteration Sleep for a small
            // amount of time before checking again
            QThread::msleep(50);
        }
        else
        {
            qCritical() << "Error receiving message from "
                           "RabbitMQ. Type:"
                        << res.reply_type;
            stopRabbitMQServer();
            qDebug() << "Attempting to reconnect...";
            reconnectToRabbitMQ();
            break;
        }

        // Process events to keep the application responsive
        QCoreApplication::processEvents();
    }
}

void SimulationServer::onDataReceivedFromRabbitMQ(
    const QJsonObject     &message,
    const amqp_envelope_t &envelope)
{
    {
        QMutexLocker locker(&mMutex);
        if (mWorkerBusy)
        {
            qInfo() << "Simulator is busy, not consuming "
                       "new messages.";
            return;
        }
        mWorkerBusy = true;
    }

    // Ensure mWorkerBusy is reset even if an exception
    // occurs
    struct WorkerGuard
    {
        SimulationServer *server;
        ~WorkerGuard()
        {
            server->onWorkerReady();
        }
    };
    WorkerGuard guard{this};

    try
    {
        processCommand(message);
    }
    catch (const std::exception &e)
    {
        qCritical()
            << "Unhandled exception in processCommand: "
            << e.what();
        onErrorOccurred("Internal server error: "
                        + QString(e.what()));
    }
    catch (...)
    {
        qCritical() << "Unknown error in processCommand";
        onErrorOccurred("Internal server error");
    }
}

QPair<bool, QString> SimulationServer::checkJsonField(
    const QJsonObject &json, const QString &fieldName,
    const QString &command, bool checkParamsObject)
{
    // Check directly in the root object
    if (json.contains(fieldName))
    {
        return {true, ""};
    }

    // If instructed, also check in the params object
    if (checkParamsObject && json.contains("params")
        && json["params"].isObject())
    {
        QJsonObject params = json["params"].toObject();
        if (params.contains(fieldName))
        {
            return {true, ""};
        }
    }

    QString error = "Missing parameter: " + fieldName
                    + " in command: " + command;
    qWarning() << error;
    return {false, error};
}

QJsonValue
SimulationServer::getJsonValue(const QJsonObject &json,
                               const QString     &fieldName)
{
    // Check directly in the root object
    if (json.contains(fieldName))
    {
        return json[fieldName];
    }

    // Also check in the params object
    if (json.contains("params")
        && json["params"].isObject())
    {
        QJsonObject params = json["params"].toObject();
        if (params.contains(fieldName))
        {
            return params[fieldName];
        }
    }

    return QJsonValue();
}

QPair<bool, QString> SimulationServer::validateArray(
    const QJsonObject &json, const QString &fieldName,
    const QString &commandName, bool allowEmpty,
    bool checkElementsAreStrings)
{
    QJsonValue value = getJsonValue(json, fieldName);

    if (value.isUndefined())
    {
        QString error = "Missing parameter: " + fieldName
                        + " in command: " + commandName;
        qWarning() << error;
        return {false, error};
    }

    if (!value.isArray())
    {
        QString error = "'" + fieldName
                        + "' must be an array for command: "
                        + commandName;
        qWarning() << error;
        return {false, error};
    }

    QJsonArray arr = value.toArray();
    if (!allowEmpty && arr.isEmpty())
    {
        QString error =
            "'" + fieldName
            + "' array cannot be empty for command: "
            + commandName;
        qWarning() << error;
        return {false, error};
    }

    if (checkElementsAreStrings)
    {
        for (const QJsonValue &val : arr)
        {
            if (!val.isString())
            {
                QString error = "'" + fieldName
                                + "' array contains "
                                  "non-string elements "
                                  "for command: "
                                + commandName;
                qWarning() << error;
                return {false, error};
            }
        }
    }
    return {true, ""};
}

void SimulationServer::processCommand(
    const QJsonObject &jsonMessage)
{

    // --------------------------- NOTE --------------------
    // The CargoNetSim C++ implementation passes data in the
    // params value in the jsonMessage but the Python
    // implementation passes them in the root of the
    // jsonMessage so this function adaptes to both for now
    // but we should only consider the params case in the
    // future.
    // --------------------------- NOTE --------------------

    // Extract the command - always at the root level
    if (!jsonMessage.contains("command"))
    {
        onErrorOccurred(
            "Missing 'command' field in the message");
        return;
    }
    QString command = jsonMessage["command"].toString();

    // Extract commandId if present
    if (jsonMessage.contains("commandId"))
    {
        commandID = jsonMessage["commandId"].toString();
    }
    else
    {
        commandID.clear();
    }

    // Handle each command with improved error checking
    if (command == "checkConnection")
    {
        // Log the event for debugging (optional but
        // recommended)
        qInfo() << "[Server] Received command: "
                   "checkConnection. "
                   "Responding with 'connected'.";

        // Create the response JSON object
        QJsonObject response;
        response["event"] =
            "connectionStatus"; // Identifies the response
                                // type
        response["status"] =
            "connected"; // Confirms the server is connected
        response["host"] =
            "NeTrainSim"; // Identifies the server
        jsonMessage["success"] = true;

        // Only include commandId if it was in the original
        // request
        if (!commandID.isEmpty())
        {
            jsonMessage["commandId"] = commandID;
        }

        // Send the response via RabbitMQ
        sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                            response);

        // Signal that the worker is ready for the next
        // command
        onWorkerReady();
        return;
    }
    else if (command == "defineSimulator")
    {
        qInfo()
            << "[Server] Received command: "
               "defineSimulator. "
               "Initializing a new simulation environment.";

        // Validate required fields
        QList<QPair<bool, QString>> checks;
        checks << checkJsonField(jsonMessage, "nodesJson",
                                 command);
        checks << checkJsonField(jsonMessage, "linksJson",
                                 command);
        checks << checkJsonField(jsonMessage, "networkName",
                                 command);
        checks << checkJsonField(jsonMessage, "timeStep",
                                 command);

        // Collect errors
        QStringList errors;
        for (const auto &check : checks)
        {
            if (!check.first)
            {
                errors << check.second;
            }
        }
        if (!errors.isEmpty())
        {
            onErrorOccurred(errors.join("; "));
            return;
        }

        // Extract fields using the new helper function
        QJsonObject nodesContent =
            getJsonValue(jsonMessage, "nodesJson")
                .toObject();
        QJsonObject linksContent =
            getJsonValue(jsonMessage, "linksJson")
                .toObject();
        QString netName =
            getJsonValue(jsonMessage, "networkName")
                .toString();
        double timeStepValue =
            getJsonValue(jsonMessage, "timeStep")
                .toDouble(-100.0);

        // Validate timeStepValue
        if (timeStepValue <= 0)
        {
            onErrorOccurred(
                "Invalid time step value; must be "
                "a positive number");
            return;
        }

        // For trains, we need to pass the full object to
        // accommodate either location
        auto trainsI =
            TrainsList::readTrainsFromJSON(jsonMessage);
        if (trainsI.empty())
        {
            // If the trains list is empty, try to read
            // from params
            QJsonObject params =
                getJsonValue(jsonMessage, "params")
                    .toObject();
            trainsI =
                TrainsList::readTrainsFromJSON(params);
        }
        auto trains = Utils::convertToQVector(trainsI);

        qDebug() << "[Server] Loading network: " << netName
                 << " with time step: " << timeStepValue
                 << "s.";

        // Create simulation environment
        try
        {
            SimulatorAPI::InteractiveMode::
                createNewSimulationEnvironment(
                    nodesContent, linksContent, netName,
                    trains, timeStepValue);
        }
        catch (const std::exception &e)
        {
            QString error =
                "Error while creating the environment: "
                + QString(e.what());
            qWarning() << error;
            onErrorOccurred(error);
            return;
        }
    }
    else if (command == "runSimulator")
    {
        qInfo()
            << "[Server] Received command: runSimulator. "
               "Running simulation.";

        // Validate networkNames array
        auto arrayCheck = validateArray(
            jsonMessage, "networkNames", command);
        if (!arrayCheck.first)
        {
            onErrorOccurred(arrayCheck.second);
            return;
        }

        // Validate byTimeSteps
        QJsonValue byTimeStepsValue =
            getJsonValue(jsonMessage, "byTimeSteps");
        if (byTimeStepsValue.isUndefined()
            || !byTimeStepsValue.isDouble())
        {
            onErrorOccurred(
                "'byTimeSteps' must be a numeric value");
            return;
        }

        // Extract network names
        QVector<QString> nets;
        QJsonArray       networkNamesArray =
            getJsonValue(jsonMessage, "networkNames")
                .toArray();
        for (const QJsonValue &value : networkNamesArray)
        {
            nets.append(value.toString());
        }

        // Extract byTimeSteps
        double runBy = byTimeStepsValue.toDouble();

        // Connect progress update if runBy <= 0
        if (runBy <= 0)
        {
            if (m_progressConnection)
            {
                disconnect(m_progressConnection);
                m_progressConnection =
                    QMetaObject::Connection();
            }
            m_progressConnection = connect(
                &SimulatorAPI::InteractiveMode::
                    getInstance(),
                &SimulatorAPI::simulationProgressUpdated,
                this,
                &SimulationServer::
                    onSimulationProgressUpdate);
        }

        // Run simulation
        try
        {
            SimulatorAPI::InteractiveMode::runSimulation(
                nets, runBy, true);
        }
        catch (const std::exception &e)
        {
            onErrorOccurred("Error running simulation: "
                            + QString(e.what()));
            return;
        }
    }
    else if (command == "terminateSimulator")
    {
        qInfo() << "[Server] Received command: "
                   "terminateSimulator. "
                   "Terminating simulation.";

        // Validate networkNames array
        auto arrayCheck = validateArray(
            jsonMessage, "networkNames", command);
        if (!arrayCheck.first)
        {
            onErrorOccurred(arrayCheck.second);
            return;
        }

        // Extract network names
        QVector<QString> nets;
        QJsonArray       networkNamesArray =
            getJsonValue(jsonMessage, "networkNames")
                .toArray();
        for (const QJsonValue &value : networkNamesArray)
        {
            nets.append(value.toString());
        }

        // Terminate simulation
        try
        {
            SimulatorAPI::InteractiveMode::
                terminateSimulation(nets);
        }
        catch (const std::exception &e)
        {
            onErrorOccurred("Error terminating simulation: "
                            + QString(e.what()));
            return;
        }
    }
    else if (command == "endSimulator")
    {
        qInfo()
            << "[Server] Received command: endSimulator. "
               "Ending simulation.";

        // Validate networkNames array
        auto arrayCheck = validateArray(
            jsonMessage, "networkNames", command);
        if (!arrayCheck.first)
        {
            onErrorOccurred(arrayCheck.second);
            return;
        }

        // Extract network names
        QVector<QString> nets;
        QJsonArray       networkNamesArray =
            getJsonValue(jsonMessage, "networkNames")
                .toArray();
        for (const QJsonValue &value : networkNamesArray)
        {
            nets.append(value.toString());
        }

        // Finalize simulation
        try
        {
            SimulatorAPI::InteractiveMode::
                finalizeSimulation(nets);
        }
        catch (const std::exception &e)
        {
            onErrorOccurred("Error ending simulation: "
                            + QString(e.what()));
            return;
        }
    }
    else if (command == "addTrainsToSimulator")
    {
        qInfo() << "[Server] Received command: "
                   "addTrainsToSimulator. "
                   "Adding trains to simulation.";

        // Validate required fields
        QList<QPair<bool, QString>> checks;
        checks << checkJsonField(jsonMessage, "network",
                                 command);
        checks << checkJsonField(jsonMessage, "trains",
                                 command);

        // Collect errors
        QStringList errors;
        for (const auto &check : checks)
        {
            if (!check.first)
            {
                errors << check.second;
            }
        }
        if (!errors.isEmpty())
        {
            onErrorOccurred(errors.join("; "));
            return;
        }

        // Extract network name and trains
        QString netName =
            getJsonValue(jsonMessage, "network").toString();

        // For trains, we need to pass the full object to
        // accommodate either location
        auto trains =
            TrainsList::ReadAndGenerateTrainsFromJSON(
                jsonMessage);
        if (trains.empty())
        {
            // If the trains list is empty, try to read
            // from params
            QJsonObject params =
                getJsonValue(jsonMessage, "params")
                    .toObject();
            trains =
                TrainsList::ReadAndGenerateTrainsFromJSON(
                    params);
        }

        // Convert to QVector<std::shared_ptr<Train>>
        QVector<std::shared_ptr<Train>> qTrains;
        for (auto &train : trains)
        {
            qTrains.push_back(std::move(train));
        }

        // Add trains to simulation
        try
        {
            SimulatorAPI::InteractiveMode::
                addTrainToSimulation(netName, qTrains);
        }
        catch (const std::exception &e)
        {
            onErrorOccurred(
                "Error adding trains to simulation: "
                + QString(e.what()));
            return;
        }
    }
    else if (command
             == "unloadContainersFromTrainAtCurrentTermina"
                "l")
    {
        qInfo() << "[Server] Received command: "
                   "unloadContainersFromTrainAtCurrentTermi"
                   "nal. "
                   "Unloading containers from a train.";

        // Validate required fields
        QList<QPair<bool, QString>> checks;
        checks << checkJsonField(jsonMessage, "networkName",
                                 command);
        checks << checkJsonField(jsonMessage, "trainID",
                                 command);

        // Collect errors
        QStringList errors;
        for (const auto &check : checks)
        {
            if (!check.first)
            {
                errors << check.second;
            }
        }
        if (!errors.isEmpty())
        {
            onErrorOccurred(errors.join("; "));
            return;
        }

        // Extract network name and train ID
        QString net =
            getJsonValue(jsonMessage, "networkName")
                .toString();
        QString trainID =
            getJsonValue(jsonMessage, "trainID").toString();

        // Extract optional ContainersDestinationNames
        QVector<QString> portNames;
        QJsonValue       containersDestValue = getJsonValue(
            jsonMessage, "ContainersDestinationNames");
        if (!containersDestValue.isUndefined())
        {
            QJsonArray portNamesArray =
                containersDestValue.toArray();
            for (const QJsonValue &value : portNamesArray)
            {
                portNames.append(value.toString());
            }
        }

        // Unload containers
        try
        {
            SimulatorAPI::InteractiveMode::
                requestUnloadContainersAtTerminal(
                    net, trainID, portNames);
        }
        catch (const std::exception &e)
        {
            onErrorOccurred("Error unloading containers: "
                            + QString(e.what()));
            return;
        }
    }
    else if (command == "addContainersToTrain")
    {
        qInfo() << "[Server] Received command: "
                   "addContainersToTrain. "
                   "Adding containers to a train.";

        // Validate required fields
        QList<QPair<bool, QString>> checks;
        checks << checkJsonField(jsonMessage, "networkName",
                                 command);
        checks << checkJsonField(jsonMessage, "trainID",
                                 command);

        // Collect errors
        QStringList errors;
        for (const auto &check : checks)
        {
            if (!check.first)
            {
                errors << check.second;
            }
        }
        if (!errors.isEmpty())
        {
            onErrorOccurred(errors.join("; "));
            return;
        }

        // Extract network name and train ID
        QString net =
            getJsonValue(jsonMessage, "networkName")
                .toString();
        QString trainID =
            getJsonValue(jsonMessage, "trainID").toString();

        // Add containers to train - pass the full
        // jsonMessage to handle parameters in either
        // location
        try
        {
            bool containersAdded = SimulatorAPI::
                InteractiveMode::addContainersToTrain(
                    net, trainID, jsonMessage);
            if (!containersAdded)
            {
                // If the containers list is empty, try to
                // read from params
                QJsonObject params =
                    getJsonValue(jsonMessage, "params")
                        .toObject();
                SimulatorAPI::InteractiveMode::
                    addContainersToTrain(net, trainID,
                                         params);
            }
        }
        catch (const std::exception &e)
        {
            onErrorOccurred(
                "Error adding containers to train: "
                + QString(e.what()));
            return;
        }
    }
    else if (command == "resetServer")
    {
        qInfo()
            << "[Server] Received command: resetServer. "
               "Resetting the server.";

        // Reset the server
        try
        {
            SimulatorAPI::InteractiveMode::resetAPI();
            onServerReset();
        }
        catch (const std::exception &e)
        {
            onErrorOccurred("Error resetting server: "
                            + QString(e.what()));
            return;
        }
    }
    else
    {
        onErrorOccurred("Unrecognized command: " + command);
        qWarning()
            << "[Server] Received unrecognized command: "
            << command;
    }
}

void SimulationServer::onWorkerReady()
{
    // Worker is now ready for the next command
    QMutexLocker locker(&mMutex);

    mWorkerBusy = false;

    mWaitCondition.wakeAll(); // Notify waiting consumers
}

void SimulationServer::sendRabbitMQMessage(
    const QString &routingKey, const QJsonObject &message)
{
    QByteArray messageData =
        QJsonDocument(message).toJson();
    amqp_bytes_t messageBytes;
    messageBytes.len   = messageData.size();
    messageBytes.bytes = messageData.data();

    int retries = MAX_SEND_COMMAND_RETRIES;
    while (retries > 0)
    {
        int publishStatus = amqp_basic_publish(
            mRabbitMQConnection, 1,
            amqp_cstring_bytes(EXCHANGE_NAME.c_str()),
            amqp_cstring_bytes(
                routingKey.toUtf8().constData()),
            0, 0, nullptr, messageBytes);

        if (publishStatus == AMQP_STATUS_OK)
        {
            return; // Success
        }
        qWarning()
            << "Failed to publish message to RabbitMQ "
               "with routing key:"
            << routingKey << ". Retrying...";
        retries--;
        QThread::msleep(
            1000); // Wait 1 second before retrying
    }
    qCritical() << "Failed to publish message to RabbitMQ "
                   "after retries with routing key:"
                << routingKey;
}

// simulation events handling
void SimulationServer::onSimulationCreated(
    QString networkName)
{
    QJsonObject jsonMessage;
    jsonMessage["event"]   = "simulationCreated";
    jsonMessage["host"]    = "NeTrainSim";
    jsonMessage["network"] = networkName;
    jsonMessage["success"] = true;

    // Only include commandId if it was in the original
    // request
    if (!commandID.isEmpty())
    {
        jsonMessage["commandId"] = commandID;
    }
    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();
    qInfo()
        << "Environemnt created successfully for network: "
        << networkName;
}

void SimulationServer::onSimulationsPaused(
    QVector<QString> networkNames)
{
    QJsonObject jsonMessage;
    jsonMessage["event"]   = "simulationPaused";
    jsonMessage["host"]    = "NeTrainSim";
    jsonMessage["success"] = true;

    // Only include commandId if it was in the original
    // request
    if (!commandID.isEmpty())
    {
        jsonMessage["commandId"] = commandID;
    }

    // Convert QVector<QString> to QJsonArray
    QJsonArray jsonNetworkNames;
    for (const QString &name : networkNames)
    {
        jsonNetworkNames.append(name);
    }

    // Add the network names to the JSON message
    jsonMessage["networkNames"] = jsonNetworkNames;

    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();
}

void SimulationServer::onSimulationsResumed(
    QVector<QString> networkNames)
{
    QJsonObject jsonMessage;
    jsonMessage["event"]   = "simulationResumed";
    jsonMessage["host"]    = "NeTrainSim";
    jsonMessage["success"] = true;

    // Only include commandId if it was in the original
    // request
    if (!commandID.isEmpty())
    {
        jsonMessage["commandId"] = commandID;
    }

    // Convert QVector<QString> to QJsonArray
    QJsonArray jsonNetworkNames;
    for (const QString &name : networkNames)
    {
        jsonNetworkNames.append(name);
    }

    // Add the network names to the JSON message
    jsonMessage["networkNames"] = jsonNetworkNames;

    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();
}

void SimulationServer::onSimulationsEnded(
    QVector<QString> networkNames)
{
    QJsonObject jsonMessage;
    jsonMessage["event"]   = "simulationEnded";
    jsonMessage["host"]    = "NeTrainSim";
    jsonMessage["success"] = true;

    // Only include commandId if it was in the original
    // request
    if (!commandID.isEmpty())
    {
        jsonMessage["commandId"] = commandID;
    }

    // Convert QVector<QString> to QJsonArray
    QJsonArray jsonNetworkNames;
    for (const QString &name : networkNames)
    {
        jsonNetworkNames.append(name);
    }

    // Add the network names to the JSON message
    jsonMessage["networkNames"] = jsonNetworkNames;

    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();

    qInfo()
        << "Simulation ended successfully for networks: "
        << networkNames.join(", ");
}

void SimulationServer::onSimulationAdvanced(
    QMap<QString, QPair<double, double>>
        networkNamesSimulationTimePairs)
{
    QJsonObject jsonMessage;
    jsonMessage["event"]   = "simulationAdvanced";
    jsonMessage["host"]    = "NeTrainSim";
    jsonMessage["success"] = true;

    // Only include commandId if it was in the original
    // request
    if (!commandID.isEmpty())
    {
        jsonMessage["commandId"] = commandID;
    }

    // Convert QVector<QString> to QJsonArray
    QJsonObject jsonNetworkTimes;
    QJsonObject jsonNetworkProgress;
    for (auto it =
             networkNamesSimulationTimePairs.constBegin();
         it != networkNamesSimulationTimePairs.constEnd();
         ++it)
    {
        // Add each network name (key) and its corresponding
        // simulation time (value) to the jsonNetworkTimes
        // object
        jsonNetworkTimes[it.key()]    = it.value().first;
        jsonNetworkProgress[it.key()] = it.value().second;
    }

    // Add the network names to the JSON message
    jsonMessage["networkNamesTimes"] = jsonNetworkTimes;
    jsonMessage["networkNamesProgress"] =
        jsonNetworkProgress;

    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();
}

void SimulationServer::onSimulationProgressUpdate(
    QString            networkName,
    QPair<double, int> progressPercentage)
{
    // Check if progressPercentage is a multiple of 5
    if (progressPercentage.second % 5 == 0)
    {
        QJsonObject jsonMessage;
        jsonMessage["event"] = "simulationProgressUpdate";
        jsonMessage["networkName"] = networkName;
        jsonMessage["newProgress"] =
            progressPercentage.second;
        jsonMessage["host"]    = "NeTrainSim";
        jsonMessage["success"] = true;

        // Only include commandId if it was in the original
        // request
        if (!commandID.isEmpty())
        {
            jsonMessage["commandId"] = commandID;
        }

        // Send the message
        sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                            jsonMessage);
    }
}

void SimulationServer::onTrainsAddedToSimulator(
    const QString          networkName,
    const QVector<QString> trainIDs)
{
    QJsonObject jsonMessage;
    jsonMessage["event"]        = "trainAddedToSimulator";
    jsonMessage["networkNames"] = networkName;
    jsonMessage["success"]      = true;

    // Only include commandId if it was in the original
    // request
    if (!commandID.isEmpty())
    {
        jsonMessage["commandId"] = commandID;
    }
    QJsonArray trainsJson;
    for (const auto &trainID : trainIDs)
    {
        trainsJson.append(trainID);
    }
    jsonMessage["trainIDs"] = trainsJson;
    jsonMessage["host"]     = "NeTrainSim";
    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();

    qInfo() << "Train ID(s): " << trainIDs.join(", ")
            << " added successfully to network: "
            << networkName;
}

void SimulationServer::onAllTrainsReachedDestination(
    const QString networkName)
{
    QJsonObject jsonMessage;
    jsonMessage["event"] = "allTrainsReachedDestination";
    jsonMessage["networkName"] = networkName;
    jsonMessage["host"]        = "NeTrainSim";
    jsonMessage["success"]     = true;

    // Only include commandId if it was in the original
    // request
    if (!commandID.isEmpty())
    {
        jsonMessage["commandId"] = commandID;
    }
    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();
}

void SimulationServer::onTrainReachedDestination(
    const QString     networkName,
    const QJsonObject trainStatus)
{
    QJsonObject jsonMessage;
    jsonMessage["event"]       = "trainReachedDestination";
    jsonMessage["networkName"] = networkName;
    jsonMessage["state"]       = trainStatus;
    jsonMessage["host"]        = "NeTrainSim";
    jsonMessage["success"]     = true;

    // Only include commandId if it was in the original
    // request
    if (!commandID.isEmpty())
    {
        jsonMessage["commandId"] = commandID;
    }
    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    // qInfo() << "Train reached destination";
}

void SimulationServer::onSimulationResultsAvailable(
    QString networkName, TrainsResults results)
{
    QJsonObject jsonMessage;
    jsonMessage["event"]   = "simulationResultsAvailable";
    jsonMessage["results"] = results.toJson();
    jsonMessage["host"]    = "NeTrainSim";
    jsonMessage["success"] = true;

    // Only include commandId if it was in the original
    // request
    if (!commandID.isEmpty())
    {
        jsonMessage["commandId"] = commandID;
    }
    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();

    qInfo() << "Simulation results sent to consumers!";
}

void SimulationServer::onContainersAddedToTrain(
    QString networkName, QString trainID)
{
    QJsonObject jsonMessage;
    jsonMessage["event"]       = "containersAddedToTrain";
    jsonMessage["networkName"] = networkName;
    jsonMessage["trainID"]     = trainID;
    jsonMessage["host"]        = "NeTrainSim";
    jsonMessage["success"]     = true;

    // Only include commandId if it was in the original
    // request
    if (!commandID.isEmpty())
    {
        jsonMessage["commandId"] = commandID;
    }

    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();

    qInfo() << "Containers successfully added to Train ID: "
            << trainID << " of network: " << networkName
            << "!";
}

void SimulationServer::onTrainReachedTerminal(
    QString networkName, QString trainID,
    QString terminalID, int containersCount)
{
    QJsonObject jsonMessage;
    jsonMessage["event"]           = "trainReachedTerminal";
    jsonMessage["networkName"]     = networkName;
    jsonMessage["trainID"]         = trainID;
    jsonMessage["terminalID"]      = terminalID;
    jsonMessage["containersCount"] = containersCount;
    jsonMessage["host"]            = "NeTrainSim";
    jsonMessage["success"]         = true;

    // Only include commandId if it was in the original
    // request
    if (!commandID.isEmpty())
    {
        jsonMessage["commandId"] = commandID;
    }

    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();
}

void SimulationServer::onContainersUnloaded(
    QString networkName, QString trainID,
    QString terminalID, QJsonArray containers)
{
    QJsonObject jsonMessage;
    jsonMessage["event"]       = "containersUnloaded";
    jsonMessage["networkName"] = networkName;
    jsonMessage["trainID"]     = trainID;
    jsonMessage["terminalID"]  = terminalID;
    jsonMessage["containers"]  = containers;
    jsonMessage["host"]        = "NeTrainSim";
    jsonMessage["success"]     = true;

    // Only include commandId if it was in the original
    // request
    if (!commandID.isEmpty())
    {
        jsonMessage["commandId"] = commandID;
    }

    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();
}

void SimulationServer::onErrorOccurred(
    const QString &errorMessage)
{
    QJsonObject jsonMessage;
    jsonMessage["event"]        = "errorOccurred";
    jsonMessage["errorMessage"] = errorMessage;
    jsonMessage["host"]         = "NeTrainSim";
    jsonMessage["success"]      = false;

    // Only include commandId if it was in the original
    // request
    if (!commandID.isEmpty())
    {
        jsonMessage["commandId"] = commandID;
    }
    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();
    qInfo() << "Error Occured: " << errorMessage;
}

void SimulationServer::onServerReset()
{
    setupServer();
    QJsonObject jsonMessage;
    jsonMessage["event"]   = "serverReset";
    jsonMessage["host"]    = "NeTrainSim";
    jsonMessage["success"] = true;

    // Only include commandId if it was in the original
    // request
    if (!commandID.isEmpty())
    {
        jsonMessage["commandId"] = commandID;
    }
    sendRabbitMQMessage(PUBLISHING_ROUTING_KEY.c_str(),
                        jsonMessage);
    onWorkerReady();
    qInfo() << "Server reset Successfully!";
}
