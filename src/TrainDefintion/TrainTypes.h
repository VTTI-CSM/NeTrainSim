/**
 * @file	~\src\trainDefintion\TrainTypes.h.
 *
 * Declares the train types class
 */
#ifndef carLocTypes_enum
#define carLocTypes_enum

#include <string>
#include <stdexcept>
#include "../util/Vector.h"
#include <map>

/** . */
namespace TrainTypes {

    /** The car type n */
    static const int carTypeN = 5;


    /** Values that represent car types */
    enum class _CarType {
        cargo,
        dieselTender,
        batteryTender,
        hydrogenTender,
        biodieselTender
    };

    /** Type of the car */
    using CarType = _CarType;

    /** The car rechargable technologies */
    static const Vector<CarType> carRechargableTechnologies = {
        CarType::batteryTender
    };

    /** The car non rechargable technologies */
    static const Vector<CarType> carNonRechargableTechnologies = {
        CarType::dieselTender,
        CarType::hydrogenTender,
        CarType::biodieselTender
    };

    /** The car type strings[] */
    static const std::string carTypeStrings[] = {
    "Cargo Car",
    "Diesel Tender",
    "Battery Tender",
    "Hydrogen Tender",
    "Biodiesel Tender"
    };

    /** The car type array[] */
    static const CarType carTypeArray[] = {
    CarType::cargo,
    CarType::dieselTender,
    CarType::batteryTender,
    CarType::hydrogenTender,
    CarType::biodieselTender
    };

    /**
     * Gets car type vector
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @returns	The car type vector.
     */
    inline Vector<CarType> getCarTypeVector() {
        Vector<CarType> carTypeVector;
        for (auto c : carTypeArray) {
            carTypeVector.push_back(c);
        }
        return carTypeVector;
    }


    /** The car type map */
    static const std::map<std::string, CarType> carTypeMap = {
    {"Cargo Car", CarType::cargo},
    {"Fuel Tank", CarType::dieselTender},
    {"Battery Tender", CarType::batteryTender},
    {"Hydrogen Tender", CarType::hydrogenTender},
    {"Biodiesel Tender", CarType::biodieselTender}
    };

    /**
     * Car type to string
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	type	The type.
     *
     * @returns	A std::string.
     */
    inline std::string carTypeToStr(CarType type) {
        return carTypeStrings[static_cast<int>(type)];
    }

    /**
     * Strto car type
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @exception	std::invalid_argument	Thrown when an invalid argument error condition occurs.
     *
     * @param 	cartype	The cartype.
     *
     * @returns	A CarType.
     */
    inline CarType strtoCarType(const std::string& cartype) {
        auto it = carTypeMap.find(cartype);
        if (it == carTypeMap.end()) {
            throw std::invalid_argument("invalid car type");
        }
        return it->second;
    }

    /**
     * Ito car type
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @exception	std::invalid_argument	Thrown when an invalid argument error condition occurs.
     *
     * @param 	cartype	The cartype.
     *
     * @returns	A CarType.
     */
    inline CarType itoCarType(int cartype) {
        if (cartype < 0 || cartype >= carTypeN) {
            throw std::invalid_argument("invalid car type");
        }
        return carTypeArray[cartype];
    }

    /**
     * *************************************************************************************
     *  Locomotives power types
     *  ************************************************************************************
     */
    static const int powerTypeN = 7;
    /** Values that represent power types */
    enum class _PowerType {
        diesel,
        electric,
        biodiesel,
        dieselElectric,
        dieselHybrid,
        hydrogenHybrid,
        biodieselHybrid
    };

    /** Type of the power */
    using PowerType = _PowerType;

    /** they can be recharged and have a battery */
    static const Vector<PowerType> locomotiveRechargableTechnologies = {
        PowerType::electric,
        PowerType::dieselHybrid,
        PowerType::hydrogenHybrid,
        PowerType::biodieselHybrid
    };

    /** they cannot be recharge and/or do not have a battery */
    static const Vector<PowerType> locomotiveNonRechargableTechnologies = {
        PowerType::diesel,
        PowerType::biodiesel,
        PowerType::dieselElectric  // it does not have a battery to store energy in
    };

    /** The locomotive tank only */
    static const Vector<PowerType> locomotiveTankOnly = {
        PowerType::diesel,
        PowerType::biodiesel,
        PowerType::dieselElectric  // it does not have a battery to store energy in
    };

    /** The locomotive battery only */
    static const Vector<PowerType> locomotiveBatteryOnly = {
        PowerType::electric
    };

    /** they have two types */
    static const Vector<PowerType> locomotiveHybrid = {
        PowerType::dieselElectric,
        PowerType::dieselHybrid,
        PowerType::hydrogenHybrid,
        PowerType::biodieselHybrid
    };

    /** The power type strings[] */
    static const std::string powerTypeStrings[] = {
    "Diesel Locomotive",
    "Electric Locomotive",
    "Biodiesel Locomotive",
    "Diesel-Electric Locomotive",
    "Diesel-Hyprid Locomotive",
    "Hydrogen-Hyprid Locomotive",
    "Biodiesel-Hyprid Locomotive"
    };

    /** The power type array[] */
    static const PowerType powerTypeArray[] = {
    PowerType::diesel, // 0
    PowerType::electric, // 1
    PowerType::biodiesel,  //2
    PowerType::dieselElectric, //3  // all locomotives now are diesel
    PowerType::dieselHybrid,  //4
    PowerType::hydrogenHybrid, //5
    PowerType::biodieselHybrid //6
    };

    /** The power type map */
    static const std::map<std::string, PowerType> powerTypeMap = {
    {"Diesel Locomotive", PowerType::diesel},
    {"Electric Locomotive", PowerType::electric},
    {"Biodiesel Locomotive", PowerType::biodiesel},
    {"Diesel-Electric Locomotive", PowerType::dieselElectric},
    {"Diesel-Hyprid Locomotive", PowerType::dieselHybrid},
    {"Hydrogen-Hyprid Locomotive", PowerType::hydrogenHybrid},
    {"biodiesel-Hyprid Locomotive", PowerType::biodieselHybrid}
    };

    /** The power to car map */
    static const std::map<PowerType, CarType> powerToCarMap = {
        {PowerType::diesel, CarType::dieselTender},
        {PowerType::electric, CarType::batteryTender},
        {PowerType::biodiesel, CarType::biodieselTender},
        {PowerType::dieselHybrid, CarType::dieselTender},
        {PowerType::hydrogenHybrid, CarType::hydrogenTender},
        {PowerType::biodieselHybrid, CarType::biodieselTender}
    };

    /**
     * Power type to string
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	type	The type.
     *
     * @returns	A std::string.
     */
    inline std::string PowerTypeToStr(PowerType type) {
        return powerTypeStrings[static_cast<int>(type)];
    }

    /**
     * Converts a powertype to a power type
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @exception	std::invalid_argument	Thrown when an invalid argument error condition occurs.
     *
     * @param 	powertype	The powertype.
     *
     * @returns	Powertype as a PowerType.
     */
    inline PowerType strToPowerType(std::string powertype) {
        auto it = powerTypeMap.find(powertype);
        if (it == powerTypeMap.end()) {
            throw std::invalid_argument("invalid car type");
        }
        return it->second;
    }

    /**
     * Converts a powertype to a power type
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @exception	std::invalid_argument	Thrown when an invalid argument error condition occurs.
     *
     * @param 	powertype	The powertype.
     *
     * @returns	Powertype as a PowerType.
     */
    inline PowerType iToPowerType(int powertype) {
        if (powertype < 0 || powertype >= powerTypeN) {
            throw std::invalid_argument("invalid car type");
        }
        return powerTypeArray[powertype];
    }

    /**
     * Stream insertion operator
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param [in,out]	ss 	The ss.
     * @param 		  	obj	The object.
     *
     * @returns	The shifted result.
     */
    inline std::ostream& operator<<(std::ostream& ss, const PowerType& obj) {
        ss << powerTypeStrings[static_cast<int>(obj)];
        return ss;
    }

}

#endif // !carLocTypes_enum
