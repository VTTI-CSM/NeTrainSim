#ifndef CSVMANAGER_H
#define CSVMANAGER_H

#include <QObject>
#include <QString>
#include <QVector>

class CSVManager : public QObject {
    Q_OBJECT
public:
    explicit CSVManager(QObject* parent = nullptr);

    QVector<QVector<QString>> readCSV(const QString& filename, const QString& delimiter, bool firstRowHeader = true);
    QVector<QVector<QString>> filterByColumn(int column, const QString& value) const;
    QVector<QVector<QString>> filterByColumn(const QVector<QVector<QString>>& data, int column, const QString& value) const;
    QStringList getDistinctColumnValues(int column) const;
    QVector<QString> getColumnValues(const QVector<QVector<QString>>& data, int column) const;

signals:
    void dataReady(const QVector<QVector<QString>>& data);

private:
    QVector<QVector<QString>> data;
};

#endif // CSVMANAGER_H
