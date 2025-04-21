#include "simulatorworker.h"
#include "simulatorapi.h"
#include "traindefinition/trainslist.h"
#include "util/vector.h"
#include "util/map.h"
#include <qthread.h>

SimulatorWorker::SimulatorWorker(QObject *parent)
    : QObject{parent}
{}

void SimulatorWorker::setupSimulator(
    APIData& apiData,
    QString networkName,
    const QVector<QMap<QString, QString> > &nodeRecords,
    const QVector<QMap<QString, QString> > &linkRecords,
    const QVector<QMap<QString, std::any> > &trainsList,
    double timeStep)
{
    qDebug() << "Creating simulator inside thread:"
             << QThread::currentThread();

    auto convertToStdVector =
        [](const QVector<QMap<QString, QString>>& qVector) {
            Vector<Map<std::string, std::string>> stdVector;
            std::transform(qVector.begin(), qVector.end(),
                           std::back_inserter(stdVector),
                           [](const QMap<QString, QString>& qMap) {
                               Map<std::string, std::string> stdMap;
                               for (auto it = qMap.begin();
                                    it != qMap.end(); ++it) {
                                   stdMap[it.key().toStdString()] =
                                       it.value().toStdString();
                               }
                               return stdMap;
                           });
            return stdVector;
        };

    try {
        auto snodeRecords = convertToStdVector(nodeRecords);
        auto slinkRecords = convertToStdVector(linkRecords);
        auto trainsRecords = Utils::convertToStdVector(trainsList);

        qInfo() << "Reading Network: " << networkName.toStdString()
                << "!                    \r";
        auto nodes = ReadWriteNetwork::generateNodes(snodeRecords);
        auto links = ReadWriteNetwork::generateLinks(nodes, slinkRecords);
        auto trains =
            TrainsList::generateTrains(trainsRecords, true);
        auto trainsq = Utils::convertToQVector(trains);

        qInfo() << "Define Simulator Space for network: "
                << networkName.toStdString()
                << "!                          \r";

        apiData.network = new Network(nodes, links, networkName.toStdString());


        // Store the train list in APIData for future reference
        apiData.trains.clear();
        for (const auto& train : trainsq) {
            apiData.trains[QString::fromStdString(train->trainUserID)] = train;
        }


        // ✅ Create Simulator inside the worker thread
        apiData.simulator = new Simulator(
            apiData.network, trainsq, timeStep);


        qDebug() << "Simulator successfully created inside thread: "
                 << QThread::currentThread();

        emit simulatorLoaded(apiData);

    } catch (std::exception &e) {
        QString error = "Error in launching the simulator " +
                        QString(e.what());
        qWarning() << error;
        emit errorOccured(error);
    }
}
