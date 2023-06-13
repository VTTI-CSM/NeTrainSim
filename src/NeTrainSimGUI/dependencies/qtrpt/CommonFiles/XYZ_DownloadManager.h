/*
Name: XYZ
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

#ifndef XYZDOWNLOADMANAGER_H
#define XYZDOWNLOADMANAGER_H

#include <QObject>
#include <QNetworkReply>
#include <QMessageBox>
#include <QFile>
#include <QApplication>
#include <QtCore>
#include <XYZ_UpdateDlg.h>

class XYZDownloadManager : public QObject
{
    Q_OBJECT
public:
    explicit XYZDownloadManager(QObject *parent = 0);
    QNetworkReply::NetworkError error;
    void download(bool ver);
    bool isDownloaded();
    void setTarget(const QString &t);
    QString fileName;

private:
    QNetworkAccessManager *manager;
    QString m_target;
    bool m_isdownloaded;
    bool m_version;
    QStringList fileList;

signals:
    void done();
    void downloadingProgress(qint64 recieved, qint64 total);

public slots:
    void downloadFinished(QNetworkReply* data);
    void downloadProgress(qint64 recieved, qint64 total);
};

#endif // XYZDOWNLOADMANAGER_H
