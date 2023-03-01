/*
Name: CommonFiles
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

#ifndef XYZ_UPDATEDLG_H
#define XYZ_UPDATEDLG_H

#include <QDialog>
#include <QFileDialog>
#include "XYZ_DownloadManager.h"

namespace Ui {
    class XYZUpdateDlg;
}

class XYZUpdateDlg : public QDialog
{
    Q_OBJECT
public:
    explicit XYZUpdateDlg(QString url, QWidget *parent = 0);
    ~XYZUpdateDlg();
    void showThis(QStringList list);

private:    
    Ui::XYZUpdateDlg *ui;
    QString m_url;

private slots:
    void doDownload();
    void downloadProgress(qint64 recieved, qint64 total);
};

#endif // XYZ_UPDATEDLG_H
