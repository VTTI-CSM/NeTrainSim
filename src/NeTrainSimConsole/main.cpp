#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include "traindefinition/train.h"
#include "traindefinition/trainslist.h"
#include "network/network.h"
#include "simulator.h"
#include "util/vector.h"
#include <iostream>
#include <sstream>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <stdio.h>
#include <filesystem>
#include "qdir.h"
#include "simulatorapi.h"
#include "errorhandler.h"
#include "VersionConfig.h"

const std::string compilation_date = __DATE__;
const std::string compilation_time = __TIME__;
/**
 * @brief checkParserValue
 * @param parser
 * @param option
 * @return
 */
bool checkParserValue(QCommandLineParser& parser, const QCommandLineOption &option, std::string s, bool isRequired = true){
    if(parser.isSet(option)) {
        return true;
    }
    if (isRequired){
        fputs(qPrintable(QString::fromStdString(s)), stdout);
        fputs("\n\n", stdout);
        fputs(qPrintable(parser.helpText()), stdout);
    }
    return false;
}

/**
 * Main entry-point for this application
 *
 * @author	Ahmed Aredah
 * @date	2/28/2023
 *
 * @param 	argc	The number of command-line arguments provided.
 * @param 	argv	An array of command-line argument strings.
 *
 * @returns	Exit-code for the process - 0 for success, else an error code.
 */
int main(int argc, char *argv[])
{
    // ####################################################
    // #                     values                       #
    // ####################################################
    std::string GithubLink = "https://github.com/VTTI-CSM/NeTrainSim";

    QCoreApplication app(argc, argv);
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "NeTrainSim_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }
    QCoreApplication::setApplicationName(NeTrainSim_NAME);
    QCoreApplication::setApplicationVersion(NeTrainSim_VERSION);
    QCoreApplication::setOrganizationName(NeTrainSim_VENDOR);

    QCommandLineParser parser;
    parser.setApplicationDescription("Open-source network train simulator");
    QCommandLineOption helpOption(QStringList() << "h" << "help" << "?",
                                  "Display this help message.");
    parser.addOption(helpOption);
    //parser.addHelpOption();
    parser.addVersionOption();


    const QCommandLineOption nodesOption(QStringList() << "n" << "nodes",
                                         QCoreApplication::translate("main", "[Required] the nodes filename."), "nodesFile", "");
    parser.addOption(nodesOption);

    const QCommandLineOption linksOption(QStringList() << "l" << "links",
                                         QCoreApplication::translate("main", "[Required] the links filename."), "linksFile", "");
    parser.addOption(linksOption);

    const QCommandLineOption trainsOption(QStringList() << "t" << "trains",
                                          QCoreApplication::translate("main", "[Required] the trains filename."), "trainsFile", "");
    parser.addOption(trainsOption);

    const QCommandLineOption outputLocationOption(QStringList() << "o" << "output",
                                                  QCoreApplication::translate("main", "[Optional] the output folder address. \nDefault is 'C:\\Users\\<USERNAME>\\Documents\\NeTrainSim\\'."), "outputLocation", "");
    parser.addOption(outputLocationOption);

    const QCommandLineOption summaryFilenameOption(QStringList() << "s" << "summary",
                                                   QCoreApplication::translate("main", "[Optional] the summary filename. \nDefault is 'trainSummary_timeStamp.txt'."), "summaryFilename", "");
    parser.addOption(summaryFilenameOption);

    const QCommandLineOption summaryExportAllOption(QStringList() << "a" << "all",
                                                    QCoreApplication::translate("main", "[Optional] bool to show summary of all trains in the summary file. \nDefault is 'false'."), "summarizeAllTrains", "false");
    parser.addOption(summaryExportAllOption);

    const QCommandLineOption exportInstaTrajOption(QStringList() << "e" << "export",
                                                   QCoreApplication::translate("main", "[Optional] bool to export instantaneous trajectory. \nDefault is 'false'."), "exportTrajectoryOptions" ,"false");
    parser.addOption(exportInstaTrajOption);

    const QCommandLineOption instaTrajOption(QStringList() << "i" << "insta",
                                             QCoreApplication::translate("main", "[Optional] the instantaneous trajectory filename. \nDefault is 'trainTrajectory_timeStamp.csv'."), "instaTrajectoryFile", "");
    parser.addOption(instaTrajOption);

    const QCommandLineOption timeStepOption(QStringList() << "p" << "timeStep",
                                            QCoreApplication::translate("main", "[Optional] the simulator time step. \nDefault is '1.0'."), "simulatorTimeStep", "1.0");
    parser.addOption(timeStepOption);

    const QCommandLineOption enableOptimization(QStringList() << "z" << "optimization",
                                                QCoreApplication::translate("main", "[Optional] bool to enable single train trajectory optimization. \nDefault is 'false'"), "optimization", "false");
    parser.addOption(enableOptimization);

    const QCommandLineOption optimizationEvery(QStringList() << "y" << "optimizeEvery",
                                               QCoreApplication::translate("main", "[Optional] the optimization frequency. \nDefault is '1'."), "optimizeEvery", "1");
    parser.addOption(optimizationEvery);

    const QCommandLineOption optimizationLookahead(QStringList() << "d" << "optimizerLookahead",
                                                   QCoreApplication::translate("main", "[Optional] the forward lookahead distance for the optimizer. \nDefault is '1'."), "optimizerLookahead", "1");
    parser.addOption(optimizationLookahead);

    const QCommandLineOption optimizationSpeedPriorityFactor(QStringList() << "k" << "OptimizationSpeedFactor",
                                                             QCoreApplication::translate("main", "[Optional] the speed priority factor in case of optimization. \n Default is '0.0'."), "OptimizationSpeedFactor", "0.0");
    parser.addOption(optimizationSpeedPriorityFactor);

    // process all the arguments
    parser.process(app);

    // display the help if requested and exit
    if (parser.isSet(helpOption)) {
        parser.showHelp(0);
    }

    // show app details
    std::stringstream hellos;
    hellos << NeTrainSim_NAME << " [Version " << NeTrainSim_VERSION << ", "  << compilation_date << " " << compilation_time << " Build" <<  "]" << endl;
    hellos << NeTrainSim_VENDOR << endl;
    hellos << GithubLink << endl;
    std::cout << hellos.str() << "\n";




    std::string nodesFile, linksFile, trainsFile, exportLocation, summaryFilename, instaTrajFilename;
    bool exportInstaTraj = false;
    double timeStep = 1.0;
    bool optimize = false;
    double optimize_speedfactor = 0.0;
    int optimizerFrequency = 0;
    int lookahead = 0;

    // read values from the cmd
    // read required values

    if (checkParserValue(parser, nodesOption, "nodes file is missing!", true)) { nodesFile = parser.value(nodesOption).toStdString(); }
    else { return 1;}
    if (checkParserValue(parser, linksOption, "links file is missing!", true)) { linksFile = parser.value(linksOption).toStdString(); }
    else { return 1;}
    if (checkParserValue(parser, trainsOption, "trains file is missing!", true)) { trainsFile = parser.value(trainsOption).toStdString(); }
    else { return 1;}

    // read optional values
    if (checkParserValue(parser, outputLocationOption, "" ,false)){
        exportLocation = parser.value(outputLocationOption).toStdString();
        QDir directory(QString::fromStdString(exportLocation));
        // check if directory is valid
        if (!directory.exists()) {
            fputs(qPrintable("export directory is not valid!"), stdout);
            return 1;
        }
    }
    else { exportLocation = ""; }

    if (checkParserValue(parser, summaryFilenameOption, "", false)){ summaryFilename = parser.value(summaryFilenameOption).toStdString(); }
    else { summaryFilename = ""; }

    if (checkParserValue(parser, exportInstaTrajOption, "", false)){
        stringstream ss(parser.value(exportInstaTrajOption).toStdString());
        ss >> std::boolalpha >> exportInstaTraj;
    }
    else { exportInstaTraj = false; }

    if (checkParserValue(parser, instaTrajOption, "", false)){ instaTrajFilename = parser.value(instaTrajOption).toStdString(); }
    else { instaTrajFilename = ""; }

    if (checkParserValue(parser, timeStepOption, "", false)) { timeStep = parser.value(timeStepOption).toDouble(); }
    else { timeStep = 1.0; }

    if (checkParserValue(parser, enableOptimization, "", false)){
        stringstream ss(parser.value(enableOptimization).toStdString());
        ss >> std::boolalpha >> optimize;
    }
    else { optimize = false; }

    if (checkParserValue(parser, optimizationEvery, "", false)) { optimizerFrequency = parser.value(optimizationEvery).toInt(); }
    else { optimizerFrequency = 1.0; }
    if (checkParserValue(parser, optimizationLookahead, "", false)) { lookahead = parser.value(optimizationLookahead).toInt(); }
    else { lookahead = 1.0; }

    if (checkParserValue(parser, optimizationSpeedPriorityFactor, "", 0.0)) {optimize_speedfactor = parser.value(optimizationSpeedPriorityFactor).toDouble(); }
    else { optimize_speedfactor = 0.0;}

//    qDebug() << "Optimize?: " << optimize
//             << ", Optimizer Frequency: " << optimizerFrequency
//             << ", Lookahead: " << lookahead
//             << ", Optimize SpeedFactor: " << optimize_speedfactor
//             << "\n";

//    exit(0);


    QString networkName = "main network";
    Network* net;
    try {
        std::cout << "Reading Trains!                 \r";

        Vector<std::shared_ptr<Train>> trains =
            TrainsList::ReadAndGenerateTrains(trainsFile);

        QVector<std::shared_ptr<Train>> qtrains;
        qtrains.reserve(trains.size());  // Reserve space for efficiency
        for (const auto& item : trains) {
            qtrains.append(item);  // Append each element to the QVector
        }

        if (trains.empty()) {
            ErrorHandler::showError("No Trains Found!                    ");
            return -1;
        }

        SimulatorAPI::ContinuousMode::createNewSimulationEnvironmentFromFiles(
            QString::fromStdString(nodesFile),
            QString::fromStdString(linksFile),
            networkName, qtrains, timeStep);

        // net = SimulatorAPI::ContinuousMode::getNetwork(networkName);
        for (auto &t: trains) {
            QEventLoop::connect(t.get(), &Train::slowSpeedOrStopped,
                    [](const auto &msg){
                        ErrorHandler::showWarning(msg);});

            QEventLoop::connect(t.get(), &Train::suddenAccelerationOccurred,
                    [](const auto &msg){
                ErrorHandler::showWarning(msg);});

            t->setOptimization(optimize, optimize_speedfactor,
                               optimizerFrequency, lookahead);
        }
        Simulator* sim =
            SimulatorAPI::ContinuousMode::getSimulator(networkName);

        if (exportLocation != "" ) { sim->setOutputFolderLocation(exportLocation); }
        if (summaryFilename != "" ) { sim->setSummaryFilename(summaryFilename); }

        sim->setExportInstantaneousTrajectory(exportInstaTraj,
                                              instaTrajFilename);

        // run the actual simulation
        std::cout <<"Starting the Simulator!                                "
                     "              \n";
        sim->runSimulation();
        std::cout << "Output folder: " << sim->getOutputFolder() << std::endl;
    } catch (const std::exception& e) {
        ErrorHandler::showError(e.what());
        return 1;
    }
    return 0;
}
