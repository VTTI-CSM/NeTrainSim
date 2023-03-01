#include "RptDsInline.h"
#include <QDebug>

RptDsInline::RptDsInline(QObject *parent)
: RptDsAbstract(parent)
{

}

void RptDsInline::loadXML(QDomElement dsElement)
{
    m_dsName          = dsElement.attribute("name");
    m_recCount        = dsElement.attribute("rowCount").toInt();
    QString delimiter = dsElement.attribute("delimiter");

    // Get column's count
    QDomNode n = dsElement.firstChild();
    QString columns = n.toElement().attribute("value");
    m_columns  = columns.split(delimiter);
    m_colCount = m_columns.size();

    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull() && e.tagName() == "Row") {
            QStringList valueList = e.attribute("value").split(delimiter);
            m_data << valueList;
        }

        n = n.nextSibling();
    }
}

QString RptDsInline::getFieldValue(QString fieldName, int recNo)
{
    int idx = m_columns.indexOf(fieldName);
    if (idx == -1)
        return QString();

    if (recNo+1 < m_data.size()) {
        auto values = m_data.at(recNo+1);

        if (idx < values.size())
            return values.at(idx);
        else
            return QString();
    }

    return QString();
}
