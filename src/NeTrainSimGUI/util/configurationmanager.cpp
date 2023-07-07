#include "configurationmanager.h"
#include <stdexcept>  // for std::runtime_error


ConfigurationManager::ConfigurationManager(const QString& iniFilePath)
    : m_settings(iniFilePath, QSettings::IniFormat)
{
    // Check if there was a problem reading the file
    if (m_settings.status() != QSettings::NoError) {
        // Throw a runtime_error exception
        throw std::runtime_error("Failed to open or read the configuration file");
    }
}

QString ConfigurationManager::getConfigValue(const QString& section, const QString& key, const QString& defaultValue) const {
    QMutexLocker locker(&m_mutex);  // ensure thread safety
    return m_settings.value(section + "/" + key, defaultValue).toString();
}

QStringList ConfigurationManager::getConfigKeys(const QString& section) {
    QMutexLocker locker(&m_mutex);  // ensure thread safety

    if (section.isEmpty()) {
        QStringList allKeys;
        QStringList sections = m_settings.childGroups();
        for (const QString &sec : sections) {
            QStringList keysInSection = getKeysInSection(sec);
            for (const QString &key : keysInSection) {
                allKeys.append(sec + "/" + key);
            }
        }
        return allKeys;
    }
    else {
        return getKeysInSection(section);
    }
}

QStringList ConfigurationManager::getConfigSections() const {
    QMutexLocker locker(&m_mutex);  // ensure thread safety
    return m_settings.childGroups();
}

void ConfigurationManager::setConfigValue(const QString& section, const QString& key, const QString& value) {
    QMutexLocker locker(&m_mutex);  // ensure thread safety
    m_settings.setValue(section + "/" + key, value);
    m_settings.sync();
}

QStringList ConfigurationManager::getKeysInSection(const QString &section) {
    m_settings.beginGroup(section);
    QStringList keys = m_settings.childKeys();
    m_settings.endGroup();
    return keys;
}
