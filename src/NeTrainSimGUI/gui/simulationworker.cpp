#include "simulationworker.h"
#include "../NeTrainSim/traindefinition/trainslist.h"
#include <any>

SimulationWorker::SimulationWorker(
    Vector<Map<std::string, std::string>> nodeRecords,
    Vector<Map<std::string, std::string>> linkRecords,
    Vector<Map<std::string, std::any>> trainRecords,
    std::string networkName,
    double endTime, double timeStep,
    double plotFrequency, std::string exportDir,
    std::string summaryFilename, bool exportInsta,
    std::string instaFilename, bool exportAllTrainsSummary)
{

    // check if the nodeRecords and linkRecords are empty
    if (nodeRecords.size() < 1) {
        emit errorOccurred("No nodes are added!"); return;
    }
    if (linkRecords.size() < 1) {
        emit errorOccurred("No links are added!"); return;
    }
    auto nodes = ReadWriteNetwork::generateNodes(nodeRecords);
    auto links = ReadWriteNetwork::generateLinks(nodes, linkRecords);
    this->net = new Network(nodes, links, networkName);
    auto trains = TrainsList::generateTrains(trainRecords);

    // check if the trainrecords is empty
    if (trains.size() < 1) {
        emit errorOccurred("No trains are added!"); return;
    }

    // report the suddent acceleration or slow speeds
    for (auto &t : trains) {
        connect(t.get(), &Train::suddenAccelerationOccurred,
                [this](const std::string& msg) {
            emit trainSuddenAcceleration(msg);
        });

        connect(t.get(), &Train::slowSpeedOrStopped,
               [this](const std::string& msg) {
            emit trainSlowSpeed(msg);
        });
    }

    this->sim = new Simulator(net, trains, timeStep);
    this->sim->setEndTime(endTime);
    this->sim->setTimeStep(timeStep);
    this->sim->setPlotFrequency(plotFrequency);
    this->sim->setOutputFolderLocation(exportDir);
    this->sim->setSummaryFilename(summaryFilename);
    if (instaFilename.size() > 1) {
        this->sim->setExportInstantaneousTrajectory(exportInsta, instaFilename);
    }
    this->sim->setExportIndividualizedTrainsSummary(exportAllTrainsSummary);

    connect(this->sim, &Simulator::finishedSimulation, this,
            &SimulationWorker::onSimulationFinished);
    connect(this->sim, &Simulator::plotTrainsUpdated, this,
            &SimulationWorker::onTrainsCoordinatesUpdated);
    connect(this->sim, &Simulator::progressUpdated, this,
            &SimulationWorker::onProgressUpdated);
    connect(this->sim, &Simulator::trainsCollided, [&](std::string& msg){
        emit this->trainsCollided(msg);});
}

void SimulationWorker::onProgressUpdated(int progressPercentage) {
    emit simulaionProgressUpdated(progressPercentage);
}

void SimulationWorker::onTrainsCoordinatesUpdated(
    Vector<std::pair<std::string,
                     Vector<std::pair<double,double>>>> trainsStartEndPoints) {
    emit trainsCoordinatesUpdated(trainsStartEndPoints);
}

void SimulationWorker::onSimulationFinished(
    const Vector<std::pair<std::string,
                           std::string>>& summaryData,
    const std::string& trajectoryFile) {
    emit simulationFinished(summaryData, trajectoryFile);
}

void SimulationWorker::doWork() {
    try {
        this->sim->runSimulation();
    } catch (const std::exception& e) {
        emit errorOccurred(e.what());
    }

}

SimulationWorker::~SimulationWorker() {
    delete this->net;
    delete this->sim;
}

