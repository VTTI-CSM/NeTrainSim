#include <iostream>
#include <fstream>
#include "netlink.h"
#include "netnode.h"
#include "../util/error.h"
#include "../util/vector.h"
#include <exception>
#include "readwritenetwork.h"
#include "../util/utils.h"
#include <QFile>

/**
 * Reads nodes file
 * @author	Ahmed
 * @date	2/14/2023
 * @exception	std::runtime_error	Raised when a runtime error condition
 *                                  occurs.
 * @param 	fileName	Filename of the file.
 * @returns	The tuple of IDs as a vector, x coordinates as a vector,
 *          y coordinates as a vector, description as a vector, x scale,
 *          and y scale as doubles.
 */
Vector<Map<std::string, std::string>> ReadWriteNetwork::readNodesFile(
    const std::string& fileName)
{
        if (fileName.empty()) {
            throw std::runtime_error(std::string("Error: ") +
                                     std::to_string(
                                         static_cast<int>(
                                         Error::nodesFileDoesNotExist)) +
                                     "\nNodes file does not exist!\n");
        }
        // Open file to read
        std::ifstream file(fileName);
        if (!file.good()) {
            throw std::runtime_error(std::string("Error: ") +
                                     std::to_string(
                                         static_cast<int>(
                                         Error::nodesFileDoesNotExist)) +
                                     "\nNodes file does not exist!\n");
        }
        // Read the entire file content into a string
        std::string fileContent((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());

        file.close(); // Close the file as we no longer need it

        // Pass the file content to the readNodesFileContent function
        return readNodesFileContent(fileContent);
}

Vector<Map<std::string, std::string>>
ReadWriteNetwork::readNodesFromJson(const QJsonObject& nodesJson) {
    Vector<Map<std::string, std::string>> records;
    try {
        // Validate that we have nodes array and scales
        if (!nodesJson.contains("nodes")) {
            throw std::runtime_error("Missing 'nodes' array in JSON");
        }
        if (!nodesJson.contains("scales")) {
            throw std::runtime_error("Missing 'scales' object in JSON");
        }

        // Validate scales
        const QJsonObject scales = nodesJson["scales"].toObject();
        if (!scales.contains("x")) {
            throw std::runtime_error("Missing 'x' scale value in scales object");
        }
        if (!scales.contains("y")) {
            throw std::runtime_error("Missing 'y' scale value in scales object");
        }

        // Get scale values
        std::string scaleX = scales["x"].toString().toStdString();
        std::string scaleY = scales["y"].toString().toStdString();

        // Get nodes array
        QJsonArray nodesArray = nodesJson["nodes"].toArray();
        if (nodesArray.isEmpty()) {
            throw std::runtime_error("Nodes array is empty");
        }

        // Reserve space for optimization
        records.reserve(nodesArray.size());

        // Process each node
        for (int i = 0; i < nodesArray.size(); i++) {
            if (!nodesArray[i].isObject()) {
                throw std::runtime_error("Node at index " + std::to_string(i) +
                                         " is not a valid JSON object");
            }

            QJsonObject node = nodesArray[i].toObject();
            Map<std::string, std::string> nodeRecord;

            // Check required fields
            const QStringList requiredFields = {"userID", "x", "y", "description",
                                                "isTerminal", "terminalDwellTime"};
            for (const auto& field : requiredFields) {
                if (!node.contains(field)) {
                    throw std::runtime_error("Node at index " + std::to_string(i) +
                                             " missing required field '" + field.toStdString() + "'");
                }
            }

            // Add required fields to nodeRecord with validation
            if (!node["userID"].isDouble() && !node["userID"].toString().toInt()) {
                throw std::runtime_error("Invalid userID for node at index " + std::to_string(i));
            }
            nodeRecord["UserID"] = QString::number(node["userID"].toInt()).toStdString();

            if (!node["x"].isDouble() && !node["x"].toString().toDouble()) {
                throw std::runtime_error("Invalid x coordinate for node at index " + std::to_string(i));
            }
            nodeRecord["XCoordinate"] = QString::number(node["x"].toDouble()).toStdString();

            if (!node["y"].isDouble() && !node["y"].toString().toDouble()) {
                throw std::runtime_error("Invalid y coordinate for node at index " + std::to_string(i));
            }
            nodeRecord["YCoordinate"] = QString::number(node["y"].toDouble()).toStdString();

            nodeRecord["Desc"] = node["description"].toString().toStdString();

            if (!node["isTerminal"].isBool()) {
                throw std::runtime_error("Invalid isTerminal value for node at index " + std::to_string(i));
            }
            nodeRecord["IsTerminal"] = node["isTerminal"].toBool() ? "1" : "0";

            if (!node["terminalDwellTime"].isDouble() && !node["terminalDwellTime"].toString().toDouble()) {
                throw std::runtime_error("Invalid terminalDwellTime for node at index " + std::to_string(i));
            }
            nodeRecord["TerminalDwellTime"] = QString::number(node["terminalDwellTime"].toDouble()).toStdString();

            // Add scales
            nodeRecord["XScale"] = scaleX;
            nodeRecord["YScale"] = scaleY;

            records.push_back(std::move(nodeRecord));
        }

        return records;

    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("Error: ") +
            std::to_string(static_cast<int>(Error::wrongNodesFileStructure)) +
            "\nFailed to parse JSON nodes data: " + e.what() + "\n");
    }
}

Vector<Map<std::string, std::string>> ReadWriteNetwork::readNodesFileContent(
    const std::string& fileContent) {

    // Split the file content into lines
    Vector<std::string> lines = Utils::split(fileContent, '\n', true);

    if (lines.empty()) {
        throw std::runtime_error(std::string("Error: ") +
                                 std::to_string(
                                     static_cast<int>(
                                         Error::emptyNodesFile)) +
                                 "\nNodes File is empty!\n");
    }

    try {
        std::vector<std::string> scales = Utils::split(lines[1],
                                                       '\t',
                                                       true);
        if (scales.size() < 3) {
            throw std::runtime_error(std::string("Error: ") +
                                     std::to_string(
                                         static_cast<int>(
                                             Error::wrongNodesFileStructure)) +
                                     "\nBad nodes file structure!\n");
        }
        std::string N = scales[0];
        std::string scaleX = scales[1];
        std::string scaleY = scales[2];

        Vector<Map<std::string, std::string>> records;

        for (unsigned long long i = 2; i < lines.size(); i++)
        {
            std::vector<std::string> vals = Utils::split(lines[i],
                                                         '\t',
                                                         true);
            // load all the values in order
            if (vals.size() <
                nodeFilekeys.size() - nodeFileIgnoreRecordsWrite.size()) {
                vals.push_back("ND");
            }

            // double check the size now
            if (vals.size() !=
                nodeFilekeys.size() - nodeFileIgnoreRecordsWrite.size())
            {
                throw std::runtime_error(std::string("Error: ") +
                                         std::to_string(
                                             static_cast<int>(
                                                 Error::wrongNodesFileStructure)) +
                                         "\nBad nodes file structure!\n");
            }

            Map<std::string, std::string> nodeRecord;

            for (const auto& key: nodeFilekeys) {
                if (nodeFileIgnoreRecordsWrite.exist(key)) {
                    continue;
                }
                int keyIndex = nodeFilekeys.index(key);
                nodeRecord[key] = vals[keyIndex];
            }
            nodeRecord["XScale"] = scaleX;
            nodeRecord["YScale"] = scaleY;

            records.push_back(nodeRecord);
        }

        return records;

    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("Error: ") +
                                 std::to_string(
                                     static_cast<int>(
                                         Error::wrongNodesFileStructure)) +
                                 "\nBad nodes file structure!\n");

    }
}

Vector<Map<std::string, std::string>>
ReadWriteNetwork::readNodesFromJsonFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error(std::string("Error: Cannot open JSON file: ") +
                                 filePath.toStdString());
    }

    QByteArray jsonData = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    if (!doc.isObject()) {
        throw std::runtime_error("Error: Invalid JSON format");
    }

    return readNodesFromJson(doc.object());
}

/**
 * Reads links file
 * @author	Ahmed
 * @date	2/14/2023
 * @exception	std::runtime_error	Raised when a runtime error
 *              condition occurs.
 * @param 	fileName	Filename of the file.
 * @returns	The links data. A vector record has the following:
 *                          1. Link User ID
 *                          2. Start Node User ID
 *                          3. End Node User ID
 *                          4. Link Length
 *                          5. Link Max Speed (m/s)
 *                          6. Signal User ID
 *                          7. Link Grade
 *                          8. Link Curvature
 *                          9. Link Number of Directions
 *                          10. Speed Variation Factor
 *                          11. Is Catenary Available for this link
 *                          12. Signal Placed At Which End Node ID
 *                          13. The Region the link is in
 */
Vector<Map<std::string, std::string>> ReadWriteNetwork::readLinksFile(
    const std::string& fileName) {

    if (fileName.empty()) {
        throw std::runtime_error(std::string("Error: ") +
                                 std::to_string(
                                     static_cast<int>(
                                         Error::linksFileDoesNotExist)) +
                                 "\nLinks File does not exist!");
    }
    // Open file to read
    std::ifstream file(fileName);
    if (!file.good()) {
        throw std::runtime_error(std::string("Error: ") +
                                 std::to_string(
                                     static_cast<int>(
                                         Error::linksFileDoesNotExist)) +
                                 "\nLinks File does not exist!");
    }
    // Read the entire file content into a string
    std::string fileContent((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

    file.close(); // Close the file as we no longer need it

    // Pass the file content to the readNodesFileContent function
    return readLinksFileContent(fileContent);

}

Vector<Map<std::string, std::string>>
ReadWriteNetwork::readLinksFromJsonFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error(std::string("Error: Cannot open JSON file: ") +
                                 filePath.toStdString());
    }

    QByteArray jsonData = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    if (!doc.isObject()) {
        throw std::runtime_error("Error: Invalid JSON format");
    }

    return readLinksFromJson(doc.object());
}

Vector<Map<std::string, std::string>> ReadWriteNetwork::readLinksFileContent(
    const std::string& fileContent) {

    // Split the file content into lines
    Vector<std::string> lines = Utils::split(fileContent, '\n', true);

    if (lines.empty()) {
        throw std::runtime_error(std::string("Error: ") +
                                 std::to_string(
                                     static_cast<int>(
                                         Error::emptyNodesFile)) +
                                 "\nLinks File is empty!\n");
    }

    try {
        // Read Scale values
        Vector<std::string> scaleValues;

        // Read links data
        Vector<Map<std::string, std::string>> records;

        // Read scale values
        std::istringstream s(lines[1]);
        std::vector<std::string> scales = Utils::split(lines[1], '\t', true);
        if (scales.size() < 3) {
            throw std::runtime_error(std::string("Error: ") +
                                     std::to_string(
                                         static_cast<int>(
                                             Error::wrongNodesFileStructure)) +
                                     "\nBad nodes file structure!\n");
        }
        std::string N = scales[0];
        std::string lengthScale = scales[1];
        std::string speedScale = scales[2];


        for (unsigned long long i = 2; i < lines.size(); i++) {
            std::vector<std::string> vals = Utils::split(lines[i], '\t', true);
            // load all the values in order

            // if the link file does not have signals location,
            // add empty string
            if (vals.size() <
                linksFilekeys.size() - linksFileIgnoreRecordsWrite.size() - 1)
            {
                vals.push_back("");
            }

            // if it does not have a region, add empty string
            if (vals.size() <
                linksFilekeys.size() - linksFileIgnoreRecordsWrite.size()) {
                vals.push_back("ND Region");
            }

            // double check the size now
            if (vals.size() !=
                linksFilekeys.size() - linksFileIgnoreRecordsWrite.size())
            {
                throw std::runtime_error(std::string("Error: ") +
                                         std::to_string(
                                             static_cast<int>(
                                                 Error::wrongNodesFileStructure)) +
                                         "\nBad nodes file structure!\n");
            }

            Map<std::string, std::string> linkRecord;

            for (const auto& key: linksFilekeys) {
                if (linksFileIgnoreRecordsWrite.exist(key)) {
                    continue;
                }
                int keyIndex = linksFilekeys.index(key);
                linkRecord[key] = vals[keyIndex];
            }
            linkRecord["LengthScale"] = lengthScale;
            linkRecord["FreeFlowSpeedScale"] = speedScale;

            records.push_back(linkRecord);
        }

        return records;

    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("Error: ") +
                                 std::to_string(
                                     static_cast<int>(
                                         Error::wrongLinksFileStructure)) +
                                 "\nBad links file structure!\n");
    }
}

Vector<Map<std::string, std::string>>
ReadWriteNetwork::readLinksFromJson(const QJsonObject& linksJson)
{
    Vector<Map<std::string, std::string>> records;

    try {
        // Validate that we have links array and scales
        if (!linksJson.contains("links")) {
            throw std::runtime_error("Missing 'links' array in JSON");
        }
        if (!linksJson.contains("scales")) {
            throw std::runtime_error("Missing 'scales' object in JSON");
        }

        // Validate scales
        const QJsonObject scales = linksJson["scales"].toObject();
        if (!scales.contains("length")) {
            throw std::runtime_error("Missing 'length' scale value in scales object");
        }
        if (!scales.contains("speed")) {
            throw std::runtime_error("Missing 'speed' scale value in scales object");
        }

        std::string lengthScale = scales["length"].toString().toStdString();
        std::string speedScale = scales["speed"].toString().toStdString();

        // Get links array
        QJsonArray linksArray = linksJson["links"].toArray();
        if (linksArray.isEmpty()) {
            throw std::runtime_error("Links array is empty");
        }

        // Reserve space for optimization
        records.reserve(linksArray.size());

        // Define JSON field mappings to linksFilekeys
        const std::map<std::string, QString> fieldMapping = {
            {"UserID", "userID"},
            {"FromNodeID", "fromNodeID"},
            {"ToNodeID", "toNodeID"},
            {"Length", "length"},
            {"FreeFlowSpeed", "maxSpeed"},
            {"SignalNo", "trafficSignalID"},
            {"DirectionalGrade", "grade"},
            {"Curvature", "curvature"},
            {"Directions", "numberOfDirections"},
            {"SpeedVariation", "speedVariationFactor"},
            {"HasCatenary", "isCatenaryAvailable"},
            {"SignalsAtNodes", "signalsAtNodes"},
            {"Region", "region"}
        };

        // Process each link
        for (int i = 0; i < linksArray.size(); i++) {
            if (!linksArray[i].isObject()) {
                throw std::runtime_error("Link at index " + std::to_string(i) +
                                         " is not a valid JSON object");
            }

            QJsonObject link = linksArray[i].toObject();
            Map<std::string, std::string> linkRecord;

            // Process fields according to linksFilekeys
            for (const auto& key : linksFilekeys) {
                if (key == "LengthScale" || key == "FreeFlowSpeedScale") {
                    continue; // We'll add these separately
                }

                // Get the corresponding JSON field name
                QString jsonFieldName = fieldMapping.at(key);

                // Check if field exists
                if (!link.contains(jsonFieldName)) {
                    throw std::runtime_error("Link at index " + std::to_string(i) +
                                             " missing required field '" + jsonFieldName.toStdString() + "'");
                }

                // Process based on expected field type
                if (key == "UserID" || key == "FromNodeID" || key == "ToNodeID" ||
                    key == "SignalNo" || key == "Directions") {
                    // Integer fields
                    if (!link[jsonFieldName].isDouble() && !link[jsonFieldName].toString().toInt()) {
                        throw std::runtime_error("Invalid " + key + " for link at index " + std::to_string(i));
                    }
                    linkRecord[key] = QString::number(link[jsonFieldName].toInt()).toStdString();
                }
                else if (key == "Length" || key == "FreeFlowSpeed" || key == "DirectionalGrade" ||
                         key == "Curvature" || key == "SpeedVariation") {
                    // Double fields
                    if (!link[jsonFieldName].isDouble() && !link[jsonFieldName].toString().toDouble()) {
                        throw std::runtime_error("Invalid " + key + " for link at index " + std::to_string(i));
                    }
                    linkRecord[key] = QString::number(link[jsonFieldName].toDouble()).toStdString();
                }
                else if (key == "HasCatenary") {
                    // Boolean field
                    if (!link[jsonFieldName].isBool()) {
                        throw std::runtime_error("Invalid " + key + " for link at index " + std::to_string(i));
                    }
                    linkRecord[key] = link[jsonFieldName].toBool() ? "1" : "0";
                }
                else {
                    // String fields (SignalsAtNodes, Region)
                    linkRecord[key] = link[jsonFieldName].toString().toStdString();
                }
            }

            // Add scales
            linkRecord["LengthScale"] = lengthScale;
            linkRecord["FreeFlowSpeedScale"] = speedScale;

            records.push_back(std::move(linkRecord));
        }

        return records;

    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("Error: ") +
            std::to_string(static_cast<int>(Error::wrongLinksFileStructure)) +
            "\nFailed to parse JSON links data: " + e.what() + "\n");
    }
}

/**
         * @brief readNodesFile
         * @param nodesRecords
         * @return
         */
Vector<std::shared_ptr<NetNode>> ReadWriteNetwork::generateNodes(
    Vector<Map<std::string, std::string>> nodesRecords)
{
    Vector<std::shared_ptr<NetNode>> _Nodes;
    try {

        int nodeSimID = 0;
        for (auto &record: nodesRecords) {
            int userID = std::stoi(record["UserID"]);
            double x = std::stod(record["XCoordinate"]);
            double y = std::stod(record["YCoordinate"]);
            double scaleX = std::stod(record["XScale"]);
            double scaleY = std::stod(record["YScale"]);
            double TerminalDwellTime = std::stod(record["TerminalDwellTime"]);
            std::string desc = record["Desc"];
            bool IsTerminal;
            stringstream ss(record["IsTerminal"]);
            ss >> IsTerminal;

            NetNode node = NetNode(nodeSimID, userID, x,
                                   y, desc, scaleX, scaleY);
            node.isTerminal = IsTerminal;
            node.dwellTimeIfTerminal = TerminalDwellTime;
            _Nodes.push_back(std::make_shared<NetNode>(node));
            nodeSimID++;
        }

    }
    catch (const std::exception &e) {
        throw std::runtime_error(std::string("Error: ") +
                                 std::to_string(
                                     static_cast<int>(
                                         Error::wrongNodesFileStructure)) +
                                 "\nBad nodes file structure!\n");
    }
    return _Nodes;

}

Vector<std::shared_ptr<NetLink>> ReadWriteNetwork::generateLinks(
    Vector<std::shared_ptr<NetNode>> theFileNodes,
    Vector<Map<std::string, std::string>> linksRecords)
{
    Vector<std::shared_ptr<NetLink>> links;
    try {
        int simulatorlinkID = 0;
        for (auto &record : linksRecords) {
            int linkID = std::stoi(record["UserID"]);
            int trafficSignalID = std::stoi(record["SignalNo"]);
            int linkNoOfDirections = std::stoi(record["Directions"]);
            int fromNode = std::stoi(record["FromNodeID"]);
            int toNode = std::stoi(record["ToNodeID"]);
            double length = std::stod(record["Length"]);
            double maxSpeed = std::stod(record["FreeFlowSpeed"]);
            double linkGrade = std::stod(record["DirectionalGrade"]);
            double linkCurvature = std::stod(record["Curvature"]);
            double speedVariationfactor = std::stod(record["SpeedVariation"]);
            double maxSpeedScale = std::stod(record["FreeFlowSpeedScale"]);
            double lengthScale = std::stod(record["LengthScale"]);
            bool isCatenaryAvailable;
            stringstream ss(record["HasCatenary"]);
            ss >> isCatenaryAvailable;

            std::string linkInRegion = record["Region"];
            std::string signalsEnds = record["SignalsAtNodes"];
            if (QString::fromStdString(signalsEnds).trimmed().toLower() ==
                "not defined") {
                signalsEnds = "";
            }

            std::shared_ptr<NetNode> fromNodeNode =
                getSimulatorNodeByUserID(theFileNodes, fromNode);
            std::shared_ptr<NetNode> toNodeNode   =
                getSimulatorNodeByUserID(theFileNodes, toNode);

            NetLink link = NetLink(simulatorlinkID, linkID, fromNodeNode,
                                   toNodeNode, length, maxSpeed,
                                   trafficSignalID,
                                   signalsEnds, linkGrade, linkCurvature,
                                   linkNoOfDirections, speedVariationfactor,
                                   isCatenaryAvailable, linkInRegion,
                                   lengthScale,
                                   maxSpeedScale);
            links.push_back(std::make_shared<NetLink>(link));
            simulatorlinkID++;
        }
    }
    catch (const std::exception &e) {
        throw std::runtime_error(std::string("Error: ") +
                                 std::to_string(
                                     static_cast<int>(
                                         Error::wrongLinksFileStructure)) +
                                 "\nBad links file structure!\n");
    }
    return links;
}


/**
         * Gets simulator node by user identifier
         * @author	Ahmed
         * @date	2/14/2023
         * @exception	std::runtime_error	Raised when a runtime error
         *                                   condition occurs.
         * @param 	oldID	Identifier for the old.
         * @returns	The simulator node by user identifier.
         */
std::shared_ptr<NetNode> ReadWriteNetwork::getSimulatorNodeByUserID(
    Vector<std::shared_ptr<NetNode>> theFileNodes,
    int oldID)
{
    for (std::shared_ptr<NetNode>& n : theFileNodes) {
        if (n->userID == oldID) {
            return n;
        }
    }
    throw std::runtime_error(std::string("Error: ") +
                             std::to_string(
                                 static_cast<int>(
                                 Error::cannotFindNode)) +
                             "\nCould not find the node ID: " +
                             std::to_string(oldID) + "\n");
}



bool ReadWriteNetwork::writeLinksFile(
    Vector<Map<std::string, std::string>> linksRecords,
    string &filename)
{
    if (filename.empty()) {
        return false;
    }
    if (linksRecords.empty()) {
        return true;
    }
    std::stringstream lines;
    lines << "File is created using NeTrainSim GUI\n";
    lines << linksRecords.size() << "\t" <<
        (linksRecords[0]["LengthScale"]) << "\t" <<
        (linksRecords[0]["FreeFlowSpeedScale"]) << "\n";

    for (auto& record: linksRecords) {
        lines << record.valuesToString(linksFilekeys,
                                       linksFileIgnoreRecordsWrite,
                                       "\t");
        lines << "\n";
    }
    return Utils::writeToFile(lines, filename);
}




bool ReadWriteNetwork::writeNodesFile(
    Vector<Map<std::string, std::string>> nodesRecords,
    string &filename)
{

    if (filename.empty() || nodesRecords.empty()) {
        return false;
    }

    std::stringstream lines;
    lines << "File is created using NeTrainSim GUI\n";
    lines << nodesRecords.size() << "\t" <<
        nodesRecords[0]["XScale"] << "\t" <<
        nodesRecords[0]["YScale"] << "\n";


    for (auto& record: nodesRecords) {
        lines << record.valuesToString(nodeFilekeys,
                                       nodeFileIgnoreRecordsWrite,
                                       "\t");
        lines << "\n";
    }
    return Utils::writeToFile(lines, filename);
}
