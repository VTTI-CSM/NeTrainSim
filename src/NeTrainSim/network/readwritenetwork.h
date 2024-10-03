/**
 * @file ReadWriteNetwork.h
 * @brief This file contains the declaration of the ReadWriteNetwork namespace.
 *        The ReadWriteNetwork namespace provides functions for reading and
 *        writing network data.
 *        It includes functions for reading nodes and links files, writing
 *        nodes and links files,
 *        and generating NetNode and NetLink objects from the read data.
 *        It is designed to work with other classes and data structures in
 *        the simulation.
 *        Note: The implementation of the functions is not provided in
 *        this declaration file.
 *        They are implemented separately in a corresponding source file.
 * @author Ahmed
 * @version 0.1
 * @date 6/1/2023
 */

#ifndef NeTrainSim_ReadWriteNetwork_h
#define NeTrainSim_ReadWriteNetwork_h

#include "export.h"
#include <iostream>
#include <fstream>
#include "netlink.h"
#include "netnode.h"
#include "../util/vector.h"
#include <exception>

namespace ReadWriteNetwork {

static Vector<std::string> nodeFilekeys = {"UserID",
                                           "XCoordinate",
                                           "YCoordinate",
                                           "Desc",
                                           "XScale",
                                           "YScale"};

static Vector<std::string> nodeFileIgnoreRecordsWrite = {"XScale",
                                                         "YScale"};

static Vector<std::string> linksFilekeys = {"UserID",                   //0
                                            "FromNodeID",               //1
                                            "ToNodeID",                 //2
                                            "Length",                   //3
                                            "FreeFlowSpeed",            //4
                                            "SignalNo",                 //5
                                            "DirectionalGrade",         //6
                                            "Curvature",                //7
                                            "Directions",               //8
                                            "SpeedVariation",           //9
                                            "HasCatenary",              //10
                                            "SignalsAtNodes",           //11
                                            "Region",                   //12
                                            "LengthScale",              //13
                                            "FreeFlowSpeedScale"};      //14

static Vector<std::string> linksFileIgnoreRecordsWrite = {"LengthScale",
                                                          "FreeFlowSpeedScale"};

/**
     * @brief Reads the nodes file.
     * @param fileName The filename of the file.
     * @return The records of nodes as a vector of tuples.
     *         Each tuple contains the ID, x-coordinate, y-coordinate,
     *         description, x-scale, and y-scale of a node.
     * @throw std::runtime_error if a runtime error occurs.
     */
Vector<Map<std::string, std::string>> NETRAINSIMCORE_EXPORT
readNodesFile(const std::string& fileName);

Vector<Map<std::string, std::string>> NETRAINSIMCORE_EXPORT
readNodesFileContent(const std::string& fileContent);

/**
     * @brief Writes the nodes file.
     * @param nodesRecords The records of nodes as a vector of tuples.
     * @param filename The filename of the file to write.
     * @return True if the write operation is successful, false otherwise.
     */
bool NETRAINSIMCORE_EXPORT writeNodesFile(
    Vector<Map<std::string, std::string>> nodesRecords,
    std::string& filename);

/**
     * @brief Reads the links file.
     * @param fileName The filename of the file.
     * @return The records of links as a vector of tuples.
     *         Each tuple contains the link attributes including the IDs
     *         of the connected nodes, length, speed, etc.
     * @throw std::runtime_error if a runtime error occurs.
     */
Vector<Map<std::string, std::string>> NETRAINSIMCORE_EXPORT
readLinksFile(const std::string& fileName);

Vector<Map<std::string, std::string>> NETRAINSIMCORE_EXPORT
readLinksFileContent(const std::string& fileContent);

/**
     * @brief Writes the links file.
     * @param linksRecords The records of links as a vector of tuples.
     * @param filename The filename of the file to write.
     * @return True if the write operation is successful, false otherwise.
     */
bool NETRAINSIMCORE_EXPORT
writeLinksFile(Vector<Map<std::string, std::string>> linksRecords,
               std::string& filename);

/**
     * @brief Generates NetNode objects from the nodes records.
     * @param nodesRecords The records of nodes as a vector of tuples.
     * @return The generated NetNode objects as a vector.
     */
Vector<std::shared_ptr<NetNode>> NETRAINSIMCORE_EXPORT
generateNodes(Vector<Map<string, string> > nodesRecords);

/**
     * @brief Generates NetLink objects from the nodes and links records.
     * @param theFileNodes The NetNode objects generated from the nodes records.
     * @param linksRecords The records of links as a vector of tuples.
     * @return The generated NetLink objects as a vector.
     */
Vector<std::shared_ptr<NetLink>> NETRAINSIMCORE_EXPORT
generateLinks(Vector<std::shared_ptr<NetNode>> theFileNodes,
              Vector<Map<std::string, std::string>> linksRecords);

/**
     * @brief Gets the NetNode object with the given user identifier.
     * @param theFileNodes The NetNode objects generated from the nodes records.
     * @param oldID The user identifier of the NetNode to retrieve.
     * @return The NetNode object with the specified user identifier, or
     *         nullptr if not found.
     * @throw std::runtime_error if a runtime error occurs.
     */
std::shared_ptr<NetNode> NETRAINSIMCORE_EXPORT
getSimulatorNodeByUserID(
    Vector<std::shared_ptr<NetNode>> theFileNodes, int oldID);

} // namespace ReadWriteNetwork

#endif // NeTrainSim_ReadWriteNetwork_h
