/**
 * @file ShapefileReader.h
 * @brief Provides an interface to read shapefiles (SHP) and associated data.
 * @author Ahmed Aredah
 * @date 7/30/2023
 *
 * This class supports reading points, multiline strings, and handling different
 * coordinate reference systems (CRS).
 *
 * Copyright (C) 2023 VTTI-CSM
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SHAPEFILEREADER_H
#define SHAPEFILEREADER_H

#include <QString>
#include <map>
#include <shapefil.h>
#include <vector>

/**
 * @struct Point
 * @brief Represents a point in 3D space.
 */
struct Point {
    double x; ///< X-coordinate
    double y; ///< Y-coordinate
    double z; ///< Z-coordinate for 3D points
};

/**
 * @class ShapefileReader
 * @brief Class to read shapefiles (SHP) and associated data.
 */
class ShapefileReader {
public:
    /**
     * @brief Constructor with the shapefile path.
     * @param filePath Path to the shapefile.
     */
    ShapefileReader(const QString& filePath);

    /**
     * @brief Destructor.
     */
    ~ShapefileReader();

    /**
     * @brief Opens the shapefile.
     * @return True if the shapefile is opened successfully, false otherwise.
     */
    bool openShapefile();

    /**
     * @brief Closes the shapefile.
     */
    void closeShapefile();

    /**
     * @brief Reads points from the shapefile.
     * @param sourceCRS Source coordinate reference system.
     * @param projectionCRS Target coordinate reference system for projection.
     * @return Vector of points and associated attributes.
     */
    std::vector<std::pair<Point,
                          std::map<QString, QString>>> readPoints(
        std::string sourceCRS, std::string projectionCRS);

    /**
     * @brief Reads multiline strings from the shapefile.
     * @param sourceCRS Source coordinate reference system.
     * @param projectionCRS Target coordinate reference system for projection.
     * @return Vector of multiline strings and associated attributes.
     */
    std::vector<std::pair<std::vector<Point>,
                          std::map<QString, QString>>> readMultiLineStrings(
        std::string sourceCRS, std::string projectionCRS);

    QStringList getAttributeNames();

    /**
     * @brief Returns the identifier for a given CRS name.
     * @param crs_name Name of the coordinate reference system.
     * @return Identifier string.
     */
    std::string getCRSIdentifier(const char* crs_name);

    /**
     * @brief Reads the projection file.
     * @return Projection file contents as string.
     */
    std::string readPrjFile();

    /**
     * @brief Extracts the source CRS.
     * @return Source CRS string.
     */
    std::string extractSourceCRS();

    /**
     * @brief Checks if the CRS is projected.
     * @param crs_name Name of the coordinate reference system.
     * @return True if it's projected, false otherwise.
     */
    bool isProjectedCRS(const std::string crs_name);


private:

    /**
     * @brief Returns the DBF file path.
     * @return Path as QString.
     */
    QString getDbfFilePath();

    /**
     * @brief Returns the PRJ file path.
     * @return Path as QString.
     */
    QString getPrjFilePath();

    /**
     * @brief Calculates the UTM zone based on longitude.
     * @param longitude Longitude value.
     * @return UTM zone as integer.
     */
    int calculateUTMZone(double longitude);


    SHPHandle hSHP; ///< Shapefile handle.
    QString filePath; ///< Path to the shapefile.
};

#endif // SHAPEFILEREADER_H
