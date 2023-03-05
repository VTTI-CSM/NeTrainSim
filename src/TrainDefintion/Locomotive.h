//
// Created by Ahmed Aredah
// Version 0.1
//

#ifndef NeTrainSim_Locomotive_h
#define NeTrainSim_Locomotive_h

#include <string>
#include <iostream>
#include "../util/List.h"
#include "../util/Vector.h"
#include "TrainTypes.h"
#include "EnergyConsumption.h"
#include "TrainComponent.h"

using namespace std;

/**
 * A locomotive.
 *
 * @author	Ahmed Aredah
 * @date	2/28/2023
 */
class Locomotive : public TrainComponent{
private:
	/** (Immutable) the default locomotive name */
	inline static const  string DefaultLocomotiveName = "locomotive";
	/** (Immutable) the default locomotive maximum speed */
	static constexpr double DefaultLocomotiveMaxSpeed = 100.0 / 3.0;
	/** (Immutable) the default locomotive maximum acheivable notch */
	static constexpr double DefaultLocomotiveMaxAcheivableNotch = 0;
	/** (Immutable) the default locomotive auxiliary power */
	static constexpr double DefaultLocomotiveAuxiliaryPower = 0.0;
	/** (Immutable) the default locomotive no of notches */
	static const int DefaultLocomotiveNoOfNotches = 8;
	/** (Immutable) the default locomotive no of axiles */
	static const int DefaultLocomotiveNoOfAxiles = 6;
	/** diesel */
	static const int DefaultLocomotiveType = 0;
	/** (Immutable) the default locomotive minimum tank sot */
	static constexpr double DefaultLocomotiveMinTankSOT = 0.2;
	/** (Immutable) the default locomotive minimum battery soc */
    static constexpr double DefaultLocomotiveMinBatterySOC = 0.0;
    /** (Immutable) the default locomotive minimum weight. */
    static constexpr double DefaultLocomotiveEmptyWeight = 180;


public:
	/** The max power of the locomotive */
	double maxPower;
	/** Transmission effeciency of the locomotive */
	double transmissionEfficiency;
	/** The type of the car */
	TrainTypes::PowerType powerType;
	/** Current used notch max speed */
	double maxSpeed;

	/** Decide if the locomotive is adding power to the train */
	bool isLocOn = true;
	/** Number of notches the locomotives have */
	int Nmax = 8;
	/** The max notch the locomotive is allowed to achieve, if zero, then use all notches */
	int maxLocNotch = 0;
	/** The current notch the locomotive is going by */
	int currentLocNotch = 0;
	/** The discretized throttlelevels */
	Vector<double> discritizedLamda;
	/** The current throttle level based on the current notch */
	double throttleLevel;
	/** The throttle levels */
	Vector<double> throttleLevels;
	/** (Immutable) the gravitation acceleration */
	const double g = 9.8067;

	/**
	 * !
	 * \brief the constructor for the locomotive class
	 * 
	 * \details the constructor sets the definition of the locomotive and initialize its default
	 *  values.
	 *  

	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	locomotiveMaxPower_kw				The locomotive maximum power kw.
	 * @param 	locomotiveTransmissionEfficiency	The locomotive transmission efficiency.
	 * @param 	locomotiveLength_m					The locomotive length m.
	 * @param 	locomotiveDragCoef					The locomotive drag coef.
	 * @param 	locomotiveFrontalArea_sqm			The locomotive frontal area sqm.
	 * @param 	locomotiveWeight_t					The locomotive weight.
	 * @param 	locomotiveNoOfAxiles				(Optional) The locomotive no of axiles.
	 * @param 	locomotivePowerType					(Optional) Type of the locomotive power.
	 * @param 	locomotiveMaxSpeed_mps				(Optional) The locomotive maximum speed mps.
	 * @param 	totalNotches						(Optional) The total notches.
	 * @param 	locomotiveMaxAchievableNotch		(Optional) The locomotive maximum achievable
	 * 												notch.
	 * @param 	locomotiveAuxiliaryPower_kw			(Optional) The locomotive auxiliary power kw.
	 * @param 	locomotiveName						(Optional) Name of the locomotive.
	 * @param 	batteryMaxCharge_kwh				(Optional) The battery maximum charge kwh.
	 * @param 	batteryInitialCharge_perc			(Optional) The battery initial charge perc.
	 * @param 	tankMaxCapacity_kg					(Optional) The tank maximum capacity kilograms.
	 * @param 	tankInitialCapacity_perc			(Optional) The tank initial capacity perc.
	 */
	Locomotive(double locomotiveMaxPower_kw, double locomotiveTransmissionEfficiency,
		double locomotiveLength_m, double locomotiveDragCoef, double locomotiveFrontalArea_sqm,
		double locomotiveWeight_t,
		int locomotiveNoOfAxiles = DefaultLocomotiveNoOfAxiles,
		int locomotivePowerType = DefaultLocomotiveType,
		double locomotiveMaxSpeed_mps = DefaultLocomotiveMaxSpeed,
		int totalNotches = DefaultLocomotiveNoOfNotches,
		int locomotiveMaxAchievableNotch = DefaultLocomotiveMaxAcheivableNotch,
		double locomotiveAuxiliaryPower_kw = DefaultLocomotiveAuxiliaryPower,
		string locomotiveName = DefaultLocomotiveName,
		double batteryMaxCharge_kwh = EC::DefaultLocomotiveBatteryMaxCharge,
		double batteryInitialCharge_perc = EC::DefaultLocomotiveBatteryInitialCharge,
		double tankMaxCapacity_kg = EC::DefaultLocomotiveTankMaxCapacity,
		double tankInitialCapacity_perc = EC::DefaultLocomotiveTankInitialCapacity);

	/**
	 * Gets power type string
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @returns	The power type string.
	 */
	string getPowerTypeString();

	/**
	 * Gets power type enum
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	powertype	The powertype.
	 *
	 * @returns	The power type enum.
	 */
	TrainTypes::PowerType getPowerTypeEnum(string powertype);

	/**
	 * Gets hyperbolic throttle coef
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param [in,out]	trainSpeed	The train speed.
	 *
	 * @returns	The hyperbolic throttle coef.
	 */
	double getHyperbolicThrottleCoef(double &trainSpeed);

	/**
	 * Getlamda discretized
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param [in,out]	lamda	The lamda.
	 *
	 * @returns	A double.
	 */
	double getlamdaDiscretized(double &lamda);

	/**
	 * Gets discretized throttle coef
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param [in,out]	trainSpeed	The train speed.
	 *
	 * @returns	The discretized throttle coef.
	 */
	double getDiscretizedThrottleCoef(double& trainSpeed);

	/**
	 * Gets throttle level
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param [in,out]	trainSpeed				The train speed.
	 * @param [in,out]	TrainAcceleration   	The train acceleration.
	 * @param [in,out]	optimize				True to optimize.
	 * @param [in,out]	optimumThrottleLevel	The optimum throttle level.
	 *
	 * @returns	The throttle level.
	 */
	double getThrottleLevel(double &trainSpeed, double& TrainAcceleration, bool &optimize, double &optimumThrottleLevel);

	/**
	 * Gets a resistance
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	trainSpeed	The train speed.
	 *
	 * @returns	The resistance.
	 */
	double getResistance(double trainSpeed) override;

	/**
	 * Gets tractive force
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param [in,out]	frictionCoef			The friction coef.
	 * @param [in,out]	trainSpeed				The train speed.
	 * @param [in,out]	trainAcceleration   	The train acceleration.
	 * @param [in,out]	optimize				True to optimize.
	 * @param [in,out]	optimumThrottleLevel	The optimum throttle level.
	 *
	 * @returns	The tractive force.
	 */
	double getTractiveForce(double &frictionCoef, double &trainSpeed, double& trainAcceleration,
		bool &optimize, double &optimumThrottleLevel);

	/**
	 * Gets net force
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param [in,out]	frictionCoef			The friction coef.
	 * @param [in,out]	trainSpeed				The train speed.
	 * @param [in,out]	trainAcceleration   	The train acceleration.
	 * @param [in,out]	optimize				True to optimize.
	 * @param [in,out]	optimumThrottleLevel	The optimum throttle level.
	 *
	 * @returns	The net force.
	 */
	double getNetForce(double &frictionCoef, double &trainSpeed, double &trainAcceleration,
		bool &optimize, double &optimumThrottleLevel);

	/**
	 * Gets shared virtual tractive power
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param [in,out]	trainSpeed		 	The train speed.
	 * @param [in,out]	trainAcceleration	The train acceleration.
	 * @param [in,out]	sharedWeight	 	The shared weight.
	 * @param [in,out]	sharedResistance 	The shared resistance.
	 *
	 * @returns	The shared virtual tractive power.
	 */
	double getSharedVirtualTractivePower(double& trainSpeed, double& trainAcceleration,
		double& sharedWeight, double& sharedResistance);

	/**
	 * Gets energy consumption
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param [in,out]	LocomotiveVirtualTractivePowerdouble	The locomotive virtual tractive
	 * 															powerdouble.
	 * @param [in,out]	speed									The speed.
	 * @param [in,out]	acceleration							The acceleration.
	 * @param [in,out]	timeStep								The time step.
	 *
	 * @returns	The energy consumption.
	 */
	double getEnergyConsumption(double& LocomotiveVirtualTractivePowerdouble, double& speed, double& acceleration,
		double& timeStep);


    /**
     * @brief consume the locomotive diesel fuel.
     * @param EC_kwh
     * @param dieselConversionFactor
     * @param dieselDensity
     * @return
     */
    bool consumeFuelDiesel(double EC_kwh, double dieselConversionFactor, double dieselDensity);

    /**
     * @brief consume the locomotive electric energy
     * @param EC_kwh
     * @return
     */
    bool consumeBattery(double EC_kwh);

    /**
     * @brief consume the locomotive hydrogen fuel.
     * @param EC_kwh
     * @param hydrogenConversionFactor
     * @return
     */
    bool consumeFuelHydrogen(double EC_kwh, double hydrogenConversionFactor);
    /**
     * @brief refill the locomtoive battery
     * @param EC_kwh
     * @return
     */
    bool refillBattery(double EC_kwh);

    /**
     * @brief Rechage catenary and grid system if they are available
     * @param EC_kwh
     * @return
     */
    bool rechageCatenary(double EC_kwh);

	/**
	 * Consume fuel
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	EC_kwh						The ec kwh.
	 * @param 	isOffGrid					True if is off grid, false if not.
	 * @param 	dieselConversionFactor  	(Optional) The diesel conversion factor.
	 * @param 	hydrogenConversionFactor	(Optional) The hydrogen conversion factor.
	 * @param 	dieselDensity				(Optional) The diesel density.
	 *
	 * @returns	True if it succeeds, false if it fails.
	 */
    bool consumeFuel(double EC_kwh, double dieselConversionFactor = EC::DefaultDieselConversionFactor,
        double hydrogenConversionFactor = EC::DefaultHydrogenConversionFactor,
        double dieselDensity = EC::DefaultDieselDensity) override;

	/**
	 * Define throttle levels
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @returns	A Vector&lt;double&gt;
	 */
	Vector<double> defineThrottleLevels();

	/**
	 * Updates the location notch described by trainSpeed
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param [in,out]	trainSpeed	The train speed.
	 */
	void updateLocNotch(double &trainSpeed);

	/**
	 * Stream insertion operator
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param [in,out]	ostr	The ostr.
	 * @param [in,out]	stud	The stud.
	 *
	 * @returns	The shifted result.
	 */
	friend ostream& operator<<(ostream& ostr, Locomotive& stud);
};



#endif // !NeTrainSim_Locomotive_h
