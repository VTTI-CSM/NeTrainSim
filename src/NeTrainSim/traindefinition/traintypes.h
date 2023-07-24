/**
 * @file	~\src\trainDefintion\TrainTypes.h.
 *
 * Declares the train types class
 */
#ifndef carLocTypes_enum
#define carLocTypes_enum

#include <string>
#include <stdexcept>
#include "../util/vector.h"
#include "../util/error.h"
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
        hydrogenFuelCell,
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
        CarType::hydrogenFuelCell,
        CarType::biodieselTender
    };

    /** The car type strings[] */
    static const std::string carTypeStrings[] = {
    "Cargo Car",
    "Diesel Tender",
    "Battery Tender",
    "Hydrogen Fuel Cell",
    "Biodiesel Tender"
    };

    /** The car type array[] */
    static const CarType carTypeArray[] = {
    CarType::cargo,
    CarType::dieselTender,
    CarType::batteryTender,
    CarType::hydrogenFuelCell,
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
    {"Diesel Tender", CarType::dieselTender},
    {"Battery Tender", CarType::batteryTender},
    {"Hydrogen Tender", CarType::hydrogenFuelCell},
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
     * @exception	std::invalid_argument	Thrown when an invalid argument
     *              error condition occurs.
     *
     * @param 	cartype	The cartype.
     *
     * @returns	A CarType.
     */
    inline CarType strtoCarType(const std::string& cartype) {
        auto it = carTypeMap.find(cartype);
        if (it == carTypeMap.end()) {
            throw std::invalid_argument("Error: " +
                                        std::to_string(
                                            static_cast<int>(
                                            Error::trainWrongCarType)) +
                                        "\nInvalid car type");
        }
        return it->second;
    }

    /**
     * Ito car type
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @exception	std::invalid_argument	Thrown when an invalid argument
     *              error condition occurs.
     *
     * @param 	cartype	The cartype.
     *
     * @returns	A CarType.
     */
    inline CarType itoCarType(int cartype) {
        if (cartype < 0 || cartype >= carTypeN) {
            throw std::invalid_argument("Error: " +
                                        std::to_string(
                                            static_cast<int>(
                                            Error::trainWrongCarType)) +
                                        "\nInvalid car type");
        }
        return carTypeArray[cartype];
    }


    /**
     * Converts a powertype string to the corresponding index
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param powerTypesString  is the target string
     *
     * @return the corresponding index
     */
    inline int carTypestrToInt(std::string carTypesString) {
        // TODO: get closest match instead of exact match
        for (int i = 0; i < carTypeN; i++) {
            if (carTypeStrings[i] == carTypesString) {
                return i;
            }
        }
        return -1;  // returns -1 if the target string is not found
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
    inline std::ostream& operator<<(std::ostream& ss, const CarType& obj) {
        ss << carTypeStrings[static_cast<int>(obj)];
        return ss;
    }

    /**
     * ***********************************************************************
     *  Locomotives power types
     *  **********************************************************************
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
        PowerType::dieselElectric //it doesnt have a battery to store energy in
    };

    /** The locomotive tank only */
    static const Vector<PowerType> locomotiveTankOnly = {
        PowerType::diesel,
        PowerType::biodiesel,
        PowerType::dieselElectric //it doesnt have a battery to store energy in
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
    "Diesel-Battery Locomotive",
    "Diesel-Hybrid Locomotive",
    "Hydrogen-Hybrid Locomotive",
    "Biodiesel-Hybrid Locomotive"
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
    {"Diesel-Battery Locomotive", PowerType::dieselElectric},
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
        {PowerType::hydrogenHybrid, CarType::hydrogenFuelCell},
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
     * @exception	std::invalid_argument	Thrown when an invalid argument
     *              error condition occurs.
     *
     * @param 	powertype	The powertype.
     *
     * @returns	Powertype as a PowerType.
     */
    inline PowerType strToPowerType(std::string powertype) {
        auto it = powerTypeMap.find(powertype);
        if (it == powerTypeMap.end()) {
            throw std::invalid_argument("Error: " +
                                        std::to_string(
                                            static_cast<int>(
                                            Error::trainWrongLocoType)) +
                                        "\nInvalid locomotive type");
        }
        return it->second;
    }

    /**
     * Converts a powertype string to the corresponding index
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param powerTypesString  is the target string
     *
     * @return the corresponding index
     */
    inline int powerTypestrToInt(std::string powerTypesString) {
        for (int i = 0; i < powerTypeN; i++) {
            if (powerTypeStrings[i] == powerTypesString) {
                return i;
            }
        }
        return -1;  // returns -1 if the target string is not found
    }

    /**
     * Converts a powertype to a power type
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @exception	std::invalid_argument	Thrown when an invalid argument
     *              error condition occurs.
     *
     * @param 	powertype	The powertype.
     *
     * @returns	Powertype as a PowerType.
     */
    inline PowerType iToPowerType(int powertype) {
        if (powertype < 0 || powertype >= powerTypeN) {
            throw std::invalid_argument("Error: " +
                                        std::to_string(
                                            static_cast<int>(
                                            Error::trainWrongLocoType)) +
                                        "\nInvalid locomotive type");
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

    /**
     * **********************************************************************
     *  Locomotives power Method
     *  *********************************************************************
     */
    static const int powerMethodN = 2;
    /** Values that represent power types */
    enum class _LocomotivePowerMethod {
        notApplicable,
        series,
        parallel
    };

    /** Type of the power */
    using LocomotivePowerMethod = _LocomotivePowerMethod;
    /** The power type strings[] */
    static const std::string powerMethodsStrings[] = {
        "Not Applicable"
        "Series Hybrid Power",
        "Parallel Hybrid Power"
    };
    /** The power type array[] */
    static const LocomotivePowerMethod powerMethodArray[] = {
        LocomotivePowerMethod::notApplicable, //0
        LocomotivePowerMethod::series, // 1
        LocomotivePowerMethod::parallel //2
    };

    /** The power type map */
    static const std::map<std::string, LocomotivePowerMethod> powerMethodMap = {
        {"Series Hybrid Power", LocomotivePowerMethod::series},
        {"Series Hybrid Power", LocomotivePowerMethod::series},
        {"Parallel Hybrid Power", LocomotivePowerMethod::parallel}
    };


    /**
     * Converts a string to a power method
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @exception	std::invalid_argument	Thrown when an invalid argument
     *              error condition occurs.
     *
     * @param 	powertype	The powertype.
     *
     * @returns	Powertype as a PowerType.
     */
    inline LocomotivePowerMethod strToPowerMethod(std::string powertype) {
        auto it = powerMethodMap.find(powertype);
        if (it == powerMethodMap.end()) {
            throw std::invalid_argument("Error: " +
                                        std::to_string(
                                            static_cast<int>(
                                            Error::trainWrongLocoType)) +
                                        "\nInvalid locomotive type");
        }
        return it->second;
    }

    /**
     * Converts a powertype to a power type
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @exception	std::invalid_argument	Thrown when an invalid argument
     *              error condition occurs.
     *
     * @param 	powertype	The powertype.
     *
     * @returns	Powertype as a PowerType.
     */
    inline LocomotivePowerMethod iToPowerMethod(int powertype) {
        if (powertype < 0 || powertype >= powerMethodN) {
            throw std::invalid_argument("Error: " +
                                        std::to_string(
                                            static_cast<int>(
                                            Error::trainWrongLocoType)) +
                                        "\nInvalid locomotive type");
        }
        return powerMethodArray[powertype];
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
    inline std::ostream& operator<<(std::ostream& ss,
                                    const LocomotivePowerMethod& obj) {
        ss << powerMethodsStrings[static_cast<int>(obj)];
        return ss;
    }


    /**
     * *********************************************************************
     * Fuel Types
     * *********************************************************************
     */
    static const int fuelN = 4;
    /** Values that represent fuel types */
    enum class _fuelTypes {
        diesel,
        biodiesel,
        hydrogen,
        noFuel
    };

    /** Type of the fuel */
    using FuelType = _fuelTypes;

    /** The fuel type strings[] */
    static const std::string fuelStrings[] = {
        "Diesel Fuel",
        "Biodiesel Fuel",
        "Hydrogen-Hyprid Fuel",
        "No Fuel"
    };

    /** The fuel type array[] */
    static const FuelType fuelArray[] = {
        FuelType::diesel,     //0
        FuelType::biodiesel,  //1
        FuelType::hydrogen,   //2
        FuelType::noFuel      //3
    };

    /** The fuel type map */
    static const std::map<std::string, FuelType> fuelMap = {
        {"Diesel Fuel", FuelType::diesel},
        {"Biodiesel Fuel", FuelType::biodiesel},
        {"Hydrogen Fuel", FuelType::hydrogen},
        {"No Fuel", FuelType::noFuel}
    };

    /** The fuel to car map */
    static const std::map<PowerType, FuelType> powerToFuelMap = {
        {PowerType::diesel, FuelType::diesel},
        {PowerType::electric, FuelType::noFuel},
        {PowerType::biodiesel, FuelType::biodiesel},
        {PowerType::dieselHybrid, FuelType::diesel},
        {PowerType::hydrogenHybrid, FuelType::hydrogen},
        {PowerType::biodieselHybrid, FuelType::biodiesel}
    };

    /** The fuel to car map */
    static const std::map<CarType, FuelType> carToFuelMap = {
        {CarType::cargo, FuelType::noFuel},
        {CarType::batteryTender, FuelType::noFuel},
        {CarType::biodieselTender, FuelType::biodiesel},
        {CarType::dieselTender, FuelType::diesel},
        {CarType::hydrogenFuelCell, FuelType::hydrogen}
    };

    inline std::string fuelTypeToStr(FuelType type) {
        return fuelStrings[static_cast<int>(type)];
    }

    inline FuelType getFuelTypeFromPowerType(PowerType theType) {
        auto it = powerToFuelMap.find(theType);
        if (it != powerToFuelMap.end()) {
            return it->second;
        } else {
            throw std::invalid_argument("Invalid PowerType");
        }
    }

}

#endif // !carLocTypes_enum
