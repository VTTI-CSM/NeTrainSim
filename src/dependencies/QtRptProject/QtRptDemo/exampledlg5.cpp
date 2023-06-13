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

#include "exampledlg5.h"
#include "ui_exampledlg5.h"
#include <QDir>
#include <QDebug>

ExampleDlg5::ExampleDlg5(QWidget *parent)
: QDialog(parent), ui(new Ui::ExampleDlg5)
{
    ui->setupUi(this);

    ui->tableWidget->setRowCount(20);
    QObject::connect(ui->btnPrint, SIGNAL(clicked()), this, SLOT(print()));

    QTableWidgetItem *newItem;
    int quant; int price; int sum;
    for (int i = 1; i < ui->tableWidget->rowCount()+1; ++i) {
        newItem = new QTableWidgetItem("Goods "+QString::number(i));
        ui->tableWidget->setItem(i-1,0,newItem);

        if (i == 3 || i == 5 || i == 7) quant = -i*10;
        else quant = i*10;

        newItem = new QTableWidgetItem(QString::number(quant));
        ui->tableWidget->setItem(i-1,1,newItem);

        price = i*100;
        newItem = new QTableWidgetItem(QString::number(price));
        ui->tableWidget->setItem(i-1,2,newItem);

        sum = quant * price;
        newItem = new QTableWidgetItem(QString::number(sum));
        ui->tableWidget->setItem(i-1,3,newItem);
    }
}

void ExampleDlg5::setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage)
{
    Q_UNUSED(reportPage);
    if (paramName == "NN")
        paramValue = recNo+1;
    if (paramName == "Goods") {
        if (ui->tableWidget->item(recNo,0) == 0) return;
        paramValue = ui->tableWidget->item(recNo,0)->text();
    }
    if (paramName == "Quantity") {
        if (ui->tableWidget->item(recNo,1) == 0) return;
        paramValue = ui->tableWidget->item(recNo,1)->text();
    }
    if (paramName == "Price") {
        if (ui->tableWidget->item(recNo,2) == 0) return;
        paramValue = ui->tableWidget->item(recNo,2)->text();
    }
    if (paramName == "Sum") {
        if (ui->tableWidget->item(recNo,3) == 0) return;
        paramValue = ui->tableWidget->item(recNo,3)->text();
    }
}

void ExampleDlg5::print()
{
    QDir dir(qApp->applicationDirPath());
    #if defined(Q_OS_MAC)
        dir.cd(QFile::decodeName("../Resources"));
    #endif

    QString fileName = dir.absolutePath()+"/examples_report/example5.xml";
    report = new QtRPT(this);

    if (report->loadReport(fileName) == false) {
        qDebug()<<"Report file not found";
    }
    QObject::connect(report, SIGNAL(setValue(const int, const QString, QVariant&, const int)),
                     this, SLOT(setValue(const int, const QString, QVariant&, const int)));
    QObject::connect(report, SIGNAL(setDSInfo(DataSetInfo &)),
                     this, SLOT(setDSInfo(DataSetInfo &)));
    report->printExec();

}

void ExampleDlg5::setDSInfo(DataSetInfo &dsInfo)
{
    dsInfo.recordCount = ui->tableWidget->rowCount();
}

ExampleDlg5::~ExampleDlg5()
{
    delete ui;
}
