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
#include "../util/Map.h"
#include "../util/Vector.h"

using namespace std;

namespace EC {
	static const char DefaultECFileName[] = "ECEDB.xml";
	// these variables will be overwritten by the energyConfiguration file
#pragma region LocomotiveECVariables
	static double DefaultLocomotiveBatteryMaxCharge = 120.0;
	static double DefaultLocomotiveBatteryInitialCharge = 0.9;

	static double DefaultLocomotiveTankMaxCapacity = 1000.0;
	static double DefaultLocomotiveTankInitialCapacity = 0.9;
#pragma endregion

#pragma region RailcarECVariables
	static double DefaultCarBatteryMaxCapacity = 10000.0;
	static double DefaultCarBatteryInitialCharge = 0.9;

	static double DefaultCarTenderMaxCapacity = 1000.0;
	static double DefaultCarTenderInitialCapacity = 0.0;
#pragma endregion

#pragma region GeneralECVariables
	static double DefaultDieselConversionFactor = 0.1005;
	static double DefaultHydrogenConversionFactor = 0.02995;

	static double DefaultDieselDensity = 0.85; //0.85 kg/l   >> should be between 0.82 to 0.85 at 15 degree celsius (average temperature)
	static double gamma = 0.65;
#pragma endregion

#pragma region EnergyConsumptionEfficiencies
	extern Map<TrainTypes::PowerType, Vector<double>> locomotiveBusToTankEff;
	extern Map<TrainTypes::PowerType, Vector<double>> locomotiveWheelToDCBusEff;

	//static Map<TrainTypes::CarType, Vector<double>> carBusToTankEff;
	//static Map<TrainTypes::CarType, Vector<double>> carWheelToDCBusEff;
#pragma endregion

	void readDefaultValuesFromConfigFile(const char configurationFile[] = EC::DefaultECFileName, string tagName = "Efficiency");
	std::vector<std::map<std::string, std::string>> readXML(const char fileName[], const std::string& tagname);
	void extractEnergyMapping(std::vector<std::map<std::string, std::string>> data);
	//std::string getEfficiencyValue(const std::string& tagname, const std::vector<std::string>& attrname, 
	//	const std::vector<std::string>& attrvalue);

	//TODO: read the driveline eff from the xml file
	double getDriveLineEff(int notchNumberIndex, TrainTypes::PowerType powerType);

}



#endif // !NeTrainSim_EnergyConsumption_h