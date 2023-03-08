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
    /** The default locomotive battery maximum charge in kWh.
     Battery capacity of up to 2.4 megawatt hours. */
    static double DefaultLocomotiveBatteryMaxCharge = 240000.0;
	/** The default locomotive battery initial charge */
	static double DefaultLocomotiveBatteryInitialCharge = 0.9;

    /** The default locomotive tank maximum capacity in Gallons.
     Tank capacity of 5,300 gallons. */
    static double DefaultLocomotiveTankMaxCapacity = 20065.0;
    /** The default locomotive tank initial capacity in Gallons */
	static double DefaultLocomotiveTankInitialCapacity = 0.9;
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
    static double DefaultCarBatteryMaxCapacity = 510000.0;
	/** The default car battery initial charge */
	static double DefaultCarBatteryInitialCharge = 0.9;

    /** The default car tender maximum capacity in litters.
     a tender has a capacity of 23000 Gallons. */
    static double DefaultCarTenderMaxCapacity = 87064.471;
	/** The default car tender initial capacity */
    static double DefaultCarTenderInitialCapacity = 0.9;
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
     Diesel (Gallons) = Energy Consumption (KW.h) * 0.028571
     Diesel (Litters) = Energy Consumption (KW.h) * 0.1005 */
    static double DefaultBiodieselConversionFactor = 0.1005;
    /** The default hydrogen conversion factor.
     Under ambient conditions, 1 litter of hydrogen = 0.003 kWh
     Hydrogen (Litters) = EnergyConsumption (kW.h) * 0.002995*/
    static double DefaultHydrogenConversionFactor = 0.002995;

	/** 0.85 kg/l   >> should be between 0.82 to 0.85 at 15 degree celsius (average temperature) */
    static double DefaultDieselDensity = 0.00085;

    /** 0.85 kg/l   >> should be between 0.84 to 0.88 at 15 degree celsius (average temperature) */
    static double DefaultBioDieselDensity = 0.00088;
    /** 0.071 kg/l */
    static double DefaultHydrogenDensity = 0.000071;
	/** The gamma */
	static double gamma = 0.65;
// ##################################################################
// #          end: general energy consumption default values        #
// ##################################################################

	/**
	 * Gets drive line eff
	 *
	 * @author	Ahmed Aredah
	 * @date	2/28/2023
	 *
	 * @param 	trainSpeed			The train speed.
	 * @param 	notchNumberIndex	Zero-based index of the notch number.
	 * @param 	powerType			Type of the power.
	 *
	 * @returns	The drive line eff.
	 */
    double getDriveLineEff(double &trainSpeed, int notchNumberIndex, TrainTypes::PowerType powerType);

    double getGeneratorEff(TrainTypes::PowerType powerType);

}



#endif // !NeTrainSim_EnergyConsumption_h
