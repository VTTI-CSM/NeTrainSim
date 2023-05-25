#include "TrainsList.h"



#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include "Locomotive.h"
#include "Car.h"
#include "Train.h"
#include "TrainTypes.h"
#include "../util/Error.h"
#include "../util/Utils.h"

    // This function readTrainsFile takes a string fileName as the file to read trains from.
    // The function returns the trains as objects
    Vector<std::shared_ptr<Train>> TrainsList::readTrainsFile(const std::string& fileName) {
        // define the trains vector
        Vector<std::shared_ptr<Train>> trains;
        // open the file of trains definitions
        std::ifstream file1(fileName);
        // check if the file exists
        if (!file1.good()) {
            std::cerr << "Trains file does not exist!" << std::endl;
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
            std::cerr << "Trains file " << fileName << " is empty!" << std::endl;
            exit(static_cast<int>(Error::emptyTrainsFile));
        }

        // loop over the lines/trains that we have
        try {
            for (int i = 2; i < lines.size(); ++i) {
                // declare the locomotives, cars vectors for each train
                std::vector<std::vector<std::string>> trainsCharacteristics;
                Vector<std::shared_ptr<Locomotive>> locomotives;
                Vector<std::shared_ptr<Car>> cars;
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
                    std::cout << "trains file has a wrong structure\n";
                    exit(static_cast<int>(Error::wrongTrainsFileStructure));
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
                            Locomotive loco = Locomotive(std::stod(loc[1]), std::stod(lv[3]), std::stod(loc[5]),
                                std::stod(loc[3]), std::stod(loc[4]), std::stod(loc[6]), std::stoi(loc[2]),
                                std::stoi(loc[7]));
                            locomotives.push_back(std::make_shared<Locomotive>(loco));
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
                            Car car = Car(std::stof(c[4]), std::stof(c[2]), std::stof(c[3]),
                                std::stof(c[6]), std::stof(c[5]), std::stoi(c[1]), cType);
                            cars.push_back(std::make_shared<Car>(car));
                        }

                    }

                }
                Vector<int> trainPath = Utils::splitStringToIntVector(lv[1]);
                Train train = Train(lv[0], trainPath, std::stod(lv[2]),
                    std::stod(lv[6]), locomotives, cars, false);

                trains.push_back(std::make_shared<Train>(train));
            }
        }
        catch(std::exception &e){
            std::cerr << "Exception caught: " << e.what() << '\n';
            exit(static_cast<int>(Error::otherTrainsFileErrors));
        }
        return trains;
    }



