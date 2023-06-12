#ifndef NeTrainSim_trainsList_h
#define NeTrainSim_trainsList_h

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "../util/vector.h"
#include <regex>
#include "train.h"

namespace TrainsList {

/**
     * @file trainsList.h
     * @brief This file declares functions related to managing a list of trains.
     *        The functions provide functionality for reading and writing train records
     *        from/to a file, generating trains from train records, and other operations.
     *        The functions are part of the TrainsList namespace and can be accessed accordingly.
     *        Note: The implementation of the functions is not provided in this declaration file.
     *              They should be implemented separately in a corresponding source file.
     * @author Ahmed Aredah
     * @date 3/20/2023
     */

/**
     * Reads a trains file and returns the train records.
     *
     * @param fileName The filename of the trains file to read.
     * @returns A Vector of train records.
     */
Vector<std::tuple<string, Vector<int>, double, double,
                  Vector<std::tuple<double, double, double, double, double, double, int, int>>,
                  Vector<std::tuple<double, double, double, double, double, int, int>>,
                  bool>> readTrainsFile(const std::string& fileName);

/**
     * Generates a Vector of trains from the given train records.
     *
     * @param trainRecords The train records to generate trains from.
     * @returns A Vector of shared pointers to Train objects.
     */
Vector<std::shared_ptr<Train>> generateTrains(Vector<std::tuple<string, Vector<int>, double, double,
                                                                Vector<std::tuple<double, double, double, double, double, double, int, int>>,
                                                                Vector<std::tuple<double, double, double, double, double, int, int>>,
                                                                bool>>& trainRecords);

/**
     * Reads a trains file, generates trains from the train records, and returns the generated trains.
     *
     * @param fileName The filename of the trains file to read.
     * @returns A Vector of shared pointers to Train objects.
     */
Vector<std::shared_ptr<Train>> ReadAndGenerateTrains(const std::string& fileName);

/**
     * Writes the train records to a trains file.
     *
     * @param trains The train records to write.
     * @param fileName The filename of the trains file to write to.
     * @returns True if the write operation is successful, false otherwise.
     */
    bool writeTrainsFile(Vector<std::tuple<string, Vector<int>, double, double,
                                           Vector<std::tuple<double, double, double, double, double, double, int, int> >,
                                           Vector<std::tuple<double, double, double, double, double, int, int> >,
                                           bool>> trains, const std::string& fileName);



};

#endif // !NeTrainSim_trainsList_h
