
#ifndef carLocTypes_enum
#define carLocTypes_enum

#include <string>
#include <stdexcept>
#include "../util/Vector.h"
#include <map>

namespace TrainTypes {

    static const int carTypeN = 5;

    enum class _CarType {
        cargo,
        dieselTender,
        batteryTender,
        hydrogenTender,
        biodieselTender
    };

    using CarType = _CarType;

    static const Vector<CarType> carRechargableTechnologies = {
        CarType::batteryTender
    };

    static const Vector<CarType> carNonRechargableTechnologies = {
        CarType::dieselTender,
        CarType::batteryTender,
        CarType::hydrogenTender,
        CarType::biodieselTender
    };

    static const std::string carTypeStrings[] = {
    "Cargo Car",
    "Diesel Tender",
    "Battery Tender",
    "Hydrogen Tender",
    "Biodiesel Tender"
    };

    static const CarType carTypeArray[] = {
    CarType::cargo,
    CarType::dieselTender,
    CarType::batteryTender,
    CarType::hydrogenTender,
    CarType::biodieselTender
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
    {"Hydrogen Tender", CarType::hydrogenTender},
    {"Biodiesel Tender", CarType::biodieselTender}
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
    static const int powerTypeN = 7;
    enum class _PowerType {
        diesel,
        electric,
        biodiesel,
        dieselElectric,
        dieselHybrid,
        hydrogenHybrid,
        biodieselHybrid
    };

    using PowerType = _PowerType;

    // they can be recharged and have a battery
    static const Vector<PowerType> locomotiveRechargableTechnologies = {
        PowerType::electric,
        PowerType::dieselHybrid,
        PowerType::hydrogenHybrid,
        PowerType::biodieselHybrid
    };

    // they cannot be recharge and/or do not have a battery
    static const Vector<PowerType> locomotiveNonRechargableTechnologies = {
        PowerType::diesel,
        PowerType::biodiesel,
        PowerType::dieselElectric  // it does not have a battery to store energy in
    };

    static const Vector<PowerType> locomotiveTankOnly = {
        PowerType::diesel,
        PowerType::biodiesel
    };

    static const Vector<PowerType> locomotiveBatteryOnly = {
        PowerType::electric
    };

    // they have two types
    static const Vector<PowerType> locomotiveHybrid = {
        PowerType::dieselElectric,
        PowerType::dieselHybrid,
        PowerType::hydrogenHybrid,
        PowerType::biodieselHybrid
    };

    static const std::string powerTypeStrings[] = {
    "Diesel Locomotive",
    "Electric Locomotive",
    "Biodiesel Locomotive",
    "Diesel-Electric Locomotive",
    "Diesel-Hyprid Locomotive",
    "Hydrogen-Hyprid Locomotive",
    "Biodiesel-Hyprid Locomotive"
    };

    static const PowerType powerTypeArray[] = {
    PowerType::diesel,
    PowerType::electric,
    PowerType::biodiesel,
    PowerType::dieselElectric,   // all locomotives now are diesel
    PowerType::dieselHybrid,
    PowerType::hydrogenHybrid,
    PowerType::biodieselHybrid
    };

    static const std::map<std::string, PowerType> powerTypeMap = {
    {"Diesel Locomotive", PowerType::diesel},
    {"Electric Locomotive", PowerType::electric},
    {"Biodiesel Locomotive", PowerType::biodiesel},
    {"Diesel-Electric Locomotive", PowerType::dieselElectric},
    {"Diesel-Hyprid Locomotive", PowerType::dieselHybrid},
    {"Hydrogen-Hyprid Locomotive", PowerType::hydrogenHybrid},
    {"biodiesel-Hyprid Locomotive", PowerType::biodieselHybrid}
    };

    static const std::map<PowerType, CarType> powerToCarMap = {
        {PowerType::diesel, CarType::dieselTender},
        {PowerType::electric, CarType::batteryTender},
        {PowerType::biodiesel, CarType::biodieselTender},
        {PowerType::dieselHybrid, CarType::dieselTender},
        {PowerType::hydrogenHybrid, CarType::hydrogenTender},
        {PowerType::biodieselHybrid, CarType::biodieselTender}
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
