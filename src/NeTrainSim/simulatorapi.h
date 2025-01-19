#ifndef SIMULATORAPI_H
#define SIMULATORAPI_H

#include "export.h"
#include <QObject>
#include <any>
#include <memory>
#include "network/network.h"
#include "simulator.h"
#include "simulatorworker.h"
#include "traindefinition/trainscommon.h"
#include "util/vector.h"
#include "util/map.h"

// Custom numpunct facet to disable thousand separator
class NoThousandsSeparator : public std::numpunct<char> {
protected:
    char do_thousands_sep() const override {
        return '\0'; // No separator
    }

    std::string do_grouping() const override {
        return ""; // No grouping
    }
};

struct NETRAINSIMCORE_EXPORT APIData {
    Network * network = nullptr;
    SimulatorWorker* simulatorWorker = nullptr;
    Simulator * simulator = nullptr;
    QThread *workerThread = nullptr;
    QMap<QString, std::shared_ptr<Train>> trains;
};

class NETRAINSIMCORE_EXPORT SimulatorAPI : public QObject
{
    Q_OBJECT
    friend struct std::default_delete<SimulatorAPI>; // Allow unique_ptr to delete
public:
    enum class Mode {
        Async,
        Sync
    };


signals:
    void simulationCreated(QString networkName);
    void simulationsInitialized(QVector<QString> networkNames);
    void simulationsPaused(QVector<QString> networkNames);
    void simulationsResumed(QVector<QString> networkNames);
    void simulationsTerminated(QVector<QString> networkNames);
    void simulationsFinished(QVector<QString> networkNames);
    void simulationProgressUpdated(QPair<QString,
                                         QPair<double, int>> simulatorProgress);
    void simulationAdvanced(QMap<QString, QPair<double, double>>
                                currentSimulorTimePairs);
    void simulationReachedReportingTime(QMap<QString, QPair<double, double>>
                                            currentSimulorTimeProgressPairs);
    void trainsReachedDestination(
        const QMap<QString, QVector<QJsonObject>>& trainNetworkPairs);
    void trainsCoordinatesUpdated(  // {NetworkName: [TrainID: [[x,y]] ] }
        const QMap<QString, QVector<
                               QPair<QString,QVector<
                                                  QPair<double,
                                                        double>>>>> trainsCoord);
    void trainAddedToSimulation(const QString networkName,
                                const QVector<QString> trainIDs);
    void simulationResultsAvailable(QMap<QString, TrainsResults>& results);
    void errorOccurred(QString error);
    void containersAddedToTrain(QString networkName, QString trainID);
    void trainReachedTerminal(QString networkName, QString trainID,
                              QString terminalID, QJsonArray containers);

protected:


    static SimulatorAPI& getInstance();
    static void resetInstance() ;

    ~SimulatorAPI();

    // Simulator control methods
    void createNewSimulationEnvironment(
        QString nodesFileContent,
        QString linksFileContent,
        QString networkName = "*",
        QVector<QMap<QString, std::any>> trainList =
            QVector<QMap<QString, std::any>>(),
        double timeStep = 1.0,
        Mode mode = Mode::Async);

    void createNewSimulationEnvironmentFromFiles(
        QString nodesFile,
        QString linksFile,
        QString networkName,
        QString trainsFile,
        double timeStep = 1.0,
        Mode mode = Mode::Async);

    void createNewSimulationEnvironment(QString networkName,
                                        QVector<QMap<QString, QString>> &nodeRecords,
                                        QVector<QMap<QString, QString>> &linkRecords,
                                        QVector<QMap<QString, any> > &trainList,
                                        double timeStep = 1.0,
                                        Mode mode = Mode::Async);

    void requestSimulationCurrentResults(QVector<QString> networkNames);

    // Train control methods
    void addTrainToSimulation(QString networkName,
                              QVector<std::shared_ptr<Train>> trains);

    // GETTERS
    Simulator* getSimulator(QString networkName);
    Network* getNetwork(QString networkName);
    std::shared_ptr<Train> getTrainByID(QString networkName,
                                        QString& trainID);
    QVector<std::shared_ptr<Train>> getAllTrains(QString networkName);
    void addContainersToTrain(QString networkName,
                              QString trainID, QJsonObject json);

    QVector<std::shared_ptr<Train> > getTrains(QString networkName);


private:
    static std::unique_ptr<SimulatorAPI> instance;
    static void registerQMeta();
    static void setLocale();

protected slots:
    void handleTrainReachedDestination(QString networkName,
                                       QJsonObject trainState,
                                       Mode mode);
    void handleTrainCoordsUpdate(
        QString networkName,
        Vector<std::pair<std::string,
                         Vector<std::pair<double,
                                          double>>>> simulatorTrainsCoords,
        Mode mode);
    void handleOneTimeStepCompleted(QString networkName,
                                    double currentSimulatorTime,
                                    double currentSimulatorProgress,
                                    Mode mode);
    void handleReportingTimeReached(QString networkName,
                                    double currentSimulatorTime,
                                    double currentSimulatorProgress,
                                    Mode mode);
    void handleResultsAvailable(QString networkName,
                                TrainsResults result,
                                Mode mode);

    void emitTrainsReachedDestination();
    void emitSimulationResults(QMap<QString, TrainsResults> &results);
    // void handleSimulationResults(
    //     const QVector<SimulationResult>& results);

protected:
    SimulatorAPI() = default;
    SimulatorAPI(const SimulatorAPI&) = delete;
    SimulatorAPI& operator=(const SimulatorAPI&) = delete;

    void setupConnections(const QString& networkName, Mode mode);

    // Network name and its related data
    QMap<QString, APIData> mData;

    int m_completedSummaryResults = 0;
    QMap<QString, TrainsResults> m_simulationResultBuffer;

    int m_completedSimulatorsTimeStep = 0;  // Track the number of completed simulators
    QMap<QString, QPair<double, double>> m_timeStepData;
    QMap<QString, QPair<double, double>> m_ReportingTimeData;
    QMap<QString, QVector<QJsonObject>> m_trainsReachedBuffer;
    QMap<QString, QVector<QPair<QString, QVector<QPair<double, double>>>>> m_trainsCoordsUpdated;

    QVector<QString> m_totalSimulatorsRunRequested;
    int m_completedSimulatorsRun = 0;
    QVector<QString> m_totalSimulatorsInitializationRequested;
    int m_completedSimulatorsInitialization = 0;
    QVector<QString> m_totalSimulatorPauseRequested;
    int m_completedSimulatorsPaused = 0;
    QVector<QString> m_totalSimulatorResumeRequested;
    int m_completedSimulatorsResumed = 0;
    QVector<QString> m_totalSimulatorTerminatedRequested;
    int m_completedSimulatorsTerminated = 0;
    QVector<QString> m_totalSimulatorFinishedRequested;
    int m_completedSimulatorsFinished = 0;
    int m_processedTrainCoordsUpdate = 0;

    void checkAndEmitSignal(int& counter,
                            int total,
                            const QVector<QString>& networkNames,
                            void(SimulatorAPI::*signal)(QVector<QString>),
                            Mode mode);

    void setupSimulator(QString &networkName,
                        QVector<QMap<QString, QString>> &nodeRecords,
                        QVector<QMap<QString, QString>> &linkRecords,
                        QVector<QMap<QString, std::any>> &trainList,
                        double &timeStep, Mode mode);



public:

    static Mode mMode;

    class NETRAINSIMCORE_EXPORT InteractiveMode {
    public:
        static SimulatorAPI& getInstance();
        static void resetAPI();
        // Simulator control methods
        static void createNewSimulationEnvironment(
            QString nodesFileContent,
            QString linksFileContent,
            QString networkName = "*",
            QVector<QMap<QString, std::any>> trainsData =
            QVector<QMap<QString, std::any>>(),
            double timeStep = 1.0,
            Mode mode = Mode::Async);
        static void createNewSimulationEnvironment(
            QString networkName,
            QVector<QMap<QString, QString>> &nodeRecords,
            QVector<QMap<QString, QString>> &linkRecords,
            QVector<QMap<QString, any> > &trainList,
            double timeStep = 1.0,
            Mode mode = Mode::Async);
        static void createNewSimulationEnvironmentFromFiles(
            QString nodesFile,
            QString linksFile,
            QString networkName,
            QString trainsFile,
            double timeStep = 1.0,
            Mode mode = Mode::Async);

        static void addTrainToSimulation(QString networkName,
                                         QVector<std::shared_ptr<Train>> trains);
        static void initSimulation(QVector<QString> networkNames);
        static void runOneTimeStep(QVector<QString> networkNames);
        static void runSimulation(QVector<QString> networkNames,
                                  double timeSteps);
        static void endSimulation(QVector<QString> networkNames);
        static Simulator* getSimulator(QString networkName);
        static Network* getNetwork(QString networkName);
        static std::shared_ptr<Train> getTrainByID(
            QString networkName, QString& trainID);
        static QVector<std::shared_ptr<Train>>
        getAllTrains(QString networkName);
        static void addContainersToTrain(QString networkName,
                                         QString trainID, QJsonObject json);
        static QVector<std::shared_ptr<Train> > getTrains(QString networkName);
    };

    class NETRAINSIMCORE_EXPORT ContinuousMode {
    public:


        static SimulatorAPI& getInstance();
        static void resetAPI();
        // Simulator control methods
        static void createNewSimulationEnvironment(
            QString nodesFileContent,
            QString linksFileContent,
            QString networkName = "*",
            QVector<QMap<QString, std::any>> trainList =
            QVector<QMap<QString, std::any>>(),
            double timeStep = 1.0,
            Mode mode = Mode::Async);
        static void createNewSimulationEnvironment(QString networkName,
                                                   QVector<QMap<QString, QString>> &nodeRecords,
                                                   QVector<QMap<QString, QString>> &linkRecords,
                                                   QVector<QMap<QString, any> > &trainList,
                                                   double timeStep = 1.0,
                                                   Mode mode = Mode::Async);
        static void createNewSimulationEnvironmentFromFiles(QString nodesFile,
                                                            QString linksFile,
                                                            QString networkName,
                                                            QString trainsFile,
                                                            double timeStep = 1.0,
                                                            Mode mode = Mode::Async);

        static void runSimulation(QVector<QString> networkNames);
        static void pauseSimulation(QVector<QString> networkNames);
        static void resumeSimulation(QVector<QString> networkNames);
        static void endSimulation(QVector<QString> networkNames);
        static Simulator* getSimulator(QString networkName);
        static Network* getNetwork(QString networkName);
        static void addContainersToTrain(QString networkName,
                                         QString trainID, QJsonObject json);
        static QVector<std::shared_ptr<Train> > getTrains(QString networkName);
    };
};

Q_DECLARE_METATYPE(APIData)


#endif // SIMULATORAPI_H
