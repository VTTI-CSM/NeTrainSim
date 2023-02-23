//
// Created by Ahmed Aredah
// Version 0.1
//

#ifndef NeTrainSim_Locomotive_h
#define NeTrainSim_Locomotive_h

#include <string>
#include <iostream>
#include "../util/List.h"
#include "../util/Vector.h"
#include "TrainTypes.h"
#include "EnergyConsumption.h"
#include "TrainComponent.h"

using namespace std;



class Locomotive : public TrainComponent{
private:
	inline static const  string DefaultLocomotiveName = "locomotive";
	static constexpr double DefaultLocomotiveMaxSpeed = 100.0 / 3.0;
	static constexpr double DefaultLocomotiveMaxAcheivableNotch = 0;
	static constexpr double DefaultLocomotiveAuxiliaryPower = 0.0;
	static const int DefaultLocomotiveNoOfNotches = 8;
	static const int DefaultLocomotiveNoOfAxiles = 6;
	static const int DefaultLocomotiveType = 0; //diesel 
	static constexpr double DefaultLocomotiveMinTankSOT = 0.2;
	static constexpr double DefaultLocomotiveMinBatterySOC = 0.0;;


public:
	// the max power of the locomotive
	double maxPower;
	// transmission effeciency of the locomotive
	double transmissionEfficiency;
	// the type of the car
	TrainTypes::PowerType powerType;
	// current used notch
	// max speed
	double maxSpeed;

	// decide if the locomotive is adding power to the train
	bool isLocOn = true;
	// number of notches the locomotives have
	int Nmax = 8;
	// the max notch the locomotive is allowed to achieve
	int maxLocNotch = 0;
	// the current notch the locomotive is going by
	int currentLocNotch = 0;
	// the discretized throttlelevels
	Vector<double> discritizedLamda;
	// the current throttle level based on the current notch
	double throttleLevel;
	Vector<double> throttleLevels;
	//the gravitation acceleration
	const double g = 9.8067;

	Locomotive(double locomotiveMaxPower_kw, double locomotiveTransmissionEfficiency,
		double locomotiveLength_m, double locomotiveDragCoef, double locomotiveFrontalArea_sqm,
		double locomotiveWeight_t,
		int locomotiveNoOfAxiles = DefaultLocomotiveNoOfAxiles,
		int locomotivePowerType = DefaultLocomotiveType,
		double locomotiveMaxSpeed_mps = DefaultLocomotiveMaxSpeed,
		int totalNotches = DefaultLocomotiveNoOfNotches,
		int locomotiveMaxAchievableNotch = DefaultLocomotiveMaxAcheivableNotch,
		double locomotiveAuxiliaryPower_kw = DefaultLocomotiveAuxiliaryPower,
		string locomotiveName = DefaultLocomotiveName,
		double batteryMaxCharge_kwh = EC::DefaultLocomotiveBatteryMaxCharge,
		double batteryInitialCharge_perc = EC::DefaultLocomotiveBatteryInitialCharge,
		double tankMaxCapacity_kg = EC::DefaultLocomotiveTankMaxCapacity,
		double tankInitialCapacity_perc = EC::DefaultLocomotiveTankInitialCapacity);

	double getResistance(double trainSpeed) override;
	bool consumeFuel(double EC_kwh, bool isOffGrid, double dieselConversionFactor = EC::DefaultDieselConversionFactor,
		double hydrogenConversionFactor = EC::DefaultHydrogenConversionFactor,
		double dieselDensity = EC::DefaultDieselDensity) override;

	double getEnergyConsumption(double& LocomotiveVirtualTractivePowerdouble, double& speed, double& acceleration,
		double& timeStep);

	string getPowerTypeString();
	TrainTypes::PowerType getPowerTypeEnum(string powertype);
	double getHyperbolicThrottleCoef(double &trainSpeed);
	double getlamdaDiscretized(double &lamda);
	double getDiscretizedThrottleCoef(double& trainSpeed);
	double getThrottleLevel(double &trainSpeed, double& TrainAcceleration, bool &optimize, double &optimumThrottleLevel);
	void updateLocNotch(double &trainSpeed);
	Vector<double> defineThrottleLevels();
	double getTractiveForce(double &frictionCoef, double &trainSpeed, double& trainAcceleration,
		bool &optimize, double &optimumThrottleLevel);
	
	double getNetForce(double &frictionCoef, double &trainSpeed, double &trainAcceleration, 
		bool &optimize, double &optimumThrottleLevel);
	double getSharedVirtualTractivePower(double& trainSpeed, double& trainAcceleration,
		double& sharedWeight, double& sharedResistance);
	

	
	friend ostream& operator<<(ostream& ostr, Locomotive& stud);
};



#endif // !NeTrainSim_Locomotive_h