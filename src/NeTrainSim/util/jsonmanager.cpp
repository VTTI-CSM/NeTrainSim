#include "jsonmanager.h"
#include "util/vector.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>


JSONManager::JSONManager(Vector<std::string> fileKeys,
                         Vector<std::string> fileCriticalKeys) : keys(fileKeys),
                                                                 criticalKeys(keys)
{

}



// Reads a JSON file and returns a vector<vector<string>>
Vector<Vector<std::string>> JSONManager::read(const std::string& filename) {
    Vector<Vector<std::string>> result;
    QFile file(QString::fromStdString(filename));
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("Failed to open file for reading.");
        return result;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray mainArray = doc.array();

    for (const auto& val : mainArray) {
        QJsonObject obj = val.toObject();
        Vector<std::string> subResult;

        for (const auto& key: keys) {
            QString theKey = QString::fromStdString(key);
            if (obj.contains(theKey)) {
                subResult.push_back(obj[theKey].toString().toStdString());
            }
            else {
                if (criticalKeys.exist(key)) {
                    throw std::runtime_error("Entry" + key + "is not found");
                }
                else {
                    subResult.push_back("");
                }
            }
        }

        result.push_back(subResult);
    }

    return result;
}

// Writes a vector<vector<string>> to a JSON file
void JSONManager::write(const std::string& filename,
                        const Vector<Vector<std::string>>& data) {
    QFile file(QString::fromStdString(filename));
    if (!file.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("Failed to open file for writing.");
        return;
    }

    QJsonArray mainArray;

    for (const auto& subVec : data) {
        QJsonObject obj;
        obj["key1"] = QString::fromStdString(subVec[0]);
        obj["key2"] = QString::fromStdString(subVec[1]);
        obj["key3"] = QString::fromStdString(subVec[2]);
        mainArray.append(obj);
    }

    QJsonDocument doc(mainArray);
    file.write(doc.toJson(QJsonDocument::Indented));
}
