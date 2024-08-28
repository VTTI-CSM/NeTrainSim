/**
 * @file	~\NeTrainSim\netrainsim.cpp.
 *
 * Implements the netrainsim class
 */
#include "netrainsimmainwindow.h"
#include "gui/clickablelabel.h"
#include "gui/comboboxdelegate.h"
#include "gui/settingswindow.h"
#include "gui/textboxdelegate.h"
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
#include "util/configurationmanager.h"
#include "util/errorhandler.h"
#include "../NeTrainSim/network/readwritenetwork.h"
#include "../NeTrainSim/util/updatechecker.h"


NeTrainSim::NeTrainSim(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::NeTrainSim)
{
    // set up the layout
    ui->setupUi(this);

    connect(&updateChecker, &UpdateChecker::updateAvailable,
            this, &NeTrainSim::handleUpdateAvailability);

    // Start checking for updates
    updateChecker.checkForUpdates();

    // load the defaut settings
    loadDefaults();

    setupGenerals();

    setupPage0();
    setupPage1();
    setupPage2();
    setupPage3();
    setupPage4();
}

void NeTrainSim::handleUpdateAvailability(bool isAvailable) {
    if (isAvailable)
    {
        rightAlignedMenu->setHidden(false);
    }
}

void NeTrainSim::setupGenerals(){
    // Create a widget that will contain the right-aligned menu
    rightAlignedWidget = new QWidget(this);
    layout = new QHBoxLayout(rightAlignedWidget);

    // Adjust layout margins and spacing
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Add a horizontal spacer
    layout->addSpacerItem(new QSpacerItem(40, 20,
                                          QSizePolicy::Expanding,
                                          QSizePolicy::Minimum));

    // Create the right-aligned menu item (e.g., a QLabel for demonstration)
    rightAlignedMenu =
        new ClickableLabel("New Release is Available! Click to Download!",
                           QUrl("https://github.com/VTTI-CSM/"
                                "NeTrainSim/releases"),
                           QColor("red"),
                           rightAlignedWidget);
    layout->addWidget(rightAlignedMenu);

    rightAlignedMenu->setHidden(true);

    // Add the widget to the menu bar
    ui->menubar->setCornerWidget(rightAlignedWidget, Qt::TopRightCorner);

    ui->progressBar->setTextVisible(true);
    ui->progressBar->setAlignment(Qt::AlignCenter);
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setFormat("%p%");
    ui->pushButton_pauseResume->setVisible(false);
    ui->pushButton_pauseResume->setOnOffText("Continue", "Pause");

    this->userBrowsePath = QString();

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

    //show the settings window
    connect(ui->actionSettings, &QAction::triggered, [this]() {
        if (this->theSettingsWindow == nullptr) {
            this->theSettingsWindow = std::make_shared<settingsWindow>(this);
        }
        if (this->theSettingsWindow != nullptr) {
            this->theSettingsWindow->show();
        }
    });

    // ########################################################################
    // ########## Connect signals and slots for project management ############
    // ########################################################################

    // create a new project and clear the form
    connect(ui->actionNew_Project, &QAction::triggered, this,
            &NeTrainSim::clearForm);

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
        QString fname = QFileDialog::getOpenFileName(
            nullptr, "Open NeTrainSim Project File", "",
            "NeTrainSim Files (*.NTS)");
        this->loadProjectFiles(fname);
    });

    // close the application when the exit app is clicked
    connect(ui->actionExit, &QAction::triggered, this,
            &NeTrainSim::closeApplication);

    // open the sample project
    connect(ui->actionLoad_Sample_Project, &QAction::triggered,
            this, &NeTrainSim::handleSampleProject);

    // define the next page and simulate buttons
    QObject::connect(ui->pushButton_projectNext,
                     &QPushButton::clicked, [=, this]() {
        // switch to the next tab page if it is not the last page
        int nextIndex = ui->tabWidget_project->currentIndex() + 1;
        if (nextIndex < ui->tabWidget_project->count() - 1) {
            ui->tabWidget_project->setCurrentIndex(nextIndex);
        }
        if (nextIndex == ui->tabWidget_project->count() - 1) {
            this->simulate();
        }
    });

    QObject::connect(ui->pushButton_pauseResume, &QPushButton::clicked, [=, this]() {
        if (ui->pushButton_pauseResume->isToggled()) {
            this->pauseSimulation();
        }
        else {
            this->resumeSimulation();
        }

    });

    // change next page button text
    QObject::connect(ui->tabWidget_project,
                     &QTabWidget::currentChanged, [=, this](int index) {
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
        this->replotHistoryNodes();
    });

    // replote the links to all plots if the links data has changed
    connect(this, &NeTrainSim::linksDataChanged, [this]() {
        this->replotHistoryLinks();
    });

    // show error if tables has only 1 row
    connect(ui->table_newNodes, &CustomTableWidget::cannotDeleteRow,
            [this]() {
        this->showWarning("Cannot delete the first row!");
    });
    connect(ui->table_newLinks, &CustomTableWidget::cannotDeleteRow,
            [this]() {
        this->showWarning("Cannot delete the first row!");
    });
    connect(ui->table_newLocomotive, &CustomTableWidget::cannotDeleteRow,
            [this]() {
        this->showWarning("Cannot delete the first row!");
    });
    connect(ui->table_newCar, &CustomTableWidget::cannotDeleteRow,
            [this]() {
        this->showWarning("Cannot delete the first row!");
    });
    connect(ui->table_newConfiguration, &CustomTableWidget::cannotDeleteRow,
            [this]() {
        this->showWarning("Cannot delete the first row!");
    });
    connect(ui->table_newTrain, &CustomTableWidget::cannotDeleteRow,
            [this]() {
        this->showWarning("Cannot delete the first row!");
    });
}


void NeTrainSim::setupPage0(){

}

void NeTrainSim::loadDefaults() {
    QString executablePath = QCoreApplication::applicationDirPath();
    QString iniFilePath = QDir(executablePath).filePath("config.ini");

    QFile pfn(iniFilePath);
    if (!pfn.exists()) {
        this->showWarning("Config file does not exist!");
        return;
    }

    this->configManager = new ConfigurationManager(iniFilePath);

    // Read all configurations in the INI file
    QStringList allKeys = configManager->getConfigKeys("");

    // set all the default settings
    foreach (const QString& fullKey, allKeys) {
        QStringList keyParts = fullKey.split("/");
        QString section = keyParts.first();
        QString key = keyParts.last();

        if (key == "browseLocation") {
            QString value = configManager->getConfigValue(section, key);
            this->defaultBrowsePath = value;
        }
    }
}


bool NeTrainSim::saveDefaults(QStringList defaults) {
    // Loop through the list of default configurations
    foreach (const QString& config, defaults) {
        // Split the configuration string into section, key, and value
        QStringList parts = config.split('.');
        if (parts.size() != 2) {
            // Invalid configuration format, return false
            return false;
        }

        QString section = parts[0].trimmed();
        QString keyValue = parts[1].trimmed();

        // Split the key-value pair
        QStringList keyValueParts = keyValue.split('=');
        if (keyValueParts.size() != 2) {
            // Invalid configuration format, return false
            return false;
        }

        QString key = keyValueParts[0].trimmed();
        QString value = keyValueParts[1].trimmed();

        // Save the default configuration
        configManager->setConfigValue(section, key, value);
    }

    return true;
}


void NeTrainSim::setupPage1(){

    QList<int> networkWidgetSizes;
    networkWidgetSizes << 250 << 400;
    ui->splitter_network->setSizes(networkWidgetSizes);

    // make the default show the old network only
    ui->widget_oldNetwork->show();
    //ui->widget_newNetwork->hide();

    // add graphs to the plot
    ui->plot_createNetwork->addGraph();
    ui->plot_createNetwork->addGraph();
    // disable viewing the axies
    ui->plot_createNetwork->xAxis->setVisible(false);
    ui->plot_createNetwork->yAxis->setVisible(false);


    // get the nodes file
    connect(ui->pushButton_nodes, &QPushButton::clicked, [this]() {
        nodesFilename = this->browseFiles(ui->lineEdit_nodes,
                                          "Select Nodes File");
    });

    // get the links file
    connect(ui->pushButton_links, &QPushButton::clicked, [this]() {
        linksFilename = this->browseFiles(ui->lineEdit_links,
                                          "Select Links File");
    });

    // read the nodes file
    connect(ui->lineEdit_nodes, &QLineEdit::textChanged, [this]() {
        auto out = this->getNodesDataFromNodesFile(ui->lineEdit_nodes->text());
        this->loadNodesDataToTable(out);
    });

    // read the links file
    connect(ui->lineEdit_links, &QLineEdit::textChanged, [this]() {
        auto out = this->getLinkesDataFromLinksFile(ui->lineEdit_links->text());
        this->loadLinksDataToTable(out);
    });

    // save the table nodes to DAT file
    connect(ui->pushButton_saveNewNodes, &QPushButton::clicked, [this](){
        if (nodesFilename.isEmpty()) {
            // Open a file dialog to choose the save location
            QString saveFilePath =
                QFileDialog::getSaveFileName(this,
                                             "Save Nodes File",
                                             QDir::homePath(),
                                             "DAT Files (*.DAT)");

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
        }
        else {
            // read the table data
            auto out = this->getNodesDataFromNodesTable();
            std::string sf = nodesFilename.toStdString();
            // write the nodes file
            if (ReadWriteNetwork::writeNodesFile(out, sf))
            {
                showNotification("File Saved Successfully");
            }
            else {
                showWarning("Could not save the file!");
            }
        }
    });

    // save the table links to DAT file
    connect(ui->pushButton_saveNewLinks, &QPushButton::clicked, [this](){
        if (linksFilename.isEmpty()) {
            // Open a file dialog to choose the save location
            QString saveFilePath =
                QFileDialog::getSaveFileName(this,
                                             "Save Links File",
                                             QDir::homePath(),
                                             "DAT Files (*.DAT)");

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
        }
        else {
            // get the links data from table
            auto out = this->getLinkesDataFromLinksTable();
            std::string sf = linksFilename.toStdString();
            // save the links file
            if (ReadWriteNetwork::writeLinksFile(out, sf))
            {
                showNotification("File Saved Successfully");
            }
            else {
                showWarning("Could not save the file!");
            }

        }

    });


// --------------------------------------------------------------------------
// ------------------------- Nodes Table ------------------------------------
// --------------------------------------------------------------------------

    // add the first row
    this->setupNodesTable();

    // connect the cellChanged signals of the QTableWidget to the updatePlot slot
    QObject::connect(ui->table_newNodes,
                     &QTableWidget::cellChanged,
                     this, &NeTrainSim::updateTheNodesPlotData);
    QObject::connect(ui->doubleSpinBox_xCoordinate,
                     &QDoubleSpinBox::valueChanged,
                     this, &NeTrainSim::updateTheNodesPlotData);
    QObject::connect(ui->doubleSpinBox_yCoordinate,
                     &QDoubleSpinBox::valueChanged,
                     this, &NeTrainSim::updateTheNodesPlotData);

    // connect the cellChanged signal of the QTableWidget to the addRow slot
    QObject::connect(ui->table_newNodes,
                     &QTableWidget::cellChanged, this,
                     &NeTrainSim::addRowToNewNode);

    connect(ui->table_newNodes, &CustomTableWidget::tableCleared,
            [=, this](){
                this->addRowToNewNode();
                ui->lineEdit_nodes->setText("");
                nodesFilename = "";
                this->networkNodes.clear();
                nodesXData.clear();
                nodesYData.clear();
                nodesLabelData.clear();
            }
            );

// --------------------------------------------------------------------------
// ------------------------- Links Table ------------------------------------
// --------------------------------------------------------------------------

    // add the first row
    this->setupLinksTable();

    // add a row to the links table everytime you edit the last row cell
    QObject::connect(ui->table_newLinks, &QTableWidget::cellChanged,
                     this, &NeTrainSim::addRowToNewLinks);

    // connect the cellChanged signal of the QTableWidget to
    // the updatePlot slot
    QObject::connect(ui->table_newLinks, &QTableWidget::cellChanged,
                     this, &NeTrainSim::updateTheLinksPlotData);

    QObject::connect(ui->table_newLinks, &CustomTableWidget::tableCleared,
                     [=, this](){
                this->addRowToNewLinks();
                ui->lineEdit_links->setText("");
                linksFilename = "";
                linksStartNodeIDs.clear();
                linksEndNodeIDs.clear();
            }
            );

}

// create a slot to add a new row to the QTableWidget
void NeTrainSim::addRowToNewNode() {
    // check if the last row has been edited
    if (ui->table_newNodes->currentRow() ==
        ui->table_newNodes->rowCount() - 1) {
        // add a new row to the QTableWidget
        int newRow = ui->table_newNodes->rowCount();
        ui->table_newNodes->insertRow(newRow);

        // set the new id count as default value for the first
        // cell of the new row
        int uniqueID = ui->table_newNodes->generateUniqueID();
        std::unique_ptr<QTableWidgetItem> newItemID(
            new QTableWidgetItem(QString::number(uniqueID)));
        ui->table_newNodes->setItem(newRow, 0, newItemID.release());

    }
};

// create a slot to add a new row to the QTableWidget
void NeTrainSim::addRowToNewLinks() {
    // check if the last row has been edited
    if (ui->table_newLinks->currentRow() ==
        ui->table_newLinks->rowCount() - 1) {
        // add a new row to the QTableWidget
        int newRow = ui->table_newLinks->rowCount();
        ui->table_newLinks->insertRow(newRow);

        // set the new id count as default value for the first
        // cell of the new row
        int uniqueID = ui->table_newLinks->generateUniqueID();
        std::unique_ptr<QTableWidgetItem> newItemID(
            new QTableWidgetItem(QString::number(uniqueID)));
        ui->table_newLinks->setItem(newRow, 0, newItemID.release());

        ui->table_newLinks->setupTable();
    }
};

void NeTrainSim::updateTheNodesPlotData() {
    auto out = this->getNodesDataFromNodesTable();
    auto plottableOut = this->getNodesPlottableData(out);
    // update the plotted data
    this->setNodesData(std::get<0>(plottableOut),
                       std::get<1>(plottableOut),
                       std::get<2>(plottableOut));
}

// update the links data for the plot
void NeTrainSim::updateTheLinksPlotData() {
    auto out = this->getLinkesDataFromLinksTable();
    auto plottableOut = this->getLinksPlottableData(out);
    this->setLinksData(std::get<0>(plottableOut),
                       std::get<1>(plottableOut));
};

void NeTrainSim::replotHistoryNodes() {
    this->updateNodesPlot(*(ui->plot_createNetwork), this->nodesXData,
                          this->nodesYData, this->nodesLabelData);
    this->updateNodesPlot(*(ui->plot_trains), this->nodesXData,
                          this->nodesYData, this->nodesLabelData);
}

void NeTrainSim::replotHistoryLinks() {
    this->updateLinksPlot(*(ui->plot_createNetwork),
                          this->linksStartNodeIDs,
                          this->linksEndNodeIDs);
    this->updateLinksPlot(*(ui->plot_trains),
                          this->linksStartNodeIDs, this->linksEndNodeIDs);
    this->updateLinksPlot(*(ui->plot_simulation),
                          this->linksStartNodeIDs, this->linksEndNodeIDs);
}

void NeTrainSim::forceReplotNodes() {
    this->updateTheNodesPlotData();
    this->replotHistoryNodes();
}

void NeTrainSim::forceReplotLinks() {
    this->updateTheLinksPlotData();
    this->replotHistoryLinks();
}


void NeTrainSim::setupPage2(){
    // disable viewing the axies
    ui->plot_trains->xAxis->setVisible(false);
    ui->plot_trains->yAxis->setVisible(false);

    // show the layout as a default
    ui->widget_oldTrainOD->show();

    ui->plot_trains->addGraph();
    ui->plot_trains->addGraph();
    QObject::connect(ui->plot_trains,
                     &CustomPlot::pointLeftSelected,
                     this, &NeTrainSim::trainPointSelected);
    QObject::connect(ui->plot_trains,
                     &CustomPlot::pointRightSelected,
                     this, &NeTrainSim::trainPointDeleted);
    this->updateNodesPlot(*(ui->plot_trains),
                          this->nodesXData,
                          this->nodesYData,
                          this->nodesLabelData);
    this->updateLinksPlot(*(ui->plot_trains),
                          this->linksStartNodeIDs,
                          this->linksEndNodeIDs);


    //ui->widget_newTrainOD->hide();

    // get the trains file
    connect(ui->pushButton_trains, &QPushButton::clicked, [this]() {
        trainsFilename = this->browseFiles(ui->lineEdit_trains,
                                           "Select Trains File");
    });

    connect(ui->lineEdit_trains, &QLineEdit::textChanged,
            [=, this](const QString& file)
            {
                std::string filename = file.toStdString();
                auto out = TrainsList::readTrainsFile(filename);
                this->loadTrainsDataToTables(out);
    });

    // write the trains file
    connect(ui->pushButton_saveNewTrains, &QPushButton::clicked, [this](){
        // Open a file dialog to choose the save location
        QString saveFilePath =
            QFileDialog::getSaveFileName(this,
                                         "Save Trains File",
                                         QDir::homePath(),
                                         "DAT Files (*.DAT)");

        if (!saveFilePath.isEmpty()) {
            Vector<Map<std::string, std::any>> out;
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


// --------------------------------------------------------------------------
// ---------------------- Locomotives Table ---------------------------------
// --------------------------------------------------------------------------

    // add the first row
    this->setupLocomotivesTable();


    // create a slot to add a new row to the QTableWidget
    auto addRowToNewLocomotives = [=, this]() {
        // check if the last row has been edited
        if (ui->table_newLocomotive->currentRow() ==
            ui->table_newLocomotive->rowCount() - 1) {
            // add a new row to the QTableWidget
            int newRow = ui->table_newLocomotive->rowCount();
            ui->table_newLocomotive->insertRow(newRow);

            // set the new id count as default value for the first
            // cell of the new row
            int uniqueID = ui->table_newLocomotive->generateUniqueID();
            std::unique_ptr<QTableWidgetItem> newItemID(
                new QTableWidgetItem(QString::number(uniqueID)));
            ui->table_newLocomotive->setItem(newRow, 0, newItemID.release());

        }
    };
    // add a new row everytime the last row cells are edited
    QObject::connect(ui->table_newLocomotive,
                     &QTableWidget::cellChanged,
                     addRowToNewLocomotives);

    QObject::connect(ui->table_newLocomotive,
                     &CustomTableWidget::tableCleared,
                     addRowToNewLocomotives);


// --------------------------------------------------------------------------
// -------------------------- Car Table -------------------------------------
// --------------------------------------------------------------------------

    // add first row
    this->setupCarsTable();

    // create a slot to add a new row to the QTableWidget
    auto addRowToNewCars = [=, this]() {
        // check if the last row has been edited
        if (ui->table_newCar->currentRow() ==
            ui->table_newCar->rowCount() - 1) {
            // add a new row to the QTableWidget
            int newRow = ui->table_newCar->rowCount();
            ui->table_newCar->insertRow(newRow);

            // set the new id count as default value for the
            // first cell of the new row
            int uniqueID = ui->table_newCar->generateUniqueID();
            std::unique_ptr<QTableWidgetItem> newItemID(
                new QTableWidgetItem(QString::number(uniqueID)));
            ui->table_newCar->setItem(newRow, 0, newItemID.release());
        }
    };
    // add a new row everytime the last row cells are edited
    QObject::connect(ui->table_newCar, &QTableWidget::cellChanged,
                     addRowToNewCars);

    QObject::connect(ui->table_newCar,
                     &CustomTableWidget::tableCleared,
                     addRowToNewCars);

// --------------------------------------------------------------------------
// --------------------- Configurations Table -------------------------------
// --------------------------------------------------------------------------

    this->setupConfigurationsTable();

    // create a slot to add a new row to the QTableWidget
    auto addRowToNewConfig = [=, this]() {
        // check if the last row has been edited
        if (ui->table_newConfiguration->currentRow() ==
            ui->table_newConfiguration->rowCount() - 1) {
            // add a new row to the QTableWidget
            int newRow = ui->table_newConfiguration->rowCount();
            ui->table_newConfiguration->insertRow(newRow);

            // set the new id count as default value for the
            // first cell of the new row
            std::unique_ptr<QTableWidgetItem> newItemID(
                new QTableWidgetItem(QString::number(1)));
            ui->table_newConfiguration->setItem(newRow, 0, newItemID.release());
        }
    };

    // add a new row everytime the last row cells are edited
    QObject::connect(ui->table_newConfiguration,
                     &QTableWidget::cellChanged, addRowToNewConfig);
    QObject::connect(ui->table_newConfiguration,
                     &CustomTableWidget::tableCleared,
                     addRowToNewConfig);


// --------------------------------------------------------------------------
// ------------------------- Trains Table -----------------------------------
// --------------------------------------------------------------------------


    QObject::connect(ui->table_newTrain, &QTableWidget::cellChanged,
                     this, &NeTrainSim::updateCombo_visualizeTrains);

    this->setupTrainsTable();

    // create a slot to add a new row to the QTableWidget
    auto addRowToNewTrain = [=, this]() {
        // check if the last row has been edited
        if (ui->table_newTrain->currentRow() ==
            ui->table_newTrain->rowCount() - 1) {
            // add a new row to the QTableWidget
            int newRow = ui->table_newTrain->rowCount();
            ui->table_newTrain->insertRow(newRow);

            // set the new id count as default value for the first
            // cell of the new row
            int uniqueID = ui->table_newTrain->generateUniqueID();
            std::unique_ptr<QTableWidgetItem> newItemID(
                new QTableWidgetItem(QString::number(uniqueID)));
            ui->table_newTrain->setItem(newRow, 0, newItemID.release());

        }
    };
    // add a new row everytime the last row cells are edited
    QObject::connect(ui->table_newTrain, &QTableWidget::cellChanged,
                     addRowToNewTrain);

    QObject::connect(ui->table_newTrain,
                     &CustomTableWidget::tableCleared,
                     addRowToNewTrain);

}


void NeTrainSim::setupPage3(){

    QList<int> simulatorWidgetSizes;
    simulatorWidgetSizes << 229 << 700;
    ui->splitter_simulator->setSizes(simulatorWidgetSizes);

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
        ui->horizontalWidget_TrajFile->setVisible(
            ui->checkBox_exportTrajectory->checkState() == Qt::Checked);
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
                    // Check if the cell at column 0 exists,
                    // otherwise skip to the next row
                    if (!ui->table_newTrain->item(i, 0)) {
                        continue;
                    }

                    // Check if the row corresponds to the currently
                    // selected item in the combo box
                    if (ui->table_newTrain->item(i, 0)->text().trimmed() ==
                        ui->combo_visualizeTrain->currentText()) {
                        // Check if the cell at column 2 exists
                        if (ui->table_newTrain->item(i, 2)) {
                            // Get the existing value in the table cell
                            QString alreadyThere =
                                ui->table_newTrain->item(i, 2)->text();

                            // Split the existing value into parts
                            // using comma as the delimiter
                            QStringList parts = alreadyThere.split(",");

                            // Get the value to be added
                            QString newValue = record.first;

                            // Check if the parts list does not contain
                            // the value
                            if (!parts.contains(newValue)) {
                                // Append the value to the existing value
                                parts.push_back(newValue);
                                alreadyThere = parts.join(',');

                                // Show the selected point ID on the plot
                                QCPItemText *label =
                                    new QCPItemText(ui->plot_trains);
                                label->setPositionAlignment(
                                    Qt::AlignLeft|Qt::AlignBottom);
                                label->position->setType(
                                    QCPItemPosition::ptPlotCoords);
                                label->position->setCoords(
                                    record.second.first, record.second.second);
                                label->setText(
                                    QString("Point %1").arg(record.first));
                                label->setFont(QFont(font().family(), 10));
                                label->setPen(QPen(Qt::NoPen));

                            }

                            // Update the table cell with the modified string
                            ui->table_newTrain->item(i, 2)->setText(
                                alreadyThere);
                        } else {
                            // Create a new QTableWidgetItem with the value
                            QTableWidgetItem* item =
                                new QTableWidgetItem(record.first);
                            ui->table_newTrain->setItem(i, 2, item);

                            // Show the selected point ID on the plot
                            QCPItemText *label =
                                new QCPItemText(ui->plot_trains);
                            label->setPositionAlignment(
                                Qt::AlignLeft|Qt::AlignBottom);
                            label->position->setType(
                                QCPItemPosition::ptPlotCoords);
                            label->position->setCoords(
                                record.second.first, record.second.second);
                            label->setText(
                                QString("Point %1").arg(record.first));
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
                    // Check if the row corresponds to the currently
                    // selected item in the combo box
                    if (ui->table_newTrain->item(i, 0)->text().trimmed() ==
                        ui->combo_visualizeTrain->currentText()) {
                        // Get the existing value in the table cell
                        QString alreadyThere =
                            ui->table_newTrain->item(i, 2)->text();

                        // Split the existing value into parts
                        // using comma as the delimiter
                        QStringList parts = alreadyThere.split(",");

                        // Get the value to be removed
                        QString oldValue = record.first;

                        // Check if the parts list contains the
                        // value to be removed
                        if (parts.contains(oldValue)) {
                            // Remove the value from the parts list
                            parts.removeOne(oldValue);

                            // Reconstruct the string by joining the
                            // remaining parts with commas
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

    // Iterate through each row in the table and add the column
    // value to the combobox
    for (int row = 0; row < ui->table_newTrain->rowCount(); ++row) {
        QTableWidgetItem* item = ui->table_newTrain->item(row, 0);
        if (item) {
            ui->combo_visualizeTrain->addItem(item->text());
        }
    }

    connect(ui->combo_visualizeTrain,
            &QComboBox::currentIndexChanged, [this](){
        // remove all labels in the plot of the old train
        // Iterate over the items in the plot
        for (int i = 0; i < ui->plot_trains->itemCount(); ++i) {
            QCPAbstractItem* abstractItem = ui->plot_trains->item(i);

            // Check if the item is a QCPItemText
            if (QCPItemText* label = qobject_cast<QCPItemText*>(abstractItem))
            {
                ui->plot_trains->removeItem(label);
            }
        }

        // add already selected nodes to the plot
        for (int i = 0; i < ui->table_newTrain->rowCount(); i++) {
            if (ui->combo_visualizeTrain->count() == 0) { continue; }
            // Check if the row corresponds to the currently selected
            // item in the combo box
            if (ui->table_newTrain->item(i, 0)->text().trimmed() ==
                    ui->combo_visualizeTrain->currentText()) {
                // Check if the cell at column 2 exists
                if (ui->table_newTrain->item(i, 2)) {
                    // Get the existing value in the table cell
                    QString alreadyThere =
                        ui->table_newTrain->item(i, 2)->text();

                    // Split the existing value into parts using comma
                    // as the delimiter
                    QStringList parts = alreadyThere.split(",");

                    for (const auto &nodeID: parts) {
                        if (!this->networkNodes.is_key(nodeID)) { continue; }
                        auto record = this->networkNodes[nodeID];
                        // Show the selected point ID on the plot
                        QCPItemText *label = new QCPItemText(ui->plot_trains);
                        label->setPositionAlignment(
                            Qt::AlignLeft|Qt::AlignBottom);
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
    // ID
    ui->table_newNodes->setItemDelegateForColumn(0,
                                                 new NonEmptyDelegate("ID", this));
    QString IDToolTip = "A numerical identifier that is unique to each Node. "
                        "This number begins from 0 and extends indefinitely";
    ui->table_newNodes->horizontalHeaderItem(0)->setToolTip(IDToolTip);

    ui->table_newNodes->
        setItemDelegateForColumn(1,
                                 new NumericDelegate(this,
                                                     9999999999999999.999,
                                                     -9999999999999999.999,3,
                                                     1, 0.0));
    QString xToolTip = "Specifies the X-value of the Node Coordinate.";
    ui->table_newNodes->horizontalHeaderItem(1)->setToolTip(xToolTip);

    ui->table_newNodes->
        setItemDelegateForColumn(2,
                                 new NumericDelegate(this,
                                                     9999999999999999.999,
                                                     -9999999999999999.999,3,
                                                     1, 0.0));
    QString yToolTip = "Specifies the Y-value of the Node Coordinate.";
    ui->table_newNodes->horizontalHeaderItem(2)->setToolTip(yToolTip);

    // ---------- insert a new row to nodes ----------
    ui->table_newNodes->insertRow(0);
    // set the new id count as default value for the first cell of the new row
    std::unique_ptr<QTableWidgetItem> newItemID(
        new QTableWidgetItem(QString::number(1)));
    ui->table_newNodes->setItem(0, 0, newItemID.release());

    ui->table_newNodes->setItemDelegateForColumn(
        3, new TextBoxDelegate(this, "Not Defined"));

    QString descToolTip = "A brief overview or explanation "
                          "regarding the specific Node.";
    ui->table_newNodes->horizontalHeaderItem(3)->setToolTip(descToolTip);
}

void NeTrainSim::setupLinksTable() {
    // add the delegates to the links columns
    ui->table_newLinks->
        setItemDelegateForColumn(0, new NonEmptyDelegate("ID", this));
    QString IDToolTip = "A unique numerical identifier assigned to "
                        "each link, ranging from 0 to infinity.";
    ui->table_newLinks->horizontalHeaderItem(0)->setToolTip(IDToolTip);

    ui->table_newLinks->
        setItemDelegateForColumn(1,
                                 new IntNumericDelegate(this,
                                                        100000000000, 0, 1, 0));
    QString fromToolTip = "Indicates the first node ID associated "
                          "with that particular link.";
    ui->table_newLinks->horizontalHeaderItem(1)->setToolTip(fromToolTip);


    ui->table_newLinks->
        setItemDelegateForColumn(2,
                                 new IntNumericDelegate(this,
                                                        100000000000, 0, 1, 0));
    QString toToolTip = "Indicates the last node ID associated "
                          "with that particular link.";
    ui->table_newLinks->horizontalHeaderItem(2)->setToolTip(toToolTip);

    ui->table_newLinks->
        setItemDelegateForColumn(3,
                                 new NumericDelegate(this,
                                                     9999999999999999.99999,
                                                     0.001, 3, 1, 10.0));
    QString lengthToolTip = "Indicates the length of the link.";
    ui->table_newLinks->horizontalHeaderItem(3)->setToolTip(lengthToolTip);

    ui->table_newLinks->
        setItemDelegateForColumn(4,
                                 new NumericDelegate(this,
                                                     150, 5, 2, 5, 35.0));
    QString freeSpeedToolTip = "Indicates the maximum speed at which a "
                        "train can traverse on a given link.";
    ui->table_newLinks->horizontalHeaderItem(4)->setToolTip(freeSpeedToolTip);

    ui->table_newLinks->
        setItemDelegateForColumn(5,
                                 new IntNumericDelegate(this,
                                                        10000000000, 0, 1, 1));
    QString signalToolTip = "Identifies a unique signal placed at "
                            "the termination of each link.";
    ui->table_newLinks->horizontalHeaderItem(5)->setToolTip(signalToolTip);

    ui->table_newLinks->
        setItemDelegateForColumn(6,
                                 new TextBoxDelegate(this, "1,2"));
    QString signalLocToolTip = "Designates the node at which a "
                               "signal is to be positioned. "
                               "This node number "
                               "should match either the 'From node' "
                               "or 'To Node', or both separated by "
                               "a comma.";
    ui->table_newLinks->horizontalHeaderItem(6)->setToolTip(signalLocToolTip);

    ui->table_newLinks->
        setItemDelegateForColumn(7,
                                 new NumericDelegate(this, 5, -5, 3,
                                                     0.05, 0.0));
    QString gradeToolTip = "Represents the incline or decline grade "
                           "of the link, denoted as a percentage "
                           "without the percentage sign.";
    ui->table_newLinks->horizontalHeaderItem(7)->setToolTip(gradeToolTip);

    ui->table_newLinks->
        setItemDelegateForColumn(8,
                                 new NumericDelegate(this, 5, -5, 3,
                                                     0.05, 0.0));
    QString curvToolTip = "Represents the curvature degree of the "
                          "link, denoted as a percentage without "
                          "the percentage sign.";
    ui->table_newLinks->horizontalHeaderItem(8)->setToolTip(curvToolTip);

    ui->table_newLinks->
        setItemDelegateForColumn(9,
                                 new CheckboxDelegate(this));
    QString dirToolTip = "Represents the directionality of the link. "
                         "tick to indicate a one-way link, "
                         "untick to indicate a two-way link.";
    ui->table_newLinks->horizontalHeaderItem(9)->setToolTip(dirToolTip);

    ui->table_newLinks->
        setItemDelegateForColumn(10,
                                 new NumericDelegate(this, 1, 0, 2, 0.05, 0.2));
    QString varToolTip = "Represents the variation in max free flow speed.";
    ui->table_newLinks->horizontalHeaderItem(10)->setToolTip(varToolTip);

    ui->table_newLinks->
        setItemDelegateForColumn(11,
                                 new CheckboxDelegate(this));
    QString catenToolTip = "A value of '0' signifies that a link does "
                           "not incorporate a catenary, whereas '1' "
                           "denotes the presence of a catenary.";
    ui->table_newLinks->horizontalHeaderItem(11)->setToolTip(catenToolTip);

    ui->table_newLinks->
        setItemDelegateForColumn(12,
                                 new TextBoxDelegate(this, "Not Defined"));
    QString regionToolTip = "Defines the region to which the link "
                            "is associated.";
    ui->table_newLinks->horizontalHeaderItem(12)->setToolTip(regionToolTip);

    // ---------- insert a new row to nodes ----------
    ui->table_newLinks->insertRow(0);
    ui->table_newLinks->setupTable();

    // set the new id count as default value for the first cell of the new row
    std::unique_ptr<QTableWidgetItem> newLinkItemID(
        new QTableWidgetItem(QString::number(1)));
    ui->table_newLinks->setItem(0, 0, newLinkItemID.release());
}

void NeTrainSim::setupLocomotivesTable() {
    //set delegates for the locomotives table ID
    ui->table_newLocomotive->
        setItemDelegateForColumn(0,
                                 new NonEmptyDelegate("ID", this)); // ID
    QString idToolTip = "Represents the unique identifier "
                            "associated with that locomotive";
    ui->table_newLocomotive->horizontalHeaderItem(0)->setToolTip(idToolTip);


    ui->table_newLocomotive->
        setItemDelegateForColumn(1,
                                 new NumericDelegate(
                                     this, 10000, 100, 2, 100, 3000)); // power
    QString powerToolTip = "The max power of the locomotive in kW.";
    ui->table_newLocomotive->horizontalHeaderItem(1)->setToolTip(powerToolTip);

    ui->table_newLocomotive->
        setItemDelegateForColumn(2,
                                 new NumericDelegate(
                                     this, 1, 0, 2,
                                     0.05, 0.85)); // transmission eff
    QString tranToolTip = "The transmission eff of the locomotive.";
    ui->table_newLocomotive->horizontalHeaderItem(2)->setToolTip(tranToolTip);

    ui->table_newLocomotive->
        setItemDelegateForColumn(3,
                                 new NumericDelegate(
                                     this, 100, 0, 2, 1, 25)); // length
    QString lenToolTip = "The full length of the locomotive in meters.";
    ui->table_newLocomotive->horizontalHeaderItem(3)->setToolTip(lenToolTip);

    ui->table_newLocomotive->
        setItemDelegateForColumn(4,
                                 new NumericDelegate(
                                     this, 0.01, 0, 5,
                                     0.0001, 0.0055)); // streamline
    QString kToolTip = "The streamlining coefficient for the locomotive.";
    ui->table_newLocomotive->horizontalHeaderItem(4)->setToolTip(kToolTip);

    ui->table_newLocomotive->
        setItemDelegateForColumn(5,
                                 new NumericDelegate(
                                     this, 100, 0, 2, 1, 15)); // area
    QString areaToolTip = "The frontal area for the locomotive in sq. meters.";
    ui->table_newLocomotive->horizontalHeaderItem(5)->setToolTip(areaToolTip);

    ui->table_newLocomotive->
        setItemDelegateForColumn(6,
                                 new NumericDelegate(
                                     this, 400, 0, 2, 10, 150)); // weight
    QString weightToolTip = "The weight for the locomotive in metric tons.";
    ui->table_newLocomotive->horizontalHeaderItem(6)->setToolTip(
        weightToolTip);

    ui->table_newLocomotive->
        setItemDelegateForColumn(7,
                                 new IntNumericDelegate(
                                     this, 12, 2, 4)); //number of axles
    QString axleToolTip = "The number of axles in the locomotive.";
    ui->table_newLocomotive->horizontalHeaderItem(7)->setToolTip(
        axleToolTip);


    // Get ridoff the diesel-battery option
    const int originalSize = sizeof(TrainTypes::powerTypeStrings) /
                             sizeof(TrainTypes::powerTypeStrings[0]);

    // Calculate the new size (original size - 1 for the excluded element)
    const int newSize = originalSize - 1;
    std::string newPowerTypeStrings[newSize];
    {
        // Index for the new array
        int newIndex = 0;

        // Copy elements except "Diesel-Battery Locomotive"
        for(int i = 0; i < originalSize; ++i) {
            if(i != 3) {
                if (newIndex < newSize) { // Check to avoid out-of-bounds error
                    newPowerTypeStrings[newIndex++] = TrainTypes::powerTypeStrings[i];
                }
            }
        }
    }

    ui->table_newLocomotive->
        setItemDelegateForColumn(8,
                                 new ComboBoxDelegate(
                                     newPowerTypeStrings,
                                     this)); // power type
    QString typeToolTip = "The Locomotive power train type.";
    ui->table_newLocomotive->horizontalHeaderItem(8)->setToolTip(
        typeToolTip);


    // ---------- insert a new row to locomotives ----------
    ui->table_newLocomotive->insertRow(0);

//    // set the new id count as default value for the first cell of the new row
    std::unique_ptr<QTableWidgetItem> newItemID(
        new QTableWidgetItem(QString::number(1)));
    ui->table_newLocomotive->setItem(0, 0, newItemID.release());

}

void NeTrainSim::setupCarsTable() {
    // set the delegates for the cars table ID
    ui->table_newCar->setItemDelegateForColumn(
        0, new NonEmptyDelegate("ID", this));
    QString idToolTip = "Represents the unique identifier "
                        "associated with that car";
    ui->table_newCar->horizontalHeaderItem(0)->setToolTip(idToolTip);

    ui->table_newCar->setItemDelegateForColumn(
        1, new NumericDelegate(this, 100, 0, 2, 1, 25)); // length
    QString lenToolTip = "The full length of the car.";
    ui->table_newCar->horizontalHeaderItem(1)->setToolTip(lenToolTip);

    ui->table_newCar->setItemDelegateForColumn(
        2, new NumericDelegate(
               this, 0.01, 0, 5, 0.0001, 0.0055)); // streamline
    QString kToolTip = "The streamlining coefficient for the car.";
    ui->table_newCar->horizontalHeaderItem(2)->setToolTip(kToolTip);

    ui->table_newCar->setItemDelegateForColumn(
        3, new NumericDelegate(this, 100, 0, 2, 1, 15)); // area
    QString areaToolTip = "The frontal area of the car in sq. meters.";
    ui->table_newCar->horizontalHeaderItem(3)->setToolTip(areaToolTip);

    ui->table_newCar->setItemDelegateForColumn(
        4, new NumericDelegate(this, 400, 0, 2, 10, 150)); // weight
    QString wToolTip = "The tare weight of the car in metric tons.";
    ui->table_newCar->horizontalHeaderItem(4)->setToolTip(wToolTip);

    ui->table_newCar->setItemDelegateForColumn(
        5, new NumericDelegate(this, 400, 0, 2, 10, 150)); // weight
    QString fwToolTip = "The gross weight of the car in metric tons.";
    ui->table_newCar->horizontalHeaderItem(5)->setToolTip(fwToolTip);

    ui->table_newCar->setItemDelegateForColumn(
        6, new IntNumericDelegate(this, 12, 2, 4));
    QString axleToolTip = "The total number of axles in the car.";
    ui->table_newCar->horizontalHeaderItem(6)->setToolTip(axleToolTip);

    QString tToolTip = "The type of the car.";
    ui->table_newCar->horizontalHeaderItem(7)->setToolTip(tToolTip);


    // ---------- insert a new row to cars ----------
    ui->table_newCar->insertRow(0);
    // set the new id count as default value for the first cell of the new row
    std::unique_ptr<QTableWidgetItem> newItemID2(
        new QTableWidgetItem(QString::number(1)));
    ui->table_newCar->setItem(0, 0, newItemID2.release());

    ui->table_newCar->setItemDelegateForColumn(
        7, new ComboBoxDelegate(TrainTypes::carTypeStrings, this));
}

void NeTrainSim::setupConfigurationsTable() {
    // ---------- insert a new row to configurations ----------
    ui->table_newConfiguration->insertRow(0);
    // set the new id count as default value for the first cell of the new row
    std::unique_ptr<QTableWidgetItem> newItemID_config(
        new QTableWidgetItem(QString::number(1)));

    ui->table_newConfiguration->setItem(0, 0, newItemID_config.release());
    QString idToolTip = "The unique numerical identification of the consist."
                        " This value must be unique for each consist.";
    ui->table_newConfiguration->horizontalHeaderItem(0)->setToolTip(idToolTip);

    QString tToolTip = "Defines which vehicle type to add.";
    ui->table_newConfiguration->horizontalHeaderItem(1)->setToolTip(tToolTip);

    QString vtToolTip = "The unique numerical identification of "
                       "the vehicle type.";
    ui->table_newConfiguration->horizontalHeaderItem(2)->setToolTip(vtToolTip);

    QString cToolTip = "The number of instances of that vehicle to be added.";
    ui->table_newConfiguration->horizontalHeaderItem(3)->setToolTip(cToolTip);

    ui->table_newConfiguration->setItemDelegateForColumn(
        2, new NonEmptyDelegate("VehicleID", this, "1"));
    // Create a new combobox and set it as the item in the last
    // column of the new row
    std::string configV[] = {"Locomotive", "Car"};

    ui->table_newConfiguration->setItemDelegateForColumn(
        1, new ComboBoxDelegate(configV, this));

    ui->table_newConfiguration->setItemDelegateForColumn(
        3, new IntNumericDelegate(this, 999999, 0, 1,1));

    ui->table_newConfiguration->horizontalHeaderItem(0)->setToolTip(
        "the configuration ID should be the same for each train consist");
}

void NeTrainSim::setupTrainsTable() {

    // set the delegates for the trains table IDs
    ui->table_newTrain->setItemDelegateForColumn(
        0, new NonEmptyDelegate("ID", this));
    QString idToolTip = "The unique identifier of the train.";
    ui->table_newTrain->horizontalHeaderItem(0)->setToolTip(idToolTip);

    ui->table_newTrain->setItemDelegateForColumn(
        1, new NonEmptyDelegate("ConfigID", this, "1"));
    QString consistToolTip = "The unique identifier of the "
                             "consist configuration that is "
                             "defined in the upper table.";
    ui->table_newTrain->horizontalHeaderItem(1)->setToolTip(consistToolTip);

    ui->table_newTrain->
        setItemDelegateForColumn(2, new TextBoxDelegate(this, "1,2"));
    QString nodesToolTip = "All the Nodes the train should path on. "
                           "You can define only the start and end nodes."
                           "These can be defined in "
                           "the 'Define Trains Path' tab";
    ui->table_newTrain->horizontalHeaderItem(2)->setToolTip(nodesToolTip);

    ui->table_newTrain->setItemDelegateForColumn(
        3, new NumericDelegate(this, 1000000000000.0, 0, 1, 100,0));
    QString timeToolTip = "Time offset (measured from simulation "
                           "start time) for the train to enter the network.";
    ui->table_newTrain->horizontalHeaderItem(3)->setToolTip(timeToolTip);

    ui->table_newTrain->setItemDelegateForColumn(
        4, new IntNumericDelegate(this, 300, 1, 1, 1));
    QString locosToolTip = "Total number of locomotives in the train.";
    ui->table_newTrain->horizontalHeaderItem(4)->setToolTip(locosToolTip);

    ui->table_newTrain->setItemDelegateForColumn(
        5, new IntNumericDelegate(this, 300, 1, 1, 1));
    QString carsToolTip = "Total number of cars in the train.";
    ui->table_newTrain->horizontalHeaderItem(5)->setToolTip(carsToolTip);

    ui->table_newTrain->setItemDelegateForColumn(
        6, new NumericDelegate(this, 1, 0, 2, 0.05, 0.95));
    QString fricToolTip = "The friction coefficient of the train's "
                          "wheels with the track.";
    ui->table_newTrain->horizontalHeaderItem(6)->setToolTip(fricToolTip);

    // ---------- insert a new row to Trains ----------
    ui->table_newTrain->insertRow(0);
    // add the train 0 to combo visualize Train
    this->updateCombo_visualizeTrains();

    // set the new id count as default value for the first cell of the new row
    std::unique_ptr<QTableWidgetItem> newItemID_train(
        new QTableWidgetItem(QString::number(1)));
    ui->table_newTrain->setItem(0, 0, newItemID_train.release());

    ui->table_newTrain->horizontalHeaderItem(1)->setToolTip(
        "add the configuration id from the table above");
    ui->table_newTrain->horizontalHeaderItem(2)->setToolTip(
        "add the node ids separated by a comma");

}


// get data from table
Vector<Map<std::string, std::string>> NeTrainSim::getNodesDataFromNodesTable()
{
    // clear the networkNodes map
    this->networkNodes.clear();
    Vector<Map<std::string, std::string>> nodesRecords;
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
            desc   = descItem? descItem->text().trimmed().toStdString() : "";


            nodesRecords.push_back({
                {"UserID", std::to_string(label.toInt())},
                {"XCoordinate", std::to_string(xCoord)},
                {"YCoordinate", std::to_string(yCoord)},
                {"Desc", desc},
                {"XScale", std::to_string(xScale)},
                {"YScale", std::to_string(yScale)}
            });

            this->networkNodes[label] = std::make_pair(xCoord * xScale,
                                                       yCoord * yScale);
        }
    }
    return nodesRecords;
}

// get data from table
Vector<Map<std::string,
           std::string>> NeTrainSim::getNodesDataFromNodesFile(
    QString fileName) {

    // clear the networkNodes map
    this->networkNodes.clear();
    Vector<Map<std::string, std::string>> nodesRecords;

    if (fileName.trimmed().isEmpty()) { return nodesRecords; }
    auto records = ReadWriteNetwork::readNodesFile(fileName.toStdString());

    for (auto& record: records) {
        this->networkNodes[QString::number(std::stoi(record["UserID"]))] =
            std::make_pair(std::stod(record["XCoordinate"]) *
                                std::stod(record["XScale"]),
                           std::stod(record["YCoordinate"]) *
                               std::stod(record["YScale"]));
    }

    return records;
}


// create a slot to update the QCustomPlot data and redraw the plot
void NeTrainSim::updateNodesPlot(CustomPlot &plot, QVector<double>xData,
                                 QVector<double>yData,
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
            if (!qIsNaN(xData[i]) &&
                !qIsNaN(yData[i] && !labels[i].trimmed().isEmpty())) {
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
Vector<Map<std::string, std::string>> NeTrainSim::getLinkesDataFromLinksFile (
    QString fileName)
{
    if (fileName.isEmpty()) {
        return Vector<Map<std::string, std::string>>();
    }
    try {
        auto records = ReadWriteNetwork::readLinksFile(fileName.toStdString());
        return records;
    }
    catch (std::exception &e) {
        this->showWarning(e.what());
        return Vector<Map<std::string, std::string>>();
    }

}


/**
 * Retrieves the link data from the links table and returns a vector
 * of link records.
 *
 * @return A vector of link records containing the link data from the table.
 */
Vector<Map<std::string, std::string>> NeTrainSim::getLinkesDataFromLinksTable()
{

    Vector<Map<std::string, std::string>> linksRecords;

    // get the data from the QTableWidget
    for (int i = 0; i < ui->table_newLinks->rowCount(); i++) {
        // get the item at row 0 and column 0 of the table widget
        QTableWidgetItem* fromItem = ui->table_newLinks->item(i, 1);
        QTableWidgetItem* toItem = ui->table_newLinks->item(i, 2);

        int direction = 1;
        int hasCaten = 0;
        // Check if the required cells are not empty
        if (fromItem && toItem && !fromItem->text().isEmpty() &&
            !toItem->text().isEmpty() ){

            // Check if the row is not empty in specific columns
            if (ui->table_newLinks->isRowEmpty(i, {0,6,9,11,12})) {
                continue;
            }

            // Substitute 'view' with your actual view object
            QAbstractItemModel *model =
                ui->table_newLinks->model();



            int id =
                ui->table_newLinks->item(i, 0)
                    ? ui->table_newLinks->item(i, 0)->text().
                      trimmed().toInt() : 0;
            int from =
                ui->table_newLinks->item(i, 1)
                    ? ui->table_newLinks->item(i, 1)->text().
                      trimmed().toInt() : 0;

            int to =
                ui->table_newLinks->item(i, 2)
                    ? ui->table_newLinks->item(i, 2)->text().
                      trimmed().toInt() : 0;

            double length =
                ui->table_newLinks->item(i, 3)
                    ? ui->table_newLinks->item(i, 3)->text().
                      trimmed().toDouble() : 0.0;

            double freespeed =
                ui->table_newLinks->item(i, 4)
                    ? ui->table_newLinks->item(i, 4)->text().
                      trimmed().toDouble() : 0.0;

            int signalNo =
                ui->table_newLinks->item(i, 5)
                    ? ui->table_newLinks->item(i, 5)->text().
                      trimmed().toInt() : 0;

            std::string signalsAt =
                ui->table_newLinks->item(i, 6)
                    ? ui->table_newLinks->item(i, 6)->text().
                      trimmed().toStdString(): "";

            double grade =
                ui->table_newLinks->item(i, 7)
                    ? ui->table_newLinks->item(i, 7)->text().
                      trimmed().toDouble() : 0.0;

            double curvature =
                ui->table_newLinks->item(i, 8)
                    ? ui->table_newLinks->item(i, 8)->text().
                      trimmed().toDouble() : 0.0;

            QModelIndex directionIndex = model->index(i, 9);
            direction = (directionIndex.
                         data(Qt::CheckStateRole).toBool())? 2: 1;

            double variation =
                ui->table_newLinks->item(i, 10)
                    ? ui->table_newLinks->item(i, 10)->text().
                      trimmed().toDouble() : 0.0;

            QModelIndex catenaryIndex = model->index(i,11);
            hasCaten = (catenaryIndex.
                        data(Qt::CheckStateRole).toBool()) ? 1 : 0;

            std::string region =
                ui->table_newLinks->item(i, 12)
                    ? ui->table_newLinks->item(i, 12)->text().
                      trimmed().toStdString() : "";

            // if the region is defined, signalsAt must be defined
            if (! region.empty() && signalsAt.empty()) { signalsAt = "NA"; }

            double lengthScale =
                ui->doubleSpinBox_LengthScale->value();
            double speedScale =
                ui->doubleSpinBox_SpeedScale->value();

            // Add the link record to the vector
            linksRecords.push_back(
                {
                    {"UserID", std::to_string(id)},
                    {"FromNodeID", std::to_string(from)},
                    {"ToNodeID", std::to_string(to)},
                    {"Length", std::to_string(length)},
                    {"SignalNo", std::to_string(signalNo)},
                    {"Directions", std::to_string(direction)},
                    {"FreeFlowSpeed", std::to_string(freespeed)},
                    {"DirectionalGrade", std::to_string(grade)},
                    {"Curvature", std::to_string(curvature)},
                    {"SpeedVariation", std::to_string(variation)},
                    {"SignalsAtNodes", signalsAt},
                    {"Region", region},
                    {"HasCatenary", std::to_string(hasCaten)},
                    {"LengthScale", std::to_string(lengthScale)},
                    {"FreeFlowSpeedScale",
                     std::to_string(speedScale)}
                });

        }
    }
    return linksRecords;
}

/**
 * Retrieves the start and end node IDs from the link records and returns
 * them as plottable data.
 *
 * @param linksRecords A vector of link records containing the link data.
 * @return A tuple containing the start and end node IDs as plottable data.
 */
tuple<QVector<QString>,
           QVector<QString>> NeTrainSim::getLinksPlottableData(
    Vector<Map<std::string, std::string>> linksRecords) {

    QVector<QString> startNodes;
    QVector<QString> endNodes;

    // Iterate through the link records
    for (auto& record: linksRecords) {
        startNodes.push_back(QString::fromStdString(record["FromNodeID"]));
        endNodes.push_back(QString::fromStdString(record["ToNodeID"]));
    }

    return std::make_tuple(startNodes, endNodes);
}

// create a slot to update the QCustomPlot data and redraw the plot
void NeTrainSim::updateLinksPlot(CustomPlot &plot,
                                 QVector<QString> startNodeIDs,
                                 QVector<QString> endNodeIDs) {
    // check if the plot has at least 2 graphs
    if (plot.graphCount() < 2) { return; }
    // get the QCPGraph object for the graph in the QCustomPlot
    QCPGraph *graph = plot.graph(1);

    graph->data()->clear();
    plot.replot();

    if (startNodeIDs.size() != endNodeIDs.size() ||
        startNodeIDs.size() < 1 || this->networkNodes.size() < 1) {
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

            std::pair<double, double> fromNode =
                this->networkNodes[fromItem.trimmed()];
            std::pair<double, double> toNode =
                this->networkNodes[toItem.trimmed()];
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
 * Retrieves the trains data from the tables and returns it
 * as a vector of train records.
 *
 * @return A vector of train records containing the trains
 *         data from the tables.
 */
Vector<Map<std::string, std::any>> NeTrainSim::getTrainsDataFromTables() {

    Vector<Map<std::string, std::any>> trains;

//    try {

    Map<QString, Map<std::string, std::string>> tableLocomotives;
    Map<QString, Map<std::string, std::string>> tableCars;

    Map<QString, Map<QString, QVector<std::pair<QString,
                                                int>>>> configTable;

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

            std::string theType = ui->table_newLocomotive->item(i, 8)
                                      ? ui->table_newLocomotive->
                                        item(i, 8)->text().
                                        trimmed().toStdString() :
                                      TrainTypes::powerTypeStrings[0];
            int type = TrainTypes::powerTypestrToInt(theType);

            Map<std::string, std::string> loco =
                {
                {"Power", ui->table_newLocomotive->item(i, 1)
                        ? ui->table_newLocomotive->item(i, 1)->text().
                          trimmed().toStdString() : "0.0"}, //power
                {"TransmissionEff", ui->table_newLocomotive->item(i, 2)
                        ? ui->table_newLocomotive->item(i, 2)->text().
                          trimmed().toStdString() : "0.0"}, //trans eff
                {"NoOfAxles", ui->table_newLocomotive->item(i, 7)
                        ? ui->table_newLocomotive->item(i, 7)->text().
                          trimmed().toStdString() : "0"}, // axles
                {"AirDragCoeff", ui->table_newLocomotive->item(i, 4)
                        ? ui->table_newLocomotive->item(i, 4)->text().
                          trimmed().toStdString() : "0.0"}, // k
                {"FrontalArea", ui->table_newLocomotive->item(i, 5)
                        ? ui->table_newLocomotive->item(i, 5)->text().
                          trimmed().toStdString() : "0.0"}, // area
                {"Length", ui->table_newLocomotive->item(i, 3)
                        ? ui->table_newLocomotive->item(i, 3)->text().
                          trimmed().toStdString() : "0.0"}, //length
                {"GrossWeight", ui->table_newLocomotive->item(i, 6)
                     ? ui->table_newLocomotive->item(i, 6)->text().
                       trimmed().toStdString() : "0.0"}, // weight
                {"Type", std::to_string(type)} // type
                };


            tableLocomotives[ui->table_newLocomotive->item(i, 0)->text().
                             trimmed()] = loco;
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

            std::string theType = ui->table_newCar->item(i, 7)
                                      ? ui->table_newCar->item(i, 7)->text().
                                        trimmed().toStdString() : "Cargo Car";
            int type = TrainTypes::carTypestrToInt(theType);

            Map<std::string, std::string> car = {
                {"NoOfAxles", ui->table_newCar->item(i, 6)
                         ? ui->table_newCar->item(i, 6)->text().
                           trimmed().toStdString() : "0"}, // axles
                {"AirDragCoeff",ui->table_newCar->item(i, 2)
                         ? ui->table_newCar->item(i, 2)->text().
                           trimmed().toStdString() : "0.0"}, // air drag
                {"FrontalArea",ui->table_newCar->item(i, 3)
                         ? ui->table_newCar->item(i, 3)->text().
                           trimmed().toStdString() : "0.0"}, // area
                {"Length", ui->table_newCar->item(i, 1)
                         ? ui->table_newCar->item(i, 1)->text().
                           trimmed().toStdString() : "0.0"}, // length
                {"GrossWeight", ui->table_newCar->item(i, 5)
                         ? ui->table_newCar->item(i, 5)->text().
                           trimmed().toStdString() : "0.0"}, // full weight
                {"TareWeight", ui->table_newCar->item(i, 4)
                         ? ui->table_newCar->item(i, 4)->text().
                           trimmed().toStdString() : "0.0"}, // empty weight
                {"Type", std::to_string(type)} // type
            };

            tableCars[ui->table_newCar->item(i, 0)->text().trimmed()] = car;

        }


        // --------------------------------------------------------------
        // ------------------ Configurations Table ----------------------
        // --------------------------------------------------------------

        // check if the Configurations table does not have
        // at least a complete row
        if (ui->table_newConfiguration->isTableIncomplete({0,1,3})) {
            throw std::invalid_argument("Configurations Table is empty!");
            return trains;
        }

        // check if the cars table has any empty cell
        if (ui->table_newConfiguration->hasEmptyCell({0,1,3})) {
            throw std::invalid_argument(
                "Configurations Table has empty cells!");
            return trains;
        }

        for (int i = 0; i<ui->table_newConfiguration->rowCount(); i++) {
            if (ui->table_newConfiguration->isRowEmpty(i, {0,1,3}))
            {
                continue;
            }

            // if the config ID exists
            if (configTable.get_keys().
                exist(ui->table_newConfiguration->item(i,0)->text().
                                             trimmed()))
            {
                std::string type = ui->table_newConfiguration->item(i, 1)
                    ? ui->table_newConfiguration->item(i, 1)->text().
                      trimmed().toStdString() : "";

                // get the number of instances
                int countInstances =
                    ui->table_newConfiguration->item(i, 3)
                        ? ui->table_newConfiguration->item(i, 3)->text().
                          trimmed().toInt() : 0;


                configTable[
                    ui->table_newConfiguration->item(i,0)->text().trimmed()][
                    QString::fromStdString(type)].push_back(      //loco or car
                        std::make_pair(
                        ui->table_newConfiguration->item(i,2)->text().trimmed(),
                        countInstances));
            }
            // if the ID does not exist
            else {
                Map<QString, QVector<std::pair<QString, int>>> instances;

                // Get the combobox widget from the cell
                std::string type =
                    ui->table_newConfiguration->item(i, 1)
                        ? ui->table_newConfiguration->item(i, 1)->text().
                          trimmed().toStdString() : "";

                // get the number of instances
                int countInstances =
                    ui->table_newConfiguration->item(i, 3)
                        ? ui->table_newConfiguration->item(i, 3)->text().
                          trimmed().toInt() : 0;

                instances[QString::fromStdString(type)].push_back(
                    std::make_pair(
                        ui->table_newConfiguration->item(i,2)->text().trimmed(),
                                                           countInstances));
                // add the map to the config table
                configTable[
                    ui->table_newConfiguration->item(i,0)->text().
                            trimmed()] = instances;
            }
        }

        // --------------------------------------------------------------
        // ----------------------- Trains Table -------------------------
        // --------------------------------------------------------------

        // check if the Configurations table does not have
        // at least a complete row
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

            auto trainID =
                ui->table_newTrain->item(i,0)->text().trimmed().toStdString();
            auto trainConfig = ui->table_newTrain->item(i,1)->text().trimmed();
            auto trainPathStrings =
                ui->table_newTrain->item(i,2)->text().trimmed().
                                    split(',').toVector();
            Vector<int> trainPath;
            for (const QString& str : trainPathStrings) {
                if (str.isEmpty()) {
                    continue;
                }
                trainPath.push_back(str.toInt());
            }
            auto startTime =
                ui->table_newTrain->item(i,3)->text().trimmed().toDouble();
            auto locoCount =
                ui->table_newTrain->item(i,4)->text().trimmed().toInt();
            auto carCount =
                ui->table_newTrain->item(i,5)->text().trimmed().toInt();
            auto frictionCof =
                ui->table_newTrain->item(i,6)->text().trimmed().toDouble();

            // declare the variables
            Vector<Map<std::string, std::string>> locomotivesRecords;

            Vector<Map<std::string, std::string>> carsRecords;

            if (! configTable.is_key(trainConfig)) {
                throw std::invalid_argument(
                    "Could not find configuration ID: " +
                    trainConfig.toStdString());
            }


            if (! configTable[trainConfig].is_key("Locomotive")) {
                throw std::invalid_argument("Consist " +
                                            trainConfig.toStdString() +
                                            " does not have locomotives");
            }

            for (auto &vehicle: configTable[trainConfig]["Locomotive"]){
                if (tableLocomotives.is_key(vehicle.first)) {
                    // get the loco record that corresponds to the id
                    auto lr = tableLocomotives[vehicle.first];

                    Map<std::string, std::string> loco = {
                        {"Count", std::to_string(vehicle.second)}
                    };
                    loco.insert(lr.begin(), lr.end());

                    locomotivesRecords.push_back(loco);
                }
                else {
                    throw std::invalid_argument(
                        "Could not find locomotive: " +
                        vehicle.first.toStdString());
                }
            }

            int locoCountPrv = 0;
            for (auto& loc: locomotivesRecords) {
                locoCountPrv += stoi(loc["Count"]);
            }
            // double check the locos and cars counts
            if (locoCount != locoCountPrv) {
                throw std::runtime_error("Error: " +
                                         std::to_string(
                                             static_cast<int>(
                                             Error::trainHasWrongLocos)) +
                                         "\nlocomotives count does" +
                                         " not match added locomotives!");
            }

            if (configTable[trainConfig].is_key("Car")) {
                for (auto &vehicle: configTable[trainConfig]["Car"]){
                    if (tableCars.is_key(vehicle.first))
                    {
                    auto cr = tableCars[vehicle.first];

                    Map<std::string, std::string> car =
                        {
                            {"Count", std::to_string(vehicle.second)}
                        };

                    car.insert(cr.begin(), cr.end());
                    carsRecords.push_back(car);
                    }
                    else {
                    throw std::invalid_argument(
                        "Could not find car: " +
                        vehicle.first.toStdString());
                    }
                }
            }

            int cCountPrv = 0;
            for (auto& c: carsRecords) {
                cCountPrv += stoi(c["Count"]);
            }

            if (carCount != cCountPrv) {
                throw std::runtime_error("Error: " +
                                         std::to_string(
                                             static_cast<int>(
                                             Error::trainHasWrongLocos)) +
                                         "\ncars count does not " +
                                         "match added cars!");
            }

            Map<std::string, std::any> record = {
                                                 {"UserID",trainID},
                {"TrainPathOnNodeIDs", trainPath},
                {"LoadTime", startTime},
                {"FrictionCoef", frictionCof},
                {"Locomotives", locomotivesRecords},
                {"Cars", carsRecords},
                {"Optimize", false}
            };
            trains.push_back(record);
        }

        return trains;
}

QString NeTrainSim::browseFiles(QLineEdit* theLineEdit,
                                const QString& theFileName) {
    QString browsLoc = QString();
    if ( this->userBrowsePath.isEmpty()) {
        browsLoc = this->defaultBrowsePath;
    }
    else {
        browsLoc = this->userBrowsePath;
    }
    QString fname = QFileDialog::getOpenFileName(nullptr, theFileName,
                                                 browsLoc, "DAT Files (*.DAT)");
    if (!fname.isEmpty()) {
        theLineEdit->setText(fname);
        QFileInfo fileInfo(fname);
        this->userBrowsePath = fileInfo.dir().path();
    }
    return fname;
}

void NeTrainSim::browseFolder(QLineEdit* theLineEdit,
                              const QString& theHelpMessage) {
    QString browsLoc = QString();
    if ( this->userBrowsePath.isEmpty()) {
        browsLoc = this->defaultBrowsePath;
    }
    else {
        browsLoc = this->userBrowsePath;
    }
    QString folderPath =
        QFileDialog::getExistingDirectory(this,
                                          theHelpMessage,
                                          browsLoc,
                                          QFileDialog::ShowDirsOnly |
                                              QFileDialog::DontResolveSymlinks);
    // Check if a folder was selected
    if (!folderPath.isEmpty()) {
        theLineEdit->setText(folderPath);
    }
}

void NeTrainSim::setNodesData(QVector<double>& xData,
                              QVector<double>& yData,
                              QVector<QString>& labels) {
    if (this->nodesXData != xData || this->nodesYData != yData ||
        this->nodesLabelData != labels) {
        this->nodesXData = xData;
        this->nodesYData = yData;
        this->nodesLabelData = labels;
        emit this->nodesDataChanged(xData, yData, labels);
    }
}

tuple<QVector<double>, QVector<double>,
           QVector<QString>> NeTrainSim::getNodesPlottableData(
    Vector<Map<std::string, std::string>> &nodeRecords)
{
    QVector<double> xData; QVector<double> yData; QVector<QString> labels;

    for (auto& record:nodeRecords) {
        labels.push_back(QString::number(std::stoi(record["UserID"])));
        xData.push_back(std::stod(record["XCoordinate"]) *
                        std::stod(record["XScale"]));
        yData.push_back(std::stod(record["YCoordinate"]) *
                        std::stod(record["YScale"]));
    }
    return std::make_tuple(xData, yData, labels);
}

void NeTrainSim::setLinksData(QVector<QString>& startNodeIDs,
                              QVector<QString> endNodeIDs) {
    if (this->linksStartNodeIDs != startNodeIDs ||
        this->linksEndNodeIDs != endNodeIDs)
    {
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

        Vector<Map<std::string, std::string>> nodeRecords;
        Vector<Map<std::string, std::string>> linkRecords;
        Vector<Map<std::string, std::any>> trainRecords;

        if (ui->table_newNodes->hasEmptyCell({0,3})) {
            this->showWarning("Missing values in nodes table!");
            return;
        }
        if (ui->table_newLinks->hasEmptyCell({0,6,9,11,12})) {
            this->showWarning("Missing values in links table!");
            return;
        }
        nodeRecords = this->getNodesDataFromNodesTable();
        linkRecords = this->getLinkesDataFromLinksTable();


        // read trains from table and generate instances of trains
        try {
            trainRecords = this->getTrainsDataFromTables();
        } catch (const std::exception& e) {
            ErrorHandler::showError(e.what());
            return;
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
            summaryFilename =
                ui->lineEdit_summaryfilename->text().trimmed().toStdString();
        }
        else {
            this->showWarning("Summary filename is not set!");
            return;
        }

        bool exportAllTrainsSummary =
            ui->checkBox_detailedTrainsSummay->checkState() == Qt::Checked;
        bool exportInta =
            ui->checkBox_exportTrajectory->checkState() == Qt::Checked;

        std::string instaFilename = "";
        if (! ui->lineEdit_trajectoryFilename->text().trimmed().isEmpty() &&
            exportInta) {
            instaFilename =
                ui->lineEdit_trajectoryFilename->text().trimmed().toStdString();
        }
        else if (exportInta &&
                   ui->lineEdit_trajectoryFilename->text().trimmed().isEmpty())
        {
            this->showWarning("Summary filename is not set!");
            return;
        }
        else {
            instaFilename = "";
        }


        std::string netName =
            ui->lineEdit_networkName->text().
                                  trimmed().isEmpty() ? "Not Defined" :
                                  ui->lineEdit_networkName->text().
                                                        trimmed().toStdString();
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

        if (worker == nullptr) {
            ErrorHandler::showError("Error!");
            return;
        }

//        connect(worker, &SimulationWorker::trainSlowSpeed,
//                [this](const std::string &msg){
//            this->showWarning(
//                QString::fromStdString(msg));
//            });

//        connect(worker, &SimulationWorker::trainSuddenAcceleration,
//                [this](const std::string &msg){
//                    this->showWarning(
//                        QString::fromStdString(msg));
//                });

        // handle any error that arise from the simulator
        connect(worker, &SimulationWorker::errorOccurred, this,
                &NeTrainSim::handleError);

        // update the progress bar
        connect(worker, &SimulationWorker::simulaionProgressUpdated,
                ui->progressBar, &QProgressBar::setValue);

        // Connect the operationCompleted signal to a slot in your GUI class
        connect(worker, &SimulationWorker::simulationFinished, this,
                &NeTrainSim::handleSimulationFinished);

        // replot the trains coordinates
        connect(worker, &SimulationWorker::trainsCoordinatesUpdated, this,
                [this](Vector<
                       std::pair<std::string,
                                 Vector<std::pair<double,
                                                  double>>>>
                                                    trainsStartEndPoints)
                {
                    updateTrainsPlot(trainsStartEndPoints);
                });

        // hide the pause button
        ui->pushButton_pauseResume->setVisible(true);

        // move the worker to the new thread
        worker->moveToThread(thread);

        // disable the simulate button
        connect(thread, &QThread::started, this, [=, this]() {
            this->ui->pushButton_projectNext->setEnabled(false);
        });
        // connect the do work to thread start
        connect(thread, &QThread::started, worker, &SimulationWorker::doWork);

        // start the simulation
        thread->start();


    } catch (const std::exception& e) {
        ErrorHandler::showError(e.what());
        // hide the pause button
        ui->pushButton_pauseResume->setVisible(false);
    }
}


// Slot to handle the simulation finished signal
void NeTrainSim::handleSimulationFinished(
    Vector<std::pair<std::string,
                     std::string>> summaryData,
    std::string trajectoryFile)
{
    // enable the results window
    ui->tabWidget_project->setTabEnabled(4, true);
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
    ui->tabWidget_project->setCurrentIndex(4);
    ui->pushButton_pauseResume->setVisible(false);
}

void NeTrainSim::saveProjectFile(bool saveAs) {
    if (projectFileName.isEmpty() || saveAs) {
        QString saveFilePath =
            QFileDialog::getSaveFileName(this, "Save Project",
                                         QDir::homePath(),
                                         "NeTrainSim Files (*.NTS)");
        if (saveFilePath.isEmpty()) {
            return;
        }
        projectFileName = saveFilePath;
    }

//    try {
        projectName =
         (!ui->lineEdit_projectName->text().trimmed().
                    isEmpty()) ? ui->lineEdit_projectName->text().
                                          trimmed() : "Not Defined";
        author =
            (!ui->lineEdit_createdBy->text().trimmed().
                   isEmpty()) ? ui->lineEdit_createdBy->text().
                                          trimmed() : "Not Defined";
        networkName =
            (!ui->lineEdit_networkName->text().trimmed().
                        isEmpty()) ? ui->lineEdit_networkName->text().
                                          trimmed() : "Not Defined";
        QString simulationEndTime =
            QString::number(std::max(ui->doubleSpinBox->text().
                                          trimmed().toDouble(), 0.0));
        QString simulationTimestep =
            QString::number(std::max(
                ui->doubleSpinBox_timeStep->text().trimmed().toDouble(), 0.1));
        QString simulationPlotTime =
            QString::number(ui->doubleSpinBox->text().trimmed().toDouble());

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

        XMLManager::createProjectFile(projectName, networkName, author,
                                      nodesFilename, linksFilename,
                                      trainsFilename, simulationEndTime,
                                      simulationTimestep, simulationPlotTime,
                                      projectFileName);

        showNotification("File Saved Successfully");
//    } catch (const std::exception& e) {
//        this->showWarning(e.what());
//    }
}

void NeTrainSim::updateTrainsPlot(
        Vector<std::pair<std::string,
                     Vector<std::pair<double,double>>>> trainsStartEndPoints)
{
    // check if the plot has at least 2 graphs, that means no links added
    if (ui->plot_simulation->graphCount() < 2) { return; }

    // check if the vector is empty
    if (trainsStartEndPoints.empty()) {return; }

    // Create a new legend if the plot does not already have one
    ui->plot_simulation->legend->setVisible(true);

    // the first graph is for nodes and the second is for links
    // starting from the third, is the trains graphs
    while((ui->plot_simulation->graphCount() - 2) <
           trainsStartEndPoints.size())
    {
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
        index = std::min(index,
                         static_cast<int>(trainsStartEndPoints.size()) - 1);

        // Set the name for the graph in the legend
        graph->setName(QString::fromStdString(
                                        trainsStartEndPoints[index].first));

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
    QObject::connect(this->report, &QtRPT::setDSInfo, this,
                     &NeTrainSim::setDSInfo);

    this->report->loadReport(":/resources/report.xml");

    this->printer = new QPrinter(QPrinter::PrinterResolution);
    this->printer->setOutputFormat(QPrinter::PdfFormat);

    QPageLayout pageLayout(QPageSize(QPageSize::A4),
                           QPageLayout::Portrait, QMarginsF(0, 0, 0, 0));
    this->printer->setPageLayout(pageLayout);
    this->printer->setFullPage(true);

    // connect the print preview to the report
    connect(ui->widget_SummaryReport, SIGNAL(paintRequested(QPrinter*)),
            this->report, SLOT(printPreview(QPrinter*)));

    auto popup = [=, this]() {
        this->report->printExec();
    };

    QObject::connect(ui->pushButton_popoutPreview,
                     &QPushButton::clicked, popup);
}


void NeTrainSim::setValue(const int recNo, const QString paramName,
                          QVariant &paramValue, const int reportPage)
{
    if (paramName == "Description") {
//        if (reportPage == 0) {
        paramValue = QString::fromStdString(this->trainsSummaryData[recNo].
                                            first);
//        }
    }
    if (paramName == "Value") {
        paramValue = QString::fromStdString(this->trainsSummaryData[recNo].
                                            second);
    }

    if (paramName == "Project") {
        paramValue = QString("Project: ") +
                     (ui->lineEdit_projectName->text().trimmed().isEmpty() ?
                          QString("Not Defined") :
                          ui->lineEdit_projectName->text().trimmed());
    }
    if (paramName == "Network") {
        paramValue = QString("Network: ") +
                     (ui->lineEdit_networkName->text().trimmed().isEmpty() ?
                          QString("Not Defined") :
                          ui->lineEdit_networkName->text().trimmed());
    }
    if (paramName == "Author") {
        paramValue = QString("Author: ") +
                     (ui->lineEdit_createdBy->text().trimmed().isEmpty() ?
                          QString("Not Defined") :
                          ui->lineEdit_createdBy->text().trimmed());
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
        auto selectedTrain =
            CSV->filterByColumn(df, 0,
                                ui->comboBox_trainsResults->currentText());
        int columnNumber =
            ui->comboBox_resultsXAxis->currentText() == "Distance" ? 2 : 1;
        QString xAxisLabel = ui->comboBox_resultsXAxis->currentText() ==
                                     "Distance" ? "Distance (km)" :
                                 "Time (hr)";
        double xDataFactor = ui->comboBox_resultsXAxis->currentText() ==
                                     "Distance" ? 1.0/1000 : 1.0 / 3600.0;
        auto xData = Utils::factorQVector(
            Utils::convertQStringVectorToDouble(
                CSV->getColumnValues(selectedTrain, columnNumber)),
            xDataFactor);
        auto grades =
            Utils::convertQStringVectorToDouble(
                CSV->getColumnValues(selectedTrain, 13));
        auto curvatures =
            Utils::convertQStringVectorToDouble(
                CSV->getColumnValues(selectedTrain, 14));
        auto speeds =
            Utils::factorQVector(
                Utils::convertQStringVectorToDouble(
                    CSV->getColumnValues(selectedTrain, 4)), 3.6);
        auto accelerations =
            Utils::convertQStringVectorToDouble(
                CSV->getColumnValues(selectedTrain, 3));
        auto EC =
            Utils::convertQStringVectorToDouble(
                CSV->getColumnValues(selectedTrain, 6));
        auto tractiveForces =
            Utils::factorQVector(
                Utils::convertQStringVectorToDouble(
                    CSV->getColumnValues(selectedTrain, 10)), 1.0/1000.0);
        auto resistance =
            Utils::factorQVector(
                Utils::convertQStringVectorToDouble(
                    CSV->getColumnValues(selectedTrain, 11)), 1.0/1000.0);
        auto totalForces =
            Utils::subtractQVector(tractiveForces, resistance);

        this->drawLineGraph(
            *ui->plot_trajectory_grades, xData, grades,
            xAxisLabel, "Percentage", "Grades", 0);
        this->drawLineGraph(
            *ui->plot_trajectory_grades, xData, curvatures,
            xAxisLabel, "Percentage", "Curvatures", 1);
        this->drawLineGraph(
            *ui->plot_trajectory_speed, xData, speeds,
            xAxisLabel, "Speed (km/h)", "Speed", 0);
        this->drawLineGraph(
            *ui->plot_trajectory_acceleration, xData,
            accelerations, xAxisLabel, "Accelerations (m/s^2)",
            "Acceleration", 0);
        this->drawLineGraph(
            *ui->plot_trajectory_EC, xData, EC, xAxisLabel,
            "Energy Consumption (kWh)", "Energy", 0);

        this->drawLineGraph(
            *ui->plot_forces_grades, xData, grades, xAxisLabel,
            "Percentage", "Grades", 0);
        this->drawLineGraph(
            *ui->plot_forces_grades, xData, curvatures, xAxisLabel,
            "Percentage", "Curvatures", 1);
        this->drawLineGraph(
            *ui->plot_forces_tractiveForces, xData, tractiveForces,
            xAxisLabel, "Forces", "Tractive Forces (kN)", 0);
        this->drawLineGraph(
            *ui->plot_forces_resistance, xData, resistance, xAxisLabel,
            "Forces", "Resistance (kN)", 0);
        this->drawLineGraph(
            *ui->plot_forces_totalForces, xData, totalForces, xAxisLabel,
            "Forces", "Net Forces (kN)", 0);
    };

    connect(ui->comboBox_trainsResults,
            &QComboBox::currentTextChanged, updateResultsCurves);
    connect(ui->comboBox_resultsXAxis,
            &QComboBox::currentTextChanged, updateResultsCurves);
}

void NeTrainSim::drawLineGraph(CustomPlot& plot,
                               const QVector<double>& xData,
                               const QVector<double>& yData,
                               QString xLabel, QString yLabel,
                               QString graphName, int plotIndex)
{

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
        thread->terminate();
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


QCPItemText *NeTrainSim::findLabelByPosition(CustomPlot *plot,
                                             const QPointF &targetPosition)
{
    // Iterate over the items in the plot
    for (int i = 0; i < plot->itemCount(); ++i) {
        QCPAbstractItem* abstractItem = plot->item(i);

        // Check if the item is a QCPItemText
        if (QCPItemText* label = qobject_cast<QCPItemText*>(abstractItem)) {
            // Compare the position of the label with the target position
            double labelX = label->position->coords().x();
            double labelY = label->position->coords().y();

            if ((labelX == targetPosition.x()) &&
                (labelY == targetPosition.y())) {
                    return label; // Return the matching label
            }
        }
    }
    return nullptr; // No label found
}


void NeTrainSim::handleSampleProject() {
    QString executablePath = QCoreApplication::applicationDirPath();
    QString filePath =
        QDir(QDir(executablePath).filePath("sampleProject")).
                       filePath("sampleProject.NTS");
    this->loadProjectFiles(filePath);
}

void NeTrainSim::loadProjectFiles(QString projectFilename) {
    QFile pfn(projectFilename);
    if (! pfn.exists()) {
        this->showWarning("Project file does not exist!");
        return;
    }
    if (!projectFilename.isEmpty()) {
        QFileInfo fileInfo(projectFilename);
        QString parentDirPath = fileInfo.dir().absolutePath();

        QString executableDirectory = QApplication::applicationDirPath();

        auto out = XMLManager::readProjectFile(projectFilename);
        ui->lineEdit_projectName->setText(std::get<0>(out));
        ui->lineEdit_networkName->setText(std::get<1>(out));
        ui->lineEdit_createdBy->setText(std::get<2>(out));
        QString nodesFile = std::get<3>(out);
        QString linksFile = std::get<4>(out);
        QString trainsFile = std::get<5>(out);
        QString PWDString = "$${PWD}";
        nodesFile.replace(PWDString, parentDirPath);
        linksFile.replace(PWDString, parentDirPath);
        trainsFile.replace(PWDString, parentDirPath);

        QString ExecutableString = "$${EXE}";
        nodesFile.replace(ExecutableString, executableDirectory);
        linksFile.replace(ExecutableString, executableDirectory);
        trainsFile.replace(ExecutableString, executableDirectory);


        QFile nfile(nodesFile);
        if (nfile.exists()) {
            ui->lineEdit_nodes->setText(nodesFile);
            //ui->checkBox_defineNewNetwork->setCheckState(Qt::Unchecked);
        }
        else {
            this->showWarning("Nodes file does not exist");
            return;
        }
        QFile lfile(linksFile);
        if (lfile.exists()) {
            ui->lineEdit_links->setText(linksFile);
            //ui->checkBox_defineNewNetwork->setCheckState(Qt::Unchecked);
        }
        else {
            this->showWarning("Links file does not exist");
            return;
        }

        QFile tfile(trainsFile);
        if (tfile.exists()) {
            ui->lineEdit_trains->setText(trainsFile);
            //ui->checkBox_TrainsOD->setCheckState(Qt::Unchecked);
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
            QMessageBox::warning(this, "Error",
                                 "Wrong Project File Structure!");
            return;
        }
        double simTimestep = std::get<7>(out).toDouble(&ok);
        if (ok) {
            ui->doubleSpinBox_timeStep->setValue(simTimestep);
        }
        else {
            QMessageBox::warning(this, "Error",
                                 "Wrong Project File Structure!");
            return;
        }
        double simTimestepPlot = std::get<8>(out).toDouble(&ok);
        if (ok) {
            ui->spinBox_plotEvery->setValue(simTimestepPlot);
        }
        else {
            QMessageBox::warning(this, "Error",
                                 "Wrong Project File Structure!");
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

    if (rightAlignedWidget)
    {
        delete rightAlignedWidget;
    }

    layout = nullptr;
    rightAlignedMenu = nullptr;

    // if (layout)
    // {
    //     delete layout;
    // }

    // if (rightAlignedMenu)
    // {
    //     delete rightAlignedMenu;
    // }

}


void NeTrainSim::pauseSimulation() {
    QMetaObject::invokeMethod(
        worker->sim, "pauseSimulation", Qt::QueuedConnection);
}

void NeTrainSim::resumeSimulation() {
    QMetaObject::invokeMethod(
        worker->sim, "resumeSimulation", Qt::QueuedConnection);
}

void NeTrainSim::loadNodesDataToTable(
    Vector<Map<std::string, std::string>> nodesRecords)
{

    if (nodesRecords.empty()) {
        return;
    }

    // First, disconnect the signal and slot
    QObject::disconnect(ui->table_newNodes,
                        &QTableWidget::cellChanged,
                        nullptr, nullptr);
    QObject::disconnect(ui->doubleSpinBox_xCoordinate,
                        &QDoubleSpinBox::valueChanged,
                        nullptr, nullptr);
    QObject::disconnect(ui->doubleSpinBox_yCoordinate,
                        &QDoubleSpinBox::valueChanged,
                        nullptr, nullptr);

    double xScale = std::stod(nodesRecords[0]["XScale"]);
    double yScale = std::stod(nodesRecords[0]["YScale"]);

    ui->doubleSpinBox_xCoordinate->setValue(xScale);
    ui->doubleSpinBox_yCoordinate->setValue(yScale);

    try {
        // get the data from the QTableWidget
        for (int i = 0; i < nodesRecords.size(); i++) {
            // get the item at row 0 and column 0 of the table widget
            double xCoord = std::stod(nodesRecords[i]["XCoordinate"]);
            double yCoord = std::stod(nodesRecords[i]["YCoordinate"]);
            std::string desc = nodesRecords[i]["Desc"];
            QString label = QString::fromStdString(nodesRecords[i]["UserID"]);

            // ID
            QModelIndex index = ui->table_newNodes->model()->index(i, 0);
            ui->table_newNodes->model()->setData(index, label, Qt::EditRole);

            // X Coordinate
            index = ui->table_newNodes->model()->index(i, 1);
            ui->table_newNodes->model()->setData(index, xCoord, Qt::EditRole);

            // Y Coordinate
            index = ui->table_newNodes->model()->index(i, 2);
            ui->table_newNodes->model()->setData(index, yCoord, Qt::EditRole);

            // Describtion
            index = ui->table_newNodes->model()->index(i, 3);
            ui->table_newNodes->model()->
                setData(index, QString::fromStdString(desc), Qt::EditRole);

            ui->table_newNodes->insertRow(ui->table_newNodes->rowCount());
            // set the new id count as default value for the first
            // cell of the new row
            int uniqueID = ui->table_newNodes->generateUniqueID();
            index = ui->table_newNodes->model()->index(i+1, 0);
            ui->table_newNodes->model()->
                setData(index, uniqueID, Qt::EditRole);
        }

    }
    catch (std::exception& e) {
        this->showWarning(e.what());
    }

    // connect the cellChanged signals of the QTableWidget to the updatePlot slot
    QObject::connect(ui->table_newNodes,
                     &QTableWidget::cellChanged,
                     this, &NeTrainSim::updateTheNodesPlotData);
    QObject::connect(ui->doubleSpinBox_xCoordinate,
                     &QDoubleSpinBox::valueChanged,
                     this, &NeTrainSim::updateTheNodesPlotData);
    QObject::connect(ui->doubleSpinBox_yCoordinate,
                     &QDoubleSpinBox::valueChanged,
                     this, &NeTrainSim::updateTheNodesPlotData);

    this->forceReplotNodes();

    // in case the user loaded the links data first
    this->forceReplotLinks();

}


void NeTrainSim::loadLinksDataToTable(
    Vector<Map<std::string, std::string>> linksRecords)
{

    if (linksRecords.empty()) {
        return;
    }

    // first disconnect the slots from cell changed

    // add a row to the links table everytime you edit the last row cell
    QObject::disconnect(ui->table_newLinks, &QTableWidget::cellChanged,
                        nullptr, nullptr);

    // connect the cellChanged signal of the QTableWidget to
    // the updatePlot slot
    QObject::disconnect(ui->table_newLinks, &QTableWidget::cellChanged,
                        nullptr, nullptr);

    double lengthScale = std::stod(linksRecords[0]["LengthScale"]);
    double speedScale = std::stod(linksRecords[0]["FreeFlowSpeedScale"]);


    ui->doubleSpinBox_LengthScale->setValue(lengthScale);
    ui->doubleSpinBox_SpeedScale->setValue(speedScale);

    try {
        // get the data from the QTableWidget
        for (int i = 0; i < linksRecords.size(); i++) {

            std::string userID = linksRecords[i]["UserID"];
            int FromNodeID = std::stoi(linksRecords[i]["FromNodeID"]);
            int ToNodeID = std::stoi(linksRecords[i]["ToNodeID"]);
            double length = std::stod(linksRecords[i]["Length"]);
            int signalNo = std::stoi(linksRecords[i]["SignalNo"]);
            int direction = std::stoi(linksRecords[i]["Directions"]);
            bool dir = (direction == 1) ? false : true;
            double freeFlow = std::stod(linksRecords[i]["FreeFlowSpeed"]);
            double grade = std::stod(linksRecords[i]["DirectionalGrade"]);
            double curv = std::stod(linksRecords[i]["Curvature"]);
            double spdVar = std::stod(linksRecords[i]["SpeedVariation"]);
            std::string signalsLoc = linksRecords[i]["SignalsAtNodes"];
            std::string region = linksRecords[i]["Region"];
            bool hasCat;
            std::stringstream s(linksRecords[i]["HasCatenary"]);
            s >> hasCat;

            QModelIndex index = ui->table_newLinks->model()->index(i, 0);
            ui->table_newLinks->model()->
                setData(index, QString::fromStdString(userID), Qt::EditRole);


            index = ui->table_newLinks->model()->index(i, 1);
            ui->table_newLinks->model()->
                setData(index, FromNodeID, Qt::EditRole);

            index = ui->table_newLinks->model()->index(i, 2);
            ui->table_newLinks->model()->
                setData(index, ToNodeID, Qt::EditRole);

            index = ui->table_newLinks->model()->index(i, 3);
            ui->table_newLinks->model()->
                setData(index, length, Qt::EditRole);

            index = ui->table_newLinks->model()->index(i, 4);
            ui->table_newLinks->model()->
                setData(index, freeFlow, Qt::EditRole);

            index = ui->table_newLinks->model()->index(i, 5);
            ui->table_newLinks->model()->
                setData(index, signalNo, Qt::EditRole);

            index = ui->table_newLinks->model()->index(i, 6);
            ui->table_newLinks->model()->
                setData(index, QString::fromStdString(signalsLoc),
                                                 Qt::EditRole);

            index = ui->table_newLinks->model()->index(i, 7);
            ui->table_newLinks->model()->
                setData(index, grade, Qt::EditRole);

            index = ui->table_newLinks->model()->index(i, 8);
            ui->table_newLinks->model()->
                setData(index, curv, Qt::EditRole);

            index = ui->table_newLinks->model()->index(i, 9);
            ui->table_newLinks->model()->
                setData(index, dir, Qt::CheckStateRole);

            index = ui->table_newLinks->model()->index(i, 10);
            ui->table_newLinks->model()->
                setData(index, spdVar, Qt::EditRole);

            index = ui->table_newLinks->model()->index(i, 11);
            ui->table_newLinks->model()->
                setData(index, hasCat, Qt::CheckStateRole);

            index = ui->table_newLinks->model()->index(i, 12);
            ui->table_newLinks->model()->
                setData(index, QString::fromStdString(region), Qt::EditRole);

            ui->table_newLinks->insertRow(ui->table_newLinks->rowCount());
            // set the new id count as default value for the first
            // cell of the new row
            int uniqueID = ui->table_newLinks->generateUniqueID();
            index = ui->table_newLinks->model()->index(i+1, 0);
            ui->table_newLinks->model()->
                setData(index, uniqueID, Qt::EditRole);
        }
    }
    catch (std::exception& e) {
        this->showWarning(e.what());
    }
    // add a row to the links table everytime you edit the last row cell
    QObject::connect(ui->table_newLinks, &QTableWidget::cellChanged,
                     this, &NeTrainSim::addRowToNewLinks);

    // connect the cellChanged signal of the QTableWidget to
    // the updatePlot slot
    QObject::connect(ui->table_newLinks, &QTableWidget::cellChanged,
                     this, &NeTrainSim::updateTheLinksPlotData);

    this->forceReplotLinks();

}

void NeTrainSim::loadLocomotivesDataToTable(
    Vector<Map<std::string, std::any>> trainsRecords)
{

    if (trainsRecords.empty()) {
        return;
    }

    ui->table_newLocomotive->clearContent();

    for (auto& trainRecord: trainsRecords) {
        // get loco values
        auto locoRecords =
            std::any_cast<Vector<Map<std::string,
                                     std::string>>>(trainRecord["Locomotives"]);
        for (int i = 0; i < locoRecords.size(); i ++) {
            // if there is a car with the same properties, continue
            if (getLocomotiveIDFromTable(locoRecords[i]) > 0) {
                    continue;
            }

            // Power
            QModelIndex index = ui->table_newLocomotive->model()->index(i, 1);
            ui->table_newLocomotive->model()->
                setData(index, stod(locoRecords[i]["Power"]), Qt::EditRole);

            // trans eff
            index = ui->table_newLocomotive->model()->index(i, 2);
            ui->table_newLocomotive->model()->
                setData(index, stod(locoRecords[i]["TransmissionEff"]),
                                                 Qt::EditRole);

            // length
            index = ui->table_newLocomotive->model()->index(i, 3);
            ui->table_newLocomotive->model()->
                setData(index, stod(locoRecords[i]["Length"]),
                        Qt::EditRole);

            // drag
            index = ui->table_newLocomotive->model()->index(i, 4);
            ui->table_newLocomotive->model()->
                setData(index, stod(locoRecords[i]["AirDragCoeff"]),
                        Qt::EditRole);

            // area
            index = ui->table_newLocomotive->model()->index(i, 5);
            ui->table_newLocomotive->model()->
                setData(index, stod(locoRecords[i]["FrontalArea"]),
                        Qt::EditRole);

            // weight
            index = ui->table_newLocomotive->model()->index(i, 6);
            ui->table_newLocomotive->model()->
                setData(index, stod(locoRecords[i]["GrossWeight"]),
                        Qt::EditRole);

            // axles
            index = ui->table_newLocomotive->model()->index(i, 7);
            ui->table_newLocomotive->model()->
                setData(index, stoi(locoRecords[i]["NoOfAxles"]),
                                                      Qt::EditRole);

            QString theType =
                QString::fromStdString(TrainTypes::powerTypeStrings[stoi(
                    locoRecords[i]["Type"])]);

            // axles
            index = ui->table_newLocomotive->model()->index(i, 8);
            ui->table_newLocomotive->model()->
                setData(index, theType, Qt::EditRole);

            // add new row to the table
            ui->table_newLocomotive->
                insertRow(ui->table_newLocomotive->rowCount());
            // set the new id count as default value for the first
            // cell of the new row
            int uniqueID = ui->table_newLocomotive->generateUniqueID();
            index = ui->table_newLocomotive->model()->index(i+1, 0);
            ui->table_newLocomotive->model()->
                setData(index, uniqueID, Qt::EditRole);

            }
    }

}


void NeTrainSim::loadCarsDataToTable(
    Vector<Map<std::string, std::any>> trainsRecords)
{

    if (trainsRecords.empty()) {
            return;
    }

    ui->table_newCar->clearContent();

    for (auto& trainRecord: trainsRecords) {
            // get loco values
            auto carRecords =
                std::any_cast<Vector<Map<std::string,
                                         std::string>>>(trainRecord["Cars"]);
            for (int i = 0; i < carRecords.size(); i ++)
            {
                    // if there is a car with the same properties, continue
                    if (getCarIDFromTable(carRecords[i]) > 0) {
                        continue;
                    }

                    // get the values
                    double len = stod(carRecords[i]["Length"]);
                    double drg = stod(carRecords[i]["AirDragCoeff"]);
                    double area = stod(carRecords[i]["FrontalArea"]);
                    double emptyW = stod(carRecords[i]["TareWeight"]);
                    double grossW = stod(carRecords[i]["GrossWeight"]);
                    int axles = stoi(carRecords[i]["NoOfAxles"]);
                    int type = stoi(carRecords[i]["Type"]);

                    // length
                    QModelIndex index =
                        ui->table_newCar->model()->index(i, 1);
                    ui->table_newCar->model()->
                        setData(index, len, Qt::EditRole);

                    // drag
                    index =
                        ui->table_newCar->model()->index(i, 2);
                    ui->table_newCar->model()->
                        setData(index, drg, Qt::EditRole);

                    // area
                    index =
                        ui->table_newCar->model()->index(i, 3);
                    ui->table_newCar->model()->
                        setData(index, area, Qt::EditRole);

                    // tare weight
                    index =
                        ui->table_newCar->model()->index(i, 4);
                    ui->table_newCar->model()->
                        setData(index, emptyW, Qt::EditRole);

                    // gross weight
                    index =
                        ui->table_newCar->model()->index(i, 5);
                    ui->table_newCar->model()->
                        setData(index, grossW, Qt::EditRole);

                    // axles
                    index =
                        ui->table_newCar->model()->index(i, 6);
                    ui->table_newCar->model()->
                        setData(index, axles, Qt::EditRole);

                    // type
                    QString theType =
                        QString::fromStdString(
                            TrainTypes::carTypeStrings[type]);
                    index =
                        ui->table_newCar->model()->index(i, 7);
                    ui->table_newCar->model()->
                        setData(index, theType, Qt::EditRole);

                    // add new row
                    ui->table_newCar->insertRow(ui->table_newCar->rowCount());
                    // set the new id count as default value for the first
                    // cell of the new row
                    int uniqueID = ui->table_newCar->generateUniqueID();
                    index = ui->table_newCar->model()->index(i+1, 0);
                    ui->table_newCar->model()->
                        setData(index, uniqueID, Qt::EditRole);
            }
    }

}

void NeTrainSim::loadConfigsDataToTable(
    Vector<Map<std::string, std::any>> trainsRecords)
{
    if (trainsRecords.empty())
    {
            return;
    }

    ui->table_newConfiguration->clearContent();

    for (auto& trainRecord: trainsRecords) {


            // get loco values
            auto locoRecords =
                std::any_cast<Vector<Map<std::string,
                                         std::string>>>(
                trainRecord["Locomotives"]);

            // get cars values
            auto carRecords =
                std::any_cast<Vector<Map<std::string,
                                         std::string>>>(trainRecord["Cars"]);

            auto trainID = std::any_cast<std::string>(trainRecord["UserID"]);

            // locomotives
            for (auto & locoRecord : locoRecords)
            {
                    int id = this->getLocomotiveIDFromTable(locoRecord);

                    if (id >= 0) {
                        int i = ui->table_newConfiguration->rowCount() - 1;
                        // ID
                        QModelIndex index =
                            ui->table_newConfiguration->model()->index(i, 0);
                        ui->table_newConfiguration->model()->
                            setData(index,
                                    QString::fromStdString(trainID),
                                    Qt::EditRole);

                        // Locomotive
                        index =
                            ui->table_newConfiguration->model()->index(i, 1);
                        QString lname = "Locomotive";
                        ui->table_newConfiguration->model()->
                            setData(index, lname, Qt::EditRole);

                        // the instance id
                        index =
                            ui->table_newConfiguration->model()->index(i, 2);
                        ui->table_newConfiguration->model()->
                            setData(index, id, Qt::EditRole);

                        // the instance id
                        index =
                            ui->table_newConfiguration->model()->index(i, 3);
                        ui->table_newConfiguration->model()->
                            setData(index,
                                    stoi(locoRecord["Count"]),
                                    Qt::EditRole);

                        // insert a new row
                        ui->table_newConfiguration->
                            insertRow(ui->table_newConfiguration->rowCount());
                        // set the new id count as default value for the first
                        // cell of the new row
                        int uniqueID = ui->table_newConfiguration->
                                       generateUniqueID();
                        index = ui->table_newConfiguration->model()->
                                index(i+1, 0);
                        ui->table_newConfiguration->model()->
                            setData(index, uniqueID, Qt::EditRole);
                    }

            }

            // cars
            for (auto& carRecord: carRecords) {
                    int id = this->getCarIDFromTable(carRecord);

                    if (id >= 0) {

                        int i = ui->table_newConfiguration->rowCount() - 1;

                        // ID
                        QModelIndex index =
                            ui->table_newConfiguration->model()->index(i, 0);
                        ui->table_newConfiguration->model()->
                            setData(index,
                                    QString::fromStdString(trainID),
                                    Qt::EditRole);

                        // Locomotive
                        index =
                            ui->table_newConfiguration->model()->index(i, 1);
                        QString lname = "Car";
                        ui->table_newConfiguration->model()->
                            setData(index, lname, Qt::EditRole);


                        // the instance id
                        index =
                            ui->table_newConfiguration->model()->index(i, 2);
                        ui->table_newConfiguration->model()->
                            setData(index, id, Qt::EditRole);

                        // the instance id
                        index =
                            ui->table_newConfiguration->model()->index(i, 3);
                        ui->table_newConfiguration->model()->
                            setData(index,
                                    stoi(carRecord["Count"]),
                                    Qt::EditRole);

                        // insert a new row
                        ui->table_newConfiguration->
                            insertRow(ui->table_newConfiguration->rowCount());
                        // set the new id count as default value for the first
                        // cell of the new row
                        int uniqueID = ui->table_newConfiguration->
                                       generateUniqueID();
                        index = ui->table_newConfiguration->model()->
                                index(i+1, 0);
                        ui->table_newConfiguration->model()->
                            setData(index, uniqueID, Qt::EditRole);
                    }
            }

    }
}

void NeTrainSim::loadTrainsDataToTable(
    Vector<Map<std::string, std::any>> trainsRecords)
{

    if (trainsRecords.empty()) {
        return;
    }

    ui->table_newTrain->clearContent();

    for (int i = 0; i < trainsRecords.size(); i++) {
        auto trainRecord = trainsRecords[i];
        std::string id = std::any_cast<std::string>(trainRecord["UserID"]);
        std::string nodes =
            std::any_cast<Vector<int>>(
            trainRecord["TrainPathOnNodeIDs"]).toNotFormattedString();
        double start =
            std::any_cast<double>(trainRecord["LoadTime"]);
        auto locos =
            std::any_cast<Vector<Map<std::string, std::string>>>(
            trainRecord["Locomotives"]);
        int locosCount = 0;
        for (auto& loc:locos) {
                   locosCount += stoi(loc["Count"]);
        }
        auto cars =
            std::any_cast<Vector<Map<std::string, std::string>>>(
                trainRecord["Cars"]);
        int carsCount = 0;
        for (auto& car:cars) {
                   carsCount += stoi(car["Count"]);
        }
        double friction =
            std::any_cast<double>(trainRecord["FrictionCoef"]);


        // ID
        QModelIndex index =
            ui->table_newTrain->model()->index(i, 0);
        ui->table_newTrain->model()->
            setData(index,
                    QString::fromStdString(id),
                    Qt::EditRole);

        // Config ID
        index =
            ui->table_newTrain->model()->index(i, 1);
        ui->table_newTrain->model()->
            setData(index,
                    QString::fromStdString(id),
                    Qt::EditRole);


        // nodes
        index =
            ui->table_newTrain->model()->index(i, 2);
        ui->table_newTrain->model()->
            setData(index,
                    QString::fromStdString(nodes),
                    Qt::EditRole);

        // start time
        index =
            ui->table_newTrain->model()->index(i, 3);
        ui->table_newTrain->model()->
            setData(index, start, Qt::EditRole);

        // locos count
        index =
            ui->table_newTrain->model()->index(i, 4);
        ui->table_newTrain->model()->
            setData(index, locosCount, Qt::EditRole);

        // cars count
        index =
            ui->table_newTrain->model()->index(i, 5);
        ui->table_newTrain->model()->
            setData(index, carsCount, Qt::EditRole);

        // friction
        index =
            ui->table_newTrain->model()->index(i, 6);
        ui->table_newTrain->model()->
            setData(index, friction, Qt::EditRole);


        ui->table_newTrain->
            insertRow(ui->table_newTrain->rowCount());

        // set the new id count as default value for the first
        // cell of the new row
        int uniqueID = ui->table_newTrain->
                       generateUniqueID();
        index = ui->table_newTrain->model()->
                index(i+1, 0);
        ui->table_newTrain->model()->
            setData(index, uniqueID, Qt::EditRole);
    }
}

void NeTrainSim::loadTrainsDataToTables(
    Vector<Map<std::string, std::any>> trainsRecords)
{
    this->loadLocomotivesDataToTable(trainsRecords);
    this->loadCarsDataToTable(trainsRecords);
    this->loadConfigsDataToTable(trainsRecords);
    this->loadTrainsDataToTable(trainsRecords);
}



int NeTrainSim::getCarIDFromTable(Map<std::string, std::string> carRecords) {
    double eLen = stod(carRecords["Length"]);
    double eDrg = stod(carRecords["AirDragCoeff"]);
    double eArea = stod(carRecords["FrontalArea"]);
    double eEmptyW = stod(carRecords["TareWeight"]);
    double eGrossW = stod(carRecords["GrossWeight"]);
    int eAxles = stoi(carRecords["NoOfAxles"]);
    int eT = stoi(carRecords["Type"]);
    std::string eType = TrainTypes::carTypeStrings[eT];

    if (ui->table_newCar->rowCount() == 0)
    {
        return -1;
    }

    for (int i = 0; i < ui->table_newCar->rowCount() ; i++ )
    {
        QTableWidgetItem* idItem = ui->table_newCar->item(i, 0);
        QTableWidgetItem* lenItem = ui->table_newCar->item(i, 1);
        QTableWidgetItem* dragItem = ui->table_newCar->item(i, 2);
        QTableWidgetItem* areaItem = ui->table_newCar->item(i, 3);
        QTableWidgetItem* eWItem = ui->table_newCar->item(i, 4);
        QTableWidgetItem* fWItem = ui->table_newCar->item(i, 5);
        QTableWidgetItem* axleItem = ui->table_newCar->item(i, 6);
        QTableWidgetItem* typeItem = ui->table_newCar->item(i, 7);

        if (idItem && lenItem && dragItem &&
            areaItem && eWItem && fWItem && axleItem) {
                int id = idItem->text().trimmed().toInt();
                double len = lenItem->text().trimmed().toDouble();
                double drag = dragItem->text().trimmed().toDouble();
                double area = areaItem->text().trimmed().toDouble();
                double ew = eWItem->text().trimmed().toDouble();
                double fw = fWItem->text().trimmed().toDouble();
                int axle = axleItem->text().trimmed().toInt();
                QString type = typeItem->text().trimmed();

                if (eLen == len && eDrg == drag && eArea == area &&
                    eEmptyW == ew && eGrossW == fw && eAxles == axle &&
                    QString::fromStdString(eType) == type)
                {
                    return id;
                }
        }
    }
    return -1;
}


int NeTrainSim::getLocomotiveIDFromTable(
    Map<std::string, std::string> locomotiveRecords)
{
    if (ui->table_newLocomotive->rowCount() == 0)
    {
        return -1;
    }
    double ePower = stod(locomotiveRecords["Power"]);
    double eTrans = stod(locomotiveRecords["TransmissionEff"]);
    double eLen = stod(locomotiveRecords["Length"]);
    double eDrg = stod(locomotiveRecords["AirDragCoeff"]);
    double eArea = stod(locomotiveRecords["FrontalArea"]);
    double eGrossW = stod(locomotiveRecords["GrossWeight"]);
    int eAxles = stoi(locomotiveRecords["NoOfAxles"]);
    int eT = stoi(locomotiveRecords["Type"]);
    std::string eType = TrainTypes::powerTypeStrings[eT];

    for (int i = 0; i < ui->table_newLocomotive->rowCount() ; i++ ) {
            QTableWidgetItem* idItem = ui->table_newLocomotive->item(i, 0);
            QTableWidgetItem* pwrItem = ui->table_newLocomotive->item(i, 1);
            QTableWidgetItem* transItem = ui->table_newLocomotive->item(i, 2);
            QTableWidgetItem* lenItem = ui->table_newLocomotive->item(i, 3);
            QTableWidgetItem* dragItem = ui->table_newLocomotive->item(i, 4);
            QTableWidgetItem* areaItem = ui->table_newLocomotive->item(i, 5);
            QTableWidgetItem* fWItem = ui->table_newLocomotive->item(i, 6);
            QTableWidgetItem* axleItem = ui->table_newLocomotive->item(i, 7);
            QTableWidgetItem* typeItem = ui->table_newLocomotive->item(i, 8);

            if (pwrItem && transItem && idItem && lenItem && dragItem &&
                areaItem && fWItem && axleItem) {
                    int id = idItem->text().trimmed().toInt();
                    double power = pwrItem->text().trimmed().toDouble();
                    double trans = transItem->text().trimmed().toDouble();
                    double len = lenItem->text().trimmed().toDouble();
                    double drag = dragItem->text().trimmed().toDouble();
                    double area = areaItem->text().trimmed().toDouble();
                    double fw = fWItem->text().trimmed().toDouble();
                    int axle = axleItem->text().trimmed().toInt();
                    QString type = typeItem->text().trimmed();

                    if (power == ePower && trans ==  eTrans &&
                        eLen == len && eDrg == drag && eArea == area &&
                        eGrossW == fw && eAxles == axle &&
                        QString::fromStdString(eType) == type)
                    {
                        return id;
                    }
            }
    }
    return -1;
}

