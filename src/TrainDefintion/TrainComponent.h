//
// Created by Ahmed Aredah
// Version 0.1.
//

#ifndef NeTrainSim_TrainComponent_h
#define NeTrainSim_TrainComponent_h

#include <string>
#include <iostream>
// #include "TrainTypes.h"
#include "../network/netlink.h"
#include "battery.h"
#include "tank.h"
#include "energyconsumption.h"
#include <memory>
using namespace std;

/**
 * Defines a train component base class. The train component could be a railcar or a locomotive.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class TrainComponent : public Battery, public Tank{
public:

    /***********************************************
    *              variables declaration           *
    ************************************************/
    /** The name of the vehicle */
	std::string name;
    /** Length of the vehicle in meter*/
	double length;
	/** The current curvature the vehicle is experiencing */
	double trackCurvature;
	/** The current grade the vehicle is experiencing */
	double trackGrade;
	/** The air drag factor for aerodynamics resistance */
	double dragCoef;
	/** The total frontal area of the vehicle for the aerodynamics resistance */
	double frontalArea;
    /** The gross weight of the vehicle when the train is travelling */
	double currentWeight;
    /** The light weight of the vehicle when the train is travelling */
	double emptyWeight;
	/** The number of axiles the car has */
	int noOfAxiles;
	/** Auxiliary power */
	double auxiliaryPower;

    /** The amount of energy consumed in kwh*/
	double energyConsumed = 0.0;
    /** The amount of cummulative energy consumed in kwh*/
	double cumEnergyConsumed = 0.0;
    /** The amount of energy regenerated in kwh*/
	double energyRegenerated = 0.0;
    /** The amount of cummulative energy regenerated in kwh*/
	double cumEnergyRegenerated = 0.0;

	/** Holds the current link this vehicle is on. */
	std::shared_ptr<NetLink> hostLink;


    /***********************************************
    *                   Methods                    *
    ************************************************/
    // no need for a constructor or deconstractor


	/**
	 * \brief Gets the resistance applied on only this vehicle.
	 * 
	 * \details This function takes in a parameter trainSpeed and returns the resistance of a vehicle at a
	 * given speed. The function performs a series of calculations using various properties of the
	 * Car object, including its weight, number of axles, frontal area, drag coefficient, track
	 * grade, and track curvature.
	 * @f{eqnarray*}
	 * Resistance = (1.5 + \frac{18}{axileWeight} + 0.03 * speed + \frac{frontalArea * dragCoef*
	 * speed^2}{vehicleWeight}) * totalWeight + 20 * vehicleWeight (trackGrade + 0.04 * curvature)\\
	 * @f}
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	trainSpeed	The current time step train speed.
	 *
	 * @returns	The resistance.
	 */
	virtual double getResistance(double trainSpeed);

	/**
	 * \brief Resets the energy consumptions data for the current time step
	 *
	 * @author	Ahmed Aredah
	 * @date	2/18/2023
	 */
	virtual void resetTimeStepConsumptions();


	/**
     * @brief sets the current weight of the vehicle
     *
     * @author	Ahmed
     * @date	2/14/2023
     *
     * @param newCurrentWeight  is the new weight of the vehicle in ton
	 */
	virtual void setCurrentWeight(double newCurrentWeight);

	/**
	 * @brief consume the locomotive diesel fuel.
     *
     * @author	Ahmed
     * @date	2/14/2023
     *
     * @param EC_kwh                    is the Energy consumption in kWH
     * @param dieselConversionFactor    is the diesel conversion factor from kWH to liters
     * @param dieselDensity             is the diesel density kg/liter
	 * @return
	 */
	virtual std::pair<bool, double> consumeFuelDiesel(double EC_kwh,
													  double dieselConversionFactor,
													  double dieselDensity);

	/**
	 * @brief consume the locomotive bio diesel fuel.
     *
     * @author	Ahmed
     * @date	2/14/2023
     *
     * @param EC_kwh                        is the Energy consumption in kWH
     * @param bioDieselConversionFactor     is the diesel conversion factor from kWH to liters
     * @param bioDieselDensity              is the diesel density kg/liter
	 * @return
	 */
	virtual std::pair<bool, double> consumeFuelBioDiesel(double EC_kwh,
														 double bioDieselConversionFactor,
														 double bioDieselDensity);

	/**
	 * @brief consume any source of electricity in the locomotive; either the catenary or the batteries.
     *
     * @author	Ahmed
     * @date	2/14/2023
     *
	 * @details the function consumes the electricity stored in the locomotive batteries or provided
	 * by the catenary if available.
	 * For battery:
	 * It checks the amout of energy required by the locomotive below the max amount the
	 * battery can provide.
	 *      1. if yes, it consumes it from the battery and returns (true, 0.0),
	 *      2. if no, it consumes the max that the battery can provide and return (true, 0.0),
	 * If the battery cannot provide any energy because it is empty, it returns (false, 0.0).
	 * For Catenary:
	 * It consumes the whole amount of energy required from the catenary.
	 *
     * @param EC_kwh        is the energy required for the locomotive to keep running
	 * @param minBatterySOC is the battery min state of charge, which below it, the battery is considered empty.
     * @return std::pair<bool, double> bool: no energy consumed and
     *                                 double: rest of energy that needs to be consumed.
	 */
	virtual std::pair<bool, double> consumeElectricity(double timeStep, double EC_kwh);

	/**
	 * @brief consume the locomotive hydrogen fuel.
     *
     * @author	Ahmed
     * @date	2/14/2023
     *
	 * @param EC_kwh
	 * @param hydrogenConversionFactor
	 * @return
	 */
	virtual std::pair<bool, double> consumeFuelHydrogen(double EC_kwh,
														double hydrogenConversionFactor,
														double hydrogenDensity);
	/**
	 * @brief refill the locomtoive battery
     *
     * @author	Ahmed
     * @date	2/14/2023
     *
	 * @param timeStep
	 * @param EC_kwh
	 * @return
	 */
    virtual double refillBattery(double timeStep, double EC_kwh);

	/**
	 * @brief Rechage catenary and grid system if they are available
     *
     * @author	Ahmed
     * @date	2/14/2023
     *
	 * @param EC_kwh
	 * @return
	 */
	virtual bool rechargeCatenary(double EC_kwh);


	/**
	 * \brief Consume fuel or battery from the vehicle. if it energy required is greater than stored energy
	 * in tank/battery return true, otherwise false.
	 * 
	 * 
	 * \details This function is responsible for fuel consumption or regeneration of energy. It takes as
	 * input the energy consumed
	 *  in kWh, a boolean flag indicating whether the locomotive is off-grid or not, conversion
	 *  factors for diesel and hydrogen, and diesel density. &lt;br&gt;
	 * First: the function checks if the energy is to be consumed. If it is, it checks the type of
	 * the power source for the vehicle. &lt;br&gt;
	 *  - If it is of diesel or hydrogen types, it computes the quantity consumed in liters (for
	 *  diesel) or in kg (for hydrogen).  
	 *  	based on the energy consumed and the provided conversion factors. If there is enough fuel,
	 *  	the function updates the variables for the energy consumed and the current state of the
	 *  	tank and returns true. Otherwise, it returns false indicating that the tank is empty. &lt;
	 *  	br&gt;
	 *  - If the power source is electric and the vehicle is off-grid, the function checks if there
	 *  is enough charge left in the battery.
	 *  	If there is enough charge, the function updates the variables for the energy consumed and
	 *  	the current state of the battery and returns true. Otherwise, it returns false indicating
	 *  	that the battery is empty. &lt;br&gt;
	 *  - If the power source is electric and the locomotive is not off-grid, the function simply
	 *  updates the variables for
	 *  - the energy consumed and the cumulative energy consumed and returns true. &lt;br&gt;
	 * Second, If the energy consumed is not greater than zero, the function checks if the power
	 * source is electric and the
	 *  locomotive is off-grid. If that is the case, the function checks if there is room for
	 *  recharging the battery. If there is, the function updates the variables for the energy
	 *  regenerated and the current state of the battery and returns true. If there is no more room
	 *  in the battery, the function updates the variables for the energy regenerated and the
	 *  battery state of charge and returns true.
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	EC_kwh						The energy consumption in kwh.
	 * @param 	isOffGrid					True if is off grid, false if not.
	 * @param 	dieselConversionFactor  	(Optional) The diesel conversion factor from kwh to
	 * 										liters.
	 * @param 	hydrogenConversionFactor	(Optional) The hydrogen conversion factor from kwh to kg.
	 * @param 	dieselDensity				(Optional) The diesel density in kg/liter.
	 *
	 * @returns	True if it succeeds, false if it fails.
	 */
	virtual std::pair<bool,double> consumeFuel(double timeStep, double trainSpeed,
                                               double EC_kwh,
                                               double LocomotiveVirtualTractivePower = std::numeric_limits<double>::quiet_NaN(),
											   double dieselConversionFactor = EC::DefaultDieselConversionFactor,
											   double biodieselConversionFactor = EC::DefaultBiodieselConversionFactor,
											   double hydrogenConversionFactor = EC::DefaultHydrogenConversionFactor,
											   double dieselDensity = EC::DefaultDieselDensity,
											   double biodieselDensity = EC::DefaultBioDieselDensity,
                                               double hydrogenDensity = EC::DefaultHydrogenDensity);

	/**
	 * \brief Stream insertion operator
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param [in,out]	ostr	The ostr.
	 * @param [in,out]	stud	The stud.
	 *
	 * @returns	The shifted result.
	 */
	friend ostream& operator<<(ostream& ostr, TrainComponent& stud);

private:
};

/**
// End of !\NeTrainSim\src\trainDefintion\TrainComponent.h
 */
#endif // !NeTrainSim_TrainComponent_h
