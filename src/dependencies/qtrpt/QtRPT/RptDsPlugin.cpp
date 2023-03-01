#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include "RptDsPlugin.h"

RptDsPlugin::RptDsPlugin(QObject *parent)
: RptDsAbstract(parent)
{
    m_pluginObject = nullptr;
}

void RptDsPlugin::loadXML(QDomElement dsElement)
{
    QString pluginName = dsElement.attribute("plugin");
    QString fileName   = dsElement.attribute("fileName");
    m_dsName           = dsElement.attribute("name");
    m_dsAliasName      = dsElement.attribute("dsAliasName");
    m_recCount         = 0;

    QDomNodeList elements = dsElement.elementsByTagName("Data");
    if (elements.size() > 0) {
        QDomNode cdataNode = elements.at(0).childNodes().at(0);
        if (cdataNode.isCDATASection()) {
            m_cdata = cdataNode.toCDATASection().data();
        }
    }


    m_lib.setFileName(pluginName);
    if(!m_lib.load()) {
        qDebug() << "Loading failed!";
        return;
    }

    typedef QObject *(*H)(void);
    H h = (H)m_lib.resolve("getHandle");
    m_pluginObject = h();

    if (!m_cdata.isEmpty())
        setDataStr(m_cdata);
    else if (!fileName.isEmpty())
        setDataFile(fileName);

    QMetaObject::invokeMethod(m_pluginObject, "loadXML", Qt::DirectConnection,
                              Q_ARG(QString, dsElement.ownerDocument().toString())
                              );
}

void RptDsPlugin::setDataStr(QString str)
{
    if (m_pluginObject) {
        QString value;
        QMetaObject::invokeMethod(m_pluginObject, "setDataStr", Qt::DirectConnection,
                                  Q_ARG(QString, str)
                                  );
    }
}

void RptDsPlugin::setDataFile(QString fileName)
{
    if (m_pluginObject) {
        QString value;
        QMetaObject::invokeMethod(m_pluginObject, "setDataFile", Qt::DirectConnection,
                                  Q_ARG(QString, fileName)
                                  );
    }
}

QString RptDsPlugin::getFieldValue(QString fieldName, int recNo)
{
    if (m_pluginObject) {
        QString value;
        QMetaObject::invokeMethod(m_pluginObject, "getValue", Qt::DirectConnection,
                                  Q_RETURN_ARG(QString, value),
                                  Q_ARG(QString, fieldName),
                                  Q_ARG(int, recNo)
                                  );
        return value;
    }

    return QString();
}

int RptDsPlugin::getRecordCount()
{
    if (m_pluginObject) {
        int value;
        QMetaObject::invokeMethod(m_pluginObject, "getRecordCount", Qt::DirectConnection,
                                  Q_RETURN_ARG(int, value),
                                  Q_ARG(QString, m_dsAliasName)
                                  );
        m_recCount = value;
    }

    return m_recCount;
}

RptDsPlugin::~RptDsPlugin()
{
    m_lib.unload();
}
