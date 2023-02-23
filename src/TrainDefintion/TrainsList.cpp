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
#include "../util/Utils.h"

    // This function readTrainsFile takes a string fileName as the file to read trains from.
    // The function returns the trains as objects
    Vector<std::shared_ptr<Train>> TrainsList::readTrainsFile(const std::string& fileName) {
        Vector<std::shared_ptr<Train>> trains;
        std::ifstream file1(fileName);
        if (!file1.good()) {
            std::cerr << "Trains file does not exist!" << std::endl;
            throw std::runtime_error("Trains file does not exist");
        }
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file1, line)) {
            lines.push_back(line);
        }
        file1.close();

        if (lines.size() == 0) {
            std::cerr << "Trains file is empty!" << std::endl;
            exit(0);
        }
        std::vector<std::vector<std::string>> trainsCharacteristics;
        Vector<std::shared_ptr<Locomotive>> locomotives;
        Vector<std::shared_ptr<Car>> cars;
        double maxSpeed = 100.0 / 3.0;
        //try {


            for (int i = 2; i < lines.size(); ++i) {
                std::vector<std::string> lv;
                std::string line = lines[i];
                //splitString by tab
                std::regex pattern(R"(\t+)");
                std::sregex_token_iterator iter(line.begin(), line.end(), pattern, -1);
                std::sregex_token_iterator end;
                for (; iter != end; ++iter) {
                    lv.push_back(*iter);
                }
                if (!lv.empty()) {
                    std::string locomotivesDef = lv[7];
                    std::vector<std::string> p = Utils::split(locomotivesDef, ';');
                    for (const auto& p_ : p) {
                        std::vector<std::string> loc = Utils::split(p_, ',');
                        for (int ii = 0; ii < std::stoi(loc[0]); ++ii) {
                            Locomotive loco = Locomotive(std::stod(loc[1]), std::stod(lv[3]), std::stod(loc[5]),
                                std::stod(loc[3]), std::stod(loc[4]), std::stod(loc[6]), std::stoi(loc[2]),
                                std::stoi(loc[7]));
                            locomotives.push_back(std::make_shared<Locomotive>(loco));
                        }
                    }

                    std::string carsDef = lv[8];
                    std::vector<std::string> cp = Utils::split(carsDef, ';');
                    for (const auto& cp_ : cp) {
                        std::vector<std::string> c = Utils::split(cp_, ',');
                        for (int ii = 0; ii < std::stoi(c[0]); ii++) {
                            int cType;

                            if (c.size() > 7) {
                                cType = static_cast<int>(TrainTypes::strtoCarType(Utils::trim(c[7])));
                            }
                            else {
                                cType = static_cast<int>(TrainTypes::CarType::cargo);
                            }
                            Car car = Car(std::stof(c[4]), std::stof(c[2]), std::stof(c[3]),
                                std::stof(c[6]), std::stof(c[5]), std::stoi(c[1]),
                                0);
                            cars.push_back(std::make_shared<Car>(car));
                        }

                    }

                }
                Vector<int> trainPath = Utils::splitStringToIntVector(lv[1]);
                Train train = Train(lv[0], trainPath, std::stod(lv[2]),
                    std::stod(lv[6]), locomotives, cars, false);

                trains.push_back(std::make_shared<Train>(train));
            }
        //}
        //catch(std::exception &e){
        //    std::cerr << "Exception caught: " << e.what() << '\n';
        //    exit(1);
        //}
        return trains;
    }



