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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#if QT_VERSION >= 0x50000
    #include <QtWidgets>
#else
    #include <QtGui>
#endif

#include <QMainWindow>
#include <qtrpt.h>
#include "exampledlg1.h"
#include "exampledlg2.h"
#include "exampledlg3.h"
#include "exampledlg4.h"
#include "exampledlg5.h"
#include "exampledlg6.h"
#include "exampledlg8.h"
#include "exampledlg13.h"
#include "exampledlg14.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    //static void getReportValue(int &recNo, QString &paramName, QVariant &paramValue);


private:
    Ui::MainWindow *ui;
    QVector<double> doubleVector;


private slots:
    void showReport();
    void setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage);
    void setField(RptFieldObject &);
    void setDSInfo(DataSetInfo &dsInfo);
    void setChart(RptFieldObject &fieldObject, QChart &chart);
    void setValueDiagram(GraphDataList &dataList);

};

#endif // MAINWINDOW_H
