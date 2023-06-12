/**
 * @file    ~\NeTrainSim\src\Car.h
 *
 * Declares the Car class.
 */

#ifndef CAR_H
#define CAR_H

#include <string>
#include <iostream>
#include "traintypes.h"
#include "energyconsumption.h"
#include "traincomponent.h"
#include <memory>

/**
 * A rail car.
 *
 * The Car class represents a rail car in a train system. It is a type of TrainComponent
 * and can carry either commodities or fuel. Unlike the locomotive, the car does not provide
 * power to the train. However, in the case of a fuel tender, it can provide power to the
 * locomotive when its stored power is low. This is achieved by consuming fuel and reducing
 * the weight of the car.
 *
 * @Author  Ahmed
 * @Date    2/14/2023
 */
class Car : public TrainComponent {
    /***********************************************
    *              Variables Declaration           *
    ************************************************/

private:
    /** The default car name */
    inline static const std::string DefaultCarName = "Railcar";

public:
    /** The type of the car */
    TrainTypes::CarType carType;

public:
    /**
     * Constructor
     *
     * @param carLength_m               The car length in meters.
     * @param carDragCoef               The car drag coefficient.
     * @param carFrontalArea_sqm        The car frontal area in square meters.
     * @param carEmptyWeight_t          The car empty weight in tons.
     * @param carCurrentWeight_t        The car current weight in tons.
     * @param carNoOfAxles              The number of axles in the car.
     * @param carType                   The type of the car.
     * @param auxiliaryPower_kw         (Optional) The auxiliary power in kW. Default is 0.0.
     * @param batteryMaxCapacity_kWh    (Optional) The battery maximum capacity in kWh. Default is 10,000.
     * @param batteryInitialCharge_perc (Optional) The battery initial charge percentage. Default is 0.9.
     * @param tenderMaxCapacity_kg_l    (Optional) The tender maximum capacity in kilograms or liters.
     * @param tenderInitialCapacity_perc (Optional) The tender initial capacity percentage. Default is 0.9.
     * @param carName                   (Optional) The name of the car.
     */
    Car(double carLength_m, double carDragCoef, double carFrontalArea_sqm,
        double carEmptyWeight_t, double carCurrentWeight_t, int carNoOfAxles,
        int carType, double auxiliaryPower_kw = EC::DefaultCarAuxiliaryPower,
        double batteryMaxCapacity_kWh = EC::DefaultCarBatteryMaxCapacity,
        double batteryInitialCharge_perc = EC::DefaultCarBatteryInitialCharge,
        double tenderMaxCapacity_kg_l = EC::DefaultCarTenderMaxCapacity,
        double tenderInitialCapacity_perc = EC::DefaultCarTenderInitialCapacity,
        std::string carName = DefaultCarName);

    /**
     * Gets the cargo net weight.
     *
     * @returns The cargo net weight in tons.
     */
    double getCargoNetWeight();

    /**
     * Sets the car's current weight.
     *
     * @param newCurrentWeight The new gross weight of the car, which cannot be less than the light weight of the car.
     */
    void setCarCurrentWeight(double newCurrentWeight);

    /**
     * Gets the resistance contributed by this car to the whole train.
     *
     * @param trainSpeed The train speed in m/s.
     * @returns The car's resistance in Newton.
     */
    double getResistance(double trainSpeed) override;

    /**
     * Gets the energy consumption of the car (in case there is auxiliary power). Otherwise, it returns 0.0.
     *
     * @param timeStep The time step of the simulator in seconds.
     * @returns The energy consumption in kWh.
     */
    double getEnergyConsumption(double& timeStep);

    /**
     * Consumes fuel from the tender.
     *
     * @param timeStep                  The time step.
     * @param trainSpeed                The train speed.
     * @param EC_kWh                    The EC kWh.
     * @param carVirtualTractivePower   (Optional) The car's virtual tractive power.
     * @param dieselConversionFactor    (Optional) The diesel conversion factor.
     * @param biodieselConversionFactor (Optional) The biodiesel conversion factor.
     * @param hydrogenConversionFactor  (Optional) The hydrogen conversion factor.
     * @param dieselDensity             (Optional) The diesel density.
     * @param biodieselDensity          (Optional) The biodiesel density.
     * @param hydrogenDensity           (Optional) The hydrogen density.
     * @returns A std::pair<bool, double>.
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
     * Gets the maximum energy provided by the car.
     *
     * @param timeStep The time step.
     * @returns The maximum provided energy in kWh.
     */
    double getMaxProvidedEnergy(double& timeStep);

    /**
     * Checks if the car can provide energy.
     *
     * @param EC        The EC.
     * @param timeStep  The time step.
     * @returns True if the car can provide energy, false otherwise.
     */
    bool canProvideEnergy(double& EC, double& timeStep);

    /**
     * Stream insertion operator.
     *
     * @param ostr The ostr.
     * @param stud The stud.
     * @returns The shifted result.
     */
    friend std::ostream& operator<<(std::ostream& ostr, Car& stud);
};

#endif // CAR_H
