#include "netrainsim.h"
#include "ui_netrainsim.h"

NeTrainSim::NeTrainSim(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::NeTrainSim)
{
    ui->setupUi(this);
}

NeTrainSim::~NeTrainSim()
{
    delete ui;
}

