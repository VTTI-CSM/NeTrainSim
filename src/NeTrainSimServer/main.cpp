#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QLocalServer>
#include <QLocalSocket>
#include "SimulationServer.h"

bool isAnotherInstanceRunning(const QString &serverName) {
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (socket.waitForConnected(100)) {
        return true; // Another instance is already running
    }
    return false; // No instance running
}

void createLocalServer(const QString &serverName) {
    QLocalServer *localServer = new QLocalServer();
    localServer->setSocketOptions(QLocalServer::WorldAccessOption);
    if (!localServer->listen(serverName)) {
        qCritical() << "Failed to create local server:" << localServer->errorString();
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // Unique name for the local server
    const QString uniqueServerName = "NeTrainSimServerInstance";

    // Check if another instance is already running
    if (isAnotherInstanceRunning(uniqueServerName)) {
        qCritical() << "Another instance of NeTrainSim Server is already running.";
        return EXIT_FAILURE;
    }

    // Create the local server to mark this instance as the active one
    createLocalServer(uniqueServerName);

    // Set up the command-line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("NeTrainSim Server with RabbitMQ");
    parser.addHelpOption();

    // Add hostname option
    QCommandLineOption hostnameOption(
        QStringList() << "n" << "hostname",
        "RabbitMQ server hostname (default: localhost).",
        "hostname",
        "localhost");
    parser.addOption(hostnameOption);

    // Add port option (default: 5672)
    QCommandLineOption portOption(
        QStringList() << "p" << "port",
        "RabbitMQ server port (default: 5672).",
        "port",
        "5672");
    parser.addOption(portOption);

    // Process the command-line arguments
    parser.process(app);

    // Start the simulation server
    // Server loads config from NeTrainSim_rabbitmq.xml in constructor
    SimulationServer server;

    // CLI arguments override config file values only if explicitly set
    std::string hostname = "localhost";
    int         port     = 5672;

    if (parser.isSet(hostnameOption))
    {
        hostname = parser.value(hostnameOption).toStdString();
    }

    if (parser.isSet(portOption))
    {
        port = parser.value(portOption).toInt();
    }

    server.startRabbitMQServer(hostname, port,
                               parser.isSet(hostnameOption),
                               parser.isSet(portOption));

    return app.exec();
}
