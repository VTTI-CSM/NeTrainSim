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
#include <memory>
using namespace std;

/**
 * A car.
 *
 * @author	Ahmed
 * @date	2/14/2023
 */
class Car : public TrainComponent {
private:
	/** (Immutable) the default car name */
	inline static const  string DefaultCarName = "Railcar";
	/** (Immutable) the default car minimum battery soc */
	static constexpr double DefaultCarMinBatterySOC = 0.2;
	/** (Immutable) the default car minimum tank sot */
	static constexpr double DefaultCarMinTankSOT = 0.0;


public:
	/** The type of the car */
	TrainTypes::CarType carType;


public:

	/**
	 * Gets cargo net weight
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @returns	The cargo net weight.
	 */
	double getCargoNetWeight();

	/**
	 * Constructor
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
        double auxiliaryPower_kw = EC::DefaultCarAuxiliaryPower,
		double batteryMaxCapacity_kwh = EC::DefaultCarBatteryMaxCapacity,
		double batteryInitialCharge_perc = EC::DefaultCarBatteryInitialCharge,
		double tenderMaxCapacity_kg_l = EC::DefaultCarTenderMaxCapacity,
		double tenderInitialCapacity_perc = EC::DefaultCarTenderInitialCapacity,
		std::string carName = DefaultCarName
		);

    /**
     * @brief setCarCurrentWeight
     * @param newCurrentWeight
     */
    void setCarCurrentWeight(double newCurrentWeight);

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
	 * Gets energy consumption
	 *
	 * @author	Ahmed
	 * @date	2/14/2023
	 *
	 * @param [in,out]	timeStep	The time step.
	 *
	 * @returns	The energy consumption.
	 */
	double getEnergyConsumption(double &timeStep);

    /**
     * @brief consume the tender diesel fuel.
     * @param EC_kwh
     * @param dieselConversionFactor
     * @param dieselDensity
     * @return
     */
    bool consumeDiesel(double EC_kwh, double dieselConversionFactor, double dieselDensity);

    /**
     * @brief consume the tender bio diesel fuel.
     * @param EC_kwh
     * @param biodieselConversionFactor
     * @param biodieselDensity
     * @return
     */
    bool consumeBioDiesel(double EC_kwh, double biodieselConversionFactor, double biodieselDensity);

    /**
     * @brief consume the tender hydrogen.
     * @param EC_kwh
     * @param hydrogenConversionFactor
     * @param hydrogenDensity
     * @return
     */
    bool consumeHydrogen(double EC_kwh, double hydrogenConversionFactor, double hydrogenDensity);
    /**
     * @brief consume the car battery.
     * @param EC_kwh
     * @return
     */
    bool consumeBattery(double EC_kwh);

    /**
     * @brief Refill the car battery.
     * @param EC_kwh
     * @return
     */
    bool RefillBattery(double EC_kwh);

	/**
	 * Consume fuel from the tender
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
                     double biodieselConversionFactor = EC::DefaultBiodieselConversionFactor,
                     double hydrogenConversionFactor = EC::DefaultHydrogenConversionFactor,
                     double dieselDensity = EC::DefaultDieselDensity,
                     double biodieselDensity = EC::DefaultBioDieselDensity,
                     double hydrogenDensity = EC::DefaultHydrogenDensity) override;

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
