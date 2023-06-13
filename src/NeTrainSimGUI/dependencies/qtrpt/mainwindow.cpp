/*
Name: QtRpt
Version: 2.2.0
Web-site: https://qtrpt.sourceforge.io
Programmer: Aleksey Osipov
E-mail: aliks-os@ukr.net
Web-site: http://www.aliks-os.tk

Copyright 2012-2021 Aleksey Osipov

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

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->btnShow, SIGNAL(clicked()), this, SLOT(showReport()));
}

void MainWindow::showReport()
{
    QDir dir(qApp->applicationDirPath());
    #if defined(Q_OS_MAC)
        dir.cd(QFile::decodeName("../Resources"));
    #endif

    QDialog *dlg = nullptr;

    if (ui->rBtn1->isChecked())
        dlg = new ExampleDlg1(this);
    else if (ui->rBtn2->isChecked())
        dlg = new ExampleDlg2(this);
    else if (ui->rBtn3->isChecked())
        dlg = new ExampleDlg3(this);
    else if (ui->rBtn4->isChecked())
        dlg = new ExampleDlg4(this);
    else if (ui->rBtn5->isChecked())
        dlg = new ExampleDlg5(this);
    else if (ui->rBtn6a->isChecked())
        dlg = new ExampleDlg6(this,1);
    else if (ui->rBtn6b->isChecked())
        dlg = new ExampleDlg6(this,2);
    else if (ui->rBtn7a->isChecked())
    {
        QString fileName = dir.absolutePath()+"/examples_report/example7a.xml";
        auto report = QtRPT::createSPtr(this);

        if (report->loadReport(fileName) == false)
            qDebug()<<"Report file not found";

        QObject::connect(report.data(), SIGNAL(setValueDiagram(GraphDataList&)), this, SLOT(setValueDiagram(GraphDataList&)));

        report->printExec();
    }
    else if (ui->rBtn7b->isChecked())
    {
        QString fileName = dir.absolutePath()+"/examples_report/example7b.xml";
        auto report = QtRPT::createSPtr(this);

        if (report->loadReport(fileName) == false)
            qDebug()<<"Report file not found";

        QObject::connect(report.data(), SIGNAL(setChart(RptFieldObject&, QChart&)), this, SLOT(setChart(RptFieldObject&, QChart&)));

        report->printExec();
    }
    else if (ui->rBtn8->isChecked())
        dlg = new ExampleDlg8(this);
    else if (ui->rBtn9->isChecked())
    {
        QString fileName = dir.absolutePath()+"/examples_report/example9.xml";
        auto report = QtRPT::createSPtr(this);
        if (report->loadReport(fileName) == false)
            qDebug() << "Report file not found";

        report->printExec();
    }
    else if (ui->rBtn10->isChecked())
    {
        QString fileName = dir.absolutePath()+"/examples_report/example10.xml";
        auto report = QtRPT::createSPtr(this);
        if (report->loadReport(fileName) == false)
            qDebug() << "Report file not found";

        QObject::connect(report.data(), SIGNAL(setDSInfo(DataSetInfo &)),
                         this, SLOT(setDSInfo(DataSetInfo &)));

        report->printExec();
    }
    else if (ui->rBtn11->isChecked())
    {
        QString fileName = dir.absolutePath()+"/examples_report/example11.xml";
        auto report = QtRPT::createSPtr(this);

        QObject::connect(report.data(), SIGNAL(setValue(const int, const QString, QVariant&, const int)),
                         this, SLOT(setValue(const int, const QString, QVariant&, const int)));
        if (report->loadReport(fileName) == false)
            qDebug() << "Report file not found";

        report->printExec();
    }
    else if (ui->rBtn12->isChecked())
    {
        QString fileName = dir.absolutePath()+"/examples_report/example12.xml";
        auto report = QtRPT::createSPtr(this);

        QObject::connect(report.data(), SIGNAL(setValue(const int, const QString, QVariant&, const int)),
                         this, SLOT(setValue(const int, const QString, QVariant&, const int)));
        QObject::connect(report.data(), SIGNAL(setDSInfo(DataSetInfo &)),
                         this, SLOT(setDSInfo(DataSetInfo &)));
        if (report->loadReport(fileName) == false)
            qDebug() << "Report file not found";

        report->printExec();
    }
    else if (ui->rBtn13->isChecked())
        dlg = new ExampleDlg13(this);
    else if (ui->rBtn14->isChecked())
        dlg = new ExampleDlg14(this);
    else if (ui->rBtn15->isChecked())
    {
        QString fileName = dir.absolutePath()+"/examples_report/example9.xml";
        auto report = QtRPT::createSPtr(this);
        if (report->loadReport(fileName) == false)
            qDebug() << "Report file not found";

        report->printPDF(dir.absolutePath()+"/example9.pdf",true);
    }
    else if (ui->rBtn16->isChecked())
    {
        QString fileName = dir.absolutePath()+"/examples_report/example16.xml";
        auto report = QtRPT::createSPtr(this);
        if (report->loadReport(fileName) == false)
            qDebug() << "Report file not found";

        QString strSQL =
                "SELECT *\n"
                "FROM artists\n"
                "ORDER BY lastname, firstname";

        report->setUserSqlConnection(0, "DB1", "QSQLITE", dir.absolutePath()+"/examples_report/example.sqlite", "", "", "", 0, "", strSQL);
        report->printExec();
    }
    else if (ui->rBtn17->isChecked())
    {
        for (quint16 i = 0; i < 15; i++)
            doubleVector.append(32767 * (float)qrand() / RAND_MAX);
        QString fileName = dir.absolutePath()+"/examples_report/example17.xml";
        auto report = QtRPT::createSPtr(this);

        QObject::connect(report.data(), SIGNAL(setValue(const int, const QString, QVariant&, const int)),
                         this, SLOT(setValue(const int, const QString, QVariant&, const int)));
        QObject::connect(report.data(), SIGNAL(setDSInfo(DataSetInfo &)),
                         this, SLOT(setDSInfo(DataSetInfo &)));
        if (report->loadReport(fileName) == false)
            qDebug() << "Report file not found";

        report->printExec();
    }
    else if (ui->rBtn18->isChecked())
    {
        QString fileName = dir.absolutePath()+"/examples_report/example18.xml";
        auto report = QtRPT::createSPtr(this);

        if (report->loadReport(fileName) == false)
            qDebug()<<"Report file not found";

        QObject::connect(report.data(), SIGNAL(setField(RptFieldObject &)), this, SLOT(setField(RptFieldObject &)));

        report->printExec(true);
    }
    else if (ui->rBtn19->isChecked())
    {
        QString fileName = dir.absolutePath()+"/examples_report/example19.xml";
        auto report = QtRPT::createSPtr(this);

        if (report->loadReport(fileName) == false)
            qDebug()<<"Report file not found";

        QObject::connect(report.data(), SIGNAL(setValue(const int, const QString, QVariant&, const int)),
                         this, SLOT(setValue(const int, const QString, QVariant&, const int)));
        QObject::connect(report.data(), SIGNAL(setDSInfo(DataSetInfo &)),
                         this, SLOT(setDSInfo(DataSetInfo &)));

        report->printExec(true);
    }
    else if (ui->rBtn20->isChecked())
    {
        QString fileName = dir.absolutePath()+"/examples_report/example20.xml";
        auto report = QtRPT::createSPtr(this);

        if (report->loadReport(fileName) == false)
            qDebug()<<"Report file not found";

        QObject::connect(report.data(), SIGNAL(setValue(const int, const QString, QVariant&, const int)),
                         this, SLOT(setValue(const int, const QString, QVariant&, const int)));
//        QObject::connect(report.data(), SIGNAL(setDSInfo(DataSetInfo &)),
//                         this, SLOT(setDSInfo(DataSetInfo &)));

        report->printExec(true);
    }
    else if (ui->rBtn21->isChecked())
    {
        QString fileName = dir.absolutePath()+"/examples_report/example21.xml";
        auto report = QtRPT::createSPtr(this);

        if (report->loadReport(fileName) == false)
            qDebug()<<"Report file not found";

        QObject::connect(report.data(), SIGNAL(setDSInfo(DataSetInfo &)),
                         this, SLOT(setDSInfo(DataSetInfo &)));

        report->printExec(true);
    }
    else if (ui->rBtn22->isChecked())
    {
        QString fileName = dir.absolutePath()+"/examples_report/example22.xml";
        auto report = QtRPT::createSPtr(this);

        if (report->loadReport(fileName) == false)
            qDebug()<<"Report file not found";

        QObject::connect(report.data(), SIGNAL(setDSInfo(DataSetInfo &)),
                         this, SLOT(setDSInfo(DataSetInfo &)));

        report->printExec(true);
    }
    else if (ui->rBtnRussian->isChecked())
    {
        QString fileName = dir.absolutePath()+"/examples_report/RussianInvaders.xml";
        auto report = QtRPT::createSPtr(this);
        QObject::connect(report.data(), SIGNAL(setValue(const int, const QString, QVariant&, const int)),
                         this, SLOT(setValue(const int, const QString, QVariant&, const int)));
        if (report->loadReport(fileName) == false)
            qDebug() << "Report file not found";

        report->printExec();
    }

    if (dlg != nullptr)
        dlg->exec();

    delete dlg;
}

void MainWindow::setDSInfo(DataSetInfo &dsInfo)
{
    if (ui->rBtn17->isChecked())
        dsInfo.recordCount = doubleVector.size();

    if (ui->rBtn10->isChecked())
        dsInfo.recordCount = 3;

    if (ui->rBtn12->isChecked())
        dsInfo.recordCount = 3;

    if (ui->rBtn19->isChecked()) {
        if (dsInfo.dataSetNo == 1)
            dsInfo.recordCount = 60;
        if (dsInfo.dataSetNo == 2)
            dsInfo.recordCount = 50;
    }
}

void MainWindow::setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage)
{
    Q_UNUSED(reportPage)

    if (ui->rBtn11->isChecked()) {
        if (paramName == "bar1") {
            QString tmp;
            if (recNo*2+1 < 10)
                tmp = "0"+QString::number(recNo*2+1);
            else
                tmp = QString::number(recNo*2+1);
            paramValue = "L261250000"+tmp;
        }
        if (paramName == "bar2") {
            QString tmp;
            if (recNo*2+2 < 10)
                tmp = "0"+QString::number(recNo*2+2);
            else
                tmp = QString::number(recNo*2+2);
            paramValue = "L261250000"+tmp;
        }
    }
    if (ui->rBtn12->isChecked()) {
        if (paramName == "text1") {
            if (recNo == 0)
                paramValue = "Mary";
            if (recNo == 1)
                paramValue = "Ella";
            if (recNo == 2)
                paramValue = "Vera";
        }
        if (paramName == "text2") {
            if (recNo == 0)
                paramValue = "chocolate";
            if (recNo == 1)
                paramValue = "banana";
            if (recNo == 2)
                paramValue = "apple";
        }
        if (paramName == "text4") {
            paramValue = "<body><font size=5>This тект is formatted by the user by means of HTML tags. You can do the text <b>bold</b>, "
                         "<i>italics</i>, <u>underline</u>. To allocate separate words in various color, as example: <font color=#0000FF>blue</font>, "
                         "<font color=#FF0000>red</font>, <font color=#CC6633>brown</font>, <font color=#00FF00>green</font> etc</font></body>";
        }
    }
    if (ui->rBtn17->isChecked()) {
        if (paramName == "number")
            paramValue = QString::number(doubleVector.at(recNo)/100,'f',2);
    }
    if (ui->rBtn19->isChecked()) {
        if (paramName == "ds1")
            paramValue = recNo * 3;
        if (paramName == "ds2")
            paramValue = recNo;
    }
    if (ui->rBtn20->isChecked()) {
        if (paramName == "var1")
            paramValue = 111;
        if (paramName == "var2")
            paramValue = 222;
        if (paramName == "var3")
            paramValue = 333;
    }
}

void MainWindow::setField(RptFieldObject &fieldObject)
{
    // ---Example 18---
    // set the row and column count of the CrossTab
    if (fieldObject.fieldType == FieldType::CrossTab) {
        int colCount = 5;

        fieldObject.crossTab->setColCount(colCount);
        fieldObject.crossTab->setRowCount(15);

        fieldObject.crossTab->columns.clear();

        RptCrossTabObject::ColumnParameters column;
        column.caption = "Header1";
        fieldObject.crossTab->columns << column;
        column.caption = "Header2";
        fieldObject.crossTab->columns << column;
        column.caption = "Header3";
        fieldObject.crossTab->columns << column;
        column.caption = "Header4";
        fieldObject.crossTab->columns << column;
        column.caption = "Header5";
        fieldObject.crossTab->columns << column;
    }
    // requiest the data for cell of the CrossTab
    if (fieldObject.parentCrossTab != nullptr) {
        int row = fieldObject.parentCrossTab->fieldRow(&fieldObject);
        int col = fieldObject.parentCrossTab->fieldCol(&fieldObject);

        fieldObject.value = QString("Col %1 Row %2").arg(col).arg(row);
    }
    // ---Example 18---
}

void MainWindow::setChart(RptFieldObject &fieldObject, QChart &chart)
{
    if (ui->rBtn7b->isChecked()) {
        // ---Example 7b---
        chart.removeAllSeries();

        if (fieldObject.name == "diagram1") {
            auto series = new QLineSeries();
            series->setProperty("graphDS", "1");
            series->append(0,0);
            series->append(3,3);
            series->setColor(Qt::blue);
            series->setName(QString("Series #%1").arg(chart.series().size()));

            chart.addSeries(series);
            chart.createDefaultAxes();
            chart.axes(Qt::Horizontal)[0]->setTitleText(QString("x [m]"));
            chart.axes(Qt::Vertical)[0]->setTitleText(QString("y [m]"));
            chart.setTitle("Simple chart example");
        }

        if (fieldObject.name == "diagram2") {
            auto set0 = new QBarSet("Jane");
            auto set1 = new QBarSet("John");
            auto set2 = new QBarSet("Axel");
            auto set3 = new QBarSet("Mary");
            auto set4 = new QBarSet("Samantha");

            *set0 << 1 << 2 << 3 << 4 << 5 << 6;
            *set1 << 5 << 0 << 0 << 4 << 0 << 7;
            *set2 << 3 << 5 << 8 << 13 << 8 << 5;
            *set3 << 5 << 6 << 7 << 3 << 4 << 5;
            *set4 << 9 << 7 << 5 << 3 << 1 << 2;

            auto series = new QStackedBarSeries();
            series->append(set0);
            series->append(set1);
            series->append(set2);
            series->append(set3);
            series->append(set4);

            chart.addSeries(series);
            chart.setTitle("Simple stackedbarchart example");

            QStringList categories;
            categories << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun";

            auto axis = new QBarCategoryAxis();
            axis->append(categories);
            chart.createDefaultAxes();
            chart.setAxisX(axis, series);

            chart.legend()->setVisible(true);
            chart.legend()->setAlignment(Qt::AlignBottom);
        }

        if (fieldObject.name == "diagram3") {
            auto series = new QPieSeries();
            series->append("Jane", 1);
            series->append("Joe", 2);
            series->append("Andy", 3);
            series->append("Barbara", 4);
            series->append("Axel", 5);

            auto slice = series->slices().at(1);
            slice->setExploded();
            slice->setLabelVisible();
            slice->setPen(QPen(Qt::darkGreen, 2));
            slice->setBrush(Qt::green);

            chart.addSeries(series);
            chart.setTitle("Simple piechart example");
            chart.legend()->hide();
        }

        if (fieldObject.name == "diagram4") {
            auto set0 = new QBarSet("Jane");
            auto set1 = new QBarSet("John");
            auto set2 = new QBarSet("Axel");
            auto set3 = new QBarSet("Mary");
            auto set4 = new QBarSet("Sam");

            *set0 << 1 << 2 << 3 << 4 << 5 << 6;
            *set1 << 5 << 0 << 0 << 4 << 0 << 7;
            *set2 << 3 << 5 << 8 << 13 << 8 << 5;
            *set3 << 5 << 6 << 7 << 3 << 4 << 5;
            *set4 << 9 << 7 << 5 << 3 << 1 << 2;

            auto barseries = new QBarSeries();
            barseries->append(set0);
            barseries->append(set1);
            barseries->append(set2);
            barseries->append(set3);
            barseries->append(set4);

            auto lineseries = new QLineSeries();
            lineseries->setName("trend");
            lineseries->append(QPoint(0, 4));
            lineseries->append(QPoint(1, 15));
            lineseries->append(QPoint(2, 20));
            lineseries->append(QPoint(3, 4));
            lineseries->append(QPoint(4, 12));
            lineseries->append(QPoint(5, 17));

            chart.addSeries(barseries);
            chart.addSeries(lineseries);
            chart.setTitle("Line and barchart example");

            QStringList categories;
            categories << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun";
            auto axisX = new QBarCategoryAxis();
            axisX->append(categories);
            chart.setAxisX(axisX, lineseries);
            chart.setAxisX(axisX, barseries);
            axisX->setRange(QString("Jan"), QString("Jun"));

            auto axisY = new QValueAxis();
            chart.setAxisY(axisY, lineseries);
            chart.setAxisY(axisY, barseries);
            axisY->setRange(0, 20);

            chart.legend()->setVisible(true);
            chart.legend()->setAlignment(Qt::AlignBottom);
        }

        QFont font = chart.titleFont();
        font.setPointSize(font.pointSize() * 2);
        chart.setTitleFont(font);

        // ---Example 7b---
    }
}

void MainWindow::setValueDiagram(GraphDataList &dataList)
{
    for (auto &data: dataList) {
        if (data.graphDS == "ds1") {
            QList<GraphValue> valueList;

            GraphValue graphValue;
            graphValue.valueX = 0; //X - for Line only
            graphValue.valueY = 0;
            valueList << graphValue;

            graphValue.valueX = 1;
            graphValue.valueY = 2;
            valueList << graphValue;

            graphValue.valueX = 2;
            graphValue.valueY = 1;
            valueList << graphValue;

            graphValue.valueX = 3;
            graphValue.valueY = 3;
            valueList << graphValue;

            data.valueList = valueList;
        }
        if (data.graphDS == "ds2-1") {
            QList<GraphValue> valueList;

            GraphValue graphValue;
            graphValue.valueY = 1;
            valueList << graphValue;

            graphValue.valueY = 2;
            valueList << graphValue;

            graphValue.valueY = 3;
            valueList << graphValue;

            data.valueList = valueList;
        }
        if (data.graphDS == "ds2-2") {
            QList<GraphValue> valueList;

            GraphValue graphValue;
            graphValue.valueY = 1;
            valueList << graphValue;

            graphValue.valueY = 2;
            valueList << graphValue;

            graphValue.valueY = 3;
            valueList << graphValue;

            data.valueList = valueList;
        }
        if (data.graphDS == "ds2-3") {
            QList<GraphValue> valueList;

            GraphValue graphValue;
            graphValue.valueY = 1;
            valueList << graphValue;

            graphValue.valueY = 2;
            valueList << graphValue;

            graphValue.valueY = 3;
            valueList << graphValue;

            data.valueList = valueList;
        }
        if (data.graphDS == "ds2-4") {
            QList<GraphValue> valueList;

            GraphValue graphValue;
            graphValue.valueY = 1;
            valueList << graphValue;

            graphValue.valueY = 2;
            valueList << graphValue;

            graphValue.valueY = 3;
            valueList << graphValue;

            data.valueList = valueList;
        }
        if (data.graphDS == "ds2-5") {
            QList<GraphValue> valueList;

            GraphValue graphValue;
            graphValue.valueY = 1;
            valueList << graphValue;

            graphValue.valueY = 2;
            valueList << graphValue;

            graphValue.valueY = 3;
            valueList << graphValue;

            data.valueList = valueList;
        }
        if (data.graphDS == "ds3-1") {
            QList<GraphValue> valueList;

            GraphValue graphValue;
            graphValue.valueY = 1;
            valueList << graphValue;

            graphValue.valueY = 2;
            valueList << graphValue;

            graphValue.valueY = 3;
            valueList << graphValue;

            data.valueList = valueList;
        }
        if (data.graphDS == "ds3-2") {
            QList<GraphValue> valueList;

            GraphValue graphValue;
            graphValue.valueY = 3;
            valueList << graphValue;

            graphValue.valueY = 2;
            valueList << graphValue;

            graphValue.valueY = 1;
            valueList << graphValue;

            data.valueList = valueList;
        }
        if (data.graphDS == "ds3-3") {
            QList<GraphValue> valueList;

            GraphValue graphValue;
            graphValue.valueY = 1;
            valueList << graphValue;

            graphValue.valueY = 1;
            valueList << graphValue;

            graphValue.valueY = 1;
            valueList << graphValue;

            data.valueList = valueList;
        }
        if (data.graphDS == "ds3-4") {
            QList<GraphValue> valueList;

            GraphValue graphValue;
            graphValue.valueY = 2;
            valueList << graphValue;

            graphValue.valueY = 2;
            valueList << graphValue;

            graphValue.valueY = 2;
            valueList << graphValue;

            data.valueList = valueList;
        }
        if (data.graphDS == "ds3-5") {
            QList<GraphValue> valueList;

            GraphValue graphValue;
            graphValue.valueY = 3;
            valueList << graphValue;

            graphValue.valueY = 3;
            valueList << graphValue;

            graphValue.valueY = 3;
            valueList << graphValue;

            data.valueList = valueList;
        }
        if (data.graphDS == "ds4") {
            QList<GraphValue> valueList;

            GraphValue graphValue;
            graphValue.valueY = 1;
            graphValue.caption = "Pie 1";
            valueList << graphValue;

            graphValue.valueY = 2;
            graphValue.caption = "Pie 2";
            valueList << graphValue;

            graphValue.valueY = 3;
            graphValue.caption = "Pie 3";
            valueList << graphValue;

            graphValue.valueY = 4;
            graphValue.caption = "Pie 4";
            valueList << graphValue;

            data.valueList = valueList;
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
