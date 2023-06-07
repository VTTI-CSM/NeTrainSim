#ifndef CSVMANAGER_H
#define CSVMANAGER_H

#include <QObject>
#include <QString>
#include <QVector>

class CSVManager : public QObject {
    Q_OBJECT
public:
    /**
     * @file CSVManager.h
     * @brief This file declares the CSVManager class for reading and manipulating CSV files.
     *        The CSVManager class provides functions for reading a CSV file, filtering data
     *        based on column values, retrieving distinct column values, and extracting column
     *        values from a data set. It inherits from QObject and emits a signal when the data
     *        is ready.
     *        Note: The implementation of the class is not provided in this declaration file.
     *              It should be implemented separately in a corresponding source file.
     */

    explicit CSVManager(QObject* parent = nullptr);

    /**
     * Reads a CSV file and returns the data as a 2D QVector of QString values.
     *
     * @param filename The filename of the CSV file to read.
     * @param delimiter The delimiter used in the CSV file.
     * @param firstRowHeader Flag indicating whether the first row is a header row.
     * @returns A 2D QVector containing the data from the CSV file.
     */
    QVector<QVector<QString>> readCSV(const QString& filename, const QString& delimiter, bool firstRowHeader = true);

    /**
     * Filters the data based on a specific column and value.
     *
     * @param column The index of the column to filter by.
     * @param value The value to filter for in the specified column.
     * @returns A filtered 2D QVector containing the rows that match the filter.
     */
    QVector<QVector<QString>> filterByColumn(int column, const QString& value) const;

    /**
     * Filters the provided data based on a specific column and value.
     *
     * @param data The data to filter.
     * @param column The index of the column to filter by.
     * @param value The value to filter for in the specified column.
     * @returns A filtered 2D QVector containing the rows that match the filter.
     */
    QVector<QVector<QString>> filterByColumn(const QVector<QVector<QString>>& data, int column, const QString& value) const;

    /**
     * Retrieves the distinct values in a specific column.
     *
     * @param column The index of the column to retrieve distinct values from.
     * @returns A QStringList containing the distinct values in the specified column.
     */
    QStringList getDistinctColumnValues(int column) const;

    /**
     * Retrieves the values in a specific column from the provided data.
     *
     * @param data The data to extract column values from.
     * @param column The index of the column to extract values from.
     * @returns A QVector containing the values in the specified column.
     */
    QVector<QString> getColumnValues(const QVector<QVector<QString>>& data, int column) const;

signals:
    /**
     * Signal emitted when the data is ready.
     *
     * @param data The data as a QVector of QVector<QString>.
     */
    void dataReady(const QVector<QVector<QString>>& data);

private:
    QVector<QVector<QString>> data;
};

#endif // CSVMANAGER_H
