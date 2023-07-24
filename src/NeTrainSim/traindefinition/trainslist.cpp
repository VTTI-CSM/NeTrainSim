#include "trainslist.h"
#include <any>
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
Vector<Map<std::string, std::any>> TrainsList::readTrainsFile(
    const std::string& fileName)
{

    Vector<Map<std::string, std::any>> trainsRecords;

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
            Vector<Map<std::string, std::string>> locomotiveRecords;
            Vector<Map<std::string, std::string>> carRecords;

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
                std::vector<std::string> p = Utils::split(locomotivesDef,
                                                          ';',
                                                          true);
                for (const auto& p_ : p) {
                    std::vector<std::string> loc = Utils::split(p_,
                                                                ',',
                                                                true);
                    if (loc.size() != locomotiveFieldsOrder.size()) {
                        throw std::runtime_error(
                            "Wrong Trains File Strucuture!");
                    }
                    // if we have multiple locomotive types, add them all
                    Map<std::string, std::string> locomotiveRecord;
                    for (const auto& key: locomotiveFieldsOrder) {
                        int keyIndex = locomotiveFieldsOrder.index(key);
                        locomotiveRecord[key] = loc[keyIndex];
                    }
                    locomotiveRecords.push_back(locomotiveRecord);
                }

                // define the cars
                std::string carsDef = lv[5];
                std::vector<std::string> cp = Utils::split(carsDef,
                                                           ';',
                                                           true);
                for (const auto& cp_ : cp) {
                    std::vector<std::string> c = Utils::split(cp_,
                                                              ',',
                                                              true);
                    if (c.size() < carFieldsOrder.size()) {
                        c.push_back("0");
                    }
                    if (c.size() != carFieldsOrder.size()) {
                        throw std::runtime_error(
                            "Wrong Trains File Strucuture!");
                    }

                    Map<std::string, std::string> carRecord;
                    for (const auto& key: carFieldsOrder) {
                        int keyIndex = carFieldsOrder.index(key);
                        carRecord[key] = c[keyIndex];
                    }
                    carRecords.push_back(carRecord);
                }

            }
            // get a vector of node IDs
            Vector<int> trainPath = Utils::splitStringToIntVector(lv[1]);

            Map<std::string, std::any> trainRecord = {
                {"UserID", lv[0]},
                {"TrainPathOnNodeIDs", trainPath},
                {"LoadTime", std::stod(lv[2])},
                {"FrictionCoef", std::stod(lv[3])},
                {"Locomotives", locomotiveRecords},
                {"Cars", carRecords},
                {"Optimize", false}
            };

            // create trains vector
            trainsRecords.push_back(trainRecord);
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
    Vector<Map<std::string, std::any>> &trainRecords)
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
        auto locoRecords =
            std::any_cast<Vector<Map<std::string,
                                     std::string>>>(trainRecord["Locomotives"]);
        for (auto& locoRecord: locoRecords) {
            for (int i = 0; i < std::stoi(locoRecord["Count"]); i++) {
                Locomotive loco =
                    Locomotive(
                        stod(locoRecord["Power"]), // Power
                        stod(locoRecord["TransmissionEff"]), // trans eff
                        stod(locoRecord["Length"]), // length
                        stod(locoRecord["AirDragCoeff"]), // drag
                        stod(locoRecord["FrontalArea"]), // area
                        stod(locoRecord["GrossWeight"]), // weight
                        stoi(locoRecord["NoOfAxles"]), // axles
                        stoi(locoRecord["Type"])); //power type
                locomotives.push_back(std::make_shared<Locomotive>(loco));
            }
        }

        // get car values
        auto carRecords =
            std::any_cast<Vector<Map<std::string,
                                     std::string>>>(trainRecord["Cars"]);
        for (auto& carRecord: carRecords) {
            for (int i = 0; i < std::stoi(carRecord["Count"]); i++) {
                Car car = Car(std::stod(carRecord["Length"]), //length
                              std::stod(carRecord["AirDragCoeff"]), //drag
                              std::stod(carRecord["FrontalArea"]), //area
                              std::stod(carRecord["TareWeight"]), //empty weight
                              std::stod(carRecord["GrossWeight"]), //full weight
                              std::stoi(carRecord["NoOfAxles"]), //axles
                              std::stoi(carRecord["Type"]));//type
                cars.push_back(std::make_shared<Car>(car));
            }
        }

        // create a new train and add it to the trains list
        trains.push_back(std::make_shared<Train>(
            trainID,
            std::any_cast<std::string>(trainRecord["UserID"]), // user id
            std::any_cast<Vector<int>>(trainRecord["TrainPathOnNodeIDs"]),//path
            std::any_cast<double>(trainRecord["LoadTime"]), // time
            std::any_cast<double>(trainRecord["FrictionCoef"]), // friction coef
            locomotives,              // locomotives
            cars,                     // cars
            std::any_cast<bool>(trainRecord["Optimize"])  // no optimization
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
    Vector<Map<std::string, std::any>> trains, const std::string& fileName) {

    std::stringstream lines;
    lines << "File is created using NeTrainSim GUI\n";
    lines << trains.size() << "\n";

    for (auto& record: trains) {
        auto locos = std::any_cast<Vector<Map<std::string,
                                              std::string>>>(
            record["Locomotives"]);
        std::size_t locosSize = locos.size();

        auto cars = std::any_cast<Vector<Map<std::string,
                                             std::string>>>(record["Cars"]);
        std::size_t carsSize = cars.size();

        lines << std::any_cast<std::string>(record["UserID"]) << "\t"; //ID
        lines << std::any_cast<Vector<int>>(
                     record["TrainPathOnNodeIDs"]).toNotFormattedString()
              << "\t"; //Path
        lines << std::any_cast<double>(record["LoadTime"]) << "\t"; // time
        lines << std::any_cast<double>(record["FrictionCoef"]) << "\t"; //friction coef

        // write locos
        for (int i = 0; i < locosSize; i++) {
            auto& loco = locos[i];
            lines << loco.valuesToString(locomotiveFieldsOrder,
                                         Vector<std::string>(),
                                         ",") <<
                (i != (locosSize -1) ? ";": "");
        }

        lines << "\t";

        // write cars
        for (int i = 0; i < carsSize; i++) {
            auto& car = cars[i];
            lines << car.valuesToString(carFieldsOrder,
                                        Vector<std::string>(),
                                        ",") <<
                (i != (carsSize -1) ? ";": "");
        }
    }

    return Utils::writeToFile(lines, fileName);
}





