#include <QDebug>
#include "errorhandler.h"
#include <QMessageBox>

void ErrorHandler::showNotification(std::string msg) {
    QMessageBox::information(nullptr, "NeTrainSim - Notification", QString::fromStdString(msg));
}

void ErrorHandler::showWarning(std::string msg) {
    QMessageBox::warning(nullptr, "NeTrainSim - Warning", QString::fromStdString(msg));
}

void ErrorHandler::showError(std::string msg) {
    QMessageBox::critical(nullptr, "NeTrainSim - Error", QString::fromStdString(msg));
}
