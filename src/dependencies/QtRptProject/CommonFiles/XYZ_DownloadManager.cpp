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

#include "XYZ_DownloadManager.h"

XYZDownloadManager::XYZDownloadManager(QObject *parent)
: QObject(parent)
{
    manager = new QNetworkAccessManager(this);
    QObject::connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(downloadFinished(QNetworkReply*)));
}

void XYZDownloadManager::setTarget(const QString &t)
{
    this->m_target = t;
    m_isdownloaded = false;
}

bool XYZDownloadManager::isDownloaded()
{
    return m_isdownloaded;
}

void XYZDownloadManager::download(bool ver)
{
    m_version = ver;
    m_isdownloaded = false;
    QUrl url = QUrl::fromEncoded(this->m_target.toLocal8Bit());
    QNetworkRequest request(url);
    QObject::connect(manager->get(request), SIGNAL(downloadProgress(qint64,qint64)),
                     this, SIGNAL(downloadingProgress(qint64,qint64)));
}

void XYZDownloadManager::downloadFinished(QNetworkReply *data)
{
    error = data->error();
    if (error != QNetworkReply::NoError) {
        //QMessageBox::warning(0,tr("Error"),tr("No file or Internet is not connected"),QMessageBox::Ok);
        emit done();
        return;
    }

    if (m_version) {
        const QByteArray sdata = data->readAll();
        QStringList fl;
        fl << sdata;
        fileList = fl.at(0).split("\n");        
        //qDebug() << fl;
        //qDebug() << fileList;

        if (QApplication::applicationVersion().replace(".","") < fileList[0].replace(".","").simplified()) {
            fileList.removeAt(0);
            XYZUpdateDlg *updateDlg = new XYZUpdateDlg(m_target.replace("version.txt",""),qobject_cast<QWidget *>(this->parent()));
            updateDlg->showThis(fileList);
        }
    } else {
        QFile localFile(fileName);
        if (!localFile.open(QIODevice::WriteOnly))
            return;

        const QByteArray sdata = data->readAll();
        localFile.write(sdata);
        localFile.close();
    }
    m_isdownloaded = true;
    emit done();
}

void XYZDownloadManager::downloadProgress(qint64 recieved, qint64 total)
{
    qDebug() << recieved << total;
}
