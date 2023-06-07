#include "SimulationWorker.h"
#include "src/TrainDefintion/TrainsList.h"


SimulationWorker::SimulationWorker(Vector<std::tuple<int, double, double, std::string,
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
                                   std::string summaryFilename, bool exportInsta, std::string instaFilename, bool exportAllTrainsSummary) {

    auto nodes = ReadWriteNetwork::generateNodes(nodeRecords);
    auto links = ReadWriteNetwork::generateLinks(nodes, linkRecords);
    this->net = new Network(nodes, links, networkName);
    auto trains = TrainsList::generateTrains(trainRecords);

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

    connect(this->sim, &Simulator::finishedSimulation, this, &SimulationWorker::onSimulationFinished);
    connect(this->sim, &Simulator::plotTrainsUpdated, this, &SimulationWorker::onTrainsCoordinatesUpdated);
    connect(this->sim, &Simulator::progressUpdated, this, &SimulationWorker::onProgressUpdated);
}

void SimulationWorker::onProgressUpdated(int progressPercentage) {
    emit simulaionProgressUpdated(progressPercentage);
}

void SimulationWorker::onTrainsCoordinatesUpdated(Vector<std::pair<std::string, Vector<std::pair<double,double>>>> trainsStartEndPoints) {
    emit trainsCoordinatesUpdated(trainsStartEndPoints);
}

void SimulationWorker::onSimulationFinished(const Vector<std::pair<std::string, std::string>>& summaryData) {
    emit simulationFinished(summaryData);
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
