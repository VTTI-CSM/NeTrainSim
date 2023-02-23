//
// Created by Ahmed Aredah
// Version 0.1
//

#ifndef NeTrainSim_Car_h
#define NeTrainSim_Car_h

#include <string>
#include <iostream>
#include "TrainTypes.h"
#include "EnergyConsumption.h"
#include "TrainComponent.h"
using namespace std;

/**
 * @class	Car Car.h C:\Users\Ahmed\source\repos\NeTrainSim\src\TrainDefintion\Car.h
 *
 * @brief	A car.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class Car : public TrainComponent {
private:
	/** @brief	(Immutable) the default car name */
	inline static const  string DefaultCarName = "Railcar";
	/** @brief	(Immutable) the default car auxiliary power */
	static constexpr double DefaultCarAuxiliaryPower = 0.0;
	/** @brief	(Immutable) the default car minimum battery soc */
	static constexpr double DefaultCarMinBatterySOC = 0.2;
	/** @brief	(Immutable) the default car minimum tank sot */
	static constexpr double DefaultCarMinTankSOT = 0.0;
	/** brief (Immutable) the car has the capability to recharge if it is deccelerating.
	(only for battery tenders)*/
	static const bool DefaultCarRechargeCapability = true;


public:
	/** @brief  the weight of the vehicle when it is empty*/
	double emptyWeight;

	/** @brief  the type of the car*/
	TrainTypes::CarType carType; // 0:cargo, 1:fuelTank, 2:batteryTender, 3:hydrogenTender


public:

	double getCargoNetWeight();

	/**
	 * @fn	Car::Car(double carLength_m, double carDragCoef, double carFrontalArea_sqm, double carEmptyWeight_t, double carCurrentWeight_t, int carNoOfAxiles, int carType, double auxiliaryPower_kw = DefaultCarAuxiliaryPower, double batteryMaxCapacity_kwh = EC::DefaultCarBatteryMaxCapacity, double batteryInitialCharge_perc = EC::DefaultCarBatteryInitialCharge, double tenderMaxCapacity_kg_l = EC::DefaultCarTenderMaxCapacity, double tenderInitialCapacity_perc = EC::DefaultCarTenderInitialCapacity, std::string carName = DefaultCarName );
	 *
	 * @brief	Constructor
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param 	carLength_m				  	The car length m.
	 * @param 	carDragCoef				  	The car drag coef.
	 * @param 	carFrontalArea_sqm		  	The car frontal area sqm.
	 * @param 	carEmptyWeight_t		  	The car empty weight.
	 * @param 	carCurrentWeight_t		  	The car current weight.
	 * @param 	carNoOfAxiles			  	The car no of axiles.
	 * @param 	carType					  	Type of the car.
	 * @param 	auxiliaryPower_kw		  	(Optional) The auxiliary power kw.
	 * @param 	batteryMaxCapacity_kwh	  	(Optional) The battery maximum capacity kwh.
	 * @param 	batteryInitialCharge_perc 	(Optional) The battery initial charge perc.
	 * @param 	tenderMaxCapacity_kg_l	  	(Optional) The tender maximum capacity kilograms l.
	 * @param 	tenderInitialCapacity_perc	(Optional) The tender initial capacity perc.
	 * @param 	carName					  	(Optional) Name of the car.
	 */
	Car(double carLength_m, double carDragCoef, double carFrontalArea_sqm, double carEmptyWeight_t,
		double carCurrentWeight_t, int carNoOfAxiles, int carType,
		double auxiliaryPower_kw = DefaultCarAuxiliaryPower,
		double batteryMaxCapacity_kwh = EC::DefaultCarBatteryMaxCapacity,
		double batteryInitialCharge_perc = EC::DefaultCarBatteryInitialCharge,
		double tenderMaxCapacity_kg_l = EC::DefaultCarTenderMaxCapacity,
		double tenderInitialCapacity_perc = EC::DefaultCarTenderInitialCapacity,
		std::string carName = DefaultCarName
		);

	double getResistance(double trainSpeed) override;


	/**
	 * @fn	double Car::getEnergyConsumption(double &timeStep);
	 *
	 * @brief	Gets energy consumption
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param [in,out]	timeStep	The time step.
	 *
	 * @returns	The energy consumption.
	 */
	double getEnergyConsumption(double &timeStep);


	bool consumeFuel(double EC_kwh, bool isOffGrid, double dieselConversionFactor = EC::DefaultDieselConversionFactor,
		double hydrogenConversionFactor = EC::DefaultHydrogenConversionFactor, 
		double dieselDensity = EC::DefaultDieselDensity) override;

	/**
	 * @fn	friend ostream& Car::operator<<(ostream& ostr, Car& stud);
	 *
	 * @brief	Stream insertion operator
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param [in,out]	ostr	The ostr.
	 * @param [in,out]	stud	The stud.
	 *
	 * @returns	The shifted result.
	 */
	friend ostream& operator<<(ostream& ostr, Car& stud);
};


#endif // !NeTrainSim_Car_h