/*
Name: QtRpt
Version: 3.0.0
Web-site: https://qtrpt.sourceforge.io
Programmer: Aleksey Osipov
E-mail: aliks-os@ukr.net
Web-site: http://www.aliks-os.tk

Copyright 2012-2022 Aleksey Osipov

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "RptSql.h"
//#include <QTextCodec>
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <QImage>

RptSql::RptSql(QObject *parent)
: RptDsAbstract(parent)
{
    /*#ifdef QT_DEBUG
      qDebug() << "Running a debug build";
    #else
      qDebug() << "Running a release build";
    #endif*/
}

bool RptSql::openQuery()
{
    QString sql = m_sqlConnection.sqlQuery;
    QString dbCoding = m_sqlConnection.dbCoding;
    QString charsetCoding = m_sqlConnection.charsetCoding;

    db.open();
    if (!db.isOpen())
        qDebug() << "Failed open DB" << db.lastError().text();
    else
        qDebug() << "open DB";

    query = new QSqlQuery(db);
    if (!dbCoding.isEmpty())
        if (db.driverName().contains("MYSQL"))
            query->exec("set names '"+dbCoding+"'");

//    if (!charsetCoding.isEmpty()) {
//        auto codec = QTextCodec::codecForName( QString(charsetCoding).toLocal8Bit().constData() );
//        QTextCodec::setCodecForLocale(codec);
//    }

    if (!query->exec(sql)) {
        qDebug() << query->lastError().text();
        return false;
    }

    if (query->isActive()) {
        query->last();
        m_recCount = query->at()+1;
        query->seek(0);

        // Get column's count
        auto record = query->record();
        for (int f = 0; f < record.count(); ++f)
            m_columns << record.fieldName(f);

        query->seek(0);
        for (int row = 0; row < m_recCount; row++) {
            QStringList valueList;
            auto record = query->record();

            for (int f = 0; f < m_columns.size(); f++)
                valueList << query->record().value(f).toString();

            m_data << valueList;
            query->next();
        }

        query->seek(0);
    } else {
        m_recCount = 0;
    }

    return true;
}

QString RptSql::getFieldValue(QString fieldName, int recNo)
{
    if (query->isActive()) {
        if (fieldName == "fieldName")
            return "as";

        if (recNo >= m_recCount) {
            qDebug() << "recNo more than recordCount";
            return "";
        } else {
            query->seek(recNo);
            int fieldNo = query->record().indexOf(fieldName);
            return query->value(fieldNo).toString();
        }
    } else {
        qDebug() << "Query is not active";
        return "";
    }
}

QImage RptSql::getFieldImage(QString fieldName, int recNo)
{
    if (query->isActive()) {
        if (recNo >= getRecordCount()) {
            qDebug() << "recNo more than recordCount";
            return QImage();
        } else {
            query->seek(recNo);
            int fieldNo = query->record().indexOf(fieldName);
            return QImage::fromData(query->value(fieldNo).toByteArray());
        }
    } else {
        qDebug() << "Query is not active";
        return QImage();
    }
}

void RptSql::loadXML(QDomElement dsElement)
{
    m_dsName                         = dsElement.attribute("name");
    m_sqlConnection.dsName           = dsElement.attribute("name");
    m_sqlConnection.dbType           = dsElement.attribute("dbType");
    m_sqlConnection.dbName           = dsElement.attribute("dbName");
    m_sqlConnection.dbHost           = dsElement.attribute("dbHost");
    m_sqlConnection.dbUser           = dsElement.attribute("dbUser");
    m_sqlConnection.dbPassword       = dsElement.attribute("dbPassword");
    m_sqlConnection.dbCoding         = dsElement.attribute("dbCoding");
    m_sqlConnection.charsetCoding    = dsElement.attribute("charsetCoding");
    m_sqlConnection.sqlQuery         = dsElement.text().trimmed();
    m_sqlConnection.dbPort           = dsElement.attribute("dbPort").toInt();
    m_sqlConnection.dbConnectionName = dsElement.attribute("dbConnectionName");

    setConnection(m_sqlConnection);
}

void RptSql::setConnection(RptSqlConnection sqlConnection)
{
    m_sqlConnection = sqlConnection;

    db = QSqlDatabase::addDatabase(m_sqlConnection.dbType, m_sqlConnection.dbConnectionName.isEmpty() ? QLatin1String(QSqlDatabase::defaultConnection) : m_sqlConnection.dbConnectionName);
    db.setDatabaseName(m_sqlConnection.dbName);
    db.setHostName(m_sqlConnection.dbHost);
    db.setUserName(m_sqlConnection.dbUser);
    db.setPassword(m_sqlConnection.dbPassword);
    if (m_sqlConnection.dbPort)
        db.setPort(m_sqlConnection.dbPort);

    this->setObjectName(m_sqlConnection.dsName);
    m_dsName = m_sqlConnection.dsName;
}
