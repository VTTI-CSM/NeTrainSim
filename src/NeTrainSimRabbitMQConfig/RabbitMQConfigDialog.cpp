#include "RabbitMQConfigDialog.h"
#include <QCoreApplication>
#include <QDir>
#include <QDomDocument>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTcpSocket>
#include <QVBoxLayout>
#ifdef HAVE_QTKEYCHAIN
#include <qt6keychain/keychain.h>
#endif

RabbitMQConfigDialog::RabbitMQConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("RabbitMQ Configuration");
    setMinimumWidth(400);

    // Create main layout
    auto *mainLayout = new QVBoxLayout(this);

    // Connection settings group
    auto *connectionGroup = new QGroupBox("Connection Settings");
    auto *formLayout      = new QFormLayout(connectionGroup);

    m_hostEdit = new QLineEdit("localhost");
    formLayout->addRow("Host:", m_hostEdit);

    m_portSpinBox = new QSpinBox();
    m_portSpinBox->setRange(1, 65535);
    m_portSpinBox->setValue(5672);
    formLayout->addRow("Port:", m_portSpinBox);

    m_usernameEdit = new QLineEdit("guest");
    formLayout->addRow("Username:", m_usernameEdit);

    m_passwordEdit = new QLineEdit("guest");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow("Password:", m_passwordEdit);

    mainLayout->addWidget(connectionGroup);

    // Status label
    m_statusLabel = new QLabel();
    m_statusLabel->setWordWrap(true);
    mainLayout->addWidget(m_statusLabel);

    // Buttons
    auto *buttonLayout = new QHBoxLayout();

    m_testButton = new QPushButton("Test Connection");
    connect(m_testButton, &QPushButton::clicked, this,
            &RabbitMQConfigDialog::onTestConnectionClicked);
    buttonLayout->addWidget(m_testButton);

    buttonLayout->addStretch();

    m_saveButton = new QPushButton("Save");
    m_saveButton->setDefault(true);
    connect(m_saveButton, &QPushButton::clicked, this,
            &RabbitMQConfigDialog::onSaveClicked);
    buttonLayout->addWidget(m_saveButton);

    m_cancelButton = new QPushButton("Cancel");
    connect(m_cancelButton, &QPushButton::clicked, this,
            &QDialog::reject);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout);

    // Load existing configuration
    loadConfig();

    // Show initial status
    QString statusMsg = "Configuration will be saved to: "
                        + findConfigDir() + "/NeTrainSim_rabbitmq.xml";
#ifndef HAVE_QTKEYCHAIN
    statusMsg +=
        "\n\nWarning: QtKeychain not available. "
        "Password will be stored in config file (less secure).";
#endif
    updateStatusLabel(statusMsg);
}

void RabbitMQConfigDialog::onSaveClicked()
{
    saveConfig();
}

void RabbitMQConfigDialog::onTestConnectionClicked()
{
    updateStatusLabel("Testing connection...");

    QTcpSocket socket;
    socket.connectToHost(m_hostEdit->text(),
                         m_portSpinBox->value());

    if (socket.waitForConnected(5000))
    {
        socket.disconnectFromHost();
        updateStatusLabel("Connection successful! "
                          "RabbitMQ server is reachable.");
    }
    else
    {
        updateStatusLabel(
            "Connection failed: " + socket.errorString(), true);
    }
}

void RabbitMQConfigDialog::loadConfig()
{
    QString configPath = findConfigDir() + "/NeTrainSim_rabbitmq.xml";

    QFile file(configPath);
    if (!file.exists())
    {
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    QDomDocument doc;
    auto result = doc.setContent(&file);
    file.close();
    if (!result)
    {
        return;
    }

    QDomElement root = doc.documentElement();
    if (root.tagName() != "rabbitmq")
    {
        return;
    }

    QDomElement hostElem = root.firstChildElement("host");
    if (!hostElem.isNull())
    {
        m_hostEdit->setText(hostElem.text());
    }

    QDomElement portElem = root.firstChildElement("port");
    if (!portElem.isNull())
    {
        bool ok;
        int  port = portElem.text().toInt(&ok);
        if (ok)
        {
            m_portSpinBox->setValue(port);
        }
    }

    QDomElement usernameElem =
        root.firstChildElement("username");
    if (!usernameElem.isNull())
    {
        m_usernameEdit->setText(usernameElem.text());
    }

    // Load password from keychain or XML
    QString password = loadPasswordFromKeychain();
    if (!password.isEmpty())
    {
        m_passwordEdit->setText(password);
    }
    else
    {
        // Fallback: load from XML
        QDomElement passwordElem =
            root.firstChildElement("password");
        if (!passwordElem.isNull())
        {
            m_passwordEdit->setText(passwordElem.text());
        }
    }
}

void RabbitMQConfigDialog::saveConfig()
{
    QString configDir  = findConfigDir();
    QString configPath = configDir + "/NeTrainSim_rabbitmq.xml";

    // Ensure config directory exists
    QDir dir(configDir);
    if (!dir.exists())
    {
        if (!dir.mkpath("."))
        {
            updateStatusLabel("Failed to create config directory",
                              true);
            return;
        }
    }

    // Create XML document
    QDomDocument doc;
    QDomElement  root = doc.createElement("rabbitmq");
    doc.appendChild(root);

    auto addElement = [&](const QString &name,
                          const QString &value) {
        QDomElement elem = doc.createElement(name);
        elem.appendChild(doc.createTextNode(value));
        root.appendChild(elem);
    };

    addElement("host", m_hostEdit->text());
    addElement("port", QString::number(m_portSpinBox->value()));
    addElement("username", m_usernameEdit->text());

#ifdef HAVE_QTKEYCHAIN
    // Try to save password to keychain
    savePasswordToKeychain(m_passwordEdit->text());
#endif

    // Save password to XML as fallback
    addElement("password", m_passwordEdit->text());

    // Save XML file
    QFile file(configPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        updateStatusLabel("Failed to save configuration file",
                          true);
        return;
    }

    QTextStream stream(&file);
    stream << doc.toString(4);
    file.close();

    updateStatusLabel("Configuration saved to: " + configPath);

#ifdef HAVE_QTKEYCHAIN
    QMessageBox::information(
        this, "Configuration Saved",
        "RabbitMQ configuration has been saved.\n"
        "Password is stored in OS keychain and config file.");
#else
    QMessageBox::warning(
        this, "Configuration Saved",
        "RabbitMQ configuration has been saved.\n\n"
        "Warning: QtKeychain is not available.\n"
        "Password is stored in the config file in plain text.\n"
        "For secure password storage, install qt6keychain-dev.");
#endif

    accept();
}

void RabbitMQConfigDialog::savePasswordToKeychain(
    const QString &password)
{
#ifdef HAVE_QTKEYCHAIN
    QKeychain::WritePasswordJob job("NeTrainSim");
    job.setAutoDelete(false);
    job.setKey("rabbitmq-password");
    job.setTextData(password);

    QEventLoop loop;
    QObject::connect(&job,
                     &QKeychain::WritePasswordJob::finished,
                     &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    if (job.error() != QKeychain::NoError)
    {
        QMessageBox::warning(
            this, "Keychain Warning",
            "Could not save password to keychain: "
                + job.errorString()
                + "\n\nPassword will be stored in config file.");
    }
#else
    Q_UNUSED(password)
#endif
}

QString RabbitMQConfigDialog::loadPasswordFromKeychain()
{
#ifdef HAVE_QTKEYCHAIN
    QKeychain::ReadPasswordJob job("NeTrainSim");
    job.setAutoDelete(false);
    job.setKey("rabbitmq-password");

    QEventLoop loop;
    QObject::connect(&job,
                     &QKeychain::ReadPasswordJob::finished,
                     &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    if (job.error() == QKeychain::NoError)
    {
        return job.textData();
    }
#endif
    return QString();
}

QString RabbitMQConfigDialog::findConfigDir() const
{
    // First, try to find existing config directory beside
    // executable
    QDir execDir(QCoreApplication::applicationDirPath());
    if (execDir.exists("config"))
    {
        return execDir.filePath("config");
    }

    // Try one directory up
    QDir parentDir = execDir;
    if (parentDir.cdUp() && parentDir.exists("config"))
    {
        return parentDir.filePath("config");
    }

    // For development: search upward for config directory
    QDir repoDir(QCoreApplication::applicationDirPath());
    while (!repoDir.exists("config") && repoDir.cdUp())
    {
        // Keep searching
    }

    if (repoDir.exists("config"))
    {
        return repoDir.filePath("config");
    }

    // Fallback to user's config location
    QString fallbackPath = QStandardPaths::writableLocation(
        QStandardPaths::AppConfigLocation);

    // Ensure directory exists
    QDir fallbackDir(fallbackPath);
    if (!fallbackDir.exists())
    {
        fallbackDir.mkpath(".");
    }

    return fallbackPath;
}

void RabbitMQConfigDialog::updateStatusLabel(
    const QString &message, bool isError)
{
    m_statusLabel->setText(message);
    if (isError)
    {
        m_statusLabel->setStyleSheet("color: red;");
    }
    else
    {
        m_statusLabel->setStyleSheet("");
    }
}
