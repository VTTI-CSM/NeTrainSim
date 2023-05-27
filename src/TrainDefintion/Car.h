//
// Created by Ahmed Aredah
// Version 0.0.1
//

#ifndef NeTrainSim_Car_h
#define NeTrainSim_Car_h

#include <string>
#include <iostream>
#include "TrainTypes.h"
#include "EnergyConsumption.h"
#include "TrainComponent.h"
#include <memory>
using namespace std;

/**
 * defines a rail car. this class inherits the TrainComponent class
 *
 * @details The car does not provide power to the train unlike the locomtotive.
 *          Cars only cary either commodities or fuel only. if it is fuel tender,
 *          it can provide power to the locomotive once the locomotive's stored
 *          power is low. In that case, the car losses weight by the fuel consumption.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class Car : public TrainComponent {
    /***********************************************
    *              variables declaration           *
    ************************************************/

private:
	/** (Immutable) the default car name */
	inline static const  string DefaultCarName = "Railcar";

public:
	/** The type of the car */
	TrainTypes::CarType carType;


public:
	/**
	 * Constructor
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
     * @param 	carLength_m				  	The car length in meters.
	 * @param 	carDragCoef				  	The car drag coef.
     * @param 	carFrontalArea_sqm		  	The car frontal area in square meters.
     * @param 	carEmptyWeight_t		  	The car empty weight in ton.
     * @param 	carCurrentWeight_t		  	The car current weight in ton.
	 * @param 	carNoOfAxiles			  	The car no of axiles.
	 * @param 	carType					  	Type of the car.
     * @param 	auxiliaryPower_kw		  	(Optional) The auxiliary power in kw. Default is 0.0.
     * @param 	batteryMaxCapacity_kwh	  	(Optional) The battery maximum capacity in kwh. Default is 10,000.
     * @param 	batteryInitialCharge_perc 	(Optional) The battery initial charge percentage. Default is 0.9.
     * @param 	tenderMaxCapacity_kg_l	  	(Optional) The tender maximum capacity kilograms or liters.
     * @param 	tenderInitialCapacity_perc	(Optional) The tender initial capacity perc. Default is 0.9.
	 * @param 	carName					  	(Optional) Name of the car.
	 */
	Car(double carLength_m, double carDragCoef, double carFrontalArea_sqm, double carEmptyWeight_t,
		double carCurrentWeight_t, int carNoOfAxiles, int carType,
		double auxiliaryPower_kw = EC::DefaultCarAuxiliaryPower,
		double batteryMaxCapacity_kwh = EC::DefaultCarBatteryMaxCapacity,
		double batteryInitialCharge_perc = EC::DefaultCarBatteryInitialCharge,
		double tenderMaxCapacity_kg_l = EC::DefaultCarTenderMaxCapacity,
		double tenderInitialCapacity_perc = EC::DefaultCarTenderInitialCapacity,
		std::string carName = DefaultCarName
		);



    /**
     * Gets cargo net weight
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	The cargo net weight in ton.
     */
    double getCargoNetWeight();


	/**
	 * @brief setCarCurrentWeight
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param newCurrentWeight  is the new gross weight of the car, this weight cannot be less
     *                          than the light weight of the car.
	 */
	void setCarCurrentWeight(double newCurrentWeight);

	/**
     * Gets the resistance this car is contributing to the whole train.
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
     * @param 	trainSpeed	The train speed in m/s.
	 *
     * @returns	The car resistance in Newton.
	 */
	double getResistance(double trainSpeed) override;

	/**
     * Gets energy consumption of the car (in case there is auxiliary power). otherwise it returns 0.0
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
     * @param [in,out]	timeStep	The time step of the simulator in seconds.
	 *
     * @returns	The energy consumption in kWH.
	 */
	double getEnergyConsumption(double &timeStep);

	/**
	 * Consume fuel from the tender
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	EC_kwh						The ec kwh.
     * @param   carVirtualTractivePower     This value is not implemented since a car does not have
     *                                      an engine.
	 * @param 	isOffGrid					True if is off grid, false if not.
	 * @param 	dieselConversionFactor  	(Optional) The diesel conversion factor.
	 * @param 	hydrogenConversionFactor	(Optional) The hydrogen conversion factor.
	 * @param 	dieselDensity				(Optional) The diesel density.
	 *
	 * @returns	True if it succeeds, false if it fails.
	 */
    std::pair<bool, double> consumeFuel(double timeStep, double trainSpeed, double EC_kwh,
                                        double carVirtualTractivePower = std::numeric_limits<double>::quiet_NaN(),
                                        double dieselConversionFactor = EC::DefaultDieselConversionFactor,
                                        double biodieselConversionFactor = EC::DefaultBiodieselConversionFactor,
                                        double hydrogenConversionFactor = EC::DefaultHydrogenConversionFactor,
                                        double dieselDensity = EC::DefaultDieselDensity,
                                        double biodieselDensity = EC::DefaultBioDieselDensity,
                                        double hydrogenDensity = EC::DefaultHydrogenDensity) override;

    /**
     * @brief getMaxProvidedEnergy
     * @param timeStep
     * @return
     */
    double getMaxProvidedEnergy(double &timeStep);

    /**
     * @brief check if the car can provide energy
     * @param EC
     * @param timeStep
     * @return
     */
    bool canProvideEnergy(double &EC, double &timeStep);


	/**
	 * Stream insertion operator
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
