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
	/** (Immutable) the default ec file name[] */
	static const char DefaultECFileName[] = "ECEDB.xml";
	/** These variables will be overwritten by the energyConfiguration file */
#pragma region LocomotiveECVariables
	static double DefaultLocomotiveBatteryMaxCharge = 120.0;
	/** The default locomotive battery initial charge */
	static double DefaultLocomotiveBatteryInitialCharge = 0.9;

	/** The default locomotive tank maximum capacity */
	static double DefaultLocomotiveTankMaxCapacity = 1000.0;
	/** The default locomotive tank initial capacity */
	static double DefaultLocomotiveTankInitialCapacity = 0.9;
#pragma endregion

	/** The default car battery maximum capacity */
#pragma region RailcarECVariables
	static double DefaultCarBatteryMaxCapacity = 10000.0;
	/** The default car battery initial charge */
	static double DefaultCarBatteryInitialCharge = 0.9;

	/** The default car tender maximum capacity */
	static double DefaultCarTenderMaxCapacity = 1000.0;
	/** The default car tender initial capacity */
	static double DefaultCarTenderInitialCapacity = 0.0;
#pragma endregion

	/** The default diesel conversion factor */
#pragma region GeneralECVariables
	static double DefaultDieselConversionFactor = 0.1005;
	/** The default hydrogen conversion factor */
	static double DefaultHydrogenConversionFactor = 0.02995;

	/** 0.85 kg/l   >> should be between 0.82 to 0.85 at 15 degree celsius (average temperature) */
	static double DefaultDieselDensity = 0.85;
	/** The gamma */
	static double gamma = 0.65;
#pragma endregion

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
	double getDriveLineEff(double trainSpeed, int notchNumberIndex, TrainTypes::PowerType powerType);

}



#endif // !NeTrainSim_EnergyConsumption_h
