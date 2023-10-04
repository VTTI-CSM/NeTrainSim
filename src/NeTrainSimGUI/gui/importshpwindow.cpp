#include "importshpwindow.h"
#include "gui/netrainsimmainwindow.h"
#include "ui_importshpwindow.h"
#include "util/ShapefileReader.h"

ImportSHPWindow::ImportSHPWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ImportSHPWindow)
{
    ui->setupUi(this);
    ui->groupBox->setVisible(false);
    this->setFixedSize(750,500);

    QString executablePath = QCoreApplication::applicationDirPath();
    QString projFilePath = QDir(executablePath).filePath("projected_crs_identifiers.txt");

    QFile pfn(projFilePath);
    if (!pfn.exists()) {
        NeTrainSim* mainWindow = qobject_cast<NeTrainSim*>(parent);
        mainWindow->showWarning("projections file does not exist!");
        mainWindow = nullptr;
        return;
    }

    proj = Utils::readEntriesFromFile(projFilePath.toStdString());

    ui->comboBox_destCRS->addItems(proj);
}

ImportSHPWindow::~ImportSHPWindow()
{
    delete ui;
}


void ImportSHPWindow::on_pushButton_browseSHP_clicked()
{
    if (!parent()) {
        return;
    }
    NeTrainSim* mainWindow = qobject_cast<NeTrainSim*>(parent());
    QString file = mainWindow->browseFiles(ui->lineEdit_SHPLocation,
                                           "Import SHP File",
                                           "SHP Files (*.SHP)");
    ShapefileReader shpfileHandler = ShapefileReader(file);
    if (shpfileHandler.openShapefile()) {
        QString sourceCRS = QString::fromStdString(shpfileHandler.extractSourceCRS());
        ui->lineEdit->setText(sourceCRS);
        bool converted = shpfileHandler.isProjectedCRS(sourceCRS.toStdString());
        setConversionVisibility(!converted);
        auto atts = shpfileHandler.getAttributeNames();

        ui->comboBox_2->addItems(atts);
        ui->comboBox_3->addItems(atts);
        ui->comboBox_4->addItems(atts);
        ui->comboBox_5->addItems(atts);
        ui->comboBox_6->addItems(atts);
        ui->comboBox_7->addItems(atts);
        ui->comboBox_8->addItems(atts);
        ui->comboBox_9->addItems(atts);
    }
}


bool ImportSHPWindow::checkCRSValidSelection()
{
    if (ui->comboBox_destCRS->currentIndex()== -1)
    {
        QMessageBox::warning(this, "ERROR",
                             "Please select a CRS value "
                             "from the dropdown menu only!");
        return false;
    }
    return true;
}


void ImportSHPWindow::on_pushButton_import_clicked()
{
    if (! this->checkCRSValidSelection()) {
        return;
    }
}

void ImportSHPWindow::setConversionVisibility(bool visible) {
    ui->groupBox->setVisible(visible);
}

bool ImportSHPWindow::isSubstringInQStringList(const QString& substring,
                                               const QStringList& list)
{
    for (const QString& item : list) {
        if (item.contains(substring, Qt::CaseInsensitive)) {
            return true; // Found the substring in the list
        }
    }
    return false; // Substring not found in the list
}

