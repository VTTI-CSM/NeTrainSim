#include "gui/netrainsimmainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
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
}
