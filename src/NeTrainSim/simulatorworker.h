#ifndef SIMULATORWORKER_H
#define SIMULATORWORKER_H

#include <QObject>
#include <any>

// Forward declare APIData and ShipNetSimCore::Ship
struct APIData;

class Train;

class SimulatorWorker : public QObject
{
    Q_OBJECT
public:
    explicit SimulatorWorker(QObject *parent = nullptr);
    void setupSimulator(APIData& apiData, QString networkName,
                        const QVector<QMap<QString, QString> > &nodeRecords,
                        const QVector<QMap<QString, QString> > &linkRecords,
                        const QVector<QMap<QString, std::any> > &trainsList,
                        double timeStep);
signals:
    void simulatorLoaded();
    void errorOccured(QString error);
};

#endif // SIMULATORWORKER_H
