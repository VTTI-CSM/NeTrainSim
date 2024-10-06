// SimulationServer.h
#ifndef SIMULATIONSERVER_H
#define SIMULATIONSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QObject>
#include <QVector>
#include <QMap>
#include <QPair>
#include <QThread>
#include <QMetaType>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQueue>
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include "simulatorapi.h"

// Define a typedef for QMap<QString, QString>
using TrainParamsMap = QMap<QString, QVariant>;
// Declare the type with the meta-object system
Q_DECLARE_METATYPE(TrainParamsMap)

// Compilation date and time are set by the preprocessor.
const QString compilation_date = __DATE__;
const QString compilation_time = __TIME__;
const QString GithubLink = "https://github.com/VTTI-CSM/NeTrainSim";

class SimulationServer : public QObject {
    Q_OBJECT

public:
    explicit SimulationServer(QObject *parent = nullptr);
    ~SimulationServer();
    void startRabbitMQServer(const std::string &hostname, int port);
    void sendRabbitMQMessage(const QString &routingKey,
                             const QJsonObject &message);
    void stopRabbitMQServer();  // stop RabbitMQ server cleanly

signals:
    void dataReceived(QJsonObject message);
    void trainReachedDestination(const QString& shipID);
    void simulationResultsAvailable(
        const QVector<std::pair<QString, QString>>& summaryData,
        const QString& trajectoryFile);

private slots:
    void onDataReceivedFromRabbitMQ(const QJsonObject &message,
                                    const amqp_envelope_t &envelope);
    void onWorkerReady();  // resume processing when the worker is ready
    void onSimulationCreated(QString networkName);
    void onSimulationsPaused(QVector<QString> networkNames);
    void onSimulationsResumed(QVector<QString> networkNames);
    void onSimulationsEnded(QVector<QString> networkNames);
    // void onTrainAddedToSimulator();
    void onTrainReachedDestination(QJsonObject networkTrainsPairs);
    void onSimulationResultsAvailable(QMap<QString, TrainsResults>& results);
    void onErrorOccurred(const QString &errorMessage);

private:
    std::string mHostname;
    int mPort;

    void processCommand(const QJsonObject &jsonMessage);

    amqp_connection_state_t mRabbitMQConnection;

    bool mWorkerBusy;  // To control the server run loop
    QQueue<QJsonObject> mCommandQueue;  // Queue to store incoming commands
};

#endif // SIMULATIONSERVER_H
