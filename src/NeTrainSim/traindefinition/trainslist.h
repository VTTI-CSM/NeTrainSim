#ifndef NeTrainSim_trainsList_h
#define NeTrainSim_trainsList_h

#include <any>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "../util/vector.h"
#include <regex>
#include "train.h"

namespace TrainsList {

static Vector<std::string> locomotiveFieldsOrder = {"Count",
                                                    "Power",
                                                    "TransmissionEff",
                                                    "NoOfAxles",
                                                    "AirDragCoeff",
                                                    "FrontalArea",
                                                    "Length",
                                                    "GrossWeight",
                                                    "Type"};
static Vector<std::string> carFieldsOrder = {"Count",
                                             "NoOfAxles",
                                             "AirDragCoeff",
                                             "FrontalArea",
                                             "Length",
                                             "GrossWeight",
                                             "TareWeight",
                                             "Type"};
/**
     * @file trainsList.h
     * @brief This file declares functions related to managing a list of trains.
     *        The functions provide functionality for reading and
     *        writing train records from/to a file, generating trains
     *        from train records, and other operations.
     *        The functions are part of the TrainsList namespace and can
     *        be accessed accordingly.
     *        Note: The implementation of the functions is not provided
     *        in this declaration file.
     *              They should be implemented separately in a corresponding
     *              source file.
     * @author Ahmed Aredah
     * @date 3/20/2023
     */

/**
     * Reads a trains file and returns the train records.
     *
     * @param fileName The filename of the trains file to read.
     * @returns A Vector of train records.
     */
Vector<Map<std::string, std::any>> readTrainsFile(const std::string& fileName);

/**
     * Generates a Vector of trains from the given train records.
     *
     * @param trainRecords The train records to generate trains from.
     * @returns A Vector of shared pointers to Train objects.
     */
Vector<std::shared_ptr<Train>> generateTrains(
    Vector<Map<std::string, std::any>> &trainRecords);

/**
     * Reads a trains file, generates trains from the train records,
     * and returns the generated trains.
     *
     * @param fileName The filename of the trains file to read.
     * @returns A Vector of shared pointers to Train objects.
     */
Vector<std::shared_ptr<Train>> ReadAndGenerateTrains(
    const std::string& fileName);

/**
     * Writes the train records to a trains file.
     *
     * @param trains The train records to write.
     * @param fileName The filename of the trains file to write to.
     * @returns True if the write operation is successful, false otherwise.
     */
bool writeTrainsFile(Vector<Map<std::string, std::any>> trains,
                     const std::string& fileName);



};

#endif // !NeTrainSim_trainsList_h
