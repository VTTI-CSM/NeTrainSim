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
Vector<std::tuple<int, double,
                  double, std::string,
                  double, double>> ReadWriteNetwork::readNodesFile(
    const std::string& fileName) {
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
        // Read scale values
        std::istringstream ss(lines[1]);
        int N;
        float scaleX, scaleY;
        ss >> N >> scaleX >> scaleY;

        // Read nodes
        Vector<int> _userIDNodes;
        Vector<double> _xNodes;
        Vector<double> _yNodes;
        Vector<std::string> _descNodes;

        Vector<std::tuple<int, double, double, std::string,
                          double, double>> records;

        for (int i = 2; i < lines.size(); i++)
        {
            std::istringstream ss(lines[i]);
            int userID;
            float x, y;
            std::string desc;
            ss >> userID >> x >> y >> desc;
            if (ss.good())
            {
                records.push_back(std::make_tuple(userID, x, y,
                                                  desc, scaleX, scaleY));
            }
            else
            {
                ss.clear();
                ss >> userID >> x >> y;

                records.push_back(std::make_tuple(userID, x, y, " ",
                                                  scaleX, scaleY));
            }
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
Vector<std::tuple<int, int, int, double, int,
                  double, double, int, double, bool, std::string,
                  std::string, double>> ReadWriteNetwork::readLinksFile(
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
        std::istringstream iss(lines[1]);
        for (std::string s; iss >> s; ) {
            scaleValues.push_back(s);
        }
        scaleValues.erase(scaleValues.begin());

        // Read links data
        Vector<std::tuple<int, int, int, double, int,
                          double, double, int, double, bool, std::string,
                          std::string, double>> records;

        for (int i = 2; i < lines.size(); i++) {
            Vector<std::string> linkValues;
            std::istringstream iss(lines[i]);
            for (std::string s; iss >> s; ) {
                linkValues.push_back(s);
            }
            if (!linkValues.empty()) {
                bool hasCaten;
                std::stringstream ss(linkValues[9]);
                ss >> hasCaten;

                if (linkValues.size() == 10) {
                    records.push_back(std::make_tuple(
                        std::stoi(linkValues[0]), // ID
                        std::stoi(linkValues[1]), // From
                        std::stoi(linkValues[2]), //TO
                        std::stod(linkValues[3]), // free flow speed
                        std::stoi(linkValues[4]), // signal
                        std::stod(linkValues[5]), // grade
                        std::stod(linkValues[6]), // curvature
                        std::stoi(linkValues[7]), // direction
                        std::stod(linkValues[8]), // var
                        hasCaten, //cat
                        "", //signals at
                        "ND Region", // region
                        std::stod(scaleValues[0])));
                }
                else if (linkValues.size() == 11) {
                    records.push_back(std::make_tuple(
                        std::stoi(linkValues[0]), // ID
                        std::stoi(linkValues[1]), // FROM
                        std::stoi(linkValues[2]), // TO
                        std::stod(linkValues[3]), // Speed
                        std::stoi(linkValues[4]), // Signal
                        std::stod(linkValues[5]), // Grade
                        std::stod(linkValues[6]), // Curvature
                        std::stoi(linkValues[7]), // Direction
                        std::stod(linkValues[8]), // Speed variation
                        hasCaten, // Has catenary ?
                        linkValues[10], // Siangls at
                        "ND Region", // Region
                        std::stod(scaleValues[0]))); // freeflow speed scale
                }
                else if (linkValues.size() == 12) {
                    records.push_back(std::tuple<int, int, int,
                                                 double, int, double,
                                                 double, int, double,
                                                 bool, std::string,
                                                 std::string, double>(
                        std::stoi(linkValues[0]),
                        std::stoi(linkValues[1]),
                        std::stoi(linkValues[2]),
                        std::stod(linkValues[3]),
                        std::stoi(linkValues[4]),
                        std::stod(linkValues[5]),
                        std::stod(linkValues[6]),
                        std::stoi(linkValues[7]),
                        std::stod(linkValues[8]),
                        hasCaten,
                        linkValues[10],
                        linkValues[11],
                        std::stod(scaleValues[0])));
                }
                else {
                    // if the link line is not correct
                    throw std::runtime_error("Error");
                }
            }
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
    Vector<std::tuple<int, double,
                      double, std::string,
                      double, double>> nodesRecords) {
    Vector<std::shared_ptr<NetNode>> _Nodes;
    int nodeSimID = 0;
    for (auto &record: nodesRecords) {
        int userID;
        double x, y, scaleX, scaleY;
        std::string desc;
        std::tie(userID, x, y, desc, scaleX, scaleY) = record;
        NetNode node = NetNode(nodeSimID, userID, x, y, desc, scaleX, scaleY);
        _Nodes.push_back(std::make_shared<NetNode>(node));
        nodeSimID++;
    }
    return _Nodes;
}

Vector<std::shared_ptr<NetLink>> ReadWriteNetwork::generateLinks(
    Vector<std::shared_ptr<NetNode>> theFileNodes,
    Vector<std::tuple<int, int, int,
                      double, int,
                      double, double, int,
                      double, bool, std::string,
                      std::string, double> > linksRecords)
{
    Vector<std::shared_ptr<NetLink>> links;
    int simulatorlinkID = 0;
    for (auto &record : linksRecords) {
        int linkID, trafficSignalID, linkNoOfDirections, fromNode, toNode;
        double maxSpeed, linkGrade, linkCurvature;
        double speedVariationfactor, maxSpeedScale;
        bool isCatenaryAvailable;
        std::string linkInRegion, signalsEnds;

        std::tie(linkID, fromNode, toNode, maxSpeed,
                 trafficSignalID, linkGrade, linkCurvature,
                 linkNoOfDirections, speedVariationfactor,
                 isCatenaryAvailable, signalsEnds, linkInRegion,
                 maxSpeedScale) = record;

        std::shared_ptr<NetNode> fromNodeNode =
            getSimulatorNodeByUserID(theFileNodes, fromNode);
        std::shared_ptr<NetNode> toNodeNode   =
            getSimulatorNodeByUserID(theFileNodes, toNode);

        NetLink link = NetLink(simulatorlinkID, linkID, fromNodeNode,
                               toNodeNode, maxSpeed, trafficSignalID,
                               signalsEnds, linkGrade, linkCurvature,
                               linkNoOfDirections, speedVariationfactor,
                               isCatenaryAvailable, linkInRegion,
                               maxSpeedScale);
        links.push_back(std::make_shared<NetLink>(link));
        simulatorlinkID++;
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
    Vector<std::tuple<int, int, int, double,
                      int, double, double, int,
                      double, bool, std::string,
                      std::string, double> > linksRecords,
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
        std::get<12>(linksRecords[0]) << "\n";

    for (auto& record: linksRecords) {
        lines << Utils::convertTupleToStringStream(record, -1, "\t").str();
        lines << "\n";
    }
    return Utils::writeToFile(lines, filename);
}




bool ReadWriteNetwork::writeNodesFile(
    Vector<std::tuple<int, double, double, std::string,
                      double, double> > nodesRecords, string &filename) {
    if (filename.empty()) {
        return false;
    }

    std::stringstream lines;
    lines << "File is created using NeTrainSim GUI\n";
    lines << std::get<4>(nodesRecords[0]) << "\t" <<
        std::get<5>(nodesRecords[0]) << "\n";

    for (auto& record: nodesRecords) {
        lines << Utils::convertTupleToStringStream(record, -2, "\t").str();
        lines << "\n";
    }
    return Utils::writeToFile(lines, filename);
}
