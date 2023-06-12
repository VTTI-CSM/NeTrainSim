#include "xmlmanager.h"
#include <QDomDocument>
#include <QFile>
#include <QDebug>
#include "src/util/error.h"

namespace XMLManager {
    void createProjectFile(const QString& projectName, const QString& networkName, const QString& authorName,
                       const QString& nodesFileName, const QString& linksFileName,
                       const QString& trainsFileName, const QString &simEndTime,
                       const QString &simTimestep, const QString &simPlotTime,
                       const QString& filename) {
        // Create a QDomDocument object
        QDomDocument doc;

        // Create the root element
        QDomElement root = doc.createElement("Data");
        doc.appendChild(root);

        // Create project name element and set its text
        QDomElement projectElement = doc.createElement("ProjectName");
        QDomText projectText = doc.createTextNode(projectName);
        projectElement.appendChild(projectText);
        root.appendChild(projectElement);

        // Create network name element and set its text
        QDomElement networkElement = doc.createElement("NetworkName");
        QDomText networkText = doc.createTextNode(networkName);
        networkElement.appendChild(networkText);
        root.appendChild(networkElement);

        // Create author name element and set its text
        QDomElement authorElement = doc.createElement("AuthorName");
        QDomText authorText = doc.createTextNode(authorName);
        authorElement.appendChild(authorText);
        root.appendChild(authorElement);

        // Create nodes file name element and set its text
        QDomElement nodesElement = doc.createElement("NodesFileName");
        QDomText nodesText = doc.createTextNode(nodesFileName);
        nodesElement.appendChild(nodesText);
        root.appendChild(nodesElement);

        // Create links file name element and set its text
        QDomElement linksElement = doc.createElement("LinksFileName");
        QDomText linksText = doc.createTextNode(linksFileName);
        linksElement.appendChild(linksText);
        root.appendChild(linksElement);

        // Create trains file name element and set its text
        QDomElement trainsElement = doc.createElement("TrainsFileName");
        QDomText trainsText = doc.createTextNode(trainsFileName);
        trainsElement.appendChild(trainsText);
        root.appendChild(trainsElement);

        // Create simEndTime name element and set its text
        QDomElement ElementSimEndTime = doc.createElement("simEndTime");
        QDomText simEndTimeText = doc.createTextNode(simEndTime);
        ElementSimEndTime.appendChild(simEndTimeText);
        root.appendChild(ElementSimEndTime);

        // Create simTimestep name element and set its text
        QDomElement ElementsimTimestep = doc.createElement("simTimestep");
        QDomText simTimestepText = doc.createTextNode(simTimestep);
        ElementsimTimestep.appendChild(simTimestepText);
        root.appendChild(ElementsimTimestep);

        // Create simTimestep name element and set its text
        QDomElement ElementsimPlotTime = doc.createElement("simPlotTime");
        QDomText simPlotTimeText = doc.createTextNode(simPlotTime);
        ElementsimPlotTime.appendChild(simPlotTimeText);
        root.appendChild(ElementsimPlotTime);

        // Create a QFile object to write the XML file
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            throw std::runtime_error("Error: " + std::to_string(static_cast<int>(Error::CouldNotOpenFile)) +
                                     "\nFailed to open the file for writing.");
        }

        // Create a QTextStream object to write the XML content to the file
        QTextStream out(&file);
        out.setDevice(&file); // Set the device of the QTextStream
        out.setEncoding(QStringConverter::Utf8);
        doc.save(out, 4); // Save the XML document with indentation

        // Close the file
        file.close();
    }

    std::tuple<QString, QString, QString, QString, QString, QString, QString, QString, QString> readProjectFile(const QString& filename) {
        // Create a QDomDocument object
        QDomDocument doc;

        // Create a QFile object to read the XML file
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error("Error: " + std::to_string(static_cast<int>(Error::CouldNotOpenFile)) +
                                     "\nFailed to open the file for reading.");
        }

        // Set the content of the QDomDocument with the file content
        if (!doc.setContent(&file)) {
            file.close();
            throw std::runtime_error("Error: " + std::to_string(static_cast<int>(Error::CouldNotOpenFile)) +
                                     "\nFailed to parse the XML file.");
        }

        // Close the file
        file.close();

        // Get the root element
        QDomElement root = doc.documentElement();

        // Get the child elements
        QDomElement projectElement      = root.firstChildElement("ProjectName");
        QDomElement networkElement      = root.firstChildElement("NetworkName");
        QDomElement authorElement       = root.firstChildElement("AuthorName");
        QDomElement nodesElement        = root.firstChildElement("NodesFileName");
        QDomElement linksElement        = root.firstChildElement("LinksFileName");
        QDomElement trainsElement       = root.firstChildElement("TrainsFileName");
        QDomElement simEndTimeElement   = root.firstChildElement("simEndTime");
        QDomElement simTimestepElement  = root.firstChildElement("simTimestep");
        QDomElement simPlotTimeElement  = root.firstChildElement("simPlotTime");

        // Get the text values
        QString projectName       = projectElement.text();
        QString networkName       = networkElement.text();
        QString authorName        = authorElement.text();
        QString nodesFileName     = nodesElement.text();
        QString linksFileName     = linksElement.text();
        QString trainsFileName    = trainsElement.text();
        QString simEndTime        = simEndTimeElement.text();
        QString simTimestep       = simTimestepElement.text();
        QString simPlotTime       = simPlotTimeElement.text();

        // Return the extracted values as a tuple
        return std::make_tuple(projectName, networkName, authorName, nodesFileName, linksFileName,
                               trainsFileName, simEndTime, simTimestep, simPlotTime);
    }
}
