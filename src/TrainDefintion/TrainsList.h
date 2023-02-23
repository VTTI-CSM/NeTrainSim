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

    Vector<std::shared_ptr<Train>> readTrainsFile(const std::string& fileName);

};

#endif // !NeTrainSim_trainsList_h