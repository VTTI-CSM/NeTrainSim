#include <QDebug>
#include "errorhandler.h"

void ErrorHandler::showNotification(std::string msg) {
    qInfo().noquote() << QString::fromStdString(msg);
}

void ErrorHandler::showWarning(std::string msg) {
    qWarning().noquote() << QString::fromStdString(msg);

}

void ErrorHandler::showError(std::string msg) {
    qCritical().noquote() << QString::fromStdString(msg);
}
