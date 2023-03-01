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

#include "exampledlg1.h"
#include "ui_exampledlg1.h"
#include <QDebug>

ExampleDlg1::ExampleDlg1(QWidget *parent)
: QDialog(parent), ui(new Ui::ExampleDlg1)
{
    ui->setupUi(this);

    ui->dtp->setDate(QDate::currentDate());
    ui->edtCustomer->setText("Write customer here");
    ui->tableWidget->setRowCount(15);
    QObject::connect(ui->btnPrint, SIGNAL(clicked()), this, SLOT(print()));

    QTableWidgetItem *newItem;
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        newItem = new QTableWidgetItem("Goods "+QString::number(i));
        ui->tableWidget->setItem(i,0,newItem);

        newItem = new QTableWidgetItem(QString::number(i*10));
        ui->tableWidget->setItem(i,1,newItem);

        newItem = new QTableWidgetItem(QString::number(i*100));
        ui->tableWidget->setItem(i,2,newItem);

        newItem = new QTableWidgetItem(QString::number(i*1000));
        ui->tableWidget->setItem(i,3,newItem);
    }
}

void ExampleDlg1::setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage)
{
    Q_UNUSED(reportPage);

    if (paramName == "Alias.Crunch.Annuitant")
        paramValue = "!!!!!!!!!!!!!!!!!!";
    if (paramName == "customer")
        paramValue = ui->edtCustomer->text();
    if (paramName == "date")
        paramValue = ui->dtp->date().toString();
    if (paramName == "NN")
        paramValue = recNo+1;
    if (paramName == "Goods") {
        if (ui->tableWidget->item(recNo,0) == nullptr)
            return;
        paramValue = ui->tableWidget->item(recNo,0)->text();
    }
    if (paramName == "Quantity") {
        if (ui->tableWidget->item(recNo,1) == nullptr)
            return;
        paramValue = ui->tableWidget->item(recNo,1)->text();
    }
    if (paramName == "Price") {
        if (ui->tableWidget->item(recNo,2) == nullptr)
            return;
        paramValue = ui->tableWidget->item(recNo,2)->text();
    }
    if (paramName == "Sum") {
        if (ui->tableWidget->item(recNo,3) == nullptr)
            return;
        paramValue = ui->tableWidget->item(recNo,3)->text();
    }
}

void ExampleDlg1::setValueImage(const int recNo, const QString paramName, QImage &paramValue, const int reportPage)
{
    Q_UNUSED(recNo);
    Q_UNUSED(reportPage);

    if (paramName == "image") {
        auto image = new QImage(QCoreApplication::applicationDirPath()+"/pdf.png");
        paramValue = *image;
    }
}

void ExampleDlg1::print()
{
    QDir dir(qApp->applicationDirPath());
    #if defined(Q_OS_MAC)
        dir.cd(QFile::decodeName("../Resources"));
    #endif

    QString fileName = dir.absolutePath()+"/examples_report/example1.xml";
    fileName = "d:/input.xml";
    auto report = new QtRPT(this);
    report->setBackgroundImage(QPixmap(dir.absolutePath()+"/examples_report/qt_background_portrait.png"));
    report->loadReport(fileName);

    QObject::connect(report, SIGNAL(setValue(const int, const QString, QVariant&, const int)),
                     this, SLOT(setValue(const int, const QString, QVariant&, const int)));
    QObject::connect(report, SIGNAL(setValueImage(const int, const QString, QImage&, const int)),
                     this, SLOT(setValueImage(const int, const QString, QImage&, const int)));
    QObject::connect(report, SIGNAL(setDSInfo(DataSetInfo &)),
                     this, SLOT(setDSInfo(DataSetInfo &)));

    //report->setCallbackFunc(getReportValue);
    report->printExec(true);
}

void ExampleDlg1::setDSInfo(DataSetInfo &dsInfo)
{
    dsInfo.recordCount = ui->tableWidget->rowCount();
}

ExampleDlg1::~ExampleDlg1()
{
    delete ui;
}
