#ifndef SERVERUTILS_H
#define SERVERUTILS_H

#include <QCoreApplication>
#include <QDir>
#include <QJsonObject>
#include <QStandardPaths>
#include <QString>

namespace ServerUtils
{

/**
 * @brief Find the path to a configuration file
 *
 * Searches for a config file in the following order:
 * 1. config/ directory next to the executable
 * 2. config/ directory one level up from the executable
 * 3. Searches upward through directories for a config/ folder
 * 4. Falls back to the user's AppConfigLocation
 *
 * @param filename The name of the config file
 *                 (e.g., "NeTrainSim_rabbitmq.xml")
 * @return The full path to the config file, or empty string
 *         if not found
 */
inline QString findConfigFilePath(const QString &filename)
{
    // First, try to find config directory beside executable
    QDir execDir(QCoreApplication::applicationDirPath());
    if (execDir.exists("config"))
    {
        QString path = execDir.filePath("config/" + filename);
        if (QFile::exists(path))
        {
            return path;
        }
    }

    // Try one directory up (for bin/ subdirectory layouts)
    QDir parentDir = execDir;
    if (parentDir.cdUp() && parentDir.exists("config"))
    {
        QString path = parentDir.filePath("config/" + filename);
        if (QFile::exists(path))
        {
            return path;
        }
    }

    // For development: search upward for config directory
    QDir repoDir(QCoreApplication::applicationDirPath());
    while (!repoDir.exists("config") && repoDir.cdUp())
    {
        // Keep searching upward
    }

    if (repoDir.exists("config"))
    {
        QString path = repoDir.filePath("config/" + filename);
        if (QFile::exists(path))
        {
            return path;
        }
    }

    // Fallback to user's config location
    QString fallbackPath = QStandardPaths::writableLocation(
                               QStandardPaths::AppConfigLocation)
                           + "/" + filename;

    if (QFile::exists(fallbackPath))
    {
        return fallbackPath;
    }

    // Return empty if file not found anywhere
    return QString();
}

} // namespace ServerUtils
#endif // SERVERUTILS_H
