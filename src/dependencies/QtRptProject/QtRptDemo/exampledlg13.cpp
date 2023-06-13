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

#include "exampledlg13.h"
#include "ui_exampledlg13.h"
#include <QDir>
#include <QDebug>

ExampleDlg13::ExampleDlg13(QWidget *parent)
: QDialog(parent), ui(new Ui::ExampleDlg13)
{
    ui->setupUi(this);
    QObject::connect(ui->btnPrint, SIGNAL(clicked()), this, SLOT(print()));
}

void ExampleDlg13::print()
{
    QDir dir(qApp->applicationDirPath());
    #if defined(Q_OS_MAC)
        dir.cd(QFile::decodeName("../Resources"));
    #endif

    QString fileName = dir.absolutePath()+"/examples_report/example13.xml";
    QtRPT *report = new QtRPT(this);

    QObject::connect(report, SIGNAL(setField(RptFieldObject &)), this, SLOT(setField(RptFieldObject &)));
    QObject::connect(report, SIGNAL(setDSInfo(DataSetInfo &)),
                     this, SLOT(setDSInfo(DataSetInfo &)));

    if (report->loadReport(fileName) == false) {
        qDebug()<<"Report file not found";
        return;
    }
    report->printExec();
}

void ExampleDlg13::setDSInfo(DataSetInfo &dsInfo)
{
    dsInfo.recordCount = 3;
}

void ExampleDlg13::setField(RptFieldObject &fieldObject)
{
    RptFieldObject *fieldObject2 = fieldObject.parentBand->parentReportPage->findFieldObjectByName("field2");

    if (fieldObject.name == "field1") {
        fieldObject.value = "Get control of report building!";
        fieldObject.aligment = Qt::AlignVCenter | Qt::AlignHCenter;
        fieldObject.rect.setWidth(fieldObject.parentBand->width);
        fieldObject.rect.setLeft(0);
        fieldObject.rect.setTop(0);

        fieldObject.borderBottom = Qt::white;
        fieldObject.borderTop = Qt::white;
        fieldObject.borderLeft = Qt::white;
        fieldObject.borderRight = Qt::white;

        fieldObject.font.setBold(true);
        fieldObject.font.setPointSize(16);
    }

    if (fieldObject.name == "field2" || fieldObject.name == "field3" || fieldObject.name == "field4" || fieldObject.name == "field5") {
        fieldObject.borderBottom = Qt::blue;
        fieldObject.borderTop = Qt::blue;
        fieldObject.borderLeft = Qt::blue;
        fieldObject.borderRight = Qt::blue;
        fieldObject.borderWidth = 1;
        fieldObject.aligment = Qt::AlignLeft | Qt::AlignHCenter;
        fieldObject.rect.setTop(0);
        fieldObject.rect.setHeight(20);
        fieldObject.rect.setWidth(150);
    }
    if (fieldObject.name == "field2") {
        fieldObject.value = "Column 1 Row "+QString::number(fieldObject.recNo());
        fieldObject.rect.setLeft(0);
        fieldObject.parentBand->height = fieldObject.rect.height();  //Change the hight of the band

        if (fieldObject.recNo() == 0)
            fieldObject.backgroundColor = Qt::yellow;
        if (fieldObject.recNo() == 1)
            fieldObject.backgroundColor = Qt::magenta;
        if (fieldObject.recNo() == 2)
            fieldObject.backgroundColor = Qt::cyan;
    }
    if (fieldObject.name == "field3") {
        fieldObject.value = "Column 2 Row "+QString::number(fieldObject.recNo());
        if (fieldObject2 != 0) {
            fieldObject.rect.setLeft(fieldObject2->rect.width() * 1);
        }
        fieldObject.parentBand->height = fieldObject.rect.height();  //Change the hight of the band

        if (fieldObject.recNo() == 1)
            fieldObject.backgroundColor = Qt::yellow;
        if (fieldObject.recNo() == 2)
            fieldObject.backgroundColor = Qt::magenta;
        if (fieldObject.recNo() == 0)
            fieldObject.backgroundColor = Qt::cyan;
    }
    if (fieldObject.name == "field4") {
        fieldObject.value = "Column 3 Row "+QString::number(fieldObject.recNo());
        if (fieldObject2 != 0) {
            fieldObject.rect.setLeft(fieldObject2->rect.width() * 2);
        }
        fieldObject.parentBand->height = fieldObject.rect.height();  //Change the hight of the band

        if (fieldObject.recNo() == 2)
            fieldObject.backgroundColor = Qt::yellow;
        if (fieldObject.recNo() == 0)
            fieldObject.backgroundColor = Qt::magenta;
        if (fieldObject.recNo() == 1)
            fieldObject.backgroundColor = Qt::cyan;
    }
    if (fieldObject.name == "field5") {
        fieldObject.value = "Column 4 Row "+QString::number(fieldObject.recNo());
        if (fieldObject2 != 0) {
            fieldObject.rect.setLeft(fieldObject2->rect.width() * 3);
        }
        fieldObject.parentBand->height = fieldObject.rect.height();  //Change the hight of the band

        if (fieldObject.recNo() == 0)
            fieldObject.backgroundColor = Qt::yellow;
        if (fieldObject.recNo() == 1)
            fieldObject.backgroundColor = Qt::magenta;
        if (fieldObject.recNo() == 2)
            fieldObject.backgroundColor = Qt::cyan;
    }
}

ExampleDlg13::~ExampleDlg13()
{
    delete ui;
}
