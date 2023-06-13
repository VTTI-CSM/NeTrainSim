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

#include "XYZ_UpdateDlg.h"
#include "ui_XYZ_UpdateDlg.h"

XYZUpdateDlg::XYZUpdateDlg(QString url,QWidget *parent)
: QDialog(parent), ui(new Ui::XYZUpdateDlg)
{
    ui->setupUi(this);
    m_url = url;
    QObject::connect(ui->btnUpdate, SIGNAL(clicked()),this, SLOT(doDownload()));
}

void XYZUpdateDlg::showThis(QStringList list)
{
    ui->pb1->setValue(0);
    ui->pb2->setValue(0);
    ui->tableWidget->setColumnWidth(0,0);
    ui->tableWidget->setColumnWidth(1,300);
    ui->tableWidget->setRowCount(list.size());

    QTableWidgetItem *newItem;
    for (int i = 0; i < list.size(); ++i) {
        newItem = new QTableWidgetItem(list.at(i));
        newItem->setCheckState(Qt::Checked);
        ui->tableWidget->setItem(i,1,newItem);
    }

    this->exec();
}

void XYZUpdateDlg::doDownload()
{
    QString dir = QCoreApplication::applicationDirPath();
    dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                         QCoreApplication::applicationDirPath(),
                                         QFileDialog::ShowDirsOnly
                                         | QFileDialog::DontResolveSymlinks);

    int checked = 0;
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i)
        if (ui->tableWidget->item(i,1)->checkState() == Qt::Checked)
            checked += 1;

    ui->pb2->setMaximum(checked);
    XYZDownloadManager dl1;

    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        if (ui->tableWidget->item(i,1)->checkState() == Qt::Checked) {
            ui->pb1->setValue(0);
            QString url = m_url+ui->tableWidget->item(i,1)->text();
            QString fileName = ui->tableWidget->item(i,1)->text().simplified();
            int p = fileName.lastIndexOf("/");
            fileName = fileName.remove(0,p+1);
            dl1.fileName = dir+"/"+fileName;

            ui->lblInfo->setText(tr("Downloading %0. ..").arg(ui->tableWidget->item(i,1)->text()));
            dl1.setTarget(url.replace(" ","%20").simplified());
            QObject::connect(&dl1, SIGNAL(downloadingProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
            dl1.download(false);

            QEventLoop loop;
            connect(&dl1,SIGNAL(done()),&loop,SLOT(quit()));
            loop.exec();

            ui->pb2->setValue(ui->pb2->value()+1);
        }
    }


    emit this->accepted();
    this->close();
}

void XYZUpdateDlg::downloadProgress(qint64 recieved, qint64 total)
{
    ui->pb1->setValue(recieved);
    ui->pb1->setMaximum(total);
}

XYZUpdateDlg::~XYZUpdateDlg()
{
    delete ui;
}
