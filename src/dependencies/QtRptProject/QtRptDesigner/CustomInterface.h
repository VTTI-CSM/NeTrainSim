#pragma once

#include <QString>
#include <QObject>
#include <QDomDocument>

class CustomInterface
{
public:
    virtual ~CustomInterface() {}
    virtual bool execute(QSharedPointer<QDomDocument> xmlDoc) = 0;
    virtual void saveData(QSharedPointer<QDomDocument> xmlDoc) = 0;
	virtual void loadData(QSharedPointer<QDomDocument> xmlDoc) = 0;
    virtual void showReport(QSharedPointer<QDomDocument> xmlDoc) = 0;
    virtual void clear(QSharedPointer<QDomDocument> xmlDoc) = 0;
};


Q_DECLARE_INTERFACE(CustomInterface, "qtrpt.project.CustomInterface")

