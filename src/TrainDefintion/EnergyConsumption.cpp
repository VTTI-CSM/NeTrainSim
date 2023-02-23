#include "TrainTypes.h"
#include "EnergyConsumption.h"
#include "../util/Utils.h"
#include "../dependencies/tinyxml2/tinyxml2.h"



using namespace tinyxml2;

//std::string EC::getEfficiencyValue(const std::string& tagname, const std::vector<std::string>& attrname, 
//    const std::vector<std::string>& attrvalue) {
//    XMLDocument doc;
//    if (doc.LoadFile(EC::DefaultECFileName) != XML_SUCCESS)
//    {
//        std::cerr << "Error loading file: " << EC::DefaultECFileName << std::endl;
//        return "";
//    }
//
//    XMLElement* root = doc.RootElement();
//    if (!root)
//    {
//        std::cerr << "No root element found." << std::endl;
//        return "";
//    }
//
//    for (XMLElement* elem = root->FirstChildElement(tagname.c_str()); elem != nullptr; elem = elem->NextSiblingElement(tagname.c_str()))
//    {
//        bool all_conditions_met = true;
//        for (size_t i = 0; i < attrname.size(); i++)
//        {
//            const char* attr = elem->Attribute(attrname[i].c_str());
//            if (!attr || std::string(attr) != attrvalue[i])
//            {
//                all_conditions_met = false;
//                break;
//            }
//        }
//
//        if (all_conditions_met)
//        {
//            const char* value = elem->Attribute("values");
//            if (value)
//            {
//                return std::string(value);
//            }
//        }
//    }
//
//    std::cerr << "No matching entry found." << std::endl;
//    return "";
//}

namespace EC {
    Map<TrainTypes::PowerType, Vector<double>> locomotiveBusToTankEff;
    Map<TrainTypes::PowerType, Vector<double>> locomotiveWheelToDCBusEff;


    // function to get all attributes and values of a specific tag in an XML file
    std::vector<std::map<std::string, std::string>> EC::readXML(const char fileName[], const std::string& tagname) {
        XMLDocument doc;
        std::vector<std::map<std::string, std::string>> result;

        if (doc.LoadFile(fileName) != XML_SUCCESS)
        {
            throw std::runtime_error("Error loading Energy Consumption Database!");
            exit(1);
        }
        XMLElement* root = doc.RootElement();
        if (!root)
        {
            throw std::runtime_error("No root element found!");
            exit(1);
        }


        for (tinyxml2::XMLElement* elem = root->FirstChildElement(tagname.c_str());
            elem != NULL; elem = elem->NextSiblingElement(tagname.c_str())) {

            std::map<std::string, std::string> elemMap;

            for (const tinyxml2::XMLAttribute* attr = elem->FirstAttribute(); attr != NULL; attr = attr->Next()) {
                elemMap[attr->Name()] = attr->Value();
            }

            //elemMap["value"] = elem->GetText();

            result.push_back(elemMap);
        }

        return result;

        //TrainTypes::strToPowerType(dd)
    }

    void EC::extractEnergyMapping(std::vector<std::map<std::string, std::string>> data) {
        for (auto& elementMap : data) {
            if (elementMap["type"] == "Wheel To Bus") {
                if (elementMap["vehicle"] == "locomotive") {
                    TrainTypes::PowerType locType = TrainTypes::strToPowerType(elementMap["motor"]);
                    auto dd = Utils::splitStringToDoubleVector(elementMap["values"]);
                    locomotiveWheelToDCBusEff[locType] = dd;
                }
                //else if (elementMap["vehicle"] == "RailCar") {
                //    TrainTypes::CarType carType = TrainTypes::strtoCarType(elementMap["motor"]);
                //    EC::carWheelToDCBusEff[carType] = Utils::splitStringToDoubleVector(elementMap["values"]);
                //}
            }
            else if (elementMap["type"] == "Bus To Tank") {
                if (elementMap["vehicle"] == "locomotive") {
                    TrainTypes::PowerType locType = TrainTypes::strToPowerType(elementMap["motor"]);
                    locomotiveBusToTankEff[locType] = Utils::splitStringToDoubleVector(elementMap["values"]);
                }
                //else if (elementMap["vehicle"] == "RailCar") {
                //    TrainTypes::CarType carType = TrainTypes::strtoCarType(elementMap["motor"]);
                //    EC::carBusToTankEff[carType] = Utils::splitStringToDoubleVector(elementMap["values"]);
                //}
            }
        }
    }

    void EC::readDefaultValuesFromConfigFile(const char configurationFile[], string tagName)
    {
        extractEnergyMapping(EC::readXML(configurationFile, tagName));
    }




    double EC::getDriveLineEff(int notchNumberIndex, TrainTypes::PowerType powerType) {
        if (EC::locomotiveWheelToDCBusEff[powerType].empty() ||
            EC::locomotiveBusToTankEff[powerType].empty()) {

            EC::readDefaultValuesFromConfigFile();
        }

        int busIndx = notchNumberIndex;
        int wheelIndx = notchNumberIndex;
        if (notchNumberIndex > (EC::locomotiveBusToTankEff[powerType].size() - 1)) {
            busIndx = EC::locomotiveBusToTankEff[powerType].size() - 1;
        }
        if (notchNumberIndex > (EC::locomotiveWheelToDCBusEff[powerType].size() - 1)) {
            wheelIndx = EC::locomotiveWheelToDCBusEff[powerType].size() - 1;
        }
        double busToTankE = EC::locomotiveBusToTankEff[powerType].at(busIndx);
        double wheelToDC = EC::locomotiveWheelToDCBusEff[powerType].at(wheelIndx);
        return busToTankE * wheelToDC;
    }

}
