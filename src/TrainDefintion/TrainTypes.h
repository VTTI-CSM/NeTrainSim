
#ifndef carLocTypes_enum
#define carLocTypes_enum

#include <string>
#include <stdexcept>
#include "../util/Vector.h"
#include <map>

namespace TrainTypes {

    static const int carTypeN = 4;

    enum class _CarType {
        cargo,
        dieselTender,
        batteryTender,
        hydrogenTender
    };

    using CarType = _CarType;

    static const std::string carTypeStrings[] = {
    "Cargo Car",
    "Diesel Tender",
    "Battery Tender",
    "Hydrogen Tender"
    };

    static const CarType carTypeArray[] = {
    CarType::cargo,
    CarType::dieselTender,
    CarType::batteryTender,
    CarType::hydrogenTender
    };

    inline Vector<CarType> getCarTypeVector() {
        Vector<CarType> carTypeVector;
        for (auto c : carTypeArray) {
            carTypeVector.push_back(c);
        }
        return carTypeVector;
    }

    static const std::map<std::string, CarType> carTypeMap = {
    {"Cargo Car", CarType::cargo},
    {"Fuel Tank", CarType::dieselTender},
    {"Battery Tender", CarType::batteryTender},
    {"Hydrogen Tender", CarType::hydrogenTender}
    };


    inline std::string carTypeToStr(CarType type) {
        return carTypeStrings[static_cast<int>(type)];
    }


    inline CarType strtoCarType(const std::string& cartype) {
        auto it = carTypeMap.find(cartype);
        if (it == carTypeMap.end()) {
            throw std::invalid_argument("invalid car type");
        }
        return it->second;
    }



    inline CarType itoCarType(int cartype) {
        if (cartype < 0 || cartype >= carTypeN) {
            throw std::invalid_argument("invalid car type");
        }
        return carTypeArray[cartype];
    }

    //*************************************************************************************
    // Locomotives power types
    // ************************************************************************************
    static const int powerTypeN = 4;
    enum class _PowerType {
        diesel,
        electric,
        hydrogen,
        hydrogenHybrid
	
    };

    using PowerType = _PowerType;

    static const std::string powerTypeStrings[] = {
    "Diesel Locomotive",
    "Electric Locomotive",
    "Hydrogen Locomotive",
    "Hydrogen-Hyprid Locomotive"
    };

    static const PowerType powerTypeArray[] = {
    PowerType::diesel,
    PowerType::electric,
    PowerType::hydrogen,
    PowerType::hydrogenHybrid
    };

    static const std::map<std::string, PowerType> powerTypeMap = {
    {"Diesel Locomotive", PowerType::diesel},
    {"Electric Locomotive", PowerType::electric},
    {"Hydrogen Locomotive", PowerType::hydrogen},
    {"Hydrogen-Hyprid Locomotive", PowerType::hydrogenHybrid}
    };

    static const std::map<PowerType, CarType> powerToCarMap = {
        {PowerType::diesel, CarType::dieselTender},
        {PowerType::electric, CarType::batteryTender},
        {PowerType::hydrogen, CarType::hydrogenTender},
        {PowerType::hydrogenHybrid, CarType::hydrogenTender}
    };

    inline std::string PowerTypeToStr(PowerType type) {
        return powerTypeStrings[static_cast<int>(type)];
    }

    inline PowerType strToPowerType(std::string powertype) {
        auto it = powerTypeMap.find(powertype);
        if (it == powerTypeMap.end()) {
            throw std::invalid_argument("invalid car type");
        }
        return it->second;
    }

    inline PowerType iToPowerType(int powertype) {
        if (powertype < 0 || powertype >= powerTypeN) {
            throw std::invalid_argument("invalid car type");
        }
        return powerTypeArray[powertype];
    }

    inline std::ostream& operator<<(std::ostream& ss, const PowerType& obj) {
        ss << powerTypeStrings[static_cast<int>(obj)];
        return ss;
    }

}

#endif // !carLocTypes_enum