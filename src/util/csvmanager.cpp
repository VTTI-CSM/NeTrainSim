#include "CSVManager.h"
#include <QFile>
#include <QTextStream>
#include <iostream>
#include <stdexcept>
#include <QSet>
#include "src/util/Error.h"

CSVManager::CSVManager(QObject* parent)
    : QObject(parent) {}

QVector<QVector<QString>> CSVManager::readCSV(const QString& filename, const QString& delimiter, bool firstRowHeader) {
    QFile file(filename);
    data.clear();
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("Error: " + std::to_string(static_cast<int>(Error::CouldNotOpenFile)) +
                                 "\nFailed to open file: " + filename.toStdString());
    }

    QTextStream in(&file);
    bool isFirstRow = true; // Flag to skip the first row

    while (!in.atEnd()) {
        QString line = in.readLine();

        if (isFirstRow && firstRowHeader) {
            isFirstRow = false;
            continue; // Skip the first row
        }

        QVector<QString> row = line.split(delimiter);
        data.append(row);
    }

    file.close();
    emit dataReady(data);
    return data;
}

QVector<QVector<QString>> CSVManager::filterByColumn(int column, const QString& value) const {
    QVector<QVector<QString>> filteredData;

    if (data.empty()) {return filteredData; }

    for (const auto& row : data) {
        if (column < row.size() && row[column] == value) {
            filteredData.append(row);
        }
    }

    return filteredData;
}

QVector<QVector<QString>> CSVManager::filterByColumn(const QVector<QVector<QString>>& data, int column, const QString& value) const {
    QVector<QVector<QString>> filteredData;

    if (data.size() < 1) {return filteredData; }

    for (const auto& row : data) {
        if (data.size() < 1) {continue; }

        if (column < row.size() && row[column] == value) {
            filteredData.append(row);
        }
    }

    return filteredData;
}


QStringList CSVManager::getDistinctColumnValues(int column) const {
    QSet<QString> distinctValues;
    if (data.size() < 1) {
        return distinctValues.values();
    }

    for (const auto& row : data) {
        if (column < row.size()) {
            distinctValues.insert(row[column]);
        }
    }

    QStringList sortedValues = distinctValues.values();
    std::sort(sortedValues.begin(), sortedValues.end()); // Sort the values

    return sortedValues;
}

QVector<QString> CSVManager::getColumnValues(const QVector<QVector<QString>> &data, int column) const {
    QVector<QString> columnValues;

    if (data.size() < 1) {return columnValues; }

    for (const auto& row : data) {
        if (column < row.size()) {
            columnValues.append(row[column]);
        }
    }

    return columnValues;
}
