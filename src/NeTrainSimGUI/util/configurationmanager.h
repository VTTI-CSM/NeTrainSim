#ifndef CONFIGURATIONMANAGER_H
#define CONFIGURATIONMANAGER_H

#include <QSettings>
#include <QStringList>
#include <QMutex>
#include <QMutexLocker>

class ConfigurationManager {
public:
    explicit ConfigurationManager(const QString& iniFilePath);

    QString getConfigValue(const QString& section, const QString& key, const QString& defaultValue = QString()) const;
    QStringList getConfigKeys(const QString &section);
    QStringList getConfigSections() const;
    void setConfigValue(const QString& section, const QString& key, const QString& value);

private:
    QSettings m_settings;
    mutable QMutex m_mutex;  // used for thread safety

    QStringList getKeysInSection(const QString &section);  // helper function to reduce code duplication
};

#endif // CONFIGURATIONMANAGER_H
