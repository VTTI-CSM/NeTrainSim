//
// Created by Ahmed Aredah Version 0.1.
//

#ifndef NeTrainSim_TrainComponent_h
#define NeTrainSim_TrainComponent_h

#include <string>
#include <iostream>
#include "TrainTypes.h"
#include "EnergyConsumption.h"
using namespace std;

/**
 * A train component base class.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class TrainComponent {

public:
	/** The name of the car */
	std::string name;
	/** Length of the vehicle */
	double length;
	/** The current curvature the vehicle is experiencing */
	double trackCurvature;
	/** The current grade the vehicle is experiencing */
	double trackGrade;
	/** The air drag factor for aerodynamics resistance */
	double dragCoef;
	/** The total frontal area of the vehicle for the aerodynamics resistance */
	double frontalArea;
	/** The weight of the vehicle when the train is travelling */
	double currentWeight;
	/** The number of axiles the car has */
	int noOfAxiles;
	/** Auxiliary power */
	double auxiliaryPower;

	/** Fuel cell variables if other fuel types and battery tender max capacity */
	double batteryMaxCharge;
	/** Tender initial capacity */
	double batteryInitialCharge;
	/** Tender current capacity */
	double batteryCurrentCharge;
	/** Tender fuel cell state */
	double batteryStateOfCharge;

	/** Fuel cell variables if other fuel types and battery tender max capacity */
	double tankMaxCapacity;
	/** Tender initial capacity */
	double tankInitialCapacity;
	/** Tender current capacity */
	double tankCurrentCapacity;
	/** Tender fuel cell state */
	double tankStateOfCapacity;


	/** The amount of energy consumed */
	double energyConsumed = 0.0;
	/** The amount of cummulative energy consumed */
	double cumEnergyConsumed = 0.0;
	/** The amount of energy regenerated */
	double energyRegenerated = 0.0;
	/** The amount of cummulative energy regenerated */
	double cumEnergyRegenerated = 0.0;

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
	virtual bool consumeFuel(double EC_kwh, bool isOffGrid, double dieselConversionFactor = EC::DefaultDieselConversionFactor,
		double hydrogenConversionFactor = EC::DefaultHydrogenConversionFactor,
		double dieselDensity = EC::DefaultDieselDensity);

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
};

/**
// End of C:\Users\Ahmed\OneDrive - Virginia Tech\03.Work\02.VTTI\02.ResearchWork\01.TrainModelling\02.Code\00.CPP\NeTrainSim\NeTrainSim\src\trainDefintion\TrainComponent.h
 */
#endif // !NeTrainSim_TrainComponent_h