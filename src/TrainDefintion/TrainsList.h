#ifndef NeTrainSim_trainsList_h
#define NeTrainSim_trainsList_h



#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "../util/Vector.h"
#include <regex>
#include "Train.h"
#include "TrainTypes.h"

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
    Vector<std::shared_ptr<Train>> readTrainsFile(const std::string& fileName);

};

#endif // !NeTrainSim_trainsList_h