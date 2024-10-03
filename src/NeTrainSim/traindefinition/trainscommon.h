#ifndef TRAINSCOMMON_H
#define TRAINSCOMMON_H

#include "../export.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

struct NETRAINSIMCORE_EXPORT TrainNetworkDefinition {
    QString networkName;
    QString nodesFile;
    QString linksFile;
    QString nodesFileContent;
    QString linksFileContent;

    TrainNetworkDefinition() :
        networkName(""), nodesFile(""), linksFile("") {};

    // Constructor that takes the network name, nodes file, and links file paths
    TrainNetworkDefinition(const QString& networkGivenName,
                           const QString& nodesFilePath,
                           const QString& linksFilePath)
        : networkName(networkGivenName),
        nodesFile(nodesFilePath),
        linksFile(linksFilePath) {

        // Load the content of the nodes file
        nodesFileContent = readFileContent(nodesFile);
        if (nodesFileContent.isEmpty()) {
            qWarning() << "Error: Failed to load nodes file "
                          "content for network:" << networkName;
        }

        // Load the content of the links file
        linksFileContent = readFileContent(linksFile);
        if (linksFileContent.isEmpty()) {
            qWarning() << "Error: Failed to load links file "
                          "content for network:" << networkName;
        }
    }

private:

    // Helper function to read file content
    QString readFileContent(const QString& filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Error: Cannot open file:" << filePath;
            return QString(); // Return an empty QString in case of error
        }

        QTextStream in(&file);
        return in.readAll(); // Return the full file content as QString
    }

    // Serialization operator (<<)
    friend QDataStream& operator<<(
        QDataStream& out,
        const TrainNetworkDefinition& network) {
        out << network.networkName
            << network.nodesFile
            << network.linksFile
            << network.nodesFileContent
            << network.linksFileContent;
        return out;
    }

    // Deserialization operator (>>)
    friend QDataStream& operator>>(
        QDataStream& in,
        TrainNetworkDefinition& network) {
        in >> network.networkName
            >> network.nodesFile
            >> network.linksFile
            >> network.nodesFileContent
            >> network.linksFileContent;
        return in;
    }
};
Q_DECLARE_METATYPE(TrainNetworkDefinition)



// ----------------------------------------------------------------------------

struct NETRAINSIMCORE_EXPORT TrainsResults {
    // Constants
    static const qint64 MAX_TRAJECTORY_SIZE = 1024 * 1024; // 1 MB in bytes

    QVector<QPair<QString, QString>> summaryData;  // Stores the summary
                                                   // data (key-value pairs)
    QByteArray trajectoryFileData;// Stores the content of the trajectory file
    QString trajectoryFileName;  // Stores the full path of the trajectory file
    QString summaryFileName;     // Stores the full path of the summary file

    // Default constructor
    TrainsResults() :
        summaryData(), trajectoryFileData(),
        trajectoryFileName(), summaryFileName() {}

    // Constructor with summary and trajectory file paths
    TrainsResults(QVector<QPair<QString, QString>> summary,
                  QString trajectoryFilePath, QString summaryFilePath)
        : summaryData(summary), summaryFileName(summaryFilePath) {
        // Load the trajectory file content if the file path is valid
        if (!trajectoryFilePath.isEmpty() &&
            QFile::exists(trajectoryFilePath)) {
            trajectoryFileData.clear();
            // Store the entire trajectory file path
            trajectoryFileName = trajectoryFilePath;
        } else {
            trajectoryFileData.clear(); // Handle invalid file path case
            trajectoryFileName.clear();
            qWarning("Invalid trajectory file path or file does not exist.");
        }
    }

    // Function to load the trajectory file content into QByteArray
    void loadTrajectoryFile(QString filePath = QString()) {
        QString fPath;
        fPath = (filePath.isEmpty()) ? trajectoryFileName : filePath;
        QFile file(fPath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning("Cannot open trajectory file for reading.");
            return;
        }
        trajectoryFileData = file.readAll();
    }

    // Function to save the trajectory file content to an optional new path
    bool saveTrajectoryFile(const QString& newPath = QString()) const {
        QString savePath = newPath.isEmpty() ? trajectoryFileName : newPath;

        if (savePath.isEmpty() || trajectoryFileData.isEmpty()) {
            qWarning("No valid trajectory file data to save.");
            return false;
        }

        QFile file(savePath);
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning("Cannot open trajectory file for writing.");
            return false;
        }
        file.write(trajectoryFileData);
        return true;
    }

    // Function to save the summary data to an optional new path
    bool saveSummaryFile(const QString& newPath = QString()) const {
        QString savePath = newPath.isEmpty() ? summaryFileName : newPath;

        if (savePath.isEmpty() || summaryData.isEmpty()) {
            qWarning("No valid summary data to save.");
            return false;
        }

        QFile file(savePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning("Cannot open summary file for writing.");
            return false;
        }

        QTextStream out(&file);
        for (const auto& pair : summaryData) {
            // Write each key-value pair as a line
            out << pair.first << ": " << pair.second << "\n";
        }

        return true;
    }

    // Function to get only the filename from the
    // full path of the trajectory file
    QString getTrajectoryFileName() const {
        QFileInfo fileInfo(trajectoryFileName);
        // Returns only the filename without the path
        return fileInfo.fileName();
    }

    // Function to get only the filename from the
    // full path of the summary file
    QString getSummaryFileName() const {
        QFileInfo fileInfo(summaryFileName);
        // Returns only the filename without the path
        return fileInfo.fileName();
    }

    // New function to convert TrainsResults to JSON
    QJsonObject toJson() const {
        QJsonObject json;

        // Convert summaryData to JSON array
        QJsonArray summaryArray;
        for (const auto& pair : summaryData) {
            QJsonObject pairObject;
            pairObject[pair.first] = pair.second;
            summaryArray.append(pairObject);
        }
        json["summaryData"] = summaryArray;

        // Check size of trajectoryFileData
        if (trajectoryFileData.size() <= MAX_TRAJECTORY_SIZE) {
            // Convert trajectoryFileData to base64 string
            json["trajectoryFileData"] = QString(trajectoryFileData.toBase64());
            json["trajectoryFileDataIncluded"] = true;
        } else {
            json["trajectoryFileDataIncluded"] = false;
        }

        json["trajectoryFileName"] = trajectoryFileName;
        json["summaryFileName"] = summaryFileName;

        return json;
    }

    // New function to convert JSON to TrainsResults
    static TrainsResults fromJson(const QJsonObject& json) {
        TrainsResults results;

        // Parse summaryData
        QJsonArray summaryArray = json["summaryData"].toArray();
        for (const auto& value : summaryArray) {
            QJsonObject pairObject = value.toObject();
            QString key = pairObject.keys().first();
            results.summaryData.append(qMakePair(key, pairObject[key].toString()));
        }

        // Parse trajectoryFileData if included
        if (json["trajectoryFileDataIncluded"].toBool(false)) {
            QByteArray base64Data = json["trajectoryFileData"].toString().toUtf8();
            results.trajectoryFileData = QByteArray::fromBase64(base64Data);
        } else {
            results.trajectoryFileData.clear();
        }

        results.trajectoryFileName = json["trajectoryFileName"].toString();
        results.summaryFileName = json["summaryFileName"].toString();

        return results;
    }

    // Function to get JSON string representation
    QString toJsonString() const {
        QJsonDocument doc(toJson());
        return QString(doc.toJson(QJsonDocument::Compact));
    }

    // Serialization operator (<<)
    friend QDataStream &operator<<(QDataStream &out,
                                   const TrainsResults &results) {
        out << results.summaryData << results.trajectoryFileData
            << results.trajectoryFileName << results.summaryFileName;
        return out;
    }

    // Deserialization operator (>>)
    friend QDataStream &operator>>(QDataStream &in,
                                   TrainsResults &results) {
        in >> results.summaryData >> results.trajectoryFileData
            >> results.trajectoryFileName >> results.summaryFileName;
        return in;
    }
};
Q_DECLARE_METATYPE(TrainsResults)


#endif // TRAINSCOMMON_H
