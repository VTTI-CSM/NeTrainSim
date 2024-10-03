#ifndef XMLMANAGER_H
#define XMLMANAGER_H

#include "../export.h"
#include <QString>

namespace  XMLManager {
    /**
     * @brief create the project file
     *
     * @param projectName is the project name
     * @param networkName is the network name
     * @param authorName is the author name (who run the simulation)
     * @param nodesFileName is the nodes file path
     * @param linksFileName is the links file path
     * @param trainsFileName is the trains file path
     * @param simEndTime is the simulation end time
     * @param simTimestep is the time step of the simulation
     * @param simPlotTime is the frequency of plotting the trains
     * @param filename is the project file name
     */
void NETRAINSIMCORE_EXPORT
createProjectFile(const QString& projectName, const QString& networkName, const QString& authorName,
                  const QString& nodesFileName, const QString& linksFileName,
                  const QString& trainsFileName, const QString &simEndTime,
                  const QString &simTimestep, const QString &simPlotTime, const QString &filename);

    /**
     * @brief read the project file
     * @param filename is the project file to load
     * @return a tuple of projectName, networkName, authorName, nodesFileName, linksFileName, trainsFileName, simEndTime, simTimestep, simPlotTime
     */
    std::tuple<QString, QString, QString, QString, QString, QString, QString, QString, QString> NETRAINSIMCORE_EXPORT readProjectFile(const QString &filename);
};

#endif // XMLMANAGER_H
