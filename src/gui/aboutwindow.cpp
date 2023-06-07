#include "aboutwindow.h"
#include "ui_aboutwindow.h"

AboutWindow::AboutWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AboutWindow) {

    // Initialize the AboutWindow
    ui->setupUi(this);

    // Connect the OK button click signal to hide the window
    connect(ui->pushButton_ok, &QPushButton::clicked, [this](){
        this->hide();
    });

    // Set the window title
    setWindowTitle("About NeTrainSim");
}

AboutWindow::~AboutWindow() {
    // Destructor to clean up the AboutWindow
    delete ui;
}


