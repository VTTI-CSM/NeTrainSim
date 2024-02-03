#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

// #include "QtNetwork/qnetworkreply.h"
#include <QObject>
#include <QNetworkAccessManager>

class UpdateChecker : public QObject {
    Q_OBJECT

public:
    explicit UpdateChecker(QObject *parent = nullptr);
    void checkForUpdates();

signals:
    void updateAvailable(bool available);

private slots:
    void replyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;
    QString currentVersion; // Add a member to store the current version of the app

    void handleSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
};

#endif // UPDATECHECKER_H
