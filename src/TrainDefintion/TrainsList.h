#ifndef NeTrainSim_trainsList_h
#define NeTrainSim_trainsList_h



#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "../util/Vector.h"
#include <regex>
#include "Train.h"

namespace TrainsList {

    /**
     * Reads trains file
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	fileName	Filename of the file.
     *
     * @returns	The trains file.
     */
    Vector<std::tuple<string, Vector<int>, double, double,
                      Vector<std::tuple<double, double, double, double, double, double, int, int> >,
                      Vector<std::tuple<double, double, double, double, double, int, int> >,
                      bool> > readTrainsFile(const std::string& fileName);

    Vector<std::shared_ptr<Train>> generateTrains(Vector<std::tuple<string, Vector<int>, double, double, Vector<std::tuple<double, double, double, double, double, double, int, int> >, Vector<std::tuple<double, double, double, double, double, int, int> >, bool> > &trainRecords);

    Vector<std::shared_ptr<Train>> ReadAndGenerateTrains(const std::string& fileName);

    bool writeTrainsFile(Vector<std::tuple<string, Vector<int>, double, double,
                                           Vector<std::tuple<double, double, double, double, double, double, int, int> >,
                                           Vector<std::tuple<double, double, double, double, double, int, int> >,
                                           bool>> trains, const std::string& fileName);


};

#endif // !NeTrainSim_trainsList_h
