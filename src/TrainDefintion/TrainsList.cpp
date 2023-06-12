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

// This function readTrainsFile takes a string fileName as the file to read trains from.
// The function returns the trains as objects
Vector<std::tuple<std::string, Vector<int>, double, double,
                  Vector<std::tuple<double, double, double, double, double, double, int, int>>,
                  Vector<std::tuple<double, double, double, double, double, int, int>>,
                  bool>> TrainsList::readTrainsFile(const std::string& fileName) {
    // define the trains vector
    Vector<std::tuple<double, double, double, double, double, double, int, int>> locomotivesRecords;
    Vector<std::tuple<double, double, double, double, double, int, int>> carsRecords;
    Vector<std::tuple<std::string, Vector<int>, double, double,
                      Vector<std::tuple<double, double, double, double, double, double, int, int>>,
                      Vector<std::tuple<double, double, double, double, double, int, int>>, bool>> trainsRecords;

    // open the file of trains definitions
    std::ifstream file1(fileName);
    // check if the file exists
    if (!file1.good()) {
        throw std::runtime_error(std::string("Error: ") +
                                 std::to_string(static_cast<int>(Error::trainsFileDoesNotExist)) +
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
        throw std::runtime_error("Error: " + std::to_string(static_cast<int>(Error::trainsFileEmpty)) +
                                 "\nTrains file " + fileName + " is empty!");
    }



    // loop over the lines/trains that we have
    try {
        for (int i = 2; i < lines.size(); ++i) {
            // declare the locomotives, cars vectors for each train
            std::vector<std::vector<std::string>> trainsCharacteristics;
            std::vector<std::string> lv;
            std::string line = lines[i];

            //split string by tab
            std::regex pattern(R"(\t+)");
            std::sregex_token_iterator iter(line.begin(), line.end(), pattern, -1);
            std::sregex_token_iterator end;
            for (; iter != end; ++iter) {
                lv.push_back(*iter);
            }

            if (lv.size() != 9) {
                throw std::runtime_error( "Error: " + std::to_string(static_cast<int>(Error::wrongTrainsFileStructure)) +
                                         "trains file has a wrong structure\n");
            }
            // if the line has values, continue
            if (!lv.empty()) {
                // define the locomotives
                std::string locomotivesDef = lv[7];
                std::vector<std::string> p = Utils::split(locomotivesDef, ';');
                for (const auto& p_ : p) {
                    std::vector<std::string> loc = Utils::split(p_, ',');
                    // if we have multiple locomotive types, add them all
                    for (int ii = 0; ii < std::stoi(loc[0]); ++ii) {
                        locomotivesRecords.push_back(std::make_tuple(
                                                                     std::stod(loc[1]), std::stod(lv[3]),
                                                                     std::stod(loc[5]), std::stod(loc[3]),
                                                                     std::stod(loc[4]), std::stod(loc[6]),
                                                                     std::stoi(loc[2]), std::stoi(loc[7])));

                    }
                }

                // define the cars
                std::string carsDef = lv[8];
                std::vector<std::string> cp = Utils::split(carsDef, ';');
                for (const auto& cp_ : cp) {
                    std::vector<std::string> c = Utils::split(cp_, ',');
                    for (int ii = 0; ii < std::stoi(c[0]); ii++) {
                        int cType;

                        if (c.size() > 7) {
                            cType = std::stoi(c[7]);
                        }
                        else {
                            cType = 0;
                        }
                        carsRecords.push_back(std::make_tuple(
                                                              std::stod(c[4]), std::stod(c[2]),
                                                              std::stod(c[3]), std::stod(c[6]),
                                                              std::stod(c[5]), std::stoi(c[1]), cType));

                    }

                }

            }
            Vector<int> trainPath = Utils::splitStringToIntVector(lv[1]);
            trainsRecords.push_back(std::make_tuple(
                                                    lv[0], trainPath, std::stod(lv[2]),
                                                    std::stod(lv[6]), locomotivesRecords, carsRecords, false));
        }
    }
    catch(const std::exception &e){
        std::runtime_error("Error " +
                                std::to_string(static_cast<int>(Error::otherTrainsFileErrors)) + "\n" +
                                std::string(e.what()));
    }
    return trainsRecords;
}

Vector<std::shared_ptr<Train>> TrainsList::generateTrains(Vector<std::tuple<string, Vector<int>, double, double, Vector<std::tuple<double, double, double, double, double, double, int, int> >, Vector<std::tuple<double, double, double, double, double, int, int> >, bool> > &trainRecords) {
    Vector<std::shared_ptr<Train>> trains;
    int trainID = 0;
    for (auto & trainRecord: trainRecords) {
        Vector<std::shared_ptr<Locomotive>> locomotives;
        Vector<std::shared_ptr<Car>> cars;

        auto locoRecords = std::get<4>(trainRecord);
        for (auto& locoRecord: locoRecords) {
            Locomotive loco = Locomotive(std::get<0>(locoRecord), std::get<1>(locoRecord), std::get<2>(locoRecord),
                                         std::get<3>(locoRecord), std::get<4>(locoRecord), std::get<5>(locoRecord),
                                         std::get<6>(locoRecord), std::get<7>(locoRecord));
            locomotives.push_back(std::make_shared<Locomotive>(loco));
        }
        auto carRecords = std::get<5>(trainRecord);
        for (auto& carRecord: carRecords) {
            Car car = Car(std::get<0>(carRecord), std::get<1>(carRecord), std::get<2>(carRecord),
                          std::get<3>(carRecord), std::get<4>(carRecord), std::get<5>(carRecord),
                          std::get<6>(carRecord));
            cars.push_back(std::make_shared<Car>(car));
        }

        Train train = Train(trainID, std::get<0>(trainRecord), std::get<1>(trainRecord), std::get<2>(trainRecord),
                            std::get<3>(trainRecord), locomotives, cars, std::get<6>(trainRecord));

        trains.push_back(std::make_shared<Train>(train));
        trainID++;

    }
    return trains;
}

Vector<std::shared_ptr<Train>> TrainsList::ReadAndGenerateTrains(const std::string& fileName) {
    auto out = TrainsList::readTrainsFile(fileName);
    return TrainsList::generateTrains(out);
}

bool TrainsList::writeTrainsFile(Vector<std::tuple<string, Vector<int>, double, double,
                                       Vector<std::tuple<double, double, double, double, double, double, int, int> >,
                                       Vector<std::tuple<double, double, double, double, double, int, int> >,
                                       bool>> trains, const std::string& fileName) {

    std::stringstream lines;

    for (auto& record: trains) {
        lines << std::get<0>(record) << "\t";
        lines << std::get<1>(record).toNotFormattedString() << "\t";
        lines << std::get<2>(record) << "\t";
        lines << std::get<3>(record) << "\t";

        // write locos
        std::size_t locosSize = std::get<4>(record).size();
        for (int i = 0; i < locosSize; i++) {
            auto& loco = std::get<4>(record)[i];
            lines << Utils::convertTupleToStringStream(loco, 0, ",").str() << (i == (locosSize -1) ? ";": "");
        }
        lines << "\t";

        // write cars
        std::size_t carsSize = std::get<5>(record).size();
        for (int i = 0; i < carsSize; i++) {
            auto& car = std::get<5>(record)[i];
            lines << Utils::convertTupleToStringStream(car, 0, ",").str() << (i == (carsSize -1) ? ";": "");
        }
    }

    return Utils::writeToFile(lines, fileName);
}





