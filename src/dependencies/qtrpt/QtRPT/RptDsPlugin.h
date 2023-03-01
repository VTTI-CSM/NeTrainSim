#pragma once

#include <QObject>
#include <QLibrary>
#include "RptDsAbstract.h"

class RptDsPlugin : public RptDsAbstract
{
    Q_OBJECT
public:
    RptDsPlugin(QObject *parent = nullptr);
    ~RptDsPlugin() override;
    virtual void loadXML(QDomElement dsElement) override;
    virtual QString getFieldValue(QString fieldName, int recNo) override;
    virtual int getRecordCount() override;

private:
    void setDataStr(QString str);
    void setDataFile(QString fileName);
    QObject *m_pluginObject;
    QLibrary m_lib;
    QString m_cdata;
    QString m_dsAliasName;

};


