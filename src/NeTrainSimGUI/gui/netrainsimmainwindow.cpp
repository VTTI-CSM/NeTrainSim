/**
 * @file	~\NeTrainSim\netrainsim.cpp.
 *
 * Implements the netrainsim class
 */
#include "netrainsimmainwindow.h"
#include "nonemptydelegate.h"
#include "simulationworker.h"
#include "../NeTrainSim/util/csvmanager.h"
#include "ui_netrainsimmainwindow.h"
#include <iostream>
#include <QtAlgorithms>
#include "numericdelegate.h"
#include "intnumericdelegate.h"
#include "checkboxdelegate.h"
#include <qtrpt.h>
#include <fstream>
#include "../NeTrainSim/traindefinition/trainslist.h"
#include "aboutwindow.h"
#include "../NeTrainSim/util/xmlmanager.h"
#include "util/errorhandler.h"
#include "../NeTrainSim/network/readwritenetwork.h"


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

    ui->progressBar->setTextVisible(true);
    ui->progressBar->setAlignment(Qt::AlignCenter);
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setFormat("%p%");

    // show about window
    connect(ui->actionAbout, &QAction::triggered, [this](){
        if (this->aboutWindow == nullptr) {
            this->aboutWindow = std::make_shared<AboutWindow>(this);
        }
        if (aboutWindow != nullptr) { // Check if the initialization was successful
            this->aboutWindow->show();
        }
    });

    // show the manual
    connect(ui->actionOpen_Manual, &QAction::triggered, [this]() {
        QString executablePath = QCoreApplication::applicationDirPath();
        QString fileName = QDir(executablePath).filePath("Manual.pdf");
        QFile pfn(fileName);
        if (!pfn.exists()) {
            this->showWarning("File does not exist!");
            return;
        }

        QUrl fileUrl = QUrl::fromLocalFile(fileName);
        if (! QDesktopServices::openUrl(fileUrl)) {
            this->showWarning("Failed to open the PDF file!");
        }
    });

    // ########################################################################
    // ########## Connect signals and slots for project management ############
    // ########################################################################

    // create a new project and clear the form
    connect(ui->actionNew_Project, &QAction::triggered, this, &NeTrainSim::clearForm);

    // save the project file
    connect(ui->actionSave, &QAction::triggered, [this](){
        this->saveProjectFile();
    });

    // save as the project file
    connect(ui->actionSave_As, &QAction::triggered, [this](){
        this->saveProjectFile(true);
    });

    // load a project file
    connect(ui->actionOpen_a_project, &QAction::triggered, [this](){
        QString fname = QFileDialog::getOpenFileName(nullptr, "Open NeTrainSim Project File", "", "NeTrainSim Files (*.NTS)");
        this->loadProjectFiles(fname);
    });

    // close the application when the exit app is clicked
    connect(ui->actionExit, &QAction::triggered, this, &NeTrainSim::closeApplication);

    // open the sample project
    connect(ui->actionLoad_Sample_Project, &QAction::triggered, this, &NeTrainSim::handleSampleProject);

    // define the next page and simulate buttons
    QObject::connect(ui->pushButton_projectNext, &QPushButton::clicked, [=]() {
        // switch to the next tab page if it is not the last page
        int nextIndex = ui->tabWidget_project->currentIndex() + 1;
        if (nextIndex < ui->tabWidget_project->count() - 1) {
            ui->tabWidget_project->setCurrentIndex(nextIndex);
        }
        if (nextIndex == ui->tabWidget_project->count() - 1) {
            this->simulate();
        }
    });

    // change next page button text
    QObject::connect(ui->tabWidget_project, &QTabWidget::currentChanged, [=](int index) {
        // check if the last tab page is focused and update the button text accordingly
        if (index == ui->tabWidget_project->count() - 2) {
            ui->pushButton_projectNext->setText("Simulate");
            ui->pushButton_projectNext->setVisible(true);
        }
        else if (index <= ui->tabWidget_project->count() - 2) {
            ui->pushButton_projectNext->setText("Next");
            ui->pushButton_projectNext->setVisible(true);
        }
        else {
            ui->pushButton_projectNext->setVisible(false);
        }
    });

    // replot the nodes to all plots if the nodes data has changed
    connect(this, &NeTrainSim::nodesDataChanged, [this]() {
        this->updateNodesPlot(*(ui->plot_createNetwork), this->nodesXData, this->nodesYData, this->nodesLabelData);
        this->updateNodesPlot(*(ui->plot_trains), this->nodesXData, this->nodesYData, this->nodesLabelData);
        //this->updateNodesPlot(*(ui->plot_simulation), this->nodesXData, this->nodesYData, this->nodesLabelData);
    });

    // replote the links to all plots if the links data has changed
    connect(this, &NeTrainSim::linksDataChanged, [this]() {
        this->updateLinksPlot(*(ui->plot_createNetwork), this->linksStartNodeIDs, this->linksEndNodeIDs);
        this->updateLinksPlot(*(ui->plot_trains), this->linksStartNodeIDs, this->linksEndNodeIDs);
        this->updateLinksPlot(*(ui->plot_simulation), this->linksStartNodeIDs, this->linksEndNodeIDs);
    });

    // show error if tables has only 1 row
    connect(ui->table_newNodes, &CustomTableWidget::cannotDeleteRow, [this]() {
        this->showWarning("Cannot delete the first row!");
    });
    connect(ui->table_newLinks, &CustomTableWidget::cannotDeleteRow, [this]() {
        this->showWarning("Cannot delete the first row!");
    });
    connect(ui->table_newLocomotive, &CustomTableWidget::cannotDeleteRow, [this]() {
        this->showWarning("Cannot delete the first row!");
    });
    connect(ui->table_newCar, &CustomTableWidget::cannotDeleteRow, [this]() {
        this->showWarning("Cannot delete the first row!");
    });
    connect(ui->table_newConfiguration, &CustomTableWidget::cannotDeleteRow, [this]() {
        this->showWarning("Cannot delete the first row!");
    });
    connect(ui->table_newTrain, &CustomTableWidget::cannotDeleteRow, [this]() {
        this->showWarning("Cannot delete the first row!");
    });
}


void NeTrainSim::setupPage0(){

}


void NeTrainSim::setupPage1(){

    // make the default show the old network only
    ui->widget_oldNetwork->show();
    ui->widget_newNetwork->hide();

    // add graphs to the plot
    ui->plot_createNetwork->addGraph();
    ui->plot_createNetwork->addGraph();
    // disable viewing the axies
    ui->plot_createNetwork->xAxis->setVisible(false);
    ui->plot_createNetwork->yAxis->setVisible(false);


    // get the nodes file
    connect(ui->pushButton_nodes, &QPushButton::clicked, [this]() {
        nodesFilename = this->browseFiles(ui->lineEdit_nodes, "Select Nodes File");
    });

    // get the links file
    connect(ui->pushButton_links, &QPushButton::clicked, [this]() {
        linksFilename = this->browseFiles(ui->lineEdit_links, "Select Links File");
    });

    // read the nodes file
    connect(ui->lineEdit_nodes, &QLineEdit::textChanged, [this]() {
        auto out = this->getNodesDataFromNodesFile(ui->lineEdit_nodes->text());
        auto plottableout = this->getNodesPlottableData(out);
        this->setNodesData(std::get<0>(plottableout),std::get<1>(plottableout), std::get<2>(plottableout));
    });

    // read the links file
    connect(ui->lineEdit_links, &QLineEdit::textChanged, [this]() {
        auto out = this->getLinkesDataFromLinksFile(ui->lineEdit_links->text());
        auto plottableOut = this->getLinksPlottableData(out);
        this->setLinksData(std::get<0>(plottableOut),std::get<1>(plottableOut));
    });

    // save the table nodes to DAT file
    connect(ui->pushButton_saveNewNodes, &QPushButton::clicked, [this](){
        // Open a file dialog to choose the save location
        QString saveFilePath = QFileDialog::getSaveFileName(this, "Save Nodes File", QDir::homePath(),  "DAT Files (*.DAT)");

        if (!saveFilePath.isEmpty()) {
            // read the table data
            auto out = this->getNodesDataFromNodesTable();
            std::string filename = saveFilePath.toStdString();
            // write the nodes file
            if (ReadWriteNetwork::writeNodesFile(out, filename)) {
                nodesFilename = saveFilePath;
                showNotification("File Saved Successfully");
            }
            else {
                showWarning("Could not save the file!");
            }
        }
    });

    // save the table links to DAT file
    connect(ui->pushButton_saveNewLinks, &QPushButton::clicked, [this](){
        // Open a file dialog to choose the save location
        QString saveFilePath = QFileDialog::getSaveFileName(this, "Save Links File", QDir::homePath(),  "DAT Files (*.DAT)");

        if (!saveFilePath.isEmpty()) {
            // get the links data from table
            auto out = this->getLinkesDataFromLinksTable();
            std::string filename = saveFilePath.toStdString();
            // save the links file
            if (ReadWriteNetwork::writeLinksFile(out, filename)) {
                showNotification("File Saved Successfully");
                linksFilename = saveFilePath;
            }
            else {
                showWarning("Could not save the file!");
            }
        }
    });


    // connect the stateChanged signal of the QCheckBox object to a slot that will hide or show the layout
    QObject::connect(ui->checkBox_defineNewNetwork, &QCheckBox::stateChanged, [=](int state) {
        if (state == Qt::Checked) {
            // hide the layout if the checkbox is checked
            ui->widget_oldNetwork->hide();
            ui->widget_newNetwork->show();

            // clear the lineEdit for nodes and links
            // this will trigger all the plots to clear their data as well
            ui->lineEdit_nodes->setText("");
            ui->lineEdit_links->setText("");
        } else {
            // show the layout if the checkbox is unchecked
            ui->widget_oldNetwork->show();
            ui->widget_newNetwork->hide();
        }
    });

// --------------------------------------------------------------------------
// ------------------------- Nodes Table ------------------------------------
// --------------------------------------------------------------------------

    // add the first row
    this->setupNodesTable();

    // create a slot to add a new row to the QTableWidget
    auto addRowToNewNode = [=]() {
        // check if the last row has been edited
        if (ui->table_newNodes->currentRow() == ui->table_newNodes->rowCount() - 1) {
            // add a new row to the QTableWidget
            int newRow = ui->table_newNodes->rowCount();
            ui->table_newNodes->insertRow(newRow);

            // set the new id count as default value for the first cell of the new row
            int uniqueID = ui->table_newNodes->generateUniqueID();
            std::unique_ptr<QTableWidgetItem> newItemID(new QTableWidgetItem(QString::number(uniqueID)));
            ui->table_newNodes->setItem(newRow, 0, newItemID.release());

            // set the default value for the description cell of the new row
            std::unique_ptr<QTableWidgetItem> newItemDesc(new QTableWidgetItem("ND"));
            ui->table_newNodes->setItem(newRow, 3, newItemDesc.release());
        }
    };

    // update the nodes plot everytime u edit the nodes table
     auto updatetheNodesPlot = [=]() {
        auto out = this->getNodesDataFromNodesTable();
        auto plottableOut = this->getNodesPlottableData(out);
        // update the plotted data
        this->setNodesData(std::get<0>(plottableOut), std::get<1>(plottableOut), std::get<2>(plottableOut));
    };

    // connect the cellChanged signals of the QTableWidget to the updatePlot slot
    QObject::connect(ui->table_newNodes, &QTableWidget::cellChanged, updatetheNodesPlot);
    QObject::connect(ui->doubleSpinBox_xCoordinate, &QDoubleSpinBox::valueChanged, updatetheNodesPlot);
    QObject::connect(ui->doubleSpinBox_yCoordinate, &QDoubleSpinBox::valueChanged, updatetheNodesPlot);

    // connect the cellChanged signal of the QTableWidget to the addRow slot
    QObject::connect(ui->table_newNodes, &QTableWidget::cellChanged, addRowToNewNode);
    //QObject::connect(ui->table_newNodes, &QTableWidget::keyPressEvent, deleteRow);

// --------------------------------------------------------------------------
// ------------------------- Links Table ------------------------------------
// --------------------------------------------------------------------------

    // add the first row
    this->setupLinksTable();

    // create a slot to add a new row to the QTableWidget
    auto addRowToNewLinks = [=]() {
        // check if the last row has been edited
        if (ui->table_newLinks->currentRow() == ui->table_newLinks->rowCount() - 1) {
            // add a new row to the QTableWidget
            int newRow = ui->table_newLinks->rowCount();
            ui->table_newLinks->insertRow(newRow);

            // set the new id count as default value for the first cell of the new row
            int uniqueID = ui->table_newLinks->generateUniqueID();
            std::unique_ptr<QTableWidgetItem> newItemID(new QTableWidgetItem(QString::number(uniqueID)));
            ui->table_newLinks->setItem(newRow, 0, newItemID.release());

            ui->table_newLinks->setupTable();
        }
    };
    // add a row to the links table everytime you edit the last row cell
    QObject::connect(ui->table_newLinks, &QTableWidget::cellChanged, addRowToNewLinks);


    // update the links data for the plot
    auto updateLinksPlot = [=]() {
        auto out = this->getLinkesDataFromLinksTable();
        auto plottableOut = this->getLinksPlottableData(out);
        this->setLinksData(std::get<0>(plottableOut), std::get<1>(plottableOut));
    };

    // connect the cellChanged signal of the QTableWidget to the updatePlot slot
    QObject::connect(ui->table_newLinks, &QTableWidget::cellChanged, updateLinksPlot);

}


void NeTrainSim::setupPage2(){
    // disable viewing the axies
    ui->plot_trains->xAxis->setVisible(false);
    ui->plot_trains->yAxis->setVisible(false);

    // show the layout as a default
    ui->widget_oldTrainOD->show();
    ui->widget_newTrainOD->hide();

    // get the trains file
    connect(ui->pushButton_trains, &QPushButton::clicked, [this]() {
        trainsFilename = this->browseFiles(ui->lineEdit_trains, "Select Trains File");
    });

    // write the trains file
    connect(ui->pushButton_saveNewTrains, &QPushButton::clicked, [this](){
        // Open a file dialog to choose the save location
        QString saveFilePath = QFileDialog::getSaveFileName(this, "Save Trains File", QDir::homePath(), "DAT Files (*.DAT)");

        if (!saveFilePath.isEmpty()) {
            Vector<tuple<std::string, Vector<int>, double, double,
                         Vector<tuple<double, double, double, double, double, double, int, int>>,
                         Vector<tuple<double, double, double, double, double, int, int>>,
                         bool>> out;
            try {
                out = this->getTrainsDataFromTables();
            } catch (const std::exception& e) {
                ErrorHandler::showError(e.what());
                return;
            }

            std::string filename = saveFilePath.toStdString();
            if (TrainsList::writeTrainsFile(out, filename)) {
                showNotification("Trains file saved successfully!");
                trainsFilename = saveFilePath;
            }
            else {
                showWarning("Could not save the file!");
            }
        }
    });


    ui->verticalSpacer_train->changeSize(0,0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    // connect the stateChanged signal of the QCheckBox object to a slot that will hide or show the layout
    QObject::connect(ui->checkBox_TrainsOD, &QCheckBox::stateChanged, [=](int state) {
        if (state == Qt::Checked) {
            // hide the layout if the checkbox is checked
            ui->widget_oldTrainOD->hide();
            ui->widget_newTrainOD->show();
            ui->verticalSpacer_train->changeSize(20,0, QSizePolicy::Fixed, QSizePolicy::Fixed);
            ui->plot_trains->addGraph();
            ui->plot_trains->addGraph();
            QObject::connect(ui->plot_trains, &CustomPlot::pointLeftSelected, this, &NeTrainSim::trainPointSelected);
            QObject::connect(ui->plot_trains, &CustomPlot::pointRightSelected, this, &NeTrainSim::trainPointDeleted);
            this->updateNodesPlot(*(ui->plot_trains), this->nodesXData, this->nodesYData, this->nodesLabelData);
            this->updateLinksPlot(*(ui->plot_trains), this->linksStartNodeIDs, this->linksEndNodeIDs);
        } else {
            // show the layout if the checkbox is unchecked
            ui->widget_oldTrainOD->show();
            ui->widget_newTrainOD->hide();
            ui->verticalSpacer_train->changeSize(20,0, QSizePolicy::Expanding, QSizePolicy::Expanding);
            if (ui->plot_trains->graphCount() > 0){
                ui->plot_trains->removeGraph(0);
            }
        }
    });


// --------------------------------------------------------------------------
// ---------------------- Locomotives Table ---------------------------------
// --------------------------------------------------------------------------

    // add the first row
    this->setupLocomotivesTable();


    // create a slot to add a new row to the QTableWidget
    auto addRowToNewLocomotives = [=]() {
        // check if the last row has been edited
        if (ui->table_newLocomotive->currentRow() == ui->table_newLocomotive->rowCount() - 1) {
            // add a new row to the QTableWidget
            int newRow = ui->table_newLocomotive->rowCount();
            ui->table_newLocomotive->insertRow(newRow);

            // set the new id count as default value for the first cell of the new row
            int uniqueID = ui->table_newLocomotive->generateUniqueID();
            std::unique_ptr<QTableWidgetItem> newItemID(new QTableWidgetItem(QString::number(uniqueID)));
            ui->table_newLocomotive->setItem(newRow, 0, newItemID.release());

            // Create a new combobox and set it as the item in the last column of the new row
            QComboBox* comboBox_locomotives_newRow = new QComboBox;
            // Add items to the combobox
            for (auto locType: TrainTypes::powerTypeStrings) {
                comboBox_locomotives_newRow->addItem(QString::fromStdString(Utils::removeLastWord(locType)));
            }
            ui->table_newLocomotive->setCellWidget(newRow, ui->table_newLocomotive->columnCount() - 1, comboBox_locomotives_newRow);
        }
    };
    // add a new row everytime the last row cells are edited
    QObject::connect(ui->table_newLocomotive, &QTableWidget::cellChanged, addRowToNewLocomotives);


// --------------------------------------------------------------------------
// -------------------------- Car Table -------------------------------------
// --------------------------------------------------------------------------

    // add first row
    this->setupCarsTable();

    // create a slot to add a new row to the QTableWidget
    auto addRowToNewCars = [=]() {
        // check if the last row has been edited
        if (ui->table_newCar->currentRow() == ui->table_newCar->rowCount() - 1) {
            // add a new row to the QTableWidget
            int newRow = ui->table_newCar->rowCount();
            ui->table_newCar->insertRow(newRow);

            // set the new id count as default value for the first cell of the new row
            int uniqueID = ui->table_newCar->generateUniqueID();
            std::unique_ptr<QTableWidgetItem> newItemID(new QTableWidgetItem(QString::number(uniqueID)));
            ui->table_newCar->setItem(newRow, 0, newItemID.release());

            // Create a new combobox and set it as the item in the last column of the new row
            QComboBox* comboBoxcomboBox_cars_newRow = new QComboBox;
            // Add items to the combobox
            for (auto carType: TrainTypes::carTypeStrings) {
                comboBoxcomboBox_cars_newRow->addItem(QString::fromStdString(carType));
            }
            ui->table_newCar->setCellWidget(newRow, ui->table_newCar->columnCount() - 1, comboBoxcomboBox_cars_newRow);
        }
    };
    // add a new row everytime the last row cells are edited
    QObject::connect(ui->table_newCar, &QTableWidget::cellChanged, addRowToNewCars);

// --------------------------------------------------------------------------
// --------------------- Configurations Table -------------------------------
// --------------------------------------------------------------------------

    this->setupConfigurationsTable();

    // create a slot to add a new row to the QTableWidget
    auto addRowToNewConfig = [=]() {
        // check if the last row has been edited
        if (ui->table_newConfiguration->currentRow() == ui->table_newConfiguration->rowCount() - 1) {
            // add a new row to the QTableWidget
            int newRow = ui->table_newConfiguration->rowCount();
            ui->table_newConfiguration->insertRow(newRow);

            // set the new id count as default value for the first cell of the new row
            std::unique_ptr<QTableWidgetItem> newItemID(new QTableWidgetItem(QString::number(0)));
            ui->table_newConfiguration->setItem(newRow, 0, newItemID.release());

            // Create a new combobox and set it as the item in the last column of the new row
            QComboBox* comboBoxcomboBox_config_newRow = new QComboBox;
            // Add items to the combobox
            comboBoxcomboBox_config_newRow->addItem("Locomotive");
            comboBoxcomboBox_config_newRow->addItem("Car");
            ui->table_newConfiguration->setCellWidget(newRow, 1, comboBoxcomboBox_config_newRow);

            // create a spinbox for the number of instances
            QSpinBox* spinBox_config_instances_newRow = new QSpinBox;
            spinBox_config_instances_newRow->setMinimum(1);
            spinBox_config_instances_newRow->setSingleStep(1);
            ui->table_newConfiguration->setCellWidget(newRow, 3, spinBox_config_instances_newRow);
        }
    };

    // add a new row everytime the last row cells are edited
    QObject::connect(ui->table_newConfiguration, &QTableWidget::cellChanged, addRowToNewConfig);


// --------------------------------------------------------------------------
// ------------------------- Trains Table -----------------------------------
// --------------------------------------------------------------------------


    QObject::connect(ui->table_newTrain, &QTableWidget::cellChanged, this, &NeTrainSim::updateCombo_visualizeTrains);

    this->setupTrainsTable();

    // create a slot to add a new row to the QTableWidget
    auto addRowToNewTrain = [=]() {
        // check if the last row has been edited
        if (ui->table_newTrain->currentRow() == ui->table_newTrain->rowCount() - 1) {
            // add a new row to the QTableWidget
            int newRow = ui->table_newTrain->rowCount();
            ui->table_newTrain->insertRow(newRow);

            // set the new id count as default value for the first cell of the new row
            int uniqueID = ui->table_newTrain->generateUniqueID();
            std::unique_ptr<QTableWidgetItem> newItemID(new QTableWidgetItem(QString::number(uniqueID)));
            ui->table_newTrain->setItem(newRow, 0, newItemID.release());

        }
    };
    // add a new row everytime the last row cells are edited
    QObject::connect(ui->table_newTrain, &QTableWidget::cellChanged, addRowToNewTrain);

}


void NeTrainSim::setupPage3(){
    // disable viewing the axies
    ui->plot_simulation->xAxis->setVisible(false);
    ui->plot_simulation->yAxis->setVisible(false);

    // add graphs to the plot
    auto nodegraph = ui->plot_simulation->addGraph();
    auto linksgraph = ui->plot_simulation->addGraph();

    // remove the nodes and links graphs from legends
    ui->plot_simulation->legend->removeItem(0);
    ui->plot_simulation->legend->removeItem(0);

    // make the trajectory lineedit not visible
    ui->horizontalWidget_TrajFile->setVisible(false);

    // select the output location
    connect(ui->pushButton_selectoutputPath, &QPushButton::clicked, [this]() {
        this->browseFolder(ui->lineEdit_outputPath, "Select the output path");
    });

    connect(ui->checkBox_exportTrajectory, &QCheckBox::stateChanged, [this]() {
        ui->horizontalWidget_TrajFile->setVisible(ui->checkBox_exportTrajectory->checkState() == Qt::Checked);
    });

}


void NeTrainSim::setupPage4(){
    // disable the results tab
    ui->tabWidget_project->setTabEnabled(4,false);
}

void NeTrainSim::trainPointSelected(QPointF selectedPoint) {
    // Check if the selected point is not null
    if (!(std::isnan(selectedPoint.x()) && std::isnan(selectedPoint.y()))) {
        // Iterate over the network nodes
        for (auto& record: this->networkNodes) {
            // Check if the network node matches the selected point
            if (record.second.first == selectedPoint.x() &&
                record.second.second == selectedPoint.y()) {

                // Iterate over the table rows
                for (int i = 0; i < ui->table_newTrain->rowCount(); i++) {
                    // Check if the cell at column 0 exists, otherwise skip to the next row
                    if (!ui->table_newTrain->item(i, 0)) {
                        continue;
                    }

                    // Check if the row corresponds to the currently selected item in the combo box
                    if (ui->table_newTrain->item(i, 0)->text().trimmed() == ui->combo_visualizeTrain->currentText()) {
                        // Check if the cell at column 2 exists
                        if (ui->table_newTrain->item(i, 2)) {
                            // Get the existing value in the table cell
                            QString alreadyThere = ui->table_newTrain->item(i, 2)->text();

                            // Split the existing value into parts using comma as the delimiter
                            QStringList parts = alreadyThere.split(",");

                            // Get the value to be added
                            QString newValue = record.first;

                            // Check if the parts list does not contain the value
                            if (!parts.contains(newValue)) {
                                // Append the value to the existing value
                                parts.push_back(newValue);
                                alreadyThere = parts.join(',');

                                // Show the selected point ID on the plot
                                QCPItemText *label = new QCPItemText(ui->plot_trains);
                                label->setPositionAlignment(Qt::AlignLeft|Qt::AlignBottom);
                                label->position->setType(QCPItemPosition::ptPlotCoords);
                                label->position->setCoords(record.second.first, record.second.second);
                                label->setText(QString("Point %1").arg(record.first));
                                label->setFont(QFont(font().family(), 10));
                                label->setPen(QPen(Qt::NoPen));

                            }

                            // Update the table cell with the modified string
                            ui->table_newTrain->item(i, 2)->setText(alreadyThere);
                        } else {
                            // Create a new QTableWidgetItem with the value
                            QTableWidgetItem* item = new QTableWidgetItem(record.first);
                            ui->table_newTrain->setItem(i, 2, item);

                            // Show the selected point ID on the plot
                            QCPItemText *label = new QCPItemText(ui->plot_trains);
                            label->setPositionAlignment(Qt::AlignLeft|Qt::AlignBottom);
                            label->position->setType(QCPItemPosition::ptPlotCoords);
                            label->position->setCoords(record.second.first, record.second.second);
                            label->setText(QString("Point %1").arg(record.first));
                            label->setFont(QFont(font().family(), 10));
                            label->setPen(QPen(Qt::NoPen));
                        }



                        // Call replot to update the plot
                        ui->plot_trains->replot();
                    }
                }
            }
        }
    }
}


void NeTrainSim::trainPointDeleted(QPointF selectedPoint) {
    // Find the label associated with the selected point
    auto label = this->findLabelByPosition(ui->plot_trains, selectedPoint);

    // Check if a label was found
    if (label) {
        // Iterate over the network nodes
        for (auto& record: this->networkNodes) {
            // Check if the network node matches the selected point
            if (record.second.first == selectedPoint.x() &&
                record.second.second == selectedPoint.y()) {

                // Iterate over the table rows
                for (int i = 0; i < ui->table_newTrain->rowCount(); i++) {
                    // Check if the row corresponds to the currently selected item in the combo box
                    if (ui->table_newTrain->item(i, 0)->text().trimmed() == ui->combo_visualizeTrain->currentText()) {
                        // Get the existing value in the table cell
                        QString alreadyThere = ui->table_newTrain->item(i, 2)->text();

                        // Split the existing value into parts using comma as the delimiter
                        QStringList parts = alreadyThere.split(",");

                        // Get the value to be removed
                        QString oldValue = record.first;

                        // Check if the parts list contains the value to be removed
                        if (parts.contains(oldValue)) {
                            // Remove the value from the parts list
                            parts.removeOne(oldValue);

                            // Reconstruct the string by joining the remaining parts with commas
                            alreadyThere = parts.join(",");
                        }

                        // Update the table cell with the modified string
                        ui->table_newTrain->item(i, 2)->setText(alreadyThere);
                    }
                }

            }
        }
        ui->plot_trains->removeItem(label);
        ui->plot_trains->replot();
    }
}


// update the combo_visualizeTrain
void NeTrainSim::updateCombo_visualizeTrains() {
    // Clear the current items in the combobox
    ui->combo_visualizeTrain->clear();

    // Iterate through each row in the table and add the column value to the combobox
    for (int row = 0; row < ui->table_newTrain->rowCount(); ++row) {
        QTableWidgetItem* item = ui->table_newTrain->item(row, 0);
        if (item) {
            ui->combo_visualizeTrain->addItem(item->text());
        }
    }

    connect(ui->combo_visualizeTrain, &QComboBox::currentIndexChanged, [this](){
        // remove all labels in the plot of the old train
        // Iterate over the items in the plot
        for (int i = 0; i < ui->plot_trains->itemCount(); ++i) {
            QCPAbstractItem* abstractItem = ui->plot_trains->item(i);

            // Check if the item is a QCPItemText
            if (QCPItemText* label = qobject_cast<QCPItemText*>(abstractItem)) {
                ui->plot_trains->removeItem(label);
            }
        }

        // add already selected nodes to the plot
        for (int i = 0; i < ui->table_newTrain->rowCount(); i++) {
            if (ui->combo_visualizeTrain->count() == 0) { continue; }
            // Check if the row corresponds to the currently selected item in the combo box
            if (ui->table_newTrain->item(i, 0)->text().trimmed() == ui->combo_visualizeTrain->currentText()) {
                // Check if the cell at column 2 exists
                if (ui->table_newTrain->item(i, 2)) {
                    // Get the existing value in the table cell
                    QString alreadyThere = ui->table_newTrain->item(i, 2)->text();

                    // Split the existing value into parts using comma as the delimiter
                    QStringList parts = alreadyThere.split(",");

                    for (const auto &nodeID: parts) {
                        if (!this->networkNodes.is_key(nodeID)) { continue; }
                        auto record = this->networkNodes[nodeID];
                        // Show the selected point ID on the plot
                        QCPItemText *label = new QCPItemText(ui->plot_trains);
                        label->setPositionAlignment(Qt::AlignLeft|Qt::AlignBottom);
                        label->position->setType(QCPItemPosition::ptPlotCoords);
                        label->position->setCoords(record.first, record.second);
                        label->setText(QString("Point %1").arg(nodeID));
                        label->setFont(QFont(font().family(), 10));
                        label->setPen(QPen(Qt::NoPen));
                    }
                }

                // Call replot to update the plot
                ui->plot_trains->replot();
            }
        }



    });
};

void NeTrainSim::setupNodesTable() {
    // add the delegates to the nodes columns
    ui->table_newNodes->setItemDelegateForColumn(0, new NonEmptyDelegate(this));
    ui->table_newNodes->setItemDelegateForColumn(1, new NumericDelegate(this, 9999999999999999.999, -9999999999999999.999,3, 1, 0.0));
    ui->table_newNodes->setItemDelegateForColumn(2, new NumericDelegate(this, 9999999999999999.999, -9999999999999999.999,3, 1, 0.0));

    // ---------- insert a new row to nodes ----------
    ui->table_newNodes->insertRow(0);
    // set the new id count as default value for the first cell of the new row
    std::unique_ptr<QTableWidgetItem> newItemID(new QTableWidgetItem(QString::number(0)));
    ui->table_newNodes->setItem(0, 0, newItemID.release());
    // set the default value for the description cell of the new row
    std::unique_ptr<QTableWidgetItem> newItemDesc(new QTableWidgetItem("ND"));
    ui->table_newNodes->setItem(0, 3, newItemDesc.release());
}

void NeTrainSim::setupLinksTable() {
    // add the delegates to the links columns
    ui->table_newLinks->setItemDelegateForColumn(0, new NonEmptyDelegate(this));
    ui->table_newLinks->setItemDelegateForColumn(1, new IntNumericDelegate(this, 100000000000, 0, 1, 0));
    ui->table_newLinks->setItemDelegateForColumn(2, new IntNumericDelegate(this, 100000000000, 0, 1, 0));
    ui->table_newLinks->setItemDelegateForColumn(3, new NumericDelegate(this, 150, 5, 2, 5, 55));
    ui->table_newLinks->setItemDelegateForColumn(4, new IntNumericDelegate(this, 10000000000, 0, 1, 0));
    ui->table_newLinks->setItemDelegateForColumn(5, new NumericDelegate(this, 5, -5, 3, 0.05, 0));
    ui->table_newLinks->setItemDelegateForColumn(6, new NumericDelegate(this, 5, -5, 3, 0.05, 0));
    ui->table_newLinks->setItemDelegateForColumn(7, new CheckboxDelegate());
    ui->table_newLinks->setItemDelegateForColumn(8, new NumericDelegate(this, 1, 0, 2, 0.05, 0.2));
    ui->table_newLinks->setItemDelegateForColumn(9, new CheckboxDelegate());

    // ---------- insert a new row to nodes ----------
    ui->table_newLinks->insertRow(0);
    ui->table_newLinks->setupTable();

    // set the new id count as default value for the first cell of the new row
    std::unique_ptr<QTableWidgetItem> newLinkItemID(new QTableWidgetItem(QString::number(0)));
    ui->table_newLinks->setItem(0, 0, newLinkItemID.release());
}

void NeTrainSim::setupLocomotivesTable() {
    //set delegates for the locomotives table ID
    ui->table_newLocomotive->setItemDelegateForColumn(0, new NonEmptyDelegate(this)); // ID
    ui->table_newLocomotive->setItemDelegateForColumn(1, new NumericDelegate(this, 10000, 100, 2, 100, 3000)); // power
    ui->table_newLocomotive->setItemDelegateForColumn(2, new NumericDelegate(this, 1, 0, 2, 0.05, 0.85)); // transmission eff
    ui->table_newLocomotive->setItemDelegateForColumn(3, new NumericDelegate(this, 100, 0, 2, 1, 25)); // length
    ui->table_newLocomotive->setItemDelegateForColumn(4, new NumericDelegate(this, 0.01, 0, 5, 0.0001, 0.0055)); // streamline
    ui->table_newLocomotive->setItemDelegateForColumn(5, new NumericDelegate(this, 100, 0, 2, 1, 15)); // area
    ui->table_newLocomotive->setItemDelegateForColumn(6, new NumericDelegate(this, 400, 0, 2, 10, 150)); // weight
    ui->table_newLocomotive->setItemDelegateForColumn(7, new IntNumericDelegate(this, 12, 2)); //number of axles

    // ---------- insert a new row to locomotives ----------
    ui->table_newLocomotive->insertRow(0);

    // set the new id count as default value for the first cell of the new row
    std::unique_ptr<QTableWidgetItem> newItemID(new QTableWidgetItem(QString::number(0)));
    ui->table_newLocomotive->setItem(0, 0, newItemID.release());
    // Create a new combobox and set it as the item in the last column of the new row
    QComboBox* comboBox_locomotives = new QComboBox;
    // Add items to the combobox
    for (auto locType: TrainTypes::powerTypeStrings) {
        comboBox_locomotives->addItem(QString::fromStdString(Utils::removeLastWord(locType)));
    }
    ui->table_newLocomotive->setCellWidget(0, ui->table_newLocomotive->columnCount() - 1, comboBox_locomotives);
}

void NeTrainSim::setupCarsTable() {
    // set the delegates for the cars table ID
    ui->table_newCar->setItemDelegateForColumn(0, new NonEmptyDelegate(this));
    ui->table_newCar->setItemDelegateForColumn(1, new NumericDelegate(this, 100, 0, 2, 1, 25)); // length
    ui->table_newCar->setItemDelegateForColumn(2, new NumericDelegate(this, 0.01, 0, 5, 0.0001, 0.0055)); // streamline
    ui->table_newCar->setItemDelegateForColumn(3, new NumericDelegate(this, 100, 0, 2, 1, 15)); // area
    ui->table_newCar->setItemDelegateForColumn(4, new NumericDelegate(this, 400, 0, 2, 10, 150)); // weight
    ui->table_newCar->setItemDelegateForColumn(5, new NumericDelegate(this, 400, 0, 2, 10, 150)); // weight
    ui->table_newCar->setItemDelegateForColumn(6, new IntNumericDelegate(this, 12, 2));
    // ---------- insert a new row to cars ----------
    ui->table_newCar->insertRow(0);
    // set the new id count as default value for the first cell of the new row
    std::unique_ptr<QTableWidgetItem> newItemID2(new QTableWidgetItem(QString::number(0)));
    ui->table_newCar->setItem(0, 0, newItemID2.release());
    // Create a new combobox and set it as the item in the last column of the new row
    QComboBox* comboBox_cars = new QComboBox;
    // Add items to the combobox
    for (auto carType: TrainTypes::carTypeStrings) {
        comboBox_cars->addItem(QString::fromStdString(carType));
    }
    ui->table_newCar->setCellWidget(0, ui->table_newCar->columnCount() - 1, comboBox_cars);
}

void NeTrainSim::setupConfigurationsTable() {
    // ---------- insert a new row to configurations ----------
    ui->table_newConfiguration->insertRow(0);
    // set the new id count as default value for the first cell of the new row
    std::unique_ptr<QTableWidgetItem> newItemID_config(new QTableWidgetItem(QString::number(0)));
    ui->table_newConfiguration->setItem(0, 0, newItemID_config.release());
    ui->table_newConfiguration->setItemDelegateForColumn(2, new NonEmptyDelegate(this));
    // Create a new combobox and set it as the item in the last column of the new row
    QComboBox* comboBox_config = new QComboBox;
    // Add items to the combobox
    comboBox_config->addItem("Locomotive");
    comboBox_config->addItem("Car");
    ui->table_newConfiguration->setCellWidget(0, 1, comboBox_config);

    QSpinBox* spinBox_config_instances = new QSpinBox;
    spinBox_config_instances->setMinimum(1);
    spinBox_config_instances->setSingleStep(1);
    ui->table_newConfiguration->setCellWidget(0, 3, spinBox_config_instances);

    ui->table_newConfiguration->horizontalHeaderItem(0)->setToolTip("the configuration ID should be the same for each train consist");
}

void NeTrainSim::setupTrainsTable() {

    // set the delegates for the trains table IDs
    ui->table_newTrain->setItemDelegateForColumn(0, new NonEmptyDelegate(this));
    ui->table_newTrain->setItemDelegateForColumn(1, new NonEmptyDelegate(this));
    ui->table_newTrain->setItemDelegateForColumn(3, new NumericDelegate(this, 1000000000000.0, 0, 1, 100,0));
    ui->table_newTrain->setItemDelegateForColumn(4, new IntNumericDelegate(this, 300, 1, 1, 1));
    ui->table_newTrain->setItemDelegateForColumn(5, new IntNumericDelegate(this, 300, 1, 1, 1));
    ui->table_newTrain->setItemDelegateForColumn(6, new NumericDelegate(this, 1, 0, 2, 0.05, 0.95));

    // ---------- insert a new row to Trains ----------
    ui->table_newTrain->insertRow(0);
    // add the train 0 to combo visualize Train
    this->updateCombo_visualizeTrains();

    // set the new id count as default value for the first cell of the new row
    std::unique_ptr<QTableWidgetItem> newItemID_train(new QTableWidgetItem(QString::number(0)));
    ui->table_newTrain->setItem(0, 0, newItemID_train.release());

    ui->table_newTrain->horizontalHeaderItem(1)->setToolTip("add the configuration id from the table above");
    ui->table_newTrain->horizontalHeaderItem(2)->setToolTip("add the node ids separated by a comma");

}


// get data from table
Vector<std::tuple<int, double, double, std::string,
                  double, double>>  NeTrainSim::getNodesDataFromNodesTable() {
    // clear the networkNodes map
    this->networkNodes.clear();
    Vector<tuple<int, double, double, std::string,
                      double, double>> nodesRecords;
    // get the data from the QTableWidget
    for (int i = 0; i < ui->table_newNodes->rowCount(); i++) {
        if (ui->table_newNodes->isRowEmpty(i, {0,3})) {
            continue;
        }
        // get the item at row 0 and column 0 of the table widget
        QTableWidgetItem* xItem = ui->table_newNodes->item(i, 1);
        QTableWidgetItem* yItem = ui->table_newNodes->item(i, 2);
        QTableWidgetItem* labelItem = ui->table_newNodes->item(i, 0);
        QTableWidgetItem* descItem = ui->table_newNodes->item(i, 3);
        double xCoord, yCoord, xScale, yScale;
        std::string desc;
        QString label;
        if (labelItem && xItem && yItem && !xItem->text().isEmpty() &&
            !yItem->text().isEmpty() && !labelItem->text().isEmpty()){
            label  = labelItem->text().trimmed();
            xCoord = xItem->text().toDouble();
            yCoord = yItem->text().toDouble();
            xScale = ui->doubleSpinBox_xCoordinate->value();
            yScale = ui->doubleSpinBox_yCoordinate->value();
            desc   = descItem->text().trimmed().toStdString();

            nodesRecords.push_back(std::make_tuple(label.toInt(), xCoord, yCoord, desc, xScale, yScale));

            this->networkNodes[label] = std::make_pair(xCoord * xScale, yCoord * yScale);
        }
    }
    return nodesRecords;
}

// get data from table
Vector<tuple<int, double, double, std::string,
                  double, double>> NeTrainSim::getNodesDataFromNodesFile(QString fileName) {

    // clear the networkNodes map
    this->networkNodes.clear();
    Vector<tuple<int, double, double, std::string,
                      double, double>> nodesRecords;

    if (fileName.trimmed().isEmpty()) { return nodesRecords; }
    auto records = ReadWriteNetwork::readNodesFile(fileName.toStdString());

    for (auto& record: records) {
        this->networkNodes[QString::number(std::get<0>(record))] = std::make_pair(std::get<1>(record) * std::get<4>(record),
                                                                                  std::get<2>(record) * std::get<5>(record));
    }

    return records;
}


// create a slot to update the QCustomPlot data and redraw the plot
void NeTrainSim::updateNodesPlot(CustomPlot &plot, QVector<double>xData, QVector<double>yData,
                                 QVector<QString> labels, bool showLabels) {


    // check the plot has at least 1 graph
    if (plot.graphCount() < 1) { return; }

    // get the QCPGraph object for the graph in the QCustomPlot
    QCPGraph *graph = plot.graph(0);

    // clear the graph data
    graph->data()->clear();
    plot.replot();

    if (xData.size() < 1) { return; }
    if (xData.size() != yData.size()) {
        qDebug() << "xData and yData should be equal in size!\n";
        return;
    }

    graph->setData(xData, yData);

    double pointSize = 5;
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
    plot.xAxis->setRangeLower(xmin);
    plot.xAxis->setRangeUpper(xmax);

    // Find the minimum and maximum values of the vector
    double ymin = *std::min_element(yData.begin(), yData.end());
    ymin = ymin - 0.1 * ymin;
    double ymax = *std::max_element(yData.begin(), yData.end());
    ymax = ymax + 0.1 * ymax;
    // set the range of the y-axis
    plot.yAxis->setRangeLower(ymin);
    plot.yAxis->setRangeUpper(ymax);

    if (showLabels) {
        // add the labels
        for (int i = 0; i < xData.size(); i++) {
            if (!qIsNaN(xData[i]) && !qIsNaN(yData[i] && !labels[i].trimmed().isEmpty())) {
                QCPItemText *label = new QCPItemText(&plot);
                label->setPositionAlignment(Qt::AlignLeft|Qt::AlignBottom);
                label->position->setType(QCPItemPosition::ptPlotCoords);
                label->position->setCoords(xData[i], yData[i]);
                label->setText(QString("Point %1").arg(labels[i]));
                label->setFont(QFont(font().family(), 10));
                label->setPen(QPen(Qt::NoPen));
            }
        }
    }

    plot.resetZoom();
    //plot.replot();
};

/**
 * Reads the link data from a file and returns a vector of link records.
 *
 * @param fileName The name of the file to read the link data from.
 * @return A vector of link records containing the link data.
 */
Vector<tuple<int, int, int, double, double, int,
                  double, double, int, double, bool,
                  std::string, double, double>>  NeTrainSim::getLinkesDataFromLinksFile(QString fileName) {
    auto records = ReadWriteNetwork::readLinksFile(fileName.toStdString());
    return records;
}


/**
 * Retrieves the link data from the links table and returns a vector of link records.
 *
 * @return A vector of link records containing the link data from the table.
 */
Vector<tuple<int, int, int, double, double, int,
                  double, double, int, double, bool,
                  std::string, double, double>>  NeTrainSim::getLinkesDataFromLinksTable() {

    Vector<tuple<int, int, int, double, double, int,
                      double, double, int, double, bool,
                      std::string, double, double>> linksRecords;

    // get the data from the QTableWidget
    for (int i = 0; i < ui->table_newLinks->rowCount(); i++) {
        // get the item at row 0 and column 0 of the table widget
        QTableWidgetItem* fromItem = ui->table_newLinks->item(i, 1);
        QTableWidgetItem* toItem = ui->table_newLinks->item(i, 2);

        // Check if the required cells are not empty
        if (fromItem && toItem && !fromItem->text().isEmpty() &&
            !toItem->text().isEmpty() ){

            // Check if the row is not empty in specific columns
            if (ui->table_newLinks->isRowEmpty(i, {0,7,9,10})) {
                continue;
            }

            // Add the link record to the vector
            linksRecords.push_back(std::make_tuple(
                ui->table_newLinks->item(i, 0) ? ui->table_newLinks->item(i, 0)->text().trimmed().toInt() : 0,
                ui->table_newLinks->item(i, 1) ? ui->table_newLinks->item(i, 1)->text().trimmed().toInt() : 0,
                ui->table_newLinks->item(i, 2) ? ui->table_newLinks->item(i, 2)->text().trimmed().toInt() : 0,
                1.0,
                ui->table_newLinks->item(i, 3) ? ui->table_newLinks->item(i, 3)->text().trimmed().toDouble() : 0.0,
                ui->table_newLinks->item(i, 4) ? ui->table_newLinks->item(i, 4)->text().trimmed().toInt() : 0,
                ui->table_newLinks->item(i, 5) ? ui->table_newLinks->item(i, 5)->text().trimmed().toDouble() : 0.0,
                ui->table_newLinks->item(i, 6) ? ui->table_newLinks->item(i, 6)->text().trimmed().toDouble() : 0.0,
                (ui->table_newLinks->item(i, 7) && ui->table_newLinks->item(i, 7)->checkState() == Qt::Checked) ? 2 : 1,
                ui->table_newLinks->item(i, 8) ? ui->table_newLinks->item(i, 8)->text().trimmed().toDouble() : 0.0,
                (ui->table_newLinks->item(i, 9) && ui->table_newLinks->item(i, 9)->checkState() == Qt::Checked),
                ui->table_newLinks->item(i, 10) ? ui->table_newLinks->item(i, 10)->text().trimmed().toStdString() : "",
                1.0,
                ui->doubleSpinBox_SpeedScale->value()
                ));

        }
    }
    return linksRecords;
}

/**
 * Retrieves the start and end node IDs from the link records and returns them as plottable data.
 *
 * @param linksRecords A vector of link records containing the link data.
 * @return A tuple containing the start and end node IDs as plottable data.
 */
tuple<QVector<QString>,
           QVector<QString>> NeTrainSim::getLinksPlottableData(Vector<tuple<int, int, int,
                                                                     double, double, int,
                                                                     double, double, int,
                                                                     double, bool, std::string,
                                                                     double, double>> linksRecords) {
    QVector<QString> startNodes;
    QVector<QString> endNodes;

    // Iterate through the link records
    for (auto& record: linksRecords) {
        startNodes.push_back(QString::number(std::get<1>(record)));
        endNodes.push_back(QString::number(std::get<2>(record)));
    }

    return std::make_tuple(startNodes, endNodes);
}

// create a slot to update the QCustomPlot data and redraw the plot
void NeTrainSim::updateLinksPlot(CustomPlot &plot, QVector<QString> startNodeIDs, QVector<QString> endNodeIDs) {
    // check if the plot has at least 2 graphs
    if (plot.graphCount() < 2) { return; }
    // get the QCPGraph object for the graph in the QCustomPlot
    QCPGraph *graph = plot.graph(1);

    graph->data()->clear();
    plot.replot();

    if (startNodeIDs.size() != endNodeIDs.size() || startNodeIDs.size() < 1 || this->networkNodes.size() < 1) {
        return;
    }

    // disable scatter style for the lines
    graph->setScatterStyle(QCPScatterStyle::ssNone);

    // get the data from the QTableWidget
    for (int i = 0; i < startNodeIDs.size(); i++) {
        // get the item at row 0 and column 0 of the table widget
        QString fromItem = startNodeIDs[i];
        QString toItem = endNodeIDs[i];
        if (!fromItem.isEmpty() && !toItem.isEmpty() ){
            if (!this->networkNodes.is_key(fromItem.trimmed())){ continue; }
            if (!this->networkNodes.is_key(toItem.trimmed())) { continue; }

            std::pair<double, double> fromNode = this->networkNodes[fromItem.trimmed()];
            std::pair<double, double> toNode = this->networkNodes[toItem.trimmed()];
            graph->addData(fromNode.first, fromNode.second);
            graph->addData(toNode.first, toNode.second);

        }
    }
    // set the pen style for the lines
    graph->setPen(QPen(Qt::blue, 2));
    plot.resetZoom();
    //plot.replot();
};

/**
 * Retrieves the trains data from the tables and returns it as a vector of train records.
 *
 * @return A vector of train records containing the trains data from the tables.
 */
Vector<tuple<std::string, Vector<int>, double, double,
                  Vector<tuple<double, double, double, double, double, double, int, int>>,
                  Vector<tuple<double, double, double, double, double, int, int>>,
                  bool>> NeTrainSim::getTrainsDataFromTables() {

    Vector<tuple<std::string, Vector<int>, double, double,
                 Vector<tuple<double, double, double, double, double, double, int, int>>,
                 Vector<tuple<double, double, double, double, double, int, int>>,
                 bool>> trains;

//    try {

        Map<QString, tuple<double, double, double, double, double, double, int, int>> tableLocomotives;
        Map<QString, tuple<double, double, double, double, double, int, int>> tableCars;
        Map<QString, Map<QString, QVector<std::pair<QString, int>>>> configTable;

        // --------------------------------------------------------------
        // -------------------- Locomotives Table -----------------------
        // --------------------------------------------------------------

        // check if the locos table does not have at least a complete row
        if (ui->table_newLocomotive->isTableIncomplete({8})) {
            throw std::invalid_argument("Locomotives Table is empty!");
            return trains;
        }

        // check if the locomotives table has any empty cell
        if (ui->table_newLocomotive->hasEmptyCell({8})) {
            throw std::invalid_argument("Locomotives Table has empty cells!");
            return trains;
        }

        // read locomotives
        for (int i = 0; i< ui->table_newLocomotive->rowCount(); i++) {
            if (ui->table_newLocomotive->isRowEmpty(i, {8})) { continue; }
            // Get the combobox widget from the cell
            QComboBox* comboBox = qobject_cast<QComboBox*>(ui->table_newLocomotive->cellWidget(i, 8));
            // Get the selected text from the combobox
            std::string locoType = comboBox->currentText().trimmed().toStdString() + " Locomotive";
            // get the enum index
            auto typeEnum = TrainTypes::strToPowerType(locoType);
            int type = static_cast<int>(typeEnum);

            auto loco = std::make_tuple(
                ui->table_newLocomotive->item(i, 1) ? ui->table_newLocomotive->item(i, 1)->text().trimmed().toDouble() : 0.0,
                ui->table_newLocomotive->item(i, 2) ? ui->table_newLocomotive->item(i, 2)->text().trimmed().toDouble() : 0.0,
                ui->table_newLocomotive->item(i, 3) ? ui->table_newLocomotive->item(i, 3)->text().trimmed().toDouble() : 0.0,
                ui->table_newLocomotive->item(i, 4) ? ui->table_newLocomotive->item(i, 4)->text().trimmed().toDouble() : 0.0,
                ui->table_newLocomotive->item(i, 5) ? ui->table_newLocomotive->item(i, 5)->text().trimmed().toDouble() : 0.0,
                ui->table_newLocomotive->item(i, 6) ? ui->table_newLocomotive->item(i, 6)->text().trimmed().toDouble() : 0.0,
                ui->table_newLocomotive->item(i, 7) ? ui->table_newLocomotive->item(i, 7)->text().trimmed().toInt()    : 0,
                type
                );


            tableLocomotives[ui->table_newLocomotive->item(i, 0)->text().trimmed()] = loco;
        }

        // --------------------------------------------------------------
        // ------------------------- Cars Table -------------------------
        // --------------------------------------------------------------
        // check if the cars table does not have at least a complete row
        if (ui->table_newCar->isTableIncomplete({7})) {
            throw std::invalid_argument("Cars Table is empty!");
            return trains;
        }

        // check if the cars table has any empty cell
        if (ui->table_newCar->hasEmptyCell({7})) {
            throw std::invalid_argument("Cars Table has empty cells!");
            return trains;
        }

        for (int i = 0; i< ui->table_newCar->rowCount(); i++) {
            if (ui->table_newCar->isRowEmpty(i, {7})) { continue; }
            // Get the combobox widget from the cell
            QComboBox* comboBox = qobject_cast<QComboBox*>(ui->table_newCar->cellWidget(i, 7));
            // Get the selected text from the combobox
            std::string carType = comboBox->currentText().trimmed().toStdString();
            auto typeEnum = TrainTypes::strtoCarType(carType);
            int type = static_cast<int>(typeEnum);

            auto car = std::make_tuple(
                ui->table_newCar->item(i, 1) ? ui->table_newCar->item(i, 1)->text().trimmed().toDouble() : 0.0,
                ui->table_newCar->item(i, 2) ? ui->table_newCar->item(i, 2)->text().trimmed().toDouble() : 0.0,
                ui->table_newCar->item(i, 3) ? ui->table_newCar->item(i, 3)->text().trimmed().toDouble() : 0.0,
                ui->table_newCar->item(i, 4) ? ui->table_newCar->item(i, 4)->text().trimmed().toDouble() : 0.0,
                ui->table_newCar->item(i, 5) ? ui->table_newCar->item(i, 5)->text().trimmed().toDouble() : 0.0,
                ui->table_newCar->item(i, 6) ? ui->table_newCar->item(i, 6)->text().trimmed().toInt() : 0,
                type
                );

            tableCars[ui->table_newCar->item(i, 0)->text().trimmed()] = car;

        }


        // --------------------------------------------------------------
        // ------------------ Configurations Table ----------------------
        // --------------------------------------------------------------

        // check if the Configurations table does not have at least a complete row
        if (ui->table_newConfiguration->isTableIncomplete({0,1,3})) {
            throw std::invalid_argument("Configurations Table is empty!");
            return trains;
        }

        // check if the cars table has any empty cell
        if (ui->table_newConfiguration->hasEmptyCell({0,1,3})) {
            throw std::invalid_argument("Configurations Table has empty cells!");
            return trains;
        }

        for (int i = 0; i<ui->table_newConfiguration->rowCount(); i++) {
            if (ui->table_newConfiguration->isRowEmpty(i, {0,1,3})) { continue; }

            // if the config ID exists
            if (configTable.get_keys().exist(ui->table_newConfiguration->item(i,0)->text().trimmed())) {

                // Get the combobox widget from the cell
                QComboBox* comboBox = qobject_cast<QComboBox*>(ui->table_newConfiguration->cellWidget(i, 1));
                // Get the selected text from the combobox
                std::string type = comboBox->currentText().trimmed().toStdString();

                // get the number of instances
                QSpinBox* spinbox = qobject_cast<QSpinBox*>(ui->table_newConfiguration->cellWidget(i, 3));
                // Get the selected text from the spinbox
                int countInstances = spinbox->value();


                configTable[ui->table_newConfiguration->item(i,0)->text().trimmed()][
                    QString::fromStdString(type)].push_back(                        //loco or car
                        std::make_pair(ui->table_newConfiguration->item(i,2)->text().trimmed(),
                                       countInstances));
            }
            // if the ID does not exist
            else {
                Map<QString, QVector<std::pair<QString, int>>> instances;

                // Get the combobox widget from the cell
                QComboBox* comboBox = qobject_cast<QComboBox*>(ui->table_newConfiguration->cellWidget(i, 1));
                // Get the selected text from the combobox
                QString type = comboBox->currentText().trimmed();

                // get the number of instances
                QSpinBox* spinbox = qobject_cast<QSpinBox*>(ui->table_newConfiguration->cellWidget(i, 3));
                // Get the selected text from the spinbox
                int countInstances = spinbox->value();

                instances[type].push_back(std::make_pair(ui->table_newConfiguration->item(i,2)->text().trimmed(),
                                                           countInstances));
                // add the map to the config table
                configTable[ui->table_newConfiguration->item(i,0)->text().trimmed()] = instances;
            }
        }

        // --------------------------------------------------------------
        // ----------------------- Trains Table -------------------------
        // --------------------------------------------------------------

        // check if the Configurations table does not have at least a complete row
        if (ui->table_newTrain->isTableIncomplete()) {
            throw std::invalid_argument("Trains Table is empty!");
            return trains;
        }

        // check if the cars table has any empty cell
        if (ui->table_newTrain->hasEmptyCell()) {
            throw std::invalid_argument("Trains Table has empty cells!");
            return trains;
        }

        for (int i = 0; i<ui->table_newTrain->rowCount(); i++) {
            if (ui->table_newTrain->isRowEmpty(i)) { continue; }

            auto trainID = ui->table_newTrain->item(i,0)->text().trimmed().toStdString();
            auto trainConfig = ui->table_newTrain->item(i,1)->text().trimmed();
            auto trainPathStrings = ui->table_newTrain->item(i,2)->text().trimmed().split(',').toVector();
            Vector<int> trainPath;
            for (const QString& str : trainPathStrings) {
                trainPath.push_back(str.toInt());
            }
            auto startTime = ui->table_newTrain->item(i,3)->text().trimmed().toDouble();
            auto locoCount = ui->table_newTrain->item(i,4)->text().trimmed().toInt();
            auto carCount = ui->table_newTrain->item(i,5)->text().trimmed().toInt();
            auto frictionCof = ui->table_newTrain->item(i,6)->text().trimmed().toDouble();

            Vector<tuple<double, double, double, double, double, double, int, int>> locomotivesRecords;
            Vector<tuple<double, double, double, double, double, int, int>> carsRecords;

            if (! configTable.is_key(trainConfig)) {
                throw std::invalid_argument("Could not find configuration ID: " + trainConfig.toStdString());
            }


            if (! configTable[trainConfig].is_key("Locomotive")) {
                throw std::invalid_argument("Consist " + trainConfig.toStdString() + " does not have locomotives");
            }

            for (auto &vehicle: configTable[trainConfig]["Locomotive"]){
                for (int i = 0; i < vehicle.second; i++) {
                    if (tableLocomotives.is_key(vehicle.first)) {
                        locomotivesRecords.push_back(tableLocomotives[vehicle.first]);
                    }
                    else {
                        throw std::invalid_argument("Could not find locomotive: " + vehicle.first.toStdString());
                    }
                }
            }
            // double check the locos and cars counts
            if (locoCount != locomotivesRecords.size()) {
                throw std::runtime_error("Error: " + std::to_string(static_cast<int>(Error::trainHasWrongLocos)) +
                                         "\nlocomotives count does not match added locomotives!");
            }

            if (configTable[trainConfig].is_key("Car")) {
                for (auto &vehicle: configTable[trainConfig]["Car"]){
                    for (int i = 0; i < vehicle.second; i++) {
                        if (tableCars.is_key(vehicle.first)) {
                            carsRecords.push_back(tableCars[vehicle.first]);
                        }
                        else {
                            throw std::invalid_argument("Could not find car: " + vehicle.first.toStdString());
                        }

                    }
                }
            }

            if (carCount != carsRecords.size()) {
                throw std::runtime_error("Error: " + std::to_string(static_cast<int>(Error::trainHasWrongLocos)) +
                                         "\ncars count does not match added cars!");
            }



            auto record = std::make_tuple(trainID, trainPath, startTime, frictionCof, locomotivesRecords, carsRecords, false);
            trains.push_back(record);
        }

        return trains;
//    }
//    catch (const std::exception &e) {
//        ErrorHandler::showError(e.what());
//        return Vector<tuple<std::string, Vector<int>, double, double,
//                            Vector<tuple<double, double, double, double, double, double, int, int>>,
//                            Vector<tuple<double, double, double, double, double, int, int>>,
//                            bool>>();
//    }

}

QString NeTrainSim::browseFiles(QLineEdit* theLineEdit, const QString& theFileName) {
    QString fname = QFileDialog::getOpenFileName(nullptr, theFileName, "", "DAT Files (*.DAT)");
    if (!fname.isEmpty()) {
        theLineEdit->setText(fname);
    }
    return fname;
}

void NeTrainSim::browseFolder(QLineEdit* theLineEdit, const QString& theHelpMessage) {
    QString folderPath = QFileDialog::getExistingDirectory(this,
                                                           theHelpMessage,
                                                           QDir::homePath(),
                                                           QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    // Check if a folder was selected
    if (!folderPath.isEmpty()) {
        theLineEdit->setText(folderPath);
    }
}

void NeTrainSim::setNodesData(QVector<double>& xData, QVector<double>& yData, QVector<QString>& labels) {
    if (this->nodesXData != xData || this->nodesYData != yData || this->nodesLabelData != labels) {
        this->nodesXData = xData;
        this->nodesYData = yData;
        this->nodesLabelData = labels;
        emit this->nodesDataChanged(xData, yData, labels);
    }
}

tuple<QVector<double>, QVector<double>,
           QVector<QString>> NeTrainSim::getNodesPlottableData(Vector<tuple<int, double, double, std::string,
                                                                double, double>> &nodeRecords) {
    QVector<double> xData; QVector<double> yData; QVector<QString> labels;

    for (auto& record:nodeRecords) {
        labels.push_back(QString::number(std::get<0>(record)));
        xData.push_back(std::get<1>(record) * std::get<4>(record));
        yData.push_back(std::get<2>(record) * std::get<5>(record));
    }
    return std::make_tuple(xData, yData, labels);
}

void NeTrainSim::setLinksData(QVector<QString>& startNodeIDs, QVector<QString> endNodeIDs) {
    if (this->linksStartNodeIDs != startNodeIDs || this->linksEndNodeIDs != endNodeIDs) {
        this->linksStartNodeIDs = startNodeIDs;
        this->linksEndNodeIDs = endNodeIDs;
        emit this->linksDataChanged(startNodeIDs, endNodeIDs);
    }
}



void NeTrainSim::showNotification(QString text) {
    ui->label_Notification->setTextWithTimeout(text, 3000);
    ui->label_Notification->setStyleSheet("color: black;");
}

void NeTrainSim::showWarning(QString text) {
    ui->label_Notification->setTextWithTimeout(text, 3000);
    ui->label_Notification->setStyleSheet("color: red;");
}

void NeTrainSim::simulate() {
    try {
    Vector<std::tuple<int, double, double, std::string, double, double>> nodeRecords;
    Vector<tuple<int, int, int, double, double, int, double, double, int, double, bool,
                 std::string, double, double>> linkRecords;
    Vector<tuple<std::string, Vector<int>, double, double,
                 Vector<tuple<double, double, double, double, double, double, int, int>>,
                 Vector<tuple<double, double, double, double, double, int, int>>,
                 bool>> trainRecords;

        if (ui->checkBox_defineNewNetwork->checkState() == Qt::Checked) {
            if (ui->table_newNodes->hasEmptyCell({0,3})) {
                this->showWarning("Missing values in nodes table!");
                return;
            }
            if (ui->table_newLinks->hasEmptyCell({0,7,9})) {
                this->showWarning("Missing values in links table!");
                return;
            }
            nodeRecords = this->getNodesDataFromNodesTable();
            linkRecords = this->getLinkesDataFromLinksTable();
        }
        else {

            // if no files are added to nodes, show error
            if (ui->lineEdit_nodes->text().trimmed().isEmpty()) {
                ErrorHandler::showError("No nodes file is set!");
                return;
            }
            // if no files are added to links, show error
            if (ui->lineEdit_links->text().trimmed().isEmpty()) {
                ErrorHandler::showError("No links file is set!");
                return;
            }
            // try to read the files
            try {
                nodeRecords = ReadWriteNetwork::readNodesFile(ui->lineEdit_nodes->text().trimmed().toStdString());
                linkRecords = ReadWriteNetwork::readLinksFile(ui->lineEdit_links->text().trimmed().toStdString());
            } catch (const std::exception& e) {
                ErrorHandler::showError(e.what());
                return;
            }
        }


        if (ui->checkBox_TrainsOD->checkState() == Qt::Checked) {
            // read trains from table and generate instances of trains
            try {
                trainRecords = this->getTrainsDataFromTables();
            } catch (const std::exception& e) {
                ErrorHandler::showError(e.what());
                return;
            }
        }
        else {
            trainRecords = TrainsList::readTrainsFile(ui->lineEdit_trains->text().toStdString());
        }

        std::string exportDir = "";
        if (! ui->lineEdit_outputPath->text().trimmed().isEmpty()) {
            exportDir = ui->lineEdit_outputPath->text().trimmed().toStdString();
        }
        else {
            this->showWarning("Export directory is not set!");
            return;
        }

        std::string summaryFilename = "";
        if (! ui->lineEdit_summaryfilename->text().trimmed().isEmpty()) {
            summaryFilename = ui->lineEdit_summaryfilename->text().trimmed().toStdString();
        }
        else {
            this->showWarning("Summary filename is not set!");
            return;
        }

        bool exportAllTrainsSummary = ui->checkBox_detailedTrainsSummay->checkState() == Qt::Checked;
        bool exportInta = ui->checkBox_exportTrajectory->checkState() == Qt::Checked;

        std::string instaFilename = "";
        if (! ui->lineEdit_trajectoryFilename->text().trimmed().isEmpty() &&
            exportInta) {
            instaFilename = ui->lineEdit_trajectoryFilename->text().trimmed().toStdString();
        }
        else if (exportInta &&
                   ui->lineEdit_trajectoryFilename->text().trimmed().isEmpty()) {
            this->showWarning("Summary filename is not set!");
            return;
        }
        else {
            instaFilename = "";
        }


        std::string netName = ui->lineEdit_networkName->text().trimmed().isEmpty()? "Not Defined" :
                                  ui->lineEdit_networkName->text().trimmed().toStdString();
        double endTime = ui->doubleSpinBox->value();
        double timeStep = ui->doubleSpinBox_timeStep->value();
        int plotFreq = ui->spinBox_plotEvery->value();

        if (this->thread == nullptr) {
            this->thread = new QThread(this);
        }

        if (this->thread->isRunning()) {
            this->showNotification("Simulation is still running!");
            return;
        }

        ui->progressBar->setVisible(true);
        worker = new SimulationWorker(nodeRecords, linkRecords, trainRecords,
                                            netName, endTime, timeStep, plotFreq,
                                            exportDir, summaryFilename, exportInta,
                                            instaFilename, exportAllTrainsSummary);

        // handle any error that arise from the simulator
        connect(worker, &SimulationWorker::errorOccurred, this, &NeTrainSim::handleError);

        // update the progress bar
        connect(worker, &SimulationWorker::simulaionProgressUpdated, ui->progressBar, &QProgressBar::setValue);

        // Connect the operationCompleted signal to a slot in your GUI class
        connect(worker, &SimulationWorker::simulationFinished, this, &NeTrainSim::handleSimulationFinished);

        // replot the trains coordinates
        connect(worker, &SimulationWorker::trainsCoordinatesUpdated, this,
                [this](Vector<std::pair<std::string, Vector<std::pair<double,double>>>> trainsStartEndPoints) {
                    updateTrainsPlot(trainsStartEndPoints);
                });

        // move the worker to the new thread
        worker->moveToThread(thread);

        // disable the simulate button
        connect(thread, &QThread::started, this, [=]() {
            this->ui->pushButton_projectNext->setEnabled(false);
        });
        // connect the do work to thread start
        connect(thread, &QThread::started, worker, &SimulationWorker::doWork);

        // start the simulation
        thread->start();


    } catch (const std::exception& e) {
        ErrorHandler::showError(e.what());
    }

}


// Slot to handle the simulation finished signal
void NeTrainSim::handleSimulationFinished(Vector<std::pair<std::string, std::string>> summaryData, std::string trajectoryFile) {
        ui->tabWidget_project->setTabEnabled(4, true); // enable the results window
        ui->pushButton_projectNext->setEnabled(true);
        this->ui->progressBar->setVisible(false);
        this->showNotification("Simulation finished Successfully!");
        this->trainsSummaryData = summaryData;
        this->thread->quit();
        this->thread = nullptr;
        delete this->worker;
        this->worker = nullptr;
        this->showReport();
        this->showDetailedReport(QString::fromStdString(trajectoryFile));
        //QMetaObject::invokeMethod(this, "showReport", Qt::QueuedConnection); // Call showReport in the GUI thread
        ui->tabWidget_project->setCurrentIndex(4);
}

void NeTrainSim::saveProjectFile(bool saveAs) {
    if (projectFileName.isEmpty() || saveAs) {
        QString saveFilePath = QFileDialog::getSaveFileName(this, "Save Project", QDir::homePath(),  "NeTrainSim Files (*.NTS)");
        if (saveFilePath.isEmpty()) {
            return;
        }
        projectFileName = saveFilePath;
    }

//    try {
        projectName = (!ui->lineEdit_projectName->text().trimmed().isEmpty()) ? ui->lineEdit_projectName->text().trimmed() : "Not Defined";
        author = (!ui->lineEdit_createdBy->text().trimmed().isEmpty()) ? ui->lineEdit_createdBy->text().trimmed() : "Not Defined";
        networkName = (!ui->lineEdit_networkName->text().trimmed().isEmpty()) ? ui->lineEdit_networkName->text().trimmed() : "Not Defined";
        QString simulationEndTime = QString::number(std::max(ui->doubleSpinBox->text().trimmed().toDouble(), 0.0));
        QString simulationTimestep = QString::number(std::max(ui->doubleSpinBox_timeStep->text().trimmed().toDouble(), 0.1));
        QString simulationPlotTime = QString::number(ui->doubleSpinBox->text().trimmed().toDouble());

        if (nodesFilename.isEmpty()) {
            showWarning("Save nodes file first!");
            return;
        }
        if (linksFilename.isEmpty()) {
            showWarning("Save links file first!");
            return;
        }
        if (trainsFilename.isEmpty()) {
            showWarning("Save trains file first!");
            return;
        }

        XMLManager::createProjectFile(projectName, networkName, author, nodesFilename, linksFilename,
                                  trainsFilename, simulationEndTime, simulationTimestep, simulationPlotTime,
                                  projectFileName);

        showNotification("File Saved Successfully");
//    } catch (const std::exception& e) {
//        this->showWarning(e.what());
//    }
}

void NeTrainSim::updateTrainsPlot(Vector<std::pair<std::string, Vector<std::pair<double,double>>>> trainsStartEndPoints) {
    // check if the plot has at least 2 graphs, that means no links added
    if (ui->plot_simulation->graphCount() < 2) { return; }

    // check if the vector is empty
    if (trainsStartEndPoints.empty()) {return; }

    // Create a new legend if the plot does not already have one
    ui->plot_simulation->legend->setVisible(true);

    // the first graph is for nodes and the second is for links
    // starting from the third, is the trains graphs
    while((ui->plot_simulation->graphCount() - 2) < trainsStartEndPoints.size()) {
        auto graph = ui->plot_simulation->addGraph();

        // Generate a random Qt::GlobalColor value
        QColor randomColor(QRandomGenerator::global()->bounded(0, 256),
                           QRandomGenerator::global()->bounded(0, 256),
                           QRandomGenerator::global()->bounded(0, 256));

        // set the pen style for the lines
        QPen pen(randomColor);
        pen.setWidth(4);
        graph->setPen(pen);
        graph->setLineStyle(QCPGraph::lsLine);

        // skip the first two graphs
        int index = std::max(ui->plot_simulation->graphCount() - 2, 0);
        index = std::min(index, static_cast<int>(trainsStartEndPoints.size()) - 1);

        // Set the name for the graph in the legend
        graph->setName(QString::fromStdString(trainsStartEndPoints[index].first));

    }

    // the first graph is for nodes and the second is for links
    // starting from the third, is the trains graphs
    for (int i = 2; i < ui->plot_simulation->graphCount(); i++) {
        int trainIndex = i - 2;
        if (trainIndex > trainsStartEndPoints.size() - 1){
            continue; // it needs to replot at the end
        }
        // get the QCPGraph object for the graph in the QCustomPlot
        QCPGraph *graph = ui->plot_simulation->graph(i);

        // clear the plots
        graph->data()->clear();
        ui->plot_simulation->replot();

        // disable scatter style for the lines
        graph->setScatterStyle(QCPScatterStyle::ssNone);
        auto& record =  trainsStartEndPoints[trainIndex].second;

        // get the item at row 0 and column 0 of the table widget
        std::pair<double,double> startPoint = record[0];
        std::pair<double,double> toItem = record.back();

        graph->addData(startPoint.first, startPoint.second);
        graph->addData(toItem.first, toItem.second);
    }

    // update the plot
    ui->plot_simulation->replot();
}




void NeTrainSim::showReport() {

    if (this->report == nullptr) {
        this->report = new QtRPT(this);
    }

    QObject::connect(this->report, SIGNAL(setValue(int,QString,QVariant&,int)),
                     this, SLOT(setValue(int,QString,QVariant&,int)));
    QObject::connect(this->report, &QtRPT::setDSInfo, this, &NeTrainSim::setDSInfo);

    this->report->loadReport(":/resources/report.xml");

    this->printer = new QPrinter(QPrinter::PrinterResolution);
    this->printer->setOutputFormat(QPrinter::PdfFormat);

    QPageLayout pageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF(0, 0, 0, 0));
    this->printer->setPageLayout(pageLayout);
    this->printer->setFullPage(true);

    // connect the print preview to the report
    connect(ui->widget_SummaryReport, SIGNAL(paintRequested(QPrinter*)), this->report, SLOT(printPreview(QPrinter*)));

    auto popup = [=]() {
        this->report->printExec();
    };

    QObject::connect(ui->pushButton_popoutPreview, &QPushButton::clicked, popup);
}


void NeTrainSim::setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage)
{
    if (paramName == "Description") {
//        if (reportPage == 0) {
        paramValue = QString::fromStdString(this->trainsSummaryData[recNo].first);
//        }
    }
    if (paramName == "Value") {
        paramValue = QString::fromStdString(this->trainsSummaryData[recNo].second);
    }

    if (paramName == "Project") {
        paramValue = QString("Project: ") +
                     (ui->lineEdit_projectName->text().trimmed().isEmpty() ?
                                                 QString("Not Defined") : ui->lineEdit_projectName->text().trimmed());
    }
    if (paramName == "Network") {
        paramValue = QString("Network: ") +
                     (ui->lineEdit_networkName->text().trimmed().isEmpty() ?
                          QString("Not Defined") : ui->lineEdit_networkName->text().trimmed());
    }
    if (paramName == "Author") {
        paramValue = QString("Author: ") +
                     (ui->lineEdit_createdBy->text().trimmed().isEmpty() ?
                          QString("Not Defined") : ui->lineEdit_createdBy->text().trimmed());
    }
}

void NeTrainSim::setDSInfo(DataSetInfo &dsInfo)
{
    if (dsInfo.reportPage == 0)
        dsInfo.recordCount = trainsSummaryData.size();
}



void NeTrainSim::showDetailedReport(QString trajectoryFilename) {
    if (trajectoryFilename.isEmpty()) {
        ui->tabWidget_results->setTabVisible(1, false);
        return;
    }
    std::shared_ptr<CSVManager> CSV = std::make_shared<CSVManager>();
    auto df = CSV->readCSV(trajectoryFilename, "," , true);
    auto ids = CSV->getDistinctColumnValues(0);   // get all file train ID's

    this->ui->comboBox_trainsResults->clear();
    this->ui->comboBox_trainsResults->addItem(QString("--"));
    this->ui->comboBox_trainsResults->addItems(ids);

    auto updateResultsCurves = [&CSV, df, this]() {
        if (ui->comboBox_trainsResults->currentText() == "--") {
            return;
        }
        auto selectedTrain = CSV->filterByColumn(df, 0, ui->comboBox_trainsResults->currentText());
        int columnNumber = ui->comboBox_resultsXAxis->currentText() == "Distance" ? 2 : 1;
        QString xAxisLabel = ui->comboBox_resultsXAxis->currentText() == "Distance" ? "Distance (km)" : "Time (hr)";
        double xDataFactor = ui->comboBox_resultsXAxis->currentText() == "Distance" ? 1.0/1000 : 1.0 / 3600.0;
        auto xData = Utils::factorQVector(Utils::convertQStringVectorToDouble(CSV->getColumnValues(selectedTrain, columnNumber)), xDataFactor);
        auto grades = Utils::convertQStringVectorToDouble(CSV->getColumnValues(selectedTrain, 13));
        auto curvatures = Utils::convertQStringVectorToDouble(CSV->getColumnValues(selectedTrain, 14));
        auto speeds = Utils::factorQVector(Utils::convertQStringVectorToDouble(CSV->getColumnValues(selectedTrain, 4)), 3.6);
        auto accelerations = Utils::convertQStringVectorToDouble(CSV->getColumnValues(selectedTrain, 3));
        auto EC = Utils::convertQStringVectorToDouble(CSV->getColumnValues(selectedTrain, 6));
        auto tractiveForces = Utils::factorQVector(Utils::convertQStringVectorToDouble(CSV->getColumnValues(selectedTrain, 10)), 1.0/1000.0);
        auto resistance = Utils::factorQVector(Utils::convertQStringVectorToDouble(CSV->getColumnValues(selectedTrain, 11)), 1.0/1000.0);
        auto totalForces = Utils::subtractQVector(tractiveForces, resistance);

        this->drawLineGraph(*ui->plot_trajectory_grades, xData, grades, xAxisLabel, "Percentage", "Grades", 0);
        this->drawLineGraph(*ui->plot_trajectory_grades, xData, curvatures, xAxisLabel, "Percentage", "Curvatures", 1);
        this->drawLineGraph(*ui->plot_trajectory_speed, xData, speeds, xAxisLabel, "Speed (km/h)", "Speed", 0);
        this->drawLineGraph(*ui->plot_trajectory_acceleration, xData, accelerations, xAxisLabel, "Accelerations (m/s^2)", "Acceleration", 0);
        this->drawLineGraph(*ui->plot_trajectory_EC, xData, EC, xAxisLabel, "Energy Consumption (kWh)", "Energy", 0);

        this->drawLineGraph(*ui->plot_forces_grades, xData, grades, xAxisLabel, "Percentage", "Grades", 0);
        this->drawLineGraph(*ui->plot_forces_grades, xData, curvatures, xAxisLabel, "Percentage", "Curvatures", 1);
        this->drawLineGraph(*ui->plot_forces_tractiveForces, xData, tractiveForces, xAxisLabel, "Forces", "Tractive Forces (kN)", 0);
        this->drawLineGraph(*ui->plot_forces_resistance, xData, resistance, xAxisLabel, "Forces", "Resistance (kN)", 0);
        this->drawLineGraph(*ui->plot_forces_totalForces, xData, totalForces, xAxisLabel, "Forces", "Net Forces (kN)", 0);
    };

    connect(ui->comboBox_trainsResults, &QComboBox::currentTextChanged, updateResultsCurves);
    connect(ui->comboBox_resultsXAxis, &QComboBox::currentTextChanged, updateResultsCurves);
}

void NeTrainSim::drawLineGraph(CustomPlot& plot, const QVector<double>& xData, const QVector<double>& yData,
                               QString xLabel, QString yLabel, QString graphName, int plotIndex) {

    while(plot.graphCount() - 1 < plotIndex ) {
        plot.addGraph(plot.xAxis, plot.yAxis);
    }

    QCPGraph* graph = plot.graph(plotIndex);
    graph->setData(xData, yData);

    plot.xAxis->setLabel(xLabel);
    plot.yAxis->setLabel(yLabel);

    // Define different colors for each plotIndex
    static const QVector<QColor> plotColors = {
        Qt::blue, Qt::red, Qt::green,
        Qt::cyan, Qt::magenta, Qt::darkBlue,
        Qt::darkRed, Qt::darkGreen, Qt::darkCyan,
        Qt::darkMagenta, Qt::darkYellow, Qt::yellow
    };

    // Set the pen and line style for the graph
    QPen pen(plotColors[plotIndex % plotColors.size()]);
    pen.setWidth(2);
    graph->setPen(pen);
    graph->setLineStyle(QCPGraph::lsLine);

    // Create a new legend if the plot does not already have one
    plot.legend->setVisible(true);

    // Set the name for the graph in the legend
    graph->setName(graphName);

    // Replot the plot to update the graph and legend
    plot.resetZoom();
}

void NeTrainSim::handleError(std::string error) {
    this->ui->pushButton_projectNext->setEnabled(true);
    ErrorHandler::showError(error);
}


void NeTrainSim::closeEvent(QCloseEvent* event) {
    // Stop the thread and perform any necessary cleanup
    if (this->thread != nullptr) {
        thread->quit();
        thread->wait();
    }
    if (this->worker != nullptr) {
        worker->deleteLater();
    }

    // Call the base class implementation to close the form
    QWidget::closeEvent(event);
}

void NeTrainSim::closeApplication() {
    // Close the application
    qApp->quit();
}

void NeTrainSim::clearForm() {
    QList<QLineEdit *> lineEdits = findChildren<QLineEdit *>();
    // Clear the contents of each QLineEdit
    for (QLineEdit *lineEdit : lineEdits) {
        lineEdit->clear();
    }

    QList<QCheckBox *> checkboxes = findChildren<QCheckBox *>();
    for (QCheckBox * check: checkboxes) {
        check->setCheckState(Qt::CheckState::Unchecked);
    }
    QList<CustomTableWidget *> tables = findChildren<CustomTableWidget *>();
    for (CustomTableWidget * table: tables) {
        // Remove all rows
        table->setRowCount(0);
    }

    this->setupNodesTable();
    this->setupLinksTable();
    this->setupLocomotivesTable();
    this->setupCarsTable();
    this->setupConfigurationsTable();
    this->setupTrainsTable();
}


QCPItemText *NeTrainSim::findLabelByPosition(CustomPlot *plot, const QPointF &targetPosition)
{
    // Iterate over the items in the plot
    for (int i = 0; i < plot->itemCount(); ++i) {
        QCPAbstractItem* abstractItem = plot->item(i);

        // Check if the item is a QCPItemText
        if (QCPItemText* label = qobject_cast<QCPItemText*>(abstractItem)) {
            // Compare the position of the label with the target position
            double labelX = label->position->coords().x();
            double labelY = label->position->coords().y();

            if ((labelX == targetPosition.x()) && (labelY == targetPosition.y())) {
                    return label; // Return the matching label
            }
        }
    }
    return nullptr; // No label found
}


void NeTrainSim::handleSampleProject() {
    QString executablePath = QCoreApplication::applicationDirPath();
    QString filePath = QDir(QDir(executablePath).filePath("sampleProject")).filePath("sampleProject.NTS");
    this->loadProjectFiles(filePath);
}

void NeTrainSim::loadProjectFiles(QString projectFilename) {
    QFile pfn(projectFilename);
    if (! pfn.exists()) {
        this->showWarning("Project file does not exist!");
        return;
    }
    if (!projectFilename.isEmpty()) {
        QString executableDirectory = QApplication::applicationDirPath();

        auto out = XMLManager::readProjectFile(projectFilename);
        ui->lineEdit_projectName->setText(std::get<0>(out));
        ui->lineEdit_networkName->setText(std::get<1>(out));
        ui->lineEdit_createdBy->setText(std::get<2>(out));
        QString nodesFile = std::get<3>(out);
        QString linksFile = std::get<4>(out);
        QString trainsFile = std::get<5>(out);
        QString PWDString = "$${PWD}";
        nodesFile.replace(PWDString, executableDirectory);
        linksFile.replace(PWDString, executableDirectory);
        trainsFile.replace(PWDString, executableDirectory);
        QFile nfile(nodesFile);
        if (nfile.exists()) {
            ui->lineEdit_nodes->setText(nodesFile);
        }
        else {
            this->showWarning("Nodes file does not exist");
            return;
        }
        QFile lfile(linksFile);
        if (lfile.exists()) {
            ui->lineEdit_links->setText(linksFile);
        }
        else {
            this->showWarning("Links file does not exist");
            return;
        }

        QFile tfile(trainsFile);
        if (tfile.exists()) {
            ui->lineEdit_trains->setText(trainsFile);
        }
        else {
            this->showWarning("Trains file does not exist");
            return;
        }
        bool ok;
        double simTime = std::get<6>(out).toDouble(&ok);
        if (ok) {
            ui->doubleSpinBox->setValue(simTime);
        }
        else {
            QMessageBox::warning(this, "Error", "Wrong Project File Structure!");
            return;
        }
        double simTimestep = std::get<7>(out).toDouble(&ok);
        if (ok) {
            ui->doubleSpinBox_timeStep->setValue(simTimestep);
        }
        else {
            QMessageBox::warning(this, "Error", "Wrong Project File Structure!");
            return;
        }
        double simTimestepPlot = std::get<8>(out).toDouble(&ok);
        if (ok) {
            ui->spinBox_plotEvery->setValue(simTimestepPlot);
        }
        else {
            QMessageBox::warning(this, "Error", "Wrong Project File Structure!");
            return;
        }

    }
}


NeTrainSim::~NeTrainSim()
{
    if (ui) {
        delete ui;
        ui = nullptr;
    }

    if (worker) {
        delete worker;
        worker = nullptr;
    }

    if (thread) {
        delete thread;
        thread = nullptr;
    }

    if (report) {
        delete report;
        report = nullptr;
    }

    if (printer) {
        delete printer;
        printer = nullptr;
    }

    // Delete the labels
    for (auto label : labelsVector)
    {
        delete label;
    }

    labelsVector.clear();

}

