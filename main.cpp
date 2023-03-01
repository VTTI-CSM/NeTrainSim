/**
 * @file	C:\Users\Ahmed\OneDrive - Virginia
 * 			Tech\03.Work\02.VTTI\02.ResearchWork\01.TrainModelling\02.Code\00.CPP\NeTrainSim\NeTrainSim\main.cpp.
 *
 * Implements the main class
 */
#include "netrainsim.h"

#ifdef AS_CMD
    #include "src/trainDefintion/Train.h"
    #include "src/trainDefintion/TrainsList.h"
    #include "src/network/Network.h"
    #include "src/Simulator.h"
    #include "src/util/Vector.h"
#endif

#ifndef AS_CMD
    #include <QApplication>
    #include <QLocale>
    #include <QTranslator>
#endif

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
    // -----------------------------------
    // command line application
    // -----------------------------------
#ifdef AS_CMD
    std::string NodesFile, LinksFile, trainsFile;

    if (argc == 4) {
        NodesFile = argv[1];
        LinksFile = argv[2];
        trainsFile = argv[3];
    }
    else {
        std::cerr << "Provided number of arguments is not correct!" << std::endl;
        return 1;
    }

    Vector<std::shared_ptr<Train>> trains = TrainsList::readTrainsFile(trainsFile);
    Network net = Network(NodesFile, LinksFile);
    Simulator sim = Simulator(net, trains);
    return 0;
#endif


    // -----------------------------------
    // GUI application
    // -----------------------------------
#ifndef AS_CMD
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "NeTrainSim_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    NeTrainSim w;
    w.show();
    return a.exec();
#endif
}
