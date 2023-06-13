#include "RptDsAbstract.h"

RptDsAbstract::RptDsAbstract(QObject *parent) : QObject(parent)
{
    m_recCount = 0;
    m_colCount = 0;
}

int RptDsAbstract::getRecordCount()
{
    return m_recCount;
}

QString RptDsAbstract::DSName()
{
    return m_dsName;
}
