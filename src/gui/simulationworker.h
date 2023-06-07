#ifndef SIMULATIONWORKER_H
#define SIMULATIONWORKER_H

#include <QObject>
#include "src/Simulator.h"

class SimulationWorker : public QObject {
    Q_OBJECT
public:
    SimulationWorker(Vector<std::tuple<int, double, double, std::string,
                                       double, double>> nodeRecords,
                     Vector<tuple<int, int, int, double, double, int,
                                  double, double, int, double, bool,
                                  std::string, double, double>> linkRecords,
                     Vector<tuple<std::string, Vector<int>, double, double,
                                  Vector<tuple<double, double, double, double, double, double, int, int>>,
                                  Vector<tuple<double, double, double, double, double, int, int>>,
                                  bool>> trainRecords,
                     std::string networkName,
                     double endTime, double timeStep, double plotFrequency, std::string exportDir,
                     std::string summaryFilename, bool exportInsta, std::string instaFilename, bool exportAllTrainsSummary);
    ~SimulationWorker();

signals:
    void simulationFinished(const Vector<std::pair<std::string, std::string>>& summaryData);
    void trainsCoordinatesUpdated(Vector<std::pair<std::string, Vector<std::pair<double,double>>>> trainsStartEndPoints);
    void simulaionProgressUpdated(int progressPercentage);
    void errorOccurred(std::string error);

public slots:
    void onProgressUpdated(int progressPercentage);
    void onTrainsCoordinatesUpdated(Vector<std::pair<std::string, Vector<std::pair<double,double>>>> trainsStartEndPoints);
    void onSimulationFinished(const Vector<std::pair<std::string, std::string>> &summaryData);
    void doWork();

private:
    Simulator* sim;
    Network* net;
};

#endif // SIMULATIONWORKER_H
