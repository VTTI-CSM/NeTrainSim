#include "ShapefileReader.h"
#include <QStringList>
#include "proj.h"
#include <fstream>
#include <iostream>
#include <sstream>

// Constructor: Initializes the ShapefileReader with the file path.
ShapefileReader::ShapefileReader(const QString& filePath) :
    hSHP(nullptr), filePath(filePath) {}

// Destructor: Ensures the shapefile is closed.
ShapefileReader::~ShapefileReader()
{
    closeShapefile();
}

// Open the shapefile using the given file path.
bool ShapefileReader::openShapefile()
{
    closeShapefile();

    // Open the shapefile
    hSHP = SHPOpen(filePath.toStdString().c_str(), "rb");
    return hSHP != nullptr;
}

// Close the shapefile if it is open.
void ShapefileReader::closeShapefile()
{
    if (hSHP)
    {
        SHPClose(hSHP);
        hSHP = nullptr;
    }
}

// Read points from the shapefile and project them from
// the source CRS to the target CRS.
std::vector<std::pair<Point,
                      std::map<QString, QString>>>
ShapefileReader::readPoints(std::string sourceCRS, std::string projectionCRS)
{
    std::vector<std::pair<Point, std::map<QString, QString>>> pointsAttributes;

    if (!hSHP) {
        return pointsAttributes;
    }

    // Get shapefile info
    int nEntities;
    int nShapeType;
    double adfMinBound[4];
    double adfMaxBound[4];
    SHPGetInfo(hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound);

    // Create a transformation object using proj
    PJ_CONTEXT *C = proj_context_create();
    PJ *P = proj_create_crs_to_crs(C, sourceCRS.c_str(),
                                   projectionCRS.c_str(), NULL);
    PJ_COORD a, b;

    // Open the associated DBF file
    QString dbfFilePath = getDbfFilePath();
    DBFHandle hDBF = DBFOpen(dbfFilePath.toStdString().c_str(), "rb");

    if (!hDBF)
        return pointsAttributes;

    // Loop through features
    for (int i = 0; i < nEntities; ++i)
    {
        SHPObject* psShape = SHPReadObject(hSHP, i);

        if (psShape != nullptr &&
            psShape->nSHPType == SHPT_POINTZ) // Check for 3D point
        {
            // Project the point
            a = proj_coord(psShape->padfX[0],
                           psShape->padfY[0],
                           psShape->padfZ[0],
                           0);


            b = proj_trans(P, PJ_FWD, a);

            Point point;
            point.x = b.xyz.x;
            point.y = b.xyz.y;
            point.z = b.xyz.z; // Extract Z-coordinate

            // Read attributes for this shape
            std::map<QString, QString> recordAttributes;
            int fieldCount = DBFGetFieldCount(hDBF);
            for (int j = 0; j < fieldCount; ++j)
            {
                char fieldName[12];
                DBFGetFieldInfo(hDBF, j, fieldName,
                                NULL, NULL); // Get the field name
                std::string fieldValue =
                    DBFReadStringAttribute(hDBF, i, j);
                recordAttributes[QString(fieldName)] =
                    QString::fromStdString(fieldValue);
            }

            pointsAttributes.push_back(
                std::make_pair(point, recordAttributes));

            // Release the shape
            proj_destroy(P);
        }

        // Release the shape
        SHPDestroyObject(psShape);
    }

    // Close the DBF file
    DBFClose(hDBF);

    // Clean up the proj objects
    proj_context_destroy(C);

    return pointsAttributes;
}

// Read multi-line strings from the shapefile and project
// them from the source CRS to the target CRS.
std::vector<std::pair<std::vector<Point>,
                      std::map<QString, QString>>>
ShapefileReader::readMultiLineStrings(std::string sourceCRS,
                                      std::string projectionCRS)
{
    std::vector<std::pair<std::vector<Point>,
                          std::map<QString, QString>>>
        multiLineStringsAttributes;

    if (!hSHP) {
        return multiLineStringsAttributes;
    }

    // Get shapefile info
    int nEntities;
    int nShapeType;
    double adfMinBound[4];
    double adfMaxBound[4];
    SHPGetInfo(hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound);

    // Create a transformation object using proj
    PJ_CONTEXT *C = proj_context_create();
    //std::string utmCrsString = "+proj=utm +zone=19 +datum=WGS84";
    PJ *P = proj_create_crs_to_crs(C, sourceCRS.c_str(),
                                   projectionCRS.c_str(), NULL);
    PJ_COORD a, b;

    // Open the associated DBF file
    QString dbfFilePath = getDbfFilePath();
    DBFHandle hDBF = DBFOpen(dbfFilePath.toStdString().c_str(), "rb");


    // Loop through features
    for (int i = 0; i < nEntities; ++i)
    {
        SHPObject* psShape = SHPReadObject(hSHP, i);

        if (psShape != nullptr && psShape->nSHPType == SHPT_ARC)
        {
            std::vector<Point> multiLineString;
            int numParts = psShape->nParts;
            int startVertex = 0;
            for (int partIndex = 0; partIndex < numParts; ++partIndex)
            {
                int endVertex = (partIndex == numParts - 1) ?
                                    psShape->nVertices :
                                    psShape->panPartStart[partIndex + 1];
                for (int vertexIndex = startVertex;
                     vertexIndex < endVertex;
                     ++vertexIndex)
                {
                    Point point;

                    // Project the point
                    a = proj_coord(psShape->padfX[vertexIndex],
                                   psShape->padfY[vertexIndex],
                                   psShape->padfZ[vertexIndex],
                                   0);

                    b = proj_trans(P, PJ_FWD, a);

                    point.x = b.xyz.x;
                    point.y = b.xyz.y;
                    point.z = b.xyz.z;

                    multiLineString.push_back(point);
                }
                startVertex = endVertex;
            }

            std::map<QString, QString> recordAttributes;
            if (hDBF) {
                // Read attributes for this shape
                int fieldCount = DBFGetFieldCount(hDBF);
                for (int j = 0; j < fieldCount; ++j)
                {
                    char fieldName[12];
                    DBFGetFieldInfo(hDBF, j, fieldName,
                                    NULL, NULL); // Get the field name
                    std::string fieldValue =
                        DBFReadStringAttribute(hDBF, i, j);
                    recordAttributes[QString(fieldName)] =
                        QString::fromStdString(fieldValue);
                }
            }

            multiLineStringsAttributes.emplace_back(
                std::make_pair(multiLineString, recordAttributes));
        }

        // Release the shape
        SHPDestroyObject(psShape);
    }

    // Close the DBF file
    DBFClose(hDBF);

    // Clean up the proj objects
    proj_context_destroy(C);
    proj_destroy(P);

    return multiLineStringsAttributes;
}

QStringList ShapefileReader::getAttributeNames()
{
    QStringList attributeNames;

    // Open the associated DBF file
    QString dbfFilePath = getDbfFilePath();
    DBFHandle hDBF = DBFOpen(dbfFilePath.toStdString().c_str(), "rb");

    if (!hDBF) {
        return attributeNames;
    }

    // Read the attribute names from the DBF file
    int fieldCount = DBFGetFieldCount(hDBF);
    for (int i = 0; i < fieldCount; ++i)
    {
        char fieldName[12];
        DBFGetFieldInfo(hDBF, i, fieldName, NULL, NULL); // Get the field name
        attributeNames.append(QString(fieldName));
    }

    // Close the DBF file
    DBFClose(hDBF);

    return attributeNames;
}


// Helper method to get the DBF file path associated with the shapefile.
QString ShapefileReader::getDbfFilePath()
{
    QString dbfFilePath = filePath;
    dbfFilePath.replace(dbfFilePath.length() - 3, 3, "dbf");
    return dbfFilePath;
}

// Helper method to get the PRJ file path associated with the shapefile.
QString ShapefileReader::getPrjFilePath()
{
    QString dbfFilePath = filePath;
    dbfFilePath.replace(dbfFilePath.length() - 3, 3, "prj");
    return dbfFilePath;
}


// Reads the contents of the PRJ file associated with the shapefile.
std::string ShapefileReader::readPrjFile() {
    QString prjFilePath = this->getPrjFilePath();
    std::ifstream prjFile(prjFilePath.toStdString());
    if (!prjFile.is_open()) {
        std::cerr << "Error: Unable to open .prj file." << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << prjFile.rdbuf();
    return buffer.str();
}

// Extracts the source CRS information from the PRJ file.
std::string ShapefileReader::extractSourceCRS() {
    std::string wkt = readPrjFile();
    if (wkt.empty()){
        return "";
    }
    // Find the opening double quote to extract the CRS information.
    size_t startPos = wkt.find('"');
    if (startPos == std::string::npos) {
        std::cerr << "Error: CRS information not found in WKT." << std::endl;
        return "";
    }

    // Find the closing double quote after the opening double quote.
    size_t endPos = wkt.find('"', startPos + 1);
    if (endPos == std::string::npos) {
        std::cerr << "Error: Invalid WKT format - "
                     "closing double quote not found." << std::endl;
        return "";
    }

    // Extract the CRS information from the WKT.
    std::string crsInfo = wkt.substr(startPos + 1, endPos - startPos - 1);
    return crsInfo;
}

// Function to calculate the UTM zone based on longitude
int ShapefileReader::calculateUTMZone(double longitude) {
    return static_cast<int>((longitude + 180.0) / 6) + 1;
}

// Retrieves the CRS identifier for the given CRS name.
std::string ShapefileReader::getCRSIdentifier(const char* crs_name) {
    PJ_CONTEXT* ctx = proj_context_create();

    PJ* pj = proj_create(ctx, crs_name);

    if (pj == nullptr) {
        std::cerr << "Error: Coordinate system not found.\n";
        proj_context_destroy(ctx);
        return ""; // Return an empty string to indicate an error
    }

    PJ* cs = proj_crs_get_coordinate_system(ctx, pj);

    if (cs == nullptr) {
        std::cerr << "Error: Coordinate system not found "
                     "for the coordinate system.\n";
        proj_destroy(pj);
        proj_context_destroy(ctx);
        return ""; // Return an empty string to indicate an error
    }

    const char* crs_id = proj_get_id_code(pj, 0);
    if (crs_id == nullptr) {
        std::cerr << "Error: Identifier not found for "
                     "the coordinate system.\n";
        proj_destroy(cs);
        proj_destroy(pj);
        proj_context_destroy(ctx);
        return ""; // Return an empty string to indicate an error
    }

//    std::cout << "CRS name: " << proj_get_name(pj) << "\n";
//    std::cout << "Projected CRS?: " << isProjectedCRS(pj);

    std::string out = std::string(crs_id);
    // Clean up resources
    proj_destroy(cs);
    proj_destroy(pj);
    proj_context_destroy(ctx);

    // Convert the C-style string to C++ string and return
    return out;
}

// Checks whether the CRS is a projected CRS.
bool ShapefileReader::isProjectedCRS(const std::string crs_name) {
    bool isProjected = false;
    PJ_CONTEXT* ctx = proj_context_create();

    PJ* pj = proj_create(ctx, crs_name.c_str());

    if (pj == nullptr) {
        std::cerr << "Error: Coordinate system not found.\n";
        proj_context_destroy(ctx);
        return isProjected; // Return false to indicate an error
    }

    if (pj != nullptr) {
        if (proj_get_type(pj) == PJ_TYPE_PROJECTED_CRS) {
            isProjected = true;
        }
    }

    // Clean up resources
    proj_destroy(pj);
    proj_context_destroy(ctx);
    return isProjected;
}

