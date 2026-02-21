#pragma once

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

/**
 * @brief Configuration dialog for RabbitMQ connection settings
 *
 * Provides a GUI to configure RabbitMQ connection parameters.
 * Configuration is saved to config/NeTrainSim_rabbitmq.xml with the password
 * stored securely in the OS keychain.
 */
class RabbitMQConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RabbitMQConfigDialog(QWidget *parent = nullptr);
    ~RabbitMQConfigDialog() override = default;

private slots:
    void onSaveClicked();
    void onTestConnectionClicked();

private:
    void loadConfig();
    void saveConfig();
    void savePasswordToKeychain(const QString &password);
    QString loadPasswordFromKeychain();
    QString findConfigDir() const;
    void updateStatusLabel(const QString &message,
                           bool isError = false);

    // UI elements
    QLineEdit   *m_hostEdit;
    QSpinBox    *m_portSpinBox;
    QLineEdit   *m_usernameEdit;
    QLineEdit   *m_passwordEdit;
    QPushButton *m_testButton;
    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;
    QLabel      *m_statusLabel;
};
