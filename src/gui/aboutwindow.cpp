#include "aboutwindow.h"
#include "ui_aboutwindow.h"

AboutWindow::AboutWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AboutWindow)
{
    ui->setupUi(this);

    connect(ui->pushButton_ok, &QPushButton::clicked, [this](){
        this->hide();
    });

    setWindowTitle("About NeTrainSim");
}

AboutWindow::~AboutWindow()
{
    delete ui;
}


