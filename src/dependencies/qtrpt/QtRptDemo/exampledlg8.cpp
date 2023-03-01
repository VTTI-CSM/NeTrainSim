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

#include "exampledlg8.h"
#include "ui_exampledlg8.h"
#include <QDir>
#include <QDebug>

ExampleDlg8::ExampleDlg8(QWidget *parent)
: QDialog(parent), ui(new Ui::ExampleDlg8)
{
    ui->setupUi(this);

    QObject::connect(ui->btnPrint, SIGNAL(clicked()), this, SLOT(print()));

    ui->tableWidget->setRowCount(35);
    QTableWidgetItem *newItem;

    newItem = new QTableWidgetItem("1");
    ui->tableWidget->setItem(0,0,newItem);

    newItem = new QTableWidgetItem("2");
    ui->tableWidget->setItem(1,0,newItem);

    newItem = new QTableWidgetItem("3");
    ui->tableWidget->setItem(2,0,newItem);

    newItem = new QTableWidgetItem("4");
    ui->tableWidget->setItem(3,0,newItem);

    newItem = new QTableWidgetItem("5");
    ui->tableWidget->setItem(4,0,newItem);

    newItem = new QTableWidgetItem("6");
    ui->tableWidget->setItem(5,0,newItem);

    newItem = new QTableWidgetItem("7");
    ui->tableWidget->setItem(6,0,newItem);

    newItem = new QTableWidgetItem("6");
    ui->tableWidget->setItem(7,0,newItem);

    newItem = new QTableWidgetItem("6");
    ui->tableWidget->setItem(8,0,newItem);

    newItem = new QTableWidgetItem("6");
    ui->tableWidget->setItem(9,0,newItem);

    newItem = new QTableWidgetItem("6");
    ui->tableWidget->setItem(10,0,newItem);

    newItem = new QTableWidgetItem("6");
    ui->tableWidget->setItem(11,0,newItem);

    newItem = new QTableWidgetItem("6");
    ui->tableWidget->setItem(12,0,newItem);

    newItem = new QTableWidgetItem("6");
    ui->tableWidget->setItem(13,0,newItem);

    newItem = new QTableWidgetItem("6");
    ui->tableWidget->setItem(14,0,newItem);


    QString str = "Qt Framework and tools can be installed using an online installer, offline installers, "
                  "or by building the source packages yourself. The online installers give you the option to "
                  "download or install only certain modules or add-ons. An offline installer is a single "
                  "package which contains all of Qt and Add-Ons relevant for a target platform.";
    newItem = new QTableWidgetItem(str);
    ui->tableWidget->setItem(0,1,newItem);

    str = "The open-source versions are found at the Downloads page and the commercial versions are available for a free trial from Digia.";
    newItem = new QTableWidgetItem(str);
    ui->tableWidget->setItem(1,1,newItem);

    str = "After downloading, start the installer like any executable on the development platform. "
          "Select the components that you want to install and follow the instructions of the "
          "installation program to complete the installation. Use the Maintenance Tool under "
          " <install_dir> to add, update, or remove installed components.";
    newItem = new QTableWidgetItem(str);
    ui->tableWidget->setItem(2,1,newItem);

    str = "We invite you to explore the rest of Qt. We prepared overviews which help you decide which "
          "APIs to use and our examples demonstrate how to use them.";
    newItem = new QTableWidgetItem(str);
    ui->tableWidget->setItem(3,1,newItem);

    str = "Qt features a wide range of different technologies. The following topics are key areas of "
          "functionality and can be used as a starting point for learning how to to get the most of Qt.";
    newItem = new QTableWidgetItem(str);
    ui->tableWidget->setItem(4,1,newItem);

    str = "After downloading, start the installer like any executable on the development platform. "
          "Select the components that you want to install and follow the instructions of the "
          "installation program to complete the installation. Use the Maintenance Tool under "
          " <install_dir> to add, update, or remove installed components.";
    newItem = new QTableWidgetItem(str);
    ui->tableWidget->setItem(5,1,newItem);

    str = "After downloading, start the installer like any executable on the development platform. "
          "Select the components that you want to install and follow the instructions of the "
          "installation program to complete the installation. Use the Maintenance Tool under "
          " <install_dir> to add, update, or remove installed components.";
    newItem = new QTableWidgetItem(str);
    ui->tableWidget->setItem(6,1,newItem);

    str = "Qt features a wide range of different technologies. The following topics are key areas of "
          "functionality and can be used as a starting point for learning how to to get the most of Qt.";
    newItem = new QTableWidgetItem(str);
    ui->tableWidget->setItem(7,1,newItem);

    str = "Qt features a wide range of different technologies. The following topics are key areas of "
          "functionality and can be used as a starting point for learning how to to get the most of Qt.";
    newItem = new QTableWidgetItem(str);
    ui->tableWidget->setItem(8,1,newItem);

    str = "After downloading, start the installer like any executable on the development platform. "
          "Select the components that you want to install and follow the instructions of the "
          "installation program to complete the installation. Use the Maintenance Tool under "
          " <install_dir> to add, update, or remove installed components.";
    newItem = new QTableWidgetItem(str);
    ui->tableWidget->setItem(9,1,newItem);

    str = "After downloading, start the installer like any executable on the development platform. "
          "Select the components that you want to install and follow the instructions of the "
          "installation program to complete the installation. Use the Maintenance Tool under "
          " <install_dir> to add, update, or remove installed components.";
    newItem = new QTableWidgetItem(str);
    ui->tableWidget->setItem(10,1,newItem);

    str = "After downloading, start the installer like any executable on the development platform. "
          "Select the components that you want to install and follow the instructions of the "
          "installation program to complete the installation. Use the Maintenance Tool under "
          " <install_dir> to add, update, or remove installed components.";
    newItem = new QTableWidgetItem(str);
    ui->tableWidget->setItem(11,1,newItem);

    str = "After downloading, start the installer like any executable on the development platform. "
          "Select the components that you want to install and follow the instructions of the "
          "installation program to complete the installation. Use the Maintenance Tool under "
          " <install_dir> to add, update, or remove installed components.";
    newItem = new QTableWidgetItem(str);
    ui->tableWidget->setItem(12,1,newItem);

    str = "After downloading, start the installer like any executable on the development platform. "
          "Select the components that you want to install and follow the instructions of the "
          "installation program to complete the installation. Use the Maintenance Tool under "
          " <install_dir> to add, update, or remove installed components.";
    newItem = new QTableWidgetItem(str);
    ui->tableWidget->setItem(13,1,newItem);

    str = "After downloading, start the installer like any executable on the development platform. "
          "Select the components that you want to install and follow the instructions of the "
          "installation program to complete the installation. Use the Maintenance Tool under "
          " <install_dir> to add, update, or remove installed components.";
    newItem = new QTableWidgetItem(str);
    ui->tableWidget->setItem(14,1,newItem);

    str = "Qt features a wide range of different technologies. The following topics are key areas of "
          "functionality and can be used as a starting point for learning how to to get the most of Qt.";
    newItem = new QTableWidgetItem(str);
    ui->tableWidget->setItem(15,1,newItem);
}

void ExampleDlg8::setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage)
{
    Q_UNUSED(reportPage);

    if (paramName == "Text") {
        if (ui->tableWidget->item(recNo,0) == 0) return;
        paramValue = ui->tableWidget->item(recNo,1)->text();
    }
}

void ExampleDlg8::setDSInfo(DataSetInfo &dsInfo)
{
    dsInfo.recordCount = ui->tableWidget->rowCount();
}

void ExampleDlg8::print()
{
    QDir dir(qApp->applicationDirPath());
    #if defined(Q_OS_MAC)
        dir.cd(QFile::decodeName("../Resources"));
    #endif

    QString fileName = dir.absolutePath()+"/examples_report/example8.xml";
    auto report = QtRPT::createSPtr(this);

    if (report->loadReport(fileName) == false)
        qDebug()<<"Report file not found";

    QObject::connect(report.data(), SIGNAL(setValue(const int, const QString, QVariant&, const int)),
                     this, SLOT(setValue(const int, const QString, QVariant&, const int)));
    QObject::connect(report.data(), SIGNAL(setDSInfo(DataSetInfo &)),
                     this, SLOT(setDSInfo(DataSetInfo &)));
    report->printExec();
}

ExampleDlg8::~ExampleDlg8()
{
    delete ui;
}
