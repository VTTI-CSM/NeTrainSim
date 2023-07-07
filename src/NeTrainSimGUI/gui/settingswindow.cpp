#include "settingswindow.h"
#include "ui_settingswindow.h"
#include <QFileDialog>
#include "netrainsimmainwindow.h"

settingsWindow::settingsWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::settingsWindow) {
    ui->setupUi(this);
    this->loadSavedSettings();
}

settingsWindow::~settingsWindow() {
    delete ui;
}

void settingsWindow::on_pushButton_browse_clicked() {
    this->browseFolder(ui->lineEdit_defaultBrowseLocation, "Select the default browse location");
}


void settingsWindow::browseFolder(QLineEdit* theLineEdit, const QString& theHelpMessage) {
    QString folderPath = QFileDialog::getExistingDirectory(this,
                                                           theHelpMessage,
                                                           QDir::homePath(),
                                                           QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    // Check if a folder was selected
    if (!folderPath.isEmpty()) {
        theLineEdit->setText(folderPath);
    }
}

void settingsWindow::loadSavedSettings() {

    NeTrainSim* mainWindow = qobject_cast<NeTrainSim*>(parent());
    this->ui->lineEdit_defaultBrowseLocation->setText(mainWindow->defaultBrowsePath);

    mainWindow = nullptr;
}


void settingsWindow::on_pushButton_save_clicked() {

    NeTrainSim* mainWindow = qobject_cast<NeTrainSim*>(parent());
    QStringList defaultConfigs;
    defaultConfigs << "default.browseLocation=" + ui->lineEdit_defaultBrowseLocation->text();

    bool result = mainWindow->saveDefaults(defaultConfigs);
    if (result) {
        mainWindow->showNotification("Settings saved successfully!");
        this->close();
    }
    else {
        mainWindow->showNotification("Settings could not be saved!");
    }

    mainWindow = nullptr;
}

