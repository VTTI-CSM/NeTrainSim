/**
 * @file ReadWriteNetwork.h
 * @brief This file contains the declaration of the ReadWriteNetwork namespace.
 *        The ReadWriteNetwork namespace provides functions for reading and writing network data.
 *        It includes functions for reading nodes and links files, writing nodes and links files,
 *        and generating NetNode and NetLink objects from the read data.
 *        The ReadWriteNetwork namespace is intended to be used as part of a network simulation system.
 *        It is designed to work with other classes and data structures in the simulation.
 *        The functions in the ReadWriteNetwork namespace can be used in a C++ application.
 *        Note: The implementation of some functions is not provided in this declaration file.
 *              They should be implemented separately in a corresponding source file.
 * @author Ahmed
 * @version 0.1
 * @date 6/1/2023
 */

#ifndef NeTrainSim_ReadWriteNetwork_h
#define NeTrainSim_ReadWriteNetwork_h

#include <iostream>
#include <fstream>
#include "netlink.h"
#include "netnode.h"
#include "../util/vector.h"
#include <exception>

namespace ReadWriteNetwork {

/**
     * @brief Reads the nodes file.
     * @param fileName The filename of the file.
     * @return The records of nodes as a vector of tuples.
     *         Each tuple contains the ID, x-coordinate, y-coordinate,
     *         description, x-scale, and y-scale of a node.
     * @throw std::runtime_error if a runtime error occurs.
     */
Vector<std::tuple<int, double, double, std::string,
                  double, double>> readNodesFile(const std::string& fileName);

/**
     * @brief Writes the nodes file.
     * @param nodesRecords The records of nodes as a vector of tuples.
     * @param filename The filename of the file to write.
     * @return True if the write operation is successful, false otherwise.
     */
bool writeNodesFile(Vector<std::tuple<int, double, double,
                                      std::string, double,
                                      double>> nodesRecords,
                    std::string& filename);

/**
     * @brief Reads the links file.
     * @param fileName The filename of the file.
     * @return The records of links as a vector of tuples.
     *         Each tuple contains the link attributes including the IDs
     *         of the connected nodes, length, speed, etc.
     * @throw std::runtime_error if a runtime error occurs.
     */
Vector<std::tuple<int, int, int, double, int, double,
                  double, int, double, bool, string,
                  string, double> > readLinksFile(const std::string& fileName);

/**
     * @brief Writes the links file.
     * @param linksRecords The records of links as a vector of tuples.
     * @param filename The filename of the file to write.
     * @return True if the write operation is successful, false otherwise.
     */
bool writeLinksFile(Vector<std::tuple<int, int, int, double, int,
                                      double, double, int, double,
                                      bool, string, string,
                                      double> > linksRecords,
                    std::string& filename);

/**
     * @brief Generates NetNode objects from the nodes records.
     * @param nodesRecords The records of nodes as a vector of tuples.
     * @return The generated NetNode objects as a vector.
     */
Vector<std::shared_ptr<NetNode>> generateNodes(
    Vector<std::tuple<int, double, double,
                      std::string, double, double>> nodesRecords);

/**
     * @brief Generates NetLink objects from the nodes and links records.
     * @param theFileNodes The NetNode objects generated from the nodes records.
     * @param linksRecords The records of links as a vector of tuples.
     * @return The generated NetLink objects as a vector.
     */
Vector<std::shared_ptr<NetLink>> generateLinks(Vector<std::shared_ptr<NetNode>> theFileNodes,
    Vector<std::tuple<int, int, int, double, int, double, double, int, double, bool, string, string, double> > linksRecords);

/**
     * @brief Gets the NetNode object with the given user identifier.
     * @param theFileNodes The NetNode objects generated from the nodes records.
     * @param oldID The user identifier of the NetNode to retrieve.
     * @return The NetNode object with the specified user identifier, or
     *         nullptr if not found.
     * @throw std::runtime_error if a runtime error occurs.
     */
std::shared_ptr<NetNode> getSimulatorNodeByUserID(
    Vector<std::shared_ptr<NetNode>> theFileNodes, int oldID);

} // namespace ReadWriteNetwork

#endif // NeTrainSim_ReadWriteNetwork_h
