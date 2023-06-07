/**
 * @file SimulationWorker.h
 * @brief This file contains the declaration of the SimulationWorker class.
 *        The SimulationWorker class is a QObject subclass that performs simulation work in a separate thread.
 *        It receives input data and parameters, performs the simulation using the Simulator class, and emits signals to update the UI.
 *        The SimulationWorker class is intended to be used for simulation tasks in a multi-threaded application.
 *        It is designed to work with the Simulator class and communicates with the UI through signals and slots.
 *        The SimulationWorker class takes nodeRecords, linkRecords, trainRecords, networkName, endTime, timeStep, plotFrequency,
 *        exportDir, summaryFilename, exportInsta, instaFilename, and exportAllTrainsSummary as input data for simulation.
 *        It performs the simulation, updates the progress, coordinates of trains, and emits signals to inform the UI about the progress and results.
 *        The SimulationWorker class can be used in a QWidget-based application.
 * @author Ahmed Aredah
 * @date 6/7/2023
 */

#ifndef SIMULATIONWORKER_H
#define SIMULATIONWORKER_H

#include <QObject>
#include "src/Simulator.h"

/**
 * @class SimulationWorker
 * @brief The SimulationWorker class performs simulation work in a separate thread.
 */
class SimulationWorker : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructs a SimulationWorker object with the specified input data and parameters.
     * @param nodeRecords The node records for the simulation.
     * @param linkRecords The link records for the simulation.
     * @param trainRecords The train records for the simulation.
     * @param networkName The network name for the simulation.
     * @param endTime The end time for the simulation.
     * @param timeStep The time step for the simulation.
     * @param plotFrequency The plot frequency for the simulation.
     * @param exportDir The export directory for the simulation.
     * @param summaryFilename The summary filename for the simulation.
     * @param exportInsta Indicates whether to export instant data in the simulation.
     * @param instaFilename The instant data filename for the simulation.
     * @param exportAllTrainsSummary Indicates whether to export summary data for all trains in the simulation.
     */
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

    /**
     * @brief Destroys the SimulationWorker object.
     */
    ~SimulationWorker();

signals:
    /**
     * @brief Signal emitted when the simulation is finished.
     * @param summaryData The summary data of the simulation.
     * @param trajectoryFile The trajectory file path of the simulation.
     */
    void simulationFinished(const Vector<std::pair<std::string, std::string>>& summaryData, const std::string& trajectoryFile);

    /**
     * @brief Signal emitted when the coordinates of trains are updated.
     * @param trainsStartEndPoints The start and end points of trains' coordinates.
     */
    void trainsCoordinatesUpdated(Vector<std::pair<std::string, Vector<std::pair<double,double>>>> trainsStartEndPoints);

    /**
     * @brief Signal emitted when the simulation progress is updated.
     * @param progressPercentage The progress percentage of the simulation.
     */
    void simulaionProgressUpdated(int progressPercentage);

    /**
     * @brief Signal emitted when an error occurs during the simulation.
     * @param error The error message.
     */
    void errorOccurred(std::string error);

public slots:
    /**
     * @brief Slot called when the progress is updated.
     * @param progressPercentage The progress percentage of the simulation.
     */
    void onProgressUpdated(int progressPercentage);

    /**
     * @brief Slot called when the coordinates of trains are updated.
     * @param trainsStartEndPoints The start and end points of trains' coordinates.
     */
    void onTrainsCoordinatesUpdated(Vector<std::pair<std::string, Vector<std::pair<double,double>>>> trainsStartEndPoints);

    /**
     * @brief Slot called when the simulation is finished.
     * @param summaryData The summary data of the simulation.
     * @param trajectoryFile The trajectory file path of the simulation.
     */
    void onSimulationFinished(const Vector<std::pair<std::string, std::string>> &summaryData, const string &trajectoryFile);

    /**
     * @brief Slot called to start the simulation work.
     */
    void doWork();

private:
    Simulator* sim; /**< Pointer to the Simulator object for performing the simulation. */
    Network* net; /**< Pointer to the Network object used in the simulation. */
};

#endif // SIMULATIONWORKER_H
