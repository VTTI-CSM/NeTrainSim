//
// Created by Ahmed Aredah
// Version 0.0.1
//

#ifndef NeTrainSim_Locomotive_h
#define NeTrainSim_Locomotive_h

#include <functional>
#include <string>
#include <iostream>
#include "../util/vector.h"
#include "traintypes.h"
#include "energyconsumption.h"
#include "traincomponent.h"

using namespace std;

/**
 * defines a rail locomotive. This class inherits the TrainComponent class.
 *
 * @details The locomotive provide power to the train unlike the railcar.
 *          locomotives can have different powertrains (different technologies).
 *          For every technology, there is an associated effeciencies and conversion factors.
 *          These conversion factors and efficiencies are accessible from the EC namespace.
 *          The locomotive may have a battery and/or tank based on its type.
 *          once the locomotive's stored power is low. the locomotive attempts to request
 *          energy from the tenders if available. if not, the locomotive is turned off.
 *
 * @author	Ahmed Aredah
 * @date	2/28/2023
 */
class Locomotive : public TrainComponent{
    /***********************************************
    *              variables declaration           *
    ************************************************/
private:
	/** (Immutable) the default locomotive name */
	inline static const  string DefaultLocomotiveName = "locomotive";
	/** (Immutable) the default locomotive maximum speed = 120 km/h*/
	static constexpr double DefaultLocomotiveMaxSpeed = 100.0 / 3.0;
	/** (Immutable) the default locomotive maximum acheivable notch */
	static constexpr double DefaultLocomotiveMaxAcheivableNotch = 0;
	/** (Immutable) the default locomotive no of notches */
	static const int DefaultLocomotiveNoOfNotches = 8;
	/** (Immutable) the default locomotive no of axiles */
	static const int DefaultLocomotiveNoOfAxiles = 6;
    /** (Immutable) make the diesel the default if the user does not provide it.*/
	static const int DefaultLocomotiveType = 0;
	/** (Immutable) the default locomotive minimum weight. */
	static constexpr double DefaultLocomotiveEmptyWeight = 180;

private:
    /** */
	double maxTractiveForce = 0.0;
public:
	/** The max power of the locomotive in kw. */
	double maxPower;
    /** The tractive forces of this time step. */
    double currentTractiveForce = 0.0;
	/** Transmission effeciency of the locomotive */
	double transmissionEfficiency;
	/** The type of the car */
	TrainTypes::PowerType powerType;
    /** the hybrid technology whether it is series or paralletl. */
    TrainTypes::LocomotivePowerMethod hybridMethod;
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
    /** The forced lower power factor to the locomotive in case lower energy consumption is required. */
    double locPowerReductionFactor = 1.0;
	/** The discretized throttlelevels */
	Vector<double> discritizedLamda;
	/** The current throttle level based on the current notch */
	double throttleLevel;
	/** The throttle levels */
	Vector<double> throttleLevels;
	/** (Immutable) the gravitation acceleration */
	const double g = 9.8067;

    double usedPowerPortion = 0.0;


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
			   double locomotiveAuxiliaryPower_kw = EC::DefaultLocomotiveAuxiliaryPower,
			   string locomotiveName = DefaultLocomotiveName,
               double batteryMaxCharge_kwh = std::numeric_limits<double>::quiet_NaN(), // NAN value should be resolved latter
               double batteryInitialCharge_perc = std::numeric_limits<double>::quiet_NaN(), // NAN value should be resolved latter
               double tankMaxCapacity_kg = EC::DefaultLocomotiveTankMaxCapacityDiesel,
			   double tankInitialCapacity_perc = EC::DefaultLocomotiveTankInitialCapacity,
               double batteryCRate = EC::DefaultLocomotiveBatteryCRate,
               TrainTypes::LocomotivePowerMethod theHybridMethod = TrainTypes::LocomotivePowerMethod::notApplicable);


	/**
     * Gets power type as a string
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
     * @returns	The power type as a string.
	 */
	string getPowerTypeString();

	/**
     * Gets power type as an enum
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
     * @param 	powertype	The powertype of the locomotive.
	 *
     * @returns	The power type as an enum.
	 */
	TrainTypes::PowerType getPowerTypeEnum(string powertype);

	/**
     * Gets hyperbolic throttle coef. refer to the published paper for more details.
     * @cite Aredah, A.; Fadhloun, K.; Rakha, H.; List, G. NeTrainSim:
     *       A Network Freight Train Simulator for Estimating Energy/Fuel Consumption.
     *       Preprints 2022, 2022080518. https://doi.org/10.20944/preprints202208.0518.v1.
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
     * @param [in,out]	trainSpeed	The train speed in m/s.
	 *
     * @returns	The hyperbolic throttle coefficient.
	 */
	double getHyperbolicThrottleCoef(double &trainSpeed);

	/**
     * Get lamda discretized. for more details refer to the published paper.
     * @details this function discretize the continous throttle level function 'getHyperbolicThrottleCoef'
     * @cite Aredah, A.; Fadhloun, K.; Rakha, H.; List, G. NeTrainSim:
     *       A Network Freight Train Simulator for Estimating Energy/Fuel Consumption.
     *       Preprints 2022, 2022080518. https://doi.org/10.20944/preprints202208.0518.v1.
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
     * @param [in,out]	lamda	The throttle level coefficient (as a continuous value).
     *                          This comes from the @getHyper
	 *
	 * @returns	A double.
	 */
	double getlamdaDiscretized(double &lamda);

	/**
     * Gets discretized throttle coef. for more details refer to the published paper.
     * @cite Aredah, A.; Fadhloun, K.; Rakha, H.; List, G. NeTrainSim:
     *       A Network Freight Train Simulator for Estimating Energy/Fuel Consumption.
     *       Preprints 2022, 2022080518. https://doi.org/10.20944/preprints202208.0518.v1.
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param [in,out]	trainSpeed	The train speed.
	 *
	 * @returns	The discretized throttle coef.
	 */
	double getDiscretizedThrottleCoef(double& trainSpeed);

	/**
     * Gets the throttle level the locomotive should move by. for more details refer to the published paper.
     * @cite Aredah, A.; Fadhloun, K.; Rakha, H.; List, G. NeTrainSim:
     *       A Network Freight Train Simulator for Estimating Energy/Fuel Consumption.
     *       Preprints 2022, 2022080518. https://doi.org/10.20944/preprints202208.0518.v1.
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
     * @param [in,out]	trainSpeed				The train speed in m/s.
     * @param [in,out]	optimize				True to optimize.
     * @param [in,out]	optimumThrottleLevel	The optimum throttle level in case the optimization is enabled.
	 *
	 * @returns	The throttle level.
	 */
	double getThrottleLevel(double &trainSpeed, bool &optimize, double &optimumThrottleLevel);

	/**
     *  Gets the resistance this locomotive is contributing to the whole train.
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
     * @param 	trainSpeed	The train speed in m/s.
	 *
     * @returns	The resistance in Newton.
	 */
	double getResistance(double trainSpeed) override;

	/**
     * Gets tractive force that this locomotive can generate by the current speed and notch level.
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
     * @param [in,out]	frictionCoef			The friction coef of the rail.
     * @param [in,out]	trainSpeed				The train speed in m/s.
	 * @param [in,out]	optimize				True to optimize.
     * @param [in,out]	optimumThrottleLevel	The optimum throttle level in case of optimization is eneabled.
	 *
     * @returns	The tractive force in Newton.
	 */
	double getTractiveForce(double &frictionCoef, double &trainSpeed,
		bool &optimize, double &optimumThrottleLevel);

	/**
	 * Gets net force
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
     * @param [in,out]	frictionCoef			The friction coef of the rail.
     * @param [in,out]	trainSpeed				The train speed in m/s.
	 * @param [in,out]	optimize				True to optimize.
     * @param [in,out]	optimumThrottleLevel	The optimum throttle level in cae of optimization is enabled.
	 *
     * @returns	The net force in Newton.
	 */
	double getNetForce(double &frictionCoef, double &trainAcceleration,
		bool &optimize, double &optimumThrottleLevel);

	/**
     * Gets shared virtual tractive power. The virtual power is the power when the train is on
     * a negative slope.
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
     * @param [in,out]	trainSpeed		 	The train speed in m/s.
     * @param [in,out]	trainAcceleration	The train acceleration in m/s^2.
     * @param [in,out]	sharedWeight	 	The shared weight is the weight this locomotive is pulling.
     *                                      It includes the portion of the train weight this locomotive is
     *                                      responsible for.
     * @param [in,out]	sharedResistance 	The shared resistance. This includes the portion of the train
     *                                      resistance, this locomotiv is responsible for.
	 *
     * @returns	The shared virtual tractive power in kW.
	 */
	double getSharedVirtualTractivePower(double& trainSpeed, double& trainAcceleration,
		double& sharedWeight, double& sharedResistance);

	/**
     * Gets energy consumption of this locomotive.
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
     * @param [in,out]	LocomotiveVirtualTractivePower  The locomotive virtual tractive power.
     * @param [in,out]	speed							The train speed in m/s.
     * @param [in,out]	acceleration					The train acceleration in m/s^2.
     * @param [in,out]	timeStep						The simulator time step in seconds.
	 *
     * @returns	The energy consumption in KWh.
	 */
    double getEnergyConsumption(double& LocomotiveVirtualTractivePower, double& acceleration, double& speed,
		double& timeStep);



	/**
     * Consume the locomotive fuel.
     *
     * @details If the locomotive does not have the necessary power to continue,
     * It searches for the power from any other tender. if there is no tenders available, the locomotive
     * is shut down.
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
     * @param   timeStep                    The simulator time step in seconds.
     * @param   trainSpeed                  The speed of the train in m/s.
     * @param 	EC_kwh						The energy consumption in kwh.
	 * @param 	isOffGrid					True if is off grid, false if not.
     * @param 	dieselConversionFactor  	(Optional) The diesel conversion factor that convert from kwh to liters.
     * @param 	bioDieselConversionFactor  	(Optional) The biodiesel conversion factor that convert from kwh to liters.
     * @param 	hydrogenConversionFactor	(Optional) The hydrogen conversion factor that convert from kWh to liters.
     * @param 	dieselDensity				(Optional) The diesel density in kg/liter.
     * @param 	biodieselDensity            (Optional) The biodiesel density in kg/liter.
     * @param   hydrogenDensity             (Optional) The hydrogen density in kg/liter.
     *
	 * @returns	True if it succeeds, false if it fails.
	 */
    std::pair<bool,double> consumeFuel(double timeStep, double trainSpeed,
                                       double EC_kwh,
                                       double LocomotiveVirtualTractivePower = std::numeric_limits<double>::quiet_NaN(),
									   double dieselConversionFactor = EC::DefaultDieselConversionFactor,
									   double bioDieselConversionFactor = EC::DefaultBiodieselConversionFactor,
									   double hydrogenConversionFactor = EC::DefaultHydrogenConversionFactor,
									   double dieselDensity = EC::DefaultDieselDensity,
									   double bioDieselDensity = EC::DefaultBioDieselDensity,
                                       double hydrogenDensity = EC::DefaultHydrogenDensity) override;

	/**
	 * @brief Get the max energy the locomotive can regenerate.
     * @details this depends on the max power from the generator can produce.
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param timeStep
	 * @param trainSpeed
     * @param LocomotiveVirtualTractivePower
     *
	 * @return
	 */
    double getMaxRechargeEnergy(double timeStep, double trainSpeed, double LocomotiveVirtualTractivePower);

    double getUsedPowerPortion(double trainSpeed, double LocomotiveVirtualTractivePower);

    void rechargeBatteryByMaxFlow(double timeStep, double trainSpeed, double powerPortion,
                                     double fuelConversionFactor, double fuelDensity,
                                     double LocomotiveVirtualTractivePower,
                                     std::function<std::pair<bool, double>(double, double, double)> ConsumeFuelFunc);

    std::pair<bool, double> consumeEnergyFromHybridTechnology(double timeStep, double trainSpeed, double powerPortion,
                                                          double EC_kwh, double fuelConversionFactor,
                                                          double fuelDensity, double LocomotiveVirtualTractivePower,
                                                          std::function<std::pair<bool, double>(double, double, double)> ConsumeFuelFunc);

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
     * Updates the location notch described by trainSpeed.
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
     * @param [in,out]	trainSpeed	The train speed in m/s.
	 */
	void updateLocNotch(double &trainSpeed);

	/**
	 * @brief reduce the power that the locomotive is producing by
	 * reducing the notch position to a lower position.
	 * if the locomotive could reduce the notch position to a lower position, reduce it.
	 * O.W, turn the locomotive off in case the locomotive was already moving by the notch 1.
	 *
	 * @author	Ahmed Aredah
	 * @date	3/12/2023
	 */
    void reducePower(double &reductionFactor);

	/**
	 * @brief reset lower power restriction on the locomotive.
	 *
	 * @author	Ahmed Aredah
	 * @date	3/12/2023
	 */
	void resetPowerRestriction();

    /**
     * @brief getMaxProvidedEnergy
     * @param timeStep
     * @return
     */
    double getMaxProvidedEnergy(double &timeStep);

    /**
     * @brief check if the locomotive can provide required energy
     * @param EC
     * @param timeStep
     * @return
     */
    double canProvideEnergy(double &EC, double &timeStep);

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
