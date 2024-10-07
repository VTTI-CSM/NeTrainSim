#ifndef SIMULATORAPI_H
#define SIMULATORAPI_H

#include "export.h"
#include <QObject>
#include <any>
#include <memory>
#include "network/network.h"
#include "simulator.h"
#include "traindefinition/trainscommon.h"
#include "util/vector.h"
#include "util/map.h"

class NETRAINSIMCORE_EXPORT SimulatorAPI : public QObject
{
    Q_OBJECT

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
    void simulationsEnded(QVector<QString> networkNames);
    void simulationAdvanced(QMap<QString, double>
                                currentSimulorTimePairs);
    void trainsReachedDestination(
        const QMap<QString, QVector<QString>>& trainNetworkPairs);
    void simulationResultsAvailable(QMap<QString, TrainsResults>& results);
    void errorOccurred(QString error);

protected:
    struct APIData {
        Network * network = nullptr;
        Simulator * simulator = nullptr;
        QThread *workerThread = nullptr;
        QMap<std::string, std::shared_ptr<Train>> trains;
    };

    static SimulatorAPI& getInstance() {
        static SimulatorAPI instance;
        return instance;
    }

    ~SimulatorAPI();

    // Simulator control methods
    void createNewSimulationEnvironment(
        QString nodesFileContent,
        QString linksFileContent,
        QString networkName = "*",
        QVector<std::shared_ptr<Train>> trainList =
            QVector<std::shared_ptr<Train>>(),
        double timeStep = 1.0,
        Mode mode = Mode::Async);

    void requestSimulationCurrentResults(QVector<QString> networkNames);

    // Train control methods
    void addTrainToSimulation(QString networkName,
                              QMap<QString, QVariant> trainRecords);

    // GETTERS
    Simulator* getSimulator(QString networkName);
    Network* getNetwork(QString networkName);
    std::shared_ptr<Train> getTrainByID(QString networkName,
                                        QString& trainID);
    QVector<std::shared_ptr<Train>> getAllTrains(QString networkName);





protected slots:
    void handleTrainReachedDestination(QString networkName,
                                       QString trainID,
                                       Mode mode);
    void handleOneTimeStepCompleted(QString networkName,
                                    double currentSimulatorTime,
                                    Mode mode);
    void handleResultsAvailable(QString networkName,
                                TrainsResults result,
                                Mode mode);

    void emitTrainsReachedDestination();
    void emitSimulationResults(QMap<QString, TrainsResults> results);
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
    QMap<QString, double> m_timeStepData;
    QMap<QString, QVector<QString>> m_trainsReachedBuffer;

    QVector<QString> m_totalSimulatorsRunRequested;
    int m_completedSimulatorsRun = 0;
    QVector<QString> m_totalSimulatorsInitializationRequested;
    int m_completedSimulatorsInitialization = 0;
    QVector<QString> m_totalSimulatorPauseRequested;
    int m_completedSimulatorsPaused = 0;
    QVector<QString> m_totalSimulatorResumeRequested;
    int m_completedSimulatorsResumed = 0;
    QVector<QString> m_totalSimulatorEndRequested;
    int m_completedSimulatorsEnded = 0;

    void checkAndEmitSignal(int& counter,
                            int total,
                            const QVector<QString>& networkNames,
                            void(SimulatorAPI::*signal)(QVector<QString>),
                            Mode mode);

    void setupSimulator(QString &networkName,
                        Vector<Map<std::string, std::string>> &nodeRecords,
                        Vector<Map<std::string, std::string>> &linkRecords,
                        QVector<std::shared_ptr<Train>> &trainList,
                        double &timeStep, Mode mode);

    std::any convertQVariantToAny(const QVariant& variant);
    std::map<std::string, std::any>
    convertQMapToStdMap(const QMap<QString, QVariant>& qMap);

public:

    static Mode mMode;

    class NETRAINSIMCORE_EXPORT InteractiveMode {
    public:
        // Simulator control methods
        static void createNewSimulationEnvironment(
            QString nodesFileContent,
            QString linksFileContent,
            QString networkName = "*",
            QVector<std::shared_ptr<Train>> trainList =
            QVector<std::shared_ptr<Train>>(),
            double timeStep = 1.0,
            Mode mode = Mode::Async);

        static void addTrainToSimulation(QString networkName,
                                         QMap<QString, QVariant> trainRecords);
        static void initSimulation(QVector<QString> networkNames);
        static void runOneTimeStep(QVector<QString> networkNames);
        static Simulator* getSimulator(QString networkName);
        static Network* getNetwork(QString networkName);
        static std::shared_ptr<Train> getTrainByID(
            QString networkName, QString& trainID);
        static QVector<std::shared_ptr<Train>>
        getAllTrains(QString networkName);
    };

    class NETRAINSIMCORE_EXPORT ContinuousMode {
    public:


        static SimulatorAPI& getInstance();
        // Simulator control methods
        static void createNewSimulationEnvironment(
            QString nodesFileContent,
            QString linksFileContent,
            QString networkName = "*",
            QVector<std::shared_ptr<Train>> trainList =
            QVector<std::shared_ptr<Train>>(),
            double timeStep = 1.0,
            Mode mode = Mode::Async);

        static void runSimulation(QVector<QString> networkNames);
        static void pauseSimulation(QVector<QString> networkNames);
        static void resumeSimulation(QVector<QString> networkNames);
        static void endSimulation(QVector<QString> networkNames);
        static Simulator* getSimulator(QString networkName);
        static Network* getNetwork(QString networkName);
    };
};

#endif // SIMULATORAPI_H