#include <iostream>
#include <fstream>
#include "netlink.h"
#include "netnode.h"
#include "../util/error.h"
#include "../util/vector.h"
#include <exception>
#include "readwritenetwork.h"
#include "../util/utils.h"

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

        Vector<std::string> lines;
        std::string line;
        while (std::getline(file, line))
        {
            lines.push_back(line);
        }
        file.close();

        if (lines.size() == 0) {
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

        for (int i = 2; i < lines.size(); i++)
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
        std::ifstream file(fileName);
        if (!file.good()) {
            throw std::runtime_error(std::string("Error: ") +
                                     std::to_string(
                                         static_cast<int>(
                                         Error::linksFileDoesNotExist)) +
                                     "\nLinks File does not exist!");
        }
        std::string line;
        Vector<std::string> lines;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();

        if (lines.size() == 0) {
            throw std::runtime_error(std::string("Error: ") +
                                     std::to_string(
                                         static_cast<int>(
                                         Error::emptyLinksFile)) +
                                     "\nLinks File is empty!");
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


        for (int i = 2; i < lines.size(); i++) {
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
            std::string desc = record["Desc"];

            NetNode node = NetNode(nodeSimID, userID, x,
                                   y, desc, scaleX, scaleY);
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

    if (filename.empty()) {
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
