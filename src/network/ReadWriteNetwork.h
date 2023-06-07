#include <iostream>
#include <fstream>
#include "src/network/NetLink.h"
#include "src/network/NetNode.h"
#include "src/util/Vector.h"
#include <exception>

namespace ReadWriteNetwork {

    /**
     * Reads nodes file
     * @author	Ahmed
     * @date	2/14/2023
     * @exception	std::runtime_error	Raised when a runtime error condition occurs.
     * @param 	fileName	Filename of the file.
     * @returns	The tuple of IDs as a vector, x coordinates as a vector, y coordinates as a vector,
     *                       description as a vector, x scale, and y scale as doubles.
     */
    Vector<std::tuple<int, double, double, string, double, double> > readNodesFile(const std::string& fileName);

    /**
     * @brief write nodes file
     * @param nodesRecords
     * @return
     */
    bool writeNodesFile(Vector<std::tuple<int, double, double, std::string,
                                          double, double>> nodesRecords, std::string& filename);


    /**
     * Reads links file
     * @author	Ahmed
     * @date	2/14/2023
     * @exception	std::runtime_error	Raised when a runtime error condition occurs.
     * @param 	fileName	Filename of the file.
     * @returns	The links file.
     */
    Vector<std::tuple<int, int, int, double, double, int, double, double, int, double, bool, string, double, double> > readLinksFile(const std::string& fileName);

    /**
     * @brief writeLinksFile
     * @param nodesRecords
     * @param filename
     * @return
     */
    bool writeLinksFile(Vector<std::tuple<int, int, int, double, double, int,
                                          double, double, int, double, bool,
                                          std::string, double, double>> linksRecords, std::string& filename);

    /**
     * @brief readNodesFile
     * @param nodesRecords
     * @return
     */
    Vector<std::shared_ptr<NetNode>> generateNodes(Vector<std::tuple<int, double,
                                                                        double, std::string,
                                                                            double, double>> nodesRecords);

    /**
     * @brief generateLinks
     * @param theFileNodes
     * @param linksRecords
     * @return
     */
    Vector<std::shared_ptr<NetLink>> generateLinks(Vector<std::shared_ptr<NetNode>> theFileNodes,
                                                   Vector<std::tuple<int, int, int, double,
                                                                        double, int, double, double,
                                                                        int, double, bool, std::string,
                                                                            double, double>> linksRecords);


    /**
     * Gets simulator node by user identifier
     * @author	Ahmed
     * @date	2/14/2023
     * @exception	std::runtime_error	Raised when a runtime error condition occurs.
     * @param 	oldID	Identifier for the old.
     * @returns	The simulator node by user identifier.
     */
    std::shared_ptr<NetNode> getSimulatorNodeByUserID(Vector<std::shared_ptr<NetNode>> theFileNodes,
                                                             int oldID);

}

