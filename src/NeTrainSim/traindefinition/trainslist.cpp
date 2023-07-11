#include "trainslist.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include "locomotive.h"
#include "car.h"
#include "train.h"
#include "traintypes.h"
#include "../util/error.h"
#include "../util/utils.h"

// This function readTrainsFile takes a string fileName as the file
// to read trains from.
// The function returns the trains as objects
Vector<std::tuple<std::string, Vector<int>, double, double,
                  Vector<std::tuple<
                      int, double, double,
                      int, double, double,
                      double, double, int>>,
                  Vector<std::tuple<int, int, double, double,
                                    double, double,
                                    double, int>>,
                  bool>> TrainsList::readTrainsFile(
    const std::string& fileName)
{

    // define the trains vector
    Vector<std::tuple<std::string, Vector<int>, double, double,
                      Vector<std::tuple<
                          int, double, double,
                          int, double, double,
                          double, double, int>>,
                      Vector<std::tuple<int, int, double, double,
                                        double, double,
                                        double, int>>,
                      bool>> trainsRecords;

    // open the file of trains definitions
    std::ifstream file1(fileName);
    // check if the file exists
    if (!file1.good()) {
        throw std::runtime_error(std::string("Error: ") +
                                 std::to_string(
                                     static_cast<int>(
                                     Error::trainsFileDoesNotExist)) +
                                 "\nTrains file does not exist");
    }
    // define and read the lines
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file1, line)) {
        lines.push_back(line);
    }
    // close the trains defintions file
    file1.close();

    // if the file has trains continue, else stop and throw error
    if (lines.size() == 0) {
        throw std::runtime_error("Error: " + std::to_string(
                                     static_cast<int>(
                                     Error::trainsFileEmpty)) +
                                 "\nTrains file " + fileName + " is empty!");
    }



    // loop over the lines/trains that we have
    try {
        for (int i = 2; i < lines.size(); ++i) {
            Vector<std::tuple<int,    // 0: count
                              double, // 1: power
                              double, // transmission eff
                              int,    // 2: axles
                              double, // 3: air k
                              double, // 4: area
                              double, // 5: length
                              double, // 6: weight
                              int>    // 7: type
                   > locomotivesRecords;

            Vector<std::tuple<int,    // 0: count
                              int,    // 1: axles
                              double, // 2: air k
                              double, // 3: area
                              double, // 4: length
                              double, // 5: full weight
                              double, // 6: empty weight
                              int >   // 7: type
                   > carsRecords;

            // declare the locomotives, cars vectors for each train
            //std::vector<std::vector<std::string>> trainsCharacteristics;
            std::vector<std::string> lv;
            std::string line = lines[i];

            //split string by tab
            std::regex pattern(R"(\t+)");
            std::sregex_token_iterator iter(line.begin(), line.end(),
                                            pattern, -1);
            std::sregex_token_iterator end;
            for (; iter != end; ++iter) {
                lv.push_back(*iter);
            }

            if (lv.size() != 6) {
                throw std::runtime_error( "Error: " +
                                         std::to_string(
                                             static_cast<int>(
                                             Error::wrongTrainsFileStructure)) +
                                         "trains file has a wrong structure\n");
            }
            // if the line has values, continue
            if (!lv.empty()) {
                // define the locomotives
                std::string locomotivesDef = lv[4];
                std::vector<std::string> p = Utils::split(locomotivesDef, ';');
                for (const auto& p_ : p) {
                    std::vector<std::string> loc = Utils::split(p_, ',');
                    // if we have multiple locomotive types, add them all
                        locomotivesRecords.push_back(
                            std::make_tuple(
                                std::stoi(loc[0]),      // count
                                std::stod(loc[1]),      // power
                                std::stod(loc[2]),      // transimission eff
                                std::stoi(lv[3]),       // axles
                                std::stod(loc[4]),      // air k
                                std::stod(loc[5]),      // area
                                std::stod(loc[6]),      // length
                                std::stod(loc[7]),      // weight
                                std::stoi(loc[8])));    // type
                }

                // define the cars
                std::string carsDef = lv[5];
                std::vector<std::string> cp = Utils::split(carsDef, ';');
                for (const auto& cp_ : cp) {
                    std::vector<std::string> c = Utils::split(cp_, ',');
                    int cType;

                    if (c.size() > 7) {
                        cType = std::stoi(c[7]);
                    }
                    else {
                        cType = 0;
                    }
                    carsRecords.push_back(
                        std::make_tuple(
                            std::stoi(c[0]),    // 0: count
                            std::stoi(c[1]),    // 1: axles
                            std::stod(c[2]),    // 2: air k
                            std::stod(c[3]),    // 3: area
                            std::stod(c[4]),    // 4: length
                            std::stod(c[5]),    // 5: full weight
                            std::stod(c[6]),    // 6: empty weight
                            cType               // 7: type
                            )
                        );

                }

            }
            // get a vector of node IDs
            Vector<int> trainPath = Utils::splitStringToIntVector(lv[1]);

            // create trains vector
            trainsRecords.push_back(
                std::make_tuple(
                    lv[0],              // user id
                    trainPath,          // path
                    std::stod(lv[2]),   // time
                    std::stod(lv[3]),   // friction coef
                    locomotivesRecords, // locomotives
                    carsRecords,        // cars
                    false));            // no optimization
        }
    }
    catch(const std::exception &e){
        std::runtime_error("Error " +
                                std::to_string(
                               static_cast<int>(
                               Error::otherTrainsFileErrors)) + "\n" +
                                std::string(e.what()));
    }
    return trainsRecords;
}

Vector<std::shared_ptr<Train>> TrainsList::generateTrains(
    Vector<std::tuple<std::string, Vector<int>, double, double,
                      Vector<std::tuple<
                          int, double, double,
                          int, double, double,
                          double, double, int>>,
                      Vector<std::tuple<int, int, double, double,
                                        double, double,
                                        double, int>>,
                      bool>> &trainRecords)
{
    // a vector of trains
    Vector<std::shared_ptr<Train>> trains;
    int trainID = 0; // train id starting from 0

    // iterate over all the records
    for (auto & trainRecord: trainRecords) {
        // vectors for locos and cars
        Vector<std::shared_ptr<Locomotive>> locomotives;
        Vector<std::shared_ptr<Car>> cars;

        // get loco values
        auto locoRecords = std::get<4>(trainRecord);
        for (auto& locoRecord: locoRecords) {
            for (int i = 0; i < std::get<0>(locoRecord); i++) {
                Locomotive loco =
                    Locomotive(
                        std::get<1>(locoRecord), // Power
                        std::get<2>(locoRecord), // trans eff
                        std::get<6>(locoRecord), // length
                        std::get<4>(locoRecord), // drag
                        std::get<5>(locoRecord), // area
                        std::get<7>(locoRecord), // weight
                        std::get<3>(locoRecord), // axles
                        std::get<8>(locoRecord)); //power type
                locomotives.push_back(std::make_shared<Locomotive>(loco));
            }
        }

        // get car values
        auto carRecords = std::get<5>(trainRecord);
        for (auto& carRecord: carRecords) {
            for (int i = 0; i < std::get<0>(carRecord); i++) {
                Car car = Car(std::get<4>(carRecord), //length
                              std::get<2>(carRecord), //drag
                              std::get<3>(carRecord), //area
                              std::get<6>(carRecord), //empty weight
                              std::get<5>(carRecord), //full weight
                              std::get<1>(carRecord), //axles
                              std::get<7>(carRecord));//type
                cars.push_back(std::make_shared<Car>(car));
            }

        }

        // create a new train and add it to the trains list
        trains.push_back(std::make_shared<Train>(
            trainID,
            std::get<0>(trainRecord), // user id
            std::get<1>(trainRecord), // path
            std::get<2>(trainRecord), // time
            std::get<3>(trainRecord), // friction coef
            locomotives,              // locomotives
            cars,                     // cars
            std::get<6>(trainRecord)  // no optimization
            ));

        trainID++;

    }
    return trains;
}

Vector<std::shared_ptr<Train>> TrainsList::ReadAndGenerateTrains(
    const std::string& fileName)
{
    auto out = TrainsList::readTrainsFile(fileName);
    return TrainsList::generateTrains(out);
}

bool TrainsList::writeTrainsFile(
    Vector<std::tuple<std::string, Vector<int>, double, double,
                      Vector<std::tuple<
                          int, double, double,
                          int, double, double,
                          double, double, int>>,
                      Vector<std::tuple<int, int, double, double,
                                        double, double,
                                        double, int>>,
                      bool>> trains, const std::string& fileName) {

    std::stringstream lines;
    lines << "File is created using NeTrainSim GUI\n";
    lines << trains.size() << "\n";

    for (auto& record: trains) {
        std::size_t locosSize = std::get<4>(record).size();
        std::size_t carsSize = std::get<5>(record).size();

        lines << std::get<0>(record) << "\t"; //ID
        lines << std::get<1>(record).toNotFormattedString() << "\t"; //Path
        lines << std::get<2>(record) << "\t"; // time
        lines << std::get<3>(record) << "\t"; //friction coef

        // write locos
        for (int i = 0; i < locosSize; i++) {
            auto& loco = std::get<4>(record)[i];
            lines << Utils::convertTupleToStringStream(loco, 0, ",").str() <<
                (i != (locosSize -1) ? ";": "");
        }

        lines << "\t";

        // write cars
        for (int i = 0; i < carsSize; i++) {
            auto& car = std::get<5>(record)[i];
            lines << Utils::convertTupleToStringStream(car, 0, ",").str() <<
                (i != (carsSize -1) ? ";": "");
        }
    }

    return Utils::writeToFile(lines, fileName);
}





