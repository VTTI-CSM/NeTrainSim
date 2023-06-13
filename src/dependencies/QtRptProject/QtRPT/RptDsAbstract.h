#ifndef RPTDSABSTRACT_H
#define RPTDSABSTRACT_H

#include <QObject>
#include <QDomElement>

class RptDsAbstract : public QObject
{
    Q_OBJECT
public:
    explicit RptDsAbstract(QObject *parent = nullptr);
    virtual int getRecordCount();
    virtual void loadXML(QDomElement dsElement) = 0;
    virtual QString getFieldValue(QString fieldName, int recNo) = 0;
    QString DSName();

protected:
    QString m_dsName;
    int m_recCount;
    int m_colCount;
    QStringList m_columns;
    QList<QStringList> m_data;

signals:

public slots:
};

#endif // RPTDSABSTRACT_H
