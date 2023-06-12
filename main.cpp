/**
 *
 * Implements the main class
 */

#include <QCommandLineOption>
#include <QCommandLineParser>
#ifdef AS_CMD
    #include "src/traindefinition/train.h"
    #include "src/traindefinition/trainslist.h"
    #include "src/network/network.h"
    #include "src/simulator.h"
    #include "src/util/vector.h"
    #include <sstream>
    #include <QCoreApplication>
    #include <QCommandLineParser>
    #include <stdio.h>
    #include <filesystem>
    #include "src/util/errorhandler.h"
#endif

#ifndef AS_CMD
    #include "netrainsim.h"
    #include <QApplication>
    #include <QLocale>
    #include <QTranslator>
#endif

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
int main(int argc, char *argv[]) {
    // -----------------------------------
    // command line application
    // -----------------------------------
#ifdef AS_CMD
// ####################################################
// #                     values                       #
// ####################################################
    std::string Institution =
            "(C) 2022-2023 Virginia Tech Transportation Institute - Center for Sustainable Mobility.";
    std::string GithubLink = " ";

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(MYAPP_TARGET);
    QCoreApplication::setApplicationVersion(MYAPP_VERSION );
    QCoreApplication::setOrganizationName(QString::fromStdString(Institution));

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


    // process all the arguments
    parser.process(app);

    // display the help if requested and exit
    if (parser.isSet(helpOption)) {
        parser.showHelp(0);
    }

    // show app details
    stringstream hellos;
    hellos << MYAPP_TARGET << " [Version " << MYAPP_VERSION << "]" << endl;
    hellos << Institution << endl;
    hellos << GithubLink << endl;
    std::cout << hellos.str() << "\n";




    std::string nodesFile, linksFile, trainsFile, exportLocation, summaryFilename, instaTrajFilename;
    bool exportInstaTraj = false;
    double timeStep = 1.0;

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

    Network* net;
    try {
        std::cout << "Reading Trains!                 \r";
        Vector<std::shared_ptr<Train>> trains = TrainsList::ReadAndGenerateTrains(trainsFile);
        std::cout << "Reading Network!                \r";
        net = new Network(nodesFile, linksFile);
        std::cout << "Define Simulator Space!         \r";
        Simulator sim = Simulator(net, trains, timeStep);

        if (exportLocation != "" ) { sim.setOutputFolderLocation(exportLocation); }
        if (summaryFilename != "" ) { sim.setSummaryFilename(summaryFilename); }

        sim.setExportInstantaneousTrajectory(exportInstaTraj, instaTrajFilename);

        // run the actual simulation
        std::cout <<"Starting the Simulator!          \n";
        sim.runSimulation();
        std::cout << "Output folder: " << sim.getOutputFolder() << std::endl;
    } catch (const std::exception& e) {
        ErrorHandler::showError(e.what());
        if (net) {
            delete net;
        }

        return 1;
    }
    if (net) {
        delete net;
    }
    return 0;
#endif


    // -----------------------------------
    // GUI application
    // -----------------------------------
#ifndef AS_CMD
    QApplication a(argc, argv);

    // Parse the command-line arguments
    QCommandLineParser parser;
    parser.addPositionalArgument("file", "NTS file to open");

    parser.process(a);

    const QStringList args = parser.positionalArguments();

    NeTrainSim w;

    // Check if an NTS file path was provided as a command-line argument
    if (!args.isEmpty()) {
        const QString filePath = args.first();

        // Open and process the NTS file
        QFile ntsFile(filePath);
        if (ntsFile.open(QIODevice::ReadOnly)) {
            // Process the NTS file as needed
            w.loadProjectFiles(ntsFile.fileName());
            ntsFile.close();
        } else {
            qDebug() << "Failed to open NTS file:" << filePath;
        }
    }

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "NeTrainSim_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    w.show();
    return a.exec();
#endif
}


