#include <QDebug>
#include "errorhandler.h"
#include <QMessageBox>

void ErrorHandler::showNotification(std::string msg) {
#ifdef AS_CMD
    qInfo().noquote() << QString::fromStdString(msg);
#else
    QMessageBox::information(nullptr, "NeTrainSim - Notification", QString::fromStdString(msg));
#endif
}

void ErrorHandler::showWarning(std::string msg) {
#ifdef AS_CMD
    qWarning().noquote() << QString::fromStdString(msg);
#else
    QMessageBox::warning(nullptr, "NeTrainSim - Warning", QString::fromStdString(msg));
#endif
}

void ErrorHandler::showError(std::string msg) {
#ifdef AS_CMD
    qCritical().noquote() << QString::fromStdString(msg);
#else
    QMessageBox::critical(nullptr, "NeTrainSim - Error", QString::fromStdString(msg));
#endif
}
