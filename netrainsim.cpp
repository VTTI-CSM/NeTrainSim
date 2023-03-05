/**
 * @file	C:\Users\Ahmed\OneDrive - Virginia
 * 			Tech\03.Work\02.VTTI\02.ResearchWork\01.TrainModelling\02.Code\00.CPP\NeTrainSim\NeTrainSim\netrainsim.cpp.
 *
 * Implements the netrainsim class
 */
#include "netrainsim.h"
#include "ui_netrainsim.h"
#include <iostream>
#include <QtAlgorithms>
#include "src/gui/numericdelegate.h"
#include "src/gui/IntNumericDelegate.h"
#include "src/gui/CheckboxDelegate.h"
#include <QtRPT.h>


NeTrainSim::NeTrainSim(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::NeTrainSim)
{
    // set up the layout
    ui->setupUi(this);

    setupGenerals();

    setupPage0();
    setupPage1();
    setupPage2();
    setupPage3();
    setupPage4();

}


void NeTrainSim::setupGenerals(){
    // next page button
    QObject::connect(ui->pushButton_projectNext, &QPushButton::clicked, [=]() {
        // switch to the next tab page if it is not the last page
        int nextIndex = ui->tabWidget_project->currentIndex() + 1;
        if (nextIndex < ui->tabWidget_project->count() - 1) {
            ui->tabWidget_project->setCurrentIndex(nextIndex);
        }
    });
    // change next page button text
    QObject::connect(ui->tabWidget_project, &QTabWidget::currentChanged, [=](int index) {
        // check if the last tab page is focused and update the button text accordingly
        if (index == ui->tabWidget_project->count() - 2) {
            ui->pushButton_projectNext->setText("Simulate");
        }
        else {
            ui->pushButton_projectNext->setText("Next");
        }
    });
}


void NeTrainSim::setupPage0(){

}


void NeTrainSim::setupPage1(){

    // disable viewing the axies
    ui->plot_createNetwork->xAxis->setVisible(false);
    ui->plot_createNetwork->yAxis->setVisible(false);

    // make the default show the old network only
    ui->widget_oldNetwork->show();
    ui->widget_newNetwork->hide();
    ui->verticalSpacer->changeSize(0,0, QSizePolicy::Expanding, QSizePolicy::Expanding);

    // connect the stateChanged signal of the QCheckBox object to a slot that will hide or show the layout
    QObject::connect(ui->checkBox_defineNewNetwork, &QCheckBox::stateChanged, [=](int state) {
        if (state == Qt::Checked) {
            // hide the layout if the checkbox is checked
            ui->widget_oldNetwork->hide();
            ui->widget_newNetwork->show();
            ui->verticalSpacer->changeSize(20,0, QSizePolicy::Fixed, QSizePolicy::Fixed);
            ui->plot_createNetwork->addGraph();
            ui->plot_createNetwork->addGraph();
        } else {
            // show the layout if the checkbox is unchecked
            ui->widget_oldNetwork->show();
            ui->widget_newNetwork->hide();
            ui->verticalSpacer->changeSize(20,0, QSizePolicy::Expanding, QSizePolicy::Expanding);
            if (ui->plot_createNetwork->graphCount() > 0){
                ui->plot_createNetwork->removeGraph(0);
            }
        }
    });

    ui->table_newNodes->setItemDelegateForColumn(1, new NumericDelegate);
    ui->table_newNodes->setItemDelegateForColumn(2, new NumericDelegate);

    // ---------- insert a new row to nodes ----------
    ui->table_newNodes->insertRow(0);
    // set the new id count as default value for the first cell of the new row
    std::unique_ptr<QTableWidgetItem> newItemID(new QTableWidgetItem(QString::number(0)));
    ui->table_newNodes->setItem(0, 0, newItemID.release());
    // set the default value for the description cell of the new row
    std::unique_ptr<QTableWidgetItem> newItemDesc(new QTableWidgetItem("ND"));
    ui->table_newNodes->setItem(0, 3, newItemDesc.release());


    // create a slot to add a new row to the QTableWidget
    auto addRowToNewNode = [=]() {
        // check if the last row has been edited
        if (ui->table_newNodes->currentRow() == ui->table_newNodes->rowCount() - 1) {
            // add a new row to the QTableWidget
            int newRow = ui->table_newNodes->rowCount();
            ui->table_newNodes->insertRow(newRow);

            // set the new id count as default value for the first cell of the new row
            std::unique_ptr<QTableWidgetItem> newItemID(new QTableWidgetItem(QString::number(newRow)));
            ui->table_newNodes->setItem(newRow, 0, newItemID.release());

            // set the default value for the description cell of the new row
            std::unique_ptr<QTableWidgetItem> newItemDesc(new QTableWidgetItem("ND"));
            ui->table_newNodes->setItem(newRow, 3, newItemDesc.release());
        }
    };


    // create a slot to update the QCustomPlot data and redraw the plot
    auto updateNodesPlot = [=]() {
        // clear the plot
        ui->plot_createNetwork->clearItems();

        // clear the networkNodes map
        this->networkNodes.clear();
        // get the QCPGraph object for the graph in the QCustomPlot
        QCPGraph *graph = ui->plot_createNetwork->graph(0);

        // get the data from the QTableWidget
        QVector<double> xData(ui->table_newNodes->rowCount(), qQNaN()), yData(ui->table_newNodes->rowCount(), qQNaN());
        QVector<QString> labels(ui->table_newNodes->rowCount(), "");
        for (int i = 0; i < ui->table_newNodes->rowCount(); i++) {
            // get the item at row 0 and column 0 of the table widget
            QTableWidgetItem* xItem = ui->table_newNodes->item(i, 1);
            QTableWidgetItem* yItem = ui->table_newNodes->item(i, 2);
            QTableWidgetItem* labelItem = ui->table_newNodes->item(i, 0);
            if (labelItem && xItem && yItem && !xItem->text().isEmpty() &&
                    !yItem->text().isEmpty() && !labelItem->text().isEmpty()){
                xData[i] = xItem->text().toDouble() * ui->doubleSpinBox_xCoordinate->value();
                yData[i] = yItem->text().toDouble() * ui->doubleSpinBox_yCoordinate->value();
                labels[i] = labelItem->text();
                this->networkNodes[labels[i]] = std::make_pair(xData[i], yData[i]);
            }
        }
        graph->setData(xData, yData);

        // calculate point size as a fraction of minimum plot width or height
        double pointSize = qMin(ui->plot_createNetwork->width(), ui->plot_createNetwork->height()) * 0.01;
        // set the scatter style to show only points
        QCPScatterStyle scatterStyle = graph->scatterStyle();
        scatterStyle.setShape(QCPScatterStyle::ssCircle);
        scatterStyle.setSize(pointSize);
        scatterStyle.setPen(QPen(Qt::red));
        scatterStyle.setBrush(QBrush(Qt::red));
        graph->setScatterStyle(scatterStyle);
        graph->setLineStyle(QCPGraph::lsNone);

        // Find the minimum and maximum values of the vector
        double xmin = *std::min_element(xData.begin(), xData.end());
        xmin = xmin - 0.1 * xmin;
        double xmax = *std::max_element(xData.begin(), xData.end());
        xmax = xmax + 0.1 * xmax;
        ui->plot_createNetwork->xAxis->setRangeLower(xmin);
        ui->plot_createNetwork->xAxis->setRangeUpper(xmax);

        // Find the minimum and maximum values of the vector
        double ymin = *std::min_element(yData.begin(), yData.end());
        ymin = ymin - 0.1 * ymin;
        double ymax = *std::max_element(yData.begin(), yData.end());
        ymax = ymax + 0.1 * ymax;
        // set the range of the y-axis
        ui->plot_createNetwork->yAxis->setRangeLower(ymin);
        ui->plot_createNetwork->yAxis->setRangeUpper(ymax);

        // qDeleteAll(labelsVector);
        // labelsVector.clear();
        // add labels for each point
        for (int i = 0; i < xData.size(); i++) {
            if (!qIsNaN(xData[i]) && !qIsNaN(yData[i])) {
                QCPItemText *label = new QCPItemText(ui->plot_createNetwork);
                label->setPositionAlignment(Qt::AlignLeft|Qt::AlignBottom);
                label->position->setType(QCPItemPosition::ptPlotCoords);
                label->position->setCoords(xData[i], yData[i]);
                label->setText(QString("Point %1").arg(labels[i]));
                label->setFont(QFont(font().family(), 10));
                label->setPen(QPen(Qt::NoPen));
                // labelsVector.append(label);
            }
        }

        ui->plot_createNetwork->replot();
    };


    // connect the cellChanged signal of the QTableWidget to the updatePlot slot
    QObject::connect(ui->table_newNodes, &QTableWidget::cellChanged, updateNodesPlot);
    QObject::connect(ui->doubleSpinBox_xCoordinate, &QDoubleSpinBox::valueChanged, updateNodesPlot);
    QObject::connect(ui->doubleSpinBox_yCoordinate, &QDoubleSpinBox::valueChanged, updateNodesPlot);

    // connect the cellChanged signal of the QTableWidget to the addRow slot
    QObject::connect(ui->table_newNodes, &QTableWidget::cellChanged, addRowToNewNode);
    //QObject::connect(ui->table_newNodes, &QTableWidget::keyPressEvent, deleteRow);


    ui->table_newLinks->setItemDelegateForColumn(1, new IntNumericDelegate());
    ui->table_newLinks->setItemDelegateForColumn(2, new IntNumericDelegate());
    ui->table_newLinks->setItemDelegateForColumn(3, new NumericDelegate());
    ui->table_newLinks->setItemDelegateForColumn(4, new NumericDelegate());
    ui->table_newLinks->setItemDelegateForColumn(5, new NumericDelegate());
    ui->table_newLinks->setItemDelegateForColumn(6, new CheckboxDelegate());
    ui->table_newLinks->setItemDelegateForColumn(7, new CheckboxDelegate());

    // ---------- insert a new row to nodes ----------
    ui->table_newLinks->insertRow(0);
    // set the new id count as default value for the first cell of the new row
    std::unique_ptr<QTableWidgetItem> newLinkItemID(new QTableWidgetItem(QString::number(0)));
    ui->table_newLinks->setItem(0, 0, newLinkItemID.release());
    // set the default value for the description cell of the new row
    std::unique_ptr<QTableWidgetItem> newLinkItemDesc(new QTableWidgetItem("ND"));
    ui->table_newLinks->setItem(0, 8, newLinkItemDesc.release());

    // create a slot to add a new row to the QTableWidget
    auto addRowToNewLinks = [=]() {
        // check if the last row has been edited
        if (ui->table_newLinks->currentRow() == ui->table_newLinks->rowCount() - 1) {
            // add a new row to the QTableWidget
            int newRow = ui->table_newLinks->rowCount();
            ui->table_newLinks->insertRow(newRow);

            // set the new id count as default value for the first cell of the new row
            std::unique_ptr<QTableWidgetItem> newItemID(new QTableWidgetItem(QString::number(newRow)));
            ui->table_newLinks->setItem(newRow, 0, newItemID.release());

            // set the default value for the description cell of the new row
            std::unique_ptr<QTableWidgetItem> newItemDesc(new QTableWidgetItem("ND"));
            ui->table_newLinks->setItem(newRow, 8, newItemDesc.release());
        }
    };
    QObject::connect(ui->table_newLinks, &QTableWidget::cellChanged, addRowToNewLinks);


    // create a slot to update the QCustomPlot data and redraw the plot
    auto updateLinksPlot = [=]() {
        // get the QCPGraph object for the graph in the QCustomPlot
        QCPGraph *graph = ui->plot_createNetwork->graph(1);

        // remove all values
        graph->setData(QVector<double>(), QVector<double>());

        // disable scatter style for the lines
        graph->setScatterStyle(QCPScatterStyle::ssNone);

        // get the data from the QTableWidget
        for (int i = 0; i < ui->table_newLinks->rowCount(); i++) {
            // get the item at row 0 and column 0 of the table widget
            QTableWidgetItem* fromItem = ui->table_newLinks->item(i, 1);
            QTableWidgetItem* toItem = ui->table_newLinks->item(i, 2);
            if (fromItem && toItem && !fromItem->text().isEmpty() &&
                    !toItem->text().isEmpty() ){
                if (!this->networkNodes.is_key(fromItem->text().trimmed())){ continue; }
                if (!this->networkNodes.is_key(toItem->text().trimmed())) { continue; }

                std::pair<double, double> fromNode = this->networkNodes[fromItem->text()];
                std::pair<double, double> toNode = this->networkNodes[toItem->text()];
                qDebug() << fromNode.first << fromNode.second << " to " << toNode.first << toNode.second;
                graph->addData(fromNode.first, fromNode.second);
                graph->addData(toNode.first, toNode.second);

            }
        }
        // set the pen style for the lines
        graph->setPen(QPen(Qt::blue, 2));

        // rescale the axes and redraw the plot
        // ui->plot_createNetwork->rescaleAxes();
        ui->plot_createNetwork->replot();
    };

    // connect the cellChanged signal of the QTableWidget to the updatePlot slot
    QObject::connect(ui->table_newLinks, &QTableWidget::cellChanged, updateLinksPlot);

}


void NeTrainSim::setupPage2(){

}


void NeTrainSim::setupPage3(){

}


void NeTrainSim::setupPage4(){

    // disable the results tab
    ui->tabWidget_project->setTabEnabled(4,false);


    QtRPT * report = new QtRPT(this);
    report->loadReport(":src/resources/report.xml");

    QPrinter *printer = new QPrinter(QPrinter::PrinterResolution);
    printer->setOutputFormat(QPrinter::PdfFormat);

    QPageLayout pageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF(0, 0, 0, 0));
    printer->setPageLayout(pageLayout);
    printer->setFullPage(true);

    // connect the print preview to the report
    connect(ui->widget_SummaryReport, SIGNAL(paintRequested(QPrinter*)), report, SLOT(printPreview(QPrinter*)));

    auto popup = [=]() {
        report->printExec();
    };

    QObject::connect(ui->pushButton_popoutPreview, &QPushButton::clicked, popup);
}


NeTrainSim::~NeTrainSim()
{
    delete ui;
//    qDeleteAll(labelsVector);
//    labelsVector.clear();
}

