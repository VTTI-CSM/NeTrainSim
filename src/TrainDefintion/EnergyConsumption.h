//
// Created by Ahmed Aredah
// Version 0.1
//
#pragma once
#ifndef NeTrainSim_EnergyConsumption_h
#define NeTrainSim_EnergyConsumption_h

#include <string>
#include <iostream>
#include "TrainTypes.h"


using namespace std;

namespace EC {
// ##################################################################
// #             start: define locomotive default values            #
// ##################################################################
	/** (Immutable) the default locomotive auxiliary power */
	static constexpr double DefaultLocomotiveAuxiliaryPower = 0.0;
    /** The default electric locomotive battery maximum charge in kWh.
	 Battery capacity of up to 2.5 megawatt hours. */
    static double DefaultLocomotiveBatteryMaxCharge_Electric = 4000.0;
    /** The default diesel-hybrid locomotive battery maximum charge in kWh.
     Battery capacity of up to 2.5 megawatt hours. */
    static double DefaultLocomotiveBatteryMaxCharge_DieselHybrid = 4000.0;
    /** The default biodiesel-hybrid locomotive battery maximum charge in kWh.
     Battery capacity of up to 2.5 megawatt hours. */
    static double DefaultLocomotiveBatteryMaxCharge_BioDieselHybrid = 4000.0;
    /** The default hydrogen-hybrid locomotive battery maximum charge in kWh.
     Battery capacity of up to 2.5 megawatt hours. */
    static double DefaultLocomotiveBatteryMaxCharge_HydogenHybrid = 4000.0;

    /** The default locomotive battery initial charge for the electric locomotive. */
    static double DefaultLocomotiveBatteryInitialCharge_Electric = 0.6;
    /** The default locomotive battery initial charge for the diesel hybrid locomotive. */
    static double DefaultLocomotiveBatteryInitialCharge_DieselHybrid = 0.6;
    /** The default locomotive battery initial charge for the biodiesel hybrid locomotive. */
    static double DefaultLocomotiveBatteryInitialCharge_BioDieselHybrid = 0.6;

    static double DefaultLocomotiveBatteryInitialCharge_HydrogenHybrid = 0.6;

    /** The default locomotive tank maximum capacity in Liters.
	 Tank capacity of 5,300 gallons. */
    static double DefaultLocomotiveTankMaxCapacityDiesel = 20065.0;

    /** The deafult locomotive tank maximum capcity in liters
     for hydrogen. */
    static double DefaultLocomotiveTankMaxCapacityHydrogen = 20065.0;
	/** The default locomotive tank initial capacity in Gallons */
	static double DefaultLocomotiveTankInitialCapacity = 0.9;

	/** (Immutable) the default locomotive minimum tank depth of discharge */
	static constexpr double DefaultLocomotiveMinTankDOD = 0.8;

	/** (Immutable) the default locomotive battery depth of discharge = 1 - (SOC/100) */
	static constexpr double DefaultLocomotiveBatteryDOD = 0.9;
	/** (Immutable) the default locomotive battery C-Rate */
	static constexpr double DefaultLocomotiveBatteryCRate = 2.0;
	/** (Immutable) the default locomotive battery max state of charge in 
	rechage state for diesel generator.*/
    static constexpr double DefaultLocomotiveBatteryRechargeMaxSOC_Diesel = 0.65;
	/** (Immutable) the default locomotive battery max state of charge in
	rechage state for any generator other than diesel.*/
    static constexpr double DefaultLocomotiveBatteryRechargeMaxSOC_Other  = 0.65;
	/** (Immutable) the default locomotive battery min state of charge in which 
	it requires recharge once reached state for diesel generator.*/
    static constexpr double DefaultLocomotiveBatteryRechargeMinSOC_Diesel = 0.20;
	/** (Immutable) the default locomotive battery min state of charge in which
	it requires recharge once reached state for any generator except diesel.*/
    static constexpr double DefaultLocomotiveBatteryRechargeMinSOC_Other  = 0.50;//0.5

    /** required generator power percentage. */
    static constexpr double requiredGeneratorPower[] = {1,1,0.8,0.6,0.4,0.2,0.1,0.0};

    static constexpr double DefaultLocomotivePowerReduction_DieselHybrid = 0.8;

    static constexpr double DefaultLocomotivePowerReduction_BioDieselHybrid = 0.8;

    static constexpr double DefaultLocomotivePowerReduction_HydrogenHybrid = 0.5;
// ##################################################################
// #             end: define locomotive default values            #
// ##################################################################

// ##################################################################
// #                start: define cars default values               #
// ##################################################################
	/** (Immutable) the default car auxiliary power */
	static constexpr double DefaultCarAuxiliaryPower = 0.0;
	/** The default car battery maximum capacity in kWh.
     Tender car could carry 5.1–6.2MWh*/
    static double DefaultCarBatteryMaxCapacity = 10000.0;
	/** The default car battery initial charge */
	static double DefaultCarBatteryInitialCharge = 0.9;

	/** The default car tender maximum capacity in litters.
	 a tender has a capacity of 23000 Gallons. */
	static double DefaultCarTenderMaxCapacity = 87064.471;
	/** The default car tender initial capacity */
	static double DefaultCarTenderInitialCapacity = 0.9;

	/** (Immutable) the default car minimum battery depth of discharge = 1 - (SOC/100) */
	static constexpr double DefaultCarBatteryDOD = 0.9;
	/** (Immutable) the default battery c-rate */
	static constexpr double DefaultCarBatteryCRate = 2.0;
	/** (Immutable) the default car minimum tank sot */
	static constexpr double DefaultCarMinTankDOD = 0.9;
// ##################################################################
// #                 end: define cars default values                #
// ##################################################################

// ##################################################################
// #        start: general energy consumption default values        #
// ##################################################################

	/** the default diesel conversion factor.
	 Diesel (Gallons) = Energy Consumption (KW.h) * 0.028571
	 Diesel (Litters) = Energy Consumption (KW.h) * 0.1005 */
	static double DefaultDieselConversionFactor = 0.1005;
	/** the default biodiesel conversion factor.
     -> According to https://afdc.energy.gov/fuels/properties:
        1 gallon of B100 has 93% of the energy in 1 gallon of diesel,
        and 1 gallon of B20 has 99% of the energy in 1 gallon of diesel.
     -->Assume we use B100, then we use 93% and the conversion factor is
        0.1005 (diesel conversion factor) / 0.93 = 67/620
     Bio Diesel (Litters) = Energy Consumption (KW.h) * 67/620 */
    static double DefaultBiodieselConversionFactor = 67.0/620.0;
	/** The default hydrogen conversion factor.
	 Under ambient conditions, 1 litter of hydrogen = 0.003 kWh
	 Hydrogen (Litters) = EnergyConsumption (kW.h) * 0.002995*/
    static double DefaultHydrogenConversionFactor = 0.002995;

    /** 0.85 kg/l >> the density is in tons >> should be between 0.82 to 0.85 at 15 degree celsius (average temperature) */
	static double DefaultDieselDensity = 0.00085;

    /** 0.88 kg/l >> the density is in tons >> should be between 0.84 to 0.88 at 15 degree celsius (average temperature) */
	static double DefaultBioDieselDensity = 0.00088;
    /** 0.099836 kg/l  >> the density is in tons */
    static double DefaultHydrogenDensity = 0.000099836;
    /** The gamma for regenerating energy */
    static double gamma = 0.65;

    static std::map<TrainTypes::CarType, double> fuelConversionFactor_carTypes = {
        {TrainTypes::CarType::dieselTender, DefaultDieselConversionFactor},
        {TrainTypes::CarType::biodieselTender, DefaultBiodieselConversionFactor},
        {TrainTypes::CarType::hydrogenTender, DefaultHydrogenConversionFactor},
        {TrainTypes::CarType::batteryTender, 1.0}
    };

    static std::map<TrainTypes::PowerType, double> fuelConversionFactor_powerTypes = {
        {TrainTypes::PowerType::diesel, DefaultDieselConversionFactor},
        {TrainTypes::PowerType::dieselHybrid, DefaultDieselConversionFactor},
        {TrainTypes::PowerType::dieselElectric, DefaultDieselConversionFactor},
        {TrainTypes::PowerType::biodiesel, DefaultBiodieselConversionFactor},
        {TrainTypes::PowerType::biodieselHybrid, DefaultBiodieselConversionFactor},
        {TrainTypes::PowerType::hydrogenHybrid, DefaultHydrogenConversionFactor},
    };

// ##################################################################
// #          end: general energy consumption default values        #
// ##################################################################

	/**
	 * Gets drive line eff
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	trainSpeed              The train speed.
	 * @param 	notchNumberIndex        Zero-based index of the notch number.
	 * @param   powerAtWheelProportion  The Power required for the time step of the train at the wheel.
	 * @param 	powerType               Type of the power.
	 *
	 * @returns	The drive line eff.
	 */
    double getDriveLineEff(double &trainSpeed, int notchNumberIndex, double powerAtWheelProportion,
                           TrainTypes::PowerType powerType,
                           TrainTypes::LocomotivePowerMethod hybridMethod);

    double getWheelToDCBusEff(double &trainSpeed);

    double getDCBusToTankEff(double powerAtWheelProportion,
                             TrainTypes::PowerType powerType,
                             TrainTypes::LocomotivePowerMethod hybridMethod);

	/**
	 * Gets generator effeciency by the power type
	 *
	 * @author	Ahmed Aredah
	 * @date	3/20/2023
	 *
	 * @param 	powerType	Type of the power.
	 *
     * @returns	The generator efficiency.
	 */
    double getGeneratorEff(TrainTypes::PowerType powerType, double powerAtWheelProportion);

    double getBatteryEff(TrainTypes::PowerType powerType);

    std::pair<double,double> getMaxEffeciencyRange(TrainTypes::PowerType powerType);

	/**
     * Gets required generator power percentage for recharging the battery
	 *
	 * @author	Ahmed Aredah
	 * @date	3/20/2023
	 *
	 * @param 	batterySOC	The battery soc.
	 *
	 * @returns	The required generator power for recharge.
	 */
	double getRequiredGeneratorPowerForRecharge(double batterySOC);

	/**
     * Gets the CO2 emissions
	 *
	 * @author	Ahmed Aredah
	 * @date	3/20/2023
	 *
	 * @param 	fuelConsumption	The fuel consumption.
	 *
	 * @returns	The emissions.
	 */
    double getEmissions(double fuelConsumption);

    double getLocomotivePowerReductionFactor(TrainTypes::PowerType powerType);

    double getFuelFromEC(TrainTypes::PowerType powerType, double &EC_KWh);

    double getFuelFromEC(TrainTypes::CarType carType, double &EC_KWh);

    double getFuelConversionFactor(TrainTypes::PowerType powerType);

    double getFuelConversionFactor(TrainTypes::CarType carType);
}



#endif // !NeTrainSim_EnergyConsumption_h
