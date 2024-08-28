//
// Created by Ahmed Aredah
// Version 0.0.1
//

#include <functional>
#include <iostream>
#include "locomotive.h"
#include <math.h>
#include <algorithm> 
#include "../util/vector.h"
#include "energyconsumption.h"
#include "qdebug.h"
#include "traintypes.h"
#include <cstdlib>
#include "../util/utils.h"
#define stringify( name ) #name
using namespace std;


Locomotive::Locomotive(double locomotiveMaxPower_kw,
        double locomotiveTransmissionEfficiency,
        double locomotiveLength_m,
        double locomotiveDragCoef,
        double locomotiveFrontalArea_sqm,
        double locomotiveWeight_t,
        int locomotiveNoOfAxiles,
        int locomotivePowerType,
        double locomotiveMaxSpeed_mps,
        int totalNotches,
        int locomotiveMaxAchievableNotch,
        double locomotiveAuxiliaryPower_kw,
        string locomotiveName,
        double batteryMaxCharge_kwh,
        double batteryInitialCharge_perc,
        double tankMaxCapacity_,
        double tankInitialCapacity_perc,
        double batteryCRate,
        TrainTypes::HybridCalculationMethod hybridCalculationMethod,
        TrainTypes::LocomotivePowerMethod theHybridMethod)
{
    // assign the values to the variables
	this->name = locomotiveName;
    this->maxPower = locomotiveMaxPower_kw;  // in kw
    this->engineMechanicalEfficiency = locomotiveTransmissionEfficiency; // unitless
    this->length = locomotiveLength_m; // in meters
    this->dragCoef = locomotiveDragCoef; // unitless
    this->frontalArea = locomotiveFrontalArea_sqm; // in square meters
    this->currentWeight = locomotiveWeight_t; //in tons
    this->emptyWeight = DefaultLocomotiveEmptyWeight; // in tons
    // check if the empty weight is greater than the current weight,
    // if yes, assign it a value equals to the current weight.
	if (this->emptyWeight > this->currentWeight) {
		this->emptyWeight = this->currentWeight;
    } // end if
    this->noOfAxiles = locomotiveNoOfAxiles; // count
    // convert from digits to enum type
    this->powerType = TrainTypes::iToPowerType(locomotivePowerType);
    this->maxSpeed = locomotiveMaxSpeed_mps; // in meter/second
    this->Nmax = totalNotches; // count
    this->maxLocNotch = locomotiveMaxAchievableNotch; //count
    this->auxiliaryPower = locomotiveAuxiliaryPower_kw; // in kw

    // set the calculation method of the hybrid
    if (TrainTypes::locomotiveHybrid.exist(this->powerType))
    {
        hybridCalcMethod = hybridCalculationMethod;

        if (hybridCalcMethod == TrainTypes::HybridCalculationMethod::MPC)
        {
            // create the combinations
            generateHybridControlActions(discritizationActionStepInIndependentAndMPC, forwardHorizonStepsInMPC);
        }
    }

    // set the battery initial charge percentage if no value is passed
    if (std::isnan(batteryInitialCharge_perc)) {
        if (this->powerType == TrainTypes::PowerType::diesel ||
                this->powerType == TrainTypes::PowerType::dieselElectric) {
            batteryInitialCharge_perc = (double) 0.0;
        }
        else if (this->powerType == TrainTypes::PowerType::biodiesel) {
            batteryInitialCharge_perc = (double) 0.0;
        }
        else if (this->powerType == TrainTypes::PowerType::dieselHybrid) {
            if (EC::DefaultLocomotiveBatteryInitialCharge_DieselHybrid == 0.0){
                batteryInitialCharge_perc =
                    EC::DefaultLocomotiveBatteryRechargeMinSOC_Diesel;
            }
            else {
                batteryInitialCharge_perc =
                    EC::DefaultLocomotiveBatteryInitialCharge_DieselHybrid;
            }
        }
        else if (this->powerType == TrainTypes::PowerType::biodieselHybrid) {
            if (EC::DefaultLocomotiveBatteryInitialCharge_BioDieselHybrid == 0.0)
            {
                batteryInitialCharge_perc =
                    EC::DefaultLocomotiveBatteryRechargeMinSOC_Diesel;
            }
            else {
                batteryInitialCharge_perc =
                    EC::DefaultLocomotiveBatteryInitialCharge_BioDieselHybrid;
            }
        }
        else if (this->powerType == TrainTypes::PowerType::electric) {
            if (EC::DefaultLocomotiveBatteryInitialCharge_Electric == 0.0) {
                batteryInitialCharge_perc =
                    EC::DefaultLocomotiveBatteryRechargeMinSOC_Other;
            }
            else {
                batteryInitialCharge_perc =
                    EC::DefaultLocomotiveBatteryInitialCharge_Electric;
            }
        }
        else if (this->powerType == TrainTypes::PowerType::hydrogenHybrid) {
            if (EC::DefaultLocomotiveBatteryInitialCharge_HydrogenHybrid == 0.0)
            {
                batteryInitialCharge_perc =
                    EC::DefaultLocomotiveBatteryRechargeMinSOC_Other;
            }
            else {
                batteryInitialCharge_perc =
                    EC::DefaultLocomotiveBatteryInitialCharge_HydrogenHybrid;
            }
        }
    }

    // set the battery max charge if no value is passed
    if (std::isnan(batteryMaxCharge_kwh)) {
        if (this->powerType == TrainTypes::PowerType::dieselHybrid) {
            batteryMaxCharge_kwh =
                EC::DefaultLocomotiveBatteryMaxCharge_DieselHybrid;
        }
        else if (this->powerType == TrainTypes::PowerType::biodieselHybrid) {
            batteryMaxCharge_kwh =
                EC::DefaultLocomotiveBatteryMaxCharge_BioDieselHybrid;
        }
        else if (this->powerType == TrainTypes::PowerType::electric) {
            batteryMaxCharge_kwh =
                EC::DefaultLocomotiveBatteryMaxCharge_Electric;
        }
        else if (this->powerType == TrainTypes::PowerType::hydrogenHybrid) {
            batteryMaxCharge_kwh =
                EC::DefaultLocomotiveBatteryMaxCharge_HydogenHybrid;
        }
        else {
            batteryMaxCharge_kwh = 0.0;
        }
    }

    // electric locomotives has only battery
	if (TrainTypes::locomotiveBatteryOnly.exist(this->powerType)) {
        battery.setBattery(batteryMaxCharge_kwh, batteryInitialCharge_perc,
                         EC::DefaultLocomotiveBatteryDOD, batteryCRate);
        tank.SetTank(0.0, 0.0, EC::DefaultLocomotiveMinTankDOD);

    }//end if battery tech
	// diesel and hydrogen have only tanks
	else if (TrainTypes::locomotiveTankOnly.exist(this->powerType)) {
        battery.setBattery(0.0, 0.0, 1.0, batteryCRate);
        tank.SetTank(tankMaxCapacity_, tankInitialCapacity_perc,
                      EC::DefaultLocomotiveMinTankDOD);
    } // end else if tank tech
	// hybrid locomotives have both source of energy
	else {
		double maxRSOC = 0.0;
		double minRSOC = 0.0;
        if (this->hybridCalcMethod == TrainTypes::HybridCalculationMethod::fast)
        {
            if (this->powerType == TrainTypes::PowerType::dieselHybrid ||
                    this->powerType == TrainTypes::PowerType::biodieselHybrid) {
                maxRSOC = EC::DefaultLocomotiveBatteryRechargeMaxSOC_Diesel;
                minRSOC = EC::DefaultLocomotiveBatteryRechargeMinSOC_Diesel;
            } //end if diesel or biodiesel
            else { // hydrogen technology
                maxRSOC = EC::DefaultLocomotiveBatteryRechargeMaxSOC_Other;
                minRSOC = EC::DefaultLocomotiveBatteryRechargeMinSOC_Other;
            } // end else technologies (hydrogen technology)
        }
        else
        {
            maxRSOC = EC::DefaultLocomotiveBatteryRechargeMaxSOC_HybridOpt;
            minRSOC = EC::DefaultLocomotiveBatteryRechargeMinSOC_HybridOpt;
        }
        battery.setBattery(batteryMaxCharge_kwh, batteryInitialCharge_perc,
                         EC::DefaultLocomotiveBatteryDOD,
                         batteryCRate, maxRSOC, minRSOC);
        tank.SetTank(tankMaxCapacity_, tankInitialCapacity_perc,
                      EC::DefaultLocomotiveMinTankDOD);
    } //end else

    // if the locomotive has fuel, calculate the weight of the fuel
    if (TrainTypes::locomotiveTankOnly.exist(this->powerType) ||
            TrainTypes::locomotiveHybrid.exist(this->powerType)) {
        double fuelWeight = 0.0; // in ton
        // if the tender is for diesel
        if (this->powerType == TrainTypes::PowerType::diesel ||
                this->powerType == TrainTypes::PowerType::dieselElectric ||
                this->powerType == TrainTypes::PowerType::dieselHybrid) {
            fuelWeight = tank.getTankInitialCapacity() *
                         EC::DefaultDieselDensity;
        }
        // if the tender is for biodiesel
        else if (this->powerType == TrainTypes::PowerType::biodiesel ||
                 this->powerType == TrainTypes::PowerType::biodieselHybrid) {
            fuelWeight = tank.getTankInitialCapacity() *
                         EC::DefaultBioDieselDensity;
        }
        else if (this->powerType == TrainTypes::PowerType::hydrogenHybrid) {
            fuelWeight = tank.getTankInitialCapacity() *
                         EC::DefaultHydrogenDensity;
        }
        // check if the fuel weight provided is not correct, update it.
        // maybe the use accounts for other weights on the locomotive.
        // and the gross locomotive weight is higher than the empty weight +
        // fuel weight
        if ((this->currentWeight - this->emptyWeight) < fuelWeight) {
            this->currentWeight = this->emptyWeight + fuelWeight;
        }
    }

	this->trackCurvature = 0;
	this->trackGrade = 0;
    this->hostLink = std::shared_ptr<NetLink>(); // assign empty placeholder
    this->discritizedLamda.push_back(0.0); // add idle throttle level
    // define all the throttle levels
    this->throttleLevels = this->defineThrottleLevels();
    if (theHybridMethod == TrainTypes::LocomotivePowerMethod::notApplicable &&
            TrainTypes::locomotiveHybrid.exist(this->powerType)) {
        this->hybridMethod = TrainTypes::LocomotivePowerMethod::parallel;
    }
    else if (TrainTypes::locomotiveHybrid.exist(this->powerType)) {
        this->hybridMethod = theHybridMethod;
    }
    else {
        this->hybridMethod = TrainTypes::LocomotivePowerMethod::notApplicable;
    }

};

Locomotive::~Locomotive()
{}

void Locomotive::setLocomotiveHybridParameters(
    TrainTypes::HybridCalculationMethod method,
    int discitizationCount,
    int forwardsteps)
{
    hybridCalcMethod = method;
    forwardHorizonStepsInMPC = forwardsteps;
    discritizationActionStepInIndependentAndMPC = discitizationCount;

    if (TrainTypes::locomotiveHybrid.exist(this->powerType))
    {
        hybridCalcMethod = method;

        if (hybridCalcMethod == TrainTypes::HybridCalculationMethod::MPC)
        {
            // create the tree
            generateHybridControlActions(discritizationActionStepInIndependentAndMPC,
                                         forwardHorizonStepsInMPC);
        }
    }
}

double Locomotive::getHyperbolicThrottleCoef(double & trainSpeed)
{
    double dv = 0.0; //, um;


    // ratio of current train speed by the max loco speed
    dv = trainSpeed / this->maxSpeed;

    double lambda = (double)1.0 / (1.0 + exp(-7.82605 * (dv - 0.42606)));

    if (lambda < 0.0){
        return 0.0;
    }
    else if (lambda > 1.0) {
        return 1.0;
    }

    return lambda;

    // um = 0.05 * this->maxSpeed;
    // if (dv >= 1.0) {
    //     return 1.0;
    // }

    // if (trainSpeed <= um )
    // {
    //     return abs((dv)/((0.001)+(0.05/(1-dv))+(0.030*(dv))));
    // }
    // else {
    //     return 1.0;
    // }

};

double Locomotive::getlamdaDiscretized(double &lamda)
{
    int minI;
	double lamdaD;
	int index;
	//preinitialize the variables
	Vector<double> dfr(this->Nmax);
	Vector<double> lamdaDlst(this->Nmax);
	//define the throttle levels
	for (int N = 1; N <= this->Nmax; N++) {
		lamdaD = Utils::power((double)N / (double)this->Nmax, 2);
		index = N - 1;
		lamdaDlst[index] = lamdaD;
		dfr[index] = abs(lamda - lamdaD);
	};
	// find the argmin
	minI = dfr.argmin();
	return lamdaDlst[minI];
};

double Locomotive::getDiscretizedThrottleCoef(double &trainSpeed)
{
    double lmda, lamdaDiscretized;
    if (this->maxLocNotch == 0) {
        this->maxLocNotch = this->Nmax;
    }
    if (this->maxLocNotch > this->Nmax) {
        this->maxLocNotch = this->Nmax;
    }
    // get the lambda value from the continous equation
    lmda = this->getHyperbolicThrottleCoef(trainSpeed);
    // discritize the lambda value
    lamdaDiscretized = this->getlamdaDiscretized(lmda);

    if (! this->discritizedLamda.exist(lamdaDiscretized)) {
        this->discritizedLamda.push_back(lamdaDiscretized);
        this->discritizedLamda.sort();
    }
    //get the Notch # that we drive by now
    int crntLocNotch = this->discritizedLamda.index(lamdaDiscretized);
    //if the minimum is too high, set it to the highest possible
    // value (Restrict max notch to #)
    if (crntLocNotch > this->maxLocNotch) {
        crntLocNotch = this->maxLocNotch;
        lamdaDiscretized = this->discritizedLamda[crntLocNotch];
    }
    return lamdaDiscretized;
}

double Locomotive::getThrottleLevel(double & trainSpeed,
                                    bool &optimize,
                                    double &optimumThrottleLevel)
{
	double currentThrottleLevel = 0;
	double throttleL = 0;
	throttleL = getDiscretizedThrottleCoef(trainSpeed);
	if (optimize) {
		if (optimumThrottleLevel < 0){
			optimumThrottleLevel = this->throttleLevels.max();
		}
		currentThrottleLevel = min(optimumThrottleLevel, throttleL);
	}
	else {
		currentThrottleLevel = throttleL;
	}
	return currentThrottleLevel;
}

void Locomotive::updateLocNotch(double &trainSpeed)
{
	if (trainSpeed == 0.0 || !this->isLocOn) { this->currentLocNotch = 0; }
	else {
		// get the discretized Throttle Level and compare it to the list
        double lamdaDiscretized =
            this->getDiscretizedThrottleCoef(trainSpeed);
		// the index of it is the current Loc Notch
        this->currentLocNotch =
            this->throttleLevels.index(lamdaDiscretized) + 1;
		// readjust its value if it is higher than what the locomotive have
        if (this->currentLocNotch > this->maxLocNotch) {
            this->currentLocNotch = this->maxLocNotch;
        }
	}

}

void Locomotive::reducePower(double &reductionFactor)
{
    // get the reduction factor
    // the max power supplied/ current demand power
    this->locPowerReductionFactor = reductionFactor;
    //restrict the reduction to the lower notch only.
    int lowerNotch = this->currentLocNotch - 2; // get index of previous notch
    lowerNotch = max(lowerNotch, 0);
    double lowerNotchLambda = this->throttleLevels[lowerNotch];
    if (lowerNotchLambda > this->locPowerReductionFactor) {
        this->locPowerReductionFactor = lowerNotchLambda;
    }
}

void Locomotive::resetPowerRestriction()
{
    this->locPowerReductionFactor = 1.0;
}

Vector<double> Locomotive::defineThrottleLevels()
{
	double lamdaD;
	int index;
	Vector<double> lamdaDlst(this->Nmax,0);
	//for each notch, calculate the throttle level
	for (int N = 1; N <= this->Nmax; N++) {
		double div = (double)N / (double)this->Nmax;
		lamdaD = Utils::power(div,2);
		index = N - 1;
		lamdaDlst[index] = lamdaD;
	};
	return lamdaDlst;
}

double Locomotive::getTractiveForce_N(double &frictionCoef,
                                              double &trainSpeed,
                                              bool &optimize,
                                              double &optimumThrottleLevel)
{
	if (!this->isLocOn) {
		return 0;
	}
    double f1,f = 0;
    f1 = frictionCoef * this->currentWeight * 1000 * this->g;
	if (trainSpeed == 0) {
        this->maxTractiveForce_N = f1;
		return f1;
	}
    else {
        // the notch position controls how much of the engine to be used
        // which could be regarded as power portion
        double throttleLevel =
            this->getThrottleLevel(trainSpeed, optimize,
                                   optimumThrottleLevel);

        // calculate the tractive forces at the wheels.
        // the maxPower is the power rating of the locomotive at the wheels
        f = min((this->locPowerReductionFactor * 1000.0 *
                 this->engineMechanicalEfficiency *
                 throttleLevel *
                 // EC::getLocomotivePowerReductionFactor(this->powerType)
                 (this->maxPower / trainSpeed)) * this->isLocOn, f1);

        // save the value for later reference
        this->maxTractiveForce_N = f;
		return f;
	};
}

double Locomotive::getSharedVirtualTractivePower_W(double &trainSpeed,
                                                 double& trainAcceleration,
                                                 double& sharedWeight,
                                                 double& sharedResistance)
{
    // if the locomotive does not work, provide no power
	if (! this->isLocOn) {
		return 0.0;
	}

    // power (watt) = [(weight * acceleration) + resistance ] * speed
    return ((sharedWeight * trainAcceleration) + sharedResistance) *
           trainSpeed;
}

double
Locomotive::getSharedVirtualTractivePowerAtEngine_W(double &trainSpeed,
                                                    double& trainAcceleration,
                                                    double& sharedWeight,
                                                    double& sharedResistance)
{
    if (! this->isLocOn) {
        return 0.0;
    }
    // power at the wheels
    // power (watt) = [(weight * acceleration) + resistance ] * speed
    double p = ((sharedWeight * trainAcceleration) + sharedResistance) *
               trainSpeed;

    // calculate power portion with power at the wheels
    double pp = getUsedEnginePowerPortion(p);

    // calculate the driveline eff to transfer the power to the engine
    double e = EC::getDriveLineEff(trainSpeed, pp, powerType, hybridMethod);

    // transfer the power from the wheels to the engine
    return p / e;
}

// Calculate the effeciency of regeneration from deceleration
// if the locomotive supports it
double Locomotive::getRegenerativeEffeciency(
                         double& LocomotiveVirtualTractivePower,
                         double& trainAcceleration,
                         double &trainSpeed)
{
    // if it is a regenerative locomotive, regenerate energy
    if (TrainTypes::locomotiveRechargableTechnologies.exist(this->powerType)) {
        // get regenerative eff
        double regenerativeEff = 0;
        //if there is an deceleration value, calculate the eff directly
        if (trainAcceleration != 0) {
            regenerativeEff = (1 / (exp(EC::gamma/ abs(trainAcceleration))));
        }
        //if the deceleration is zero, we cannot divide by zero
        else {
            //Calculate virtual acceleration
            double virtualAcceleration = LocomotiveVirtualTractivePower /
                                         (trainSpeed * this->currentWeight);
            // calculate regenerative eff
            // if virtual deceleration is not zero, calculate eff
            // from virtual deceleration
            if (virtualAcceleration != 0) {
                regenerativeEff = 1 / (exp(EC::gamma /
                                           abs(virtualAcceleration)));
            }
            else {
                // if there is no acceleration at all (no speed change),
                // return 0 in this case
                regenerativeEff = 0.0;
            }
        }
        return regenerativeEff;
    }
    // if it is diesel-electric, do no regenerate electricity
    else {
        return 0.0;
    }
}



std::pair<double, double> Locomotive::getEnergyConsumptionAtWheels(
    double& LocomotiveVirtualTractivePower_W,
    double& trainAcceleration,
    double &trainSpeed,
    double &timeStep)
{
	// if the locomotive is turned off already, do not consume anything
	if (!this->isLocOn) {
        return std::make_pair(0.0, 0.0);
	}
	// else calculate how much to consume in watt (kg * m^3/s^3)
    double tractivePower = LocomotiveVirtualTractivePower_W;

    // the conversion from W to kwh
    double unitConversionFactor = timeStep / (double)(3600.0 * 1000.0);

    // if there is not power used, only return the auxiliary energy
	if (tractivePower == 0) {
        return std::make_pair(0.0, this->auxiliaryPower * unitConversionFactor);
	}
    // if there is a power used, return both the drive energy
    // and the auxiliary energy
    else if(tractivePower > 0) {
        double EC = (tractivePower * unitConversionFactor);
        return std::make_pair(EC, auxiliaryPower * unitConversionFactor);
	}
    // if the power is negative, that means it is a regenerative energy
    else {
        // get regenerative eff
        double regenerativeEff =
            this->getRegenerativeEffeciency(tractivePower,
                                            trainAcceleration, trainSpeed);

        // return the regenerated energy and the auxiliary energy consumption
        return std::make_pair(tractivePower * regenerativeEff * unitConversionFactor,
                              auxiliaryPower * unitConversionFactor);
	}
}

// Gets the max gross power at the wheel in Watt.
// There is no efficiency considered in this function.
double
Locomotive::getMaxGrossPowerAtWheel_W()
{
    return this->maxPower * this->engineMechanicalEfficiency * 1000.0f;
}

// Gets the power of power portion at the wheels in Watt.
// There is no efficiency considered in this function.
double Locomotive::getPowerOfPowerPortionAtWheels_W(double powerPortion) {
    return getMaxGrossPowerAtWheel_W() * powerPortion;
}

// Gets the power of power portion at the engine in Watt.
// This accounts for the efficiency of the DC bus, Engine, and
// battery if the locomotive has one.
double Locomotive::getPowerOfPowerPortionAtEngine_W(
    double powerPortion, double trainSpeed)
{
    // calculate efficiency
    double eff =
        EC::getDriveLineEff(trainSpeed, powerPortion, powerType, hybridMethod);

    // Transfer the power at the engine
    return (getMaxGrossPowerAtWheel_W() * powerPortion) / eff;
}

// Gets the power portion used by the engine.
// This is calculated by dividing the provided power by the power
// rating of the locomotive at the wheels.
// There is not efficiency considered in this function.
double
Locomotive::getUsedEnginePowerPortion(
    double LocomotiveVirtualTractivePowerAtWheels_W)
{
    // check the virtual power is +ve, if -ve return no power portion
    if (LocomotiveVirtualTractivePowerAtWheels_W <= 0.0)
    {
        return 0.0;
    }

    // the max power of the locomotive in watt
    double limitingPower = getMaxGrossPowerAtWheel_W();

    double pPortion =
        std::min(
        1.0,
        (LocomotiveVirtualTractivePowerAtWheels_W / limitingPower));

    // // power portion watt/watt (unitless)
    // // it is bounder by 1.0 because at the begining of the deceleration, the
    // // power is very high
    // this->usedPowerPortion =
    //     {LocomotiveVirtualTractivePowerAtWheels_W, pPortion};

    return pPortion;
}


double Locomotive::getMaxTheoriticalBatteryRechargeEnergyAtBattery_kWh(
    double timeStep,
    double trainSpeed,
    double rechargePortion,
    double mainPowerPortion,
    double approximateLocomotivePowerAtWheels_W)
{
    if (rechargePortion <= 0.0 ||
        mainPowerPortion <= 0.0 ||
        mainPowerPortion >= 1.0)
    {
        return 0.0;
    }

    double maxLocomotivePowerPortion = 1.0;

    // 1) get the rest of the power portion, the locomotive can provide to
    //    recharging the battery
    double rchrgPwrAtWls = maxLocomotivePowerPortion - mainPowerPortion;
    double rchrgEAtWls =
        EC::convertPower_W_ToE_kWh(rchrgPwrAtWls, timeStep);
    // transfer the energy to the DC bus
    double rchrgEAtDC =
        EC::convertECFromWheelToDCBus(rchrgEAtWls,
                                      approximateLocomotivePowerAtWheels_W,
                                      trainSpeed);


    // transfer the energy to the engine at max loco power
    double rchrgEAtEng =
        EC::convertECFromDCBusToEngine(rchrgEAtDC, maxLocomotivePowerPortion,
                                       approximateLocomotivePowerAtWheels_W,
                                       powerType, hybridMethod);

    // transfer the energy to the battery at max loco power
    double rchrgEAtBat =
        EC::convertECFromEngineToBattery_kWh(rchrgEAtEng,
                                             maxLocomotivePowerPortion,
                                             powerType, hybridMethod);

    // 2) get the max recharge current that the battery accepts
    // calculate the max energy the battery accepts
    double requiredE = battery.getBatteryMaxRecharge(timeStep);

    // 3) the energy is the min of both
    double m = rechargePortion * std::min(rchrgEAtBat, requiredE);

    // the amount that should be recharged to the battery
    return std::max(m, 0.0);

}

double Locomotive::getMaxRechargeBatteryEnergyAtBattery_kWh(
    double timeStep,
    double trainSpeed,
    double rechargePortion,  // portion of recharge (percentage or recharge of max allowable recharge)
    double totalPowerPortion, // the total power the engine will be running at with main required power and battery recharge power
    double rechargePowerPortion, // power portion of the reacharge power to the locomotive power
    double approximateLocomotivePowerAtWheels_W) // the approx required power at the wheel
{
    if (rechargePortion <= 0.0)
    {
        return 0.0;
    }

    // 1) calculate the max Energy the locomotive can generate for the battery

    // calculate the power at the wheels that is used for recharging the bat
    double rchrgPwrAtWls =
        getPowerOfPowerPortionAtWheels_W(rechargePowerPortion);

    // calculate the energy of that power
    double rchrgEAtWls =
        EC::convertPower_W_ToE_kWh(rchrgPwrAtWls, timeStep);

    // transfer the energy to the DC bus
    double rchrgEAtDC =
        EC::convertECFromWheelToDCBus(rchrgEAtWls,
                                  approximateLocomotivePowerAtWheels_W,
                                  trainSpeed);

    // transfer the energy to the engine
    double rchrgEAtEng =
        EC::convertECFromDCBusToEngine(rchrgEAtDC, totalPowerPortion,
                                       approximateLocomotivePowerAtWheels_W,
                                       powerType, hybridMethod);

    // transfer the energy to the battery
    double rchrgEAtBat =
        EC::convertECFromEngineToBattery_kWh(rchrgEAtEng, totalPowerPortion,
                                             powerType, hybridMethod);


    // 2) get the max recharge current that the battery accepts

    // calculate the max energy the battery accepts
    double requiredE = battery.getBatteryMaxRecharge(timeStep);

    // 3) the energy is the min of both
    double m = rechargePortion * std::min(rchrgEAtBat, requiredE);

    // the amount that should be recharged to the battery
    return std::max(m, 0.0);
}


double
Locomotive::getUsedEnginePowerPortionForHybrids(double timeStep,
                                                double trainSpeed,
                                                double LocomotiveVirtualTractivePowerAtWheels_W,
                                                double EC_kwh_atWheels,
                                                double engineConsumePortion,
                                                double batteryRechargePortion)
{
    // initial value of power portion at the wheels
    double enginePowerPortion =
        getUsedEnginePowerPortion(LocomotiveVirtualTractivePowerAtWheels_W);

    double enginePower_w = LocomotiveVirtualTractivePowerAtWheels_W;

    double positiveVPower = 1.0; // this is for recharge so it must be +ve

    double EC_kWh_atDC = 0.0;

    int i = 0;
    while(i < timeOut) {
        // convert total EC to DC
        EC_kWh_atDC =
            EC::convertECFromWheelToDCBus(EC_kwh_atWheels,
                                          enginePowerPortion,
                                          trainSpeed);

        // 1) Calculate energy consumption at the engine and battery sources

        double engineEC_kWh_atDC =
            EC_kWh_atDC * engineConsumePortion;
        double engineEC_kWh_atEngine =
            EC::convertECFromDCBusToEngine(engineEC_kWh_atDC,
                                           enginePowerPortion,
                                           LocomotiveVirtualTractivePowerAtWheels_W,
                                           powerType, hybridMethod);

        double batteryEC_kWh_atDC = EC_kWh_atDC - engineEC_kWh_atDC;
        double batteryEC_kWh_atBat =
            EC::convertECFromDCToBattery(batteryEC_kWh_atDC, powerType);

        // 2) Calculate energy used for battery recharge
        double batteryRechargePowerPortion =
            getMaxPowerPortionUsedForBatteryRecharge(
            timeStep, trainSpeed, batteryEC_kWh_atBat, enginePower_w);

        double batteryRecharge_kWh_atBattery =
            getMaxRechargeBatteryEnergyAtBattery_kWh(
                timeStep, trainSpeed, batteryRechargePortion,
                enginePowerPortion, batteryRechargePowerPortion,
                enginePower_w);


        // calculate portion of energy required for the battery recharge
        double batteryRecharge_kWh_atDC =
            EC::convertECFromBatteryToDC(batteryRecharge_kWh_atBattery,
                                         powerType);
        double batteryRecharge_kWh_atEngine =
            EC::convertECFromDCBusToEngine(batteryRecharge_kWh_atDC,
                                           enginePowerPortion,
                                           positiveVPower,
                                           powerType, hybridMethod);

        double totalECAtEng_kWh =
            (engineEC_kWh_atEngine + batteryRecharge_kWh_atEngine);
        double totalECAtDC_kWh =
            EC::convertECFromEnginToDCBus(
                totalECAtEng_kWh, enginePowerPortion,
                LocomotiveVirtualTractivePowerAtWheels_W,
                powerType,
                hybridMethod);

        double totalECAtWheels_kWh =
            EC::convertECFromDCBusToWheels(
            totalECAtDC_kWh, enginePower_w, trainSpeed);

        enginePower_w =
            EC::convertE_kWh_ToPower_W(totalECAtWheels_kWh, timeStep);

        double powerP = getUsedEnginePowerPortion(enginePower_w);
        double accuratePowerPortion =
            std::min(1.0, powerP);

        // check if the calculate power portion is correct
        if (Utils::isValueInRange(enginePowerPortion,
                                  accuratePowerPortion - 0.025,
                                  accuratePowerPortion + 0.025))
        {
            break;
        }

        // Assign the correct power portion and recalculate
        enginePowerPortion = accuratePowerPortion;

        i++;
    }
    return getUsedEnginePowerPortion(enginePower_w);
}

// double Locomotive::getMaxRechargeEnergy(double timeStep, double trainSpeed,
//                                         double LocomotiveVirtualTractivePower)
// {

//     // get the actual power portion used at this time step
//     double locomotiveUsedPower =
//         this->getUsedEnginePowerPortion(trainSpeed,
//                                   LocomotiveVirtualTractivePower);
//     // get the feasible range of the locomotive power portion that
//     // could be used to recharge the battery
//     double locomotiveFeasiblePower = 0.0;
//     if (hybridCalcMethod == TrainTypes::HybridCalculationMethod::fast)
//     {
//         // get locomotive power portion required to recharge the locomotive
//         double generatorMaxRequired =
//             EC::getRequiredGeneratorPowerPortionForBatteryRecharge(battery.getBatteryStateOfCharge());

//         // the locomotive should not exceed the generator max required
//         locomotiveFeasiblePower = min(1.0 - locomotiveUsedPower,
//                                       generatorMaxRequired);
//     }
//     else
//     {
//         locomotiveFeasiblePower = 1.0 - locomotiveUsedPower;
//     }


//     // portion of the max locomotive power
//     return (this->maxTractiveForce_N * trainSpeed * timeStep / ((double)3600.0)) *
//             locomotiveFeasiblePower;
// }




// double Locomotive::getRequiredEnergyForRechargeFromGenerator(
//     double timeStep,
//     double trainSpeed,
//     double powerPortion,
//     double rechargePortion,
//     double LocomotiveVirtualTractivePower)
// {
//     double minE = getMaxRechargeBatteryEnergy(
//         timeStep, trainSpeed, rechargePortion, LocomotiveVirtualTractivePower);

//     if (minE > 0.0) {
//         double minEC =
//             convertECFromBatteryToEngine(minE, powerPortion);
//         return minEC;
//     }
//     return 0.0;
// }

void Locomotive::rechargeBatteryByFlowPortion(double timeStep,
                                              double trainSpeed,
                                              double totalPowerPortion,
                                              double rechargePowerPortion,
                                              double fuelConversionFactor,
                                              double fuelDensity,
                                              double LocomotiveVirtualTractivePower,
                                              std::function<
                                                  std::pair<bool, double>(double,
                                                                          double,
                                                                          double)>
                                                  ConsumeFuelFunc)
{

    // double ec = getRequiredEnergyForRechargeFromGenerator(
    //     timeStep, trainSpeed,
    //     totalPowerPortion, rechargePortion,
    //     LocomotiveVirtualTractivePower);
    double p = getPowerOfPowerPortionAtWheels_W(rechargePowerPortion);
    double rechargeECAtWheels = EC::convertPower_W_ToE_kWh(p, timeStep);
    double ECAtDC =
        EC::convertECFromWheelToDCBus(rechargeECAtWheels,
                                      LocomotiveVirtualTractivePower,
                                      trainSpeed);
    double ECAtEng =
        EC::convertECFromDCBusToEngine(ECAtDC, totalPowerPortion,
                                       LocomotiveVirtualTractivePower,
                                       powerType, hybridMethod);

    // if the locomotive has fuel, consume the required amound of energy
    // from Diesel
    if (ConsumeFuelFunc(ECAtEng,
                        fuelConversionFactor,
                        fuelDensity).first) {
        this->cumEnergyConsumed -= ECAtEng;
        // recharge battery
        double rechargeE =
            EC::convertECFromEngineToBattery_kWh(ECAtEng, totalPowerPortion,
                                                 powerType, hybridMethod);
        battery.rechargeBatteryForHybrids(timeStep, rechargeE);
    }
}

hybridEnergyDistribution Locomotive::getCheapestHeuristicHybridCost(
    double timeStep,
    double trainSpeed,
    Vector<double> virtualPower_W_atWheels,
    Vector<double> EC_kwh_atWheels,
    Vector<hybridEnergyDistribution> ECDistribution,
    Vector<double> routeProgress)
{
    double cheapestCost = std::numeric_limits<double>::max();
    hybridEnergyDistribution cheapestComb;


    // loop over the combinations to check which is cheaper
    for (int ci = 0; ci < ECDistribution.size(); ci++)
    {
        double batterySOCDeviation = (battery.getBatteryInitialCharge() -
                                      battery.getBatteryCurrentCharge()) /
                                     battery.getBatteryMaxCharge();

        double cost = 0;

        // loop over the horizon to get a heuristic cost
        // assuming the same actions will continue
        for (int hS = 0; hS < EC_kwh_atWheels.size(); hS++)
        {
            // get the battery deviation as a result of this combination first
            batterySOCDeviation -=
                ((1.0 - ECDistribution[ci].engineConsumePortion) *
                                    EC_kwh_atWheels[hS] /
                                    battery.getBatteryMaxCharge());
            double batteryRecharge_EC_kWh =
                getMaxTheoriticalBatteryRechargeEnergyAtBattery_kWh(
                    timeStep, trainSpeed,
                    ECDistribution[ci].rechargePortion,
                    ECDistribution[ci].engineConsumePortion,
                    virtualPower_W_atWheels[hS]);
            batterySOCDeviation +=
                (batteryRecharge_EC_kWh /
                 battery.getBatteryMaxCharge());

            cost +=
                computeHybridsCost2(timeStep, trainSpeed,
                                    virtualPower_W_atWheels[hS],
                                    EC_kwh_atWheels[hS],
                                    ECDistribution[ci],
                                    routeProgress[hS], batterySOCDeviation);

        }

        if (cost < cheapestCost)
        {
            cheapestCost = cost;
            cheapestComb =
                {ECDistribution[ci].engineConsumePortion,
                 ECDistribution[ci].rechargePortion};
        }
    }
    hybridCost = cheapestCost;
    return cheapestComb;
}


double Locomotive::computeHybridsCost2(
    double timeStep,
    double trainSpeed,
    double virtualPower_W,
    double EC_kwh_atWheels, // EC at wheels
    hybridEnergyDistribution energyDistribution,
    double routeProgress,
    double batterySOCDeviation)
{
    double enginePowerPortion = getUsedEnginePowerPortionForHybrids(
        timeStep, trainSpeed, virtualPower_W, EC_kwh_atWheels,
        energyDistribution.engineConsumePortion,
        energyDistribution.rechargePortion);

    // initial value of power portion at the wheels
    // double enginePowerPortion = getUsedEnginePowerPortion(trainSpeed, virtualPower_W);
    double positiveVPower = 1.0; // this is for recharge so it must be +ve

    // Get the efficiency of the engine the maximize the eff
    double maxEfficientPowerPortion =
        EC::getMaxEffeciencyRange(this->powerType).highestPoint;

    double EC_kWh_atDC = 0.0;

    // Initialize cost
    double cost = 0.0;

    double batteryDeviation = 0.0;


    // convert total EC to DC
    EC_kWh_atDC =
        EC::convertECFromWheelToDCBus(EC_kwh_atWheels,
                                  enginePowerPortion,
                                  trainSpeed);

    // Calculate portion of energy provided by the engine
    double engineEC_kWh_atDC =
        EC_kWh_atDC * energyDistribution.engineConsumePortion;
    double engineEC_kWh_atEngine =
        EC::convertECFromDCBusToEngine(engineEC_kWh_atDC,
                                   enginePowerPortion,
                                   virtualPower_W, powerType, hybridMethod);

    // Calculate portion of energy provided by the battery
    double batteryEC_kWh_atDC =
        EC_kWh_atDC * (1.0 - energyDistribution.engineConsumePortion);
    double batteryEC_kWh_atBattery =
        EC::convertECFromDCToBattery(batteryEC_kWh_atDC, powerType);

    double batteryRecharge_EC_kWh =
        getMaxTheoriticalBatteryRechargeEnergyAtBattery_kWh(
            timeStep, trainSpeed,
            energyDistribution.rechargePortion,
            energyDistribution.engineConsumePortion,
            virtualPower_W);

    // calculate portion of energy required for the battery recharge
    double batteryRecharge_kWh_atBattery =
        energyDistribution.rechargePortion * batteryRecharge_EC_kWh;
    double batteryRecharge_kWh_atDC =
        EC::convertECFromBatteryToDC(batteryRecharge_kWh_atBattery, powerType);
    double batteryRecharge_kWh_atEngine =
        EC::convertECFromDCBusToEngine(batteryRecharge_kWh_atDC,
                                   enginePowerPortion,
                                   positiveVPower, powerType, hybridMethod);

    double powerP = (engineEC_kWh_atEngine + batteryRecharge_kWh_atEngine) /
                (maxPower * (timeStep / 3600.0));



    bool isBattery_kwh_overMaxOutput =
        (batteryEC_kWh_atBattery > battery.getBatteryMaxDischarge(timeStep) ||
         !battery.isBatteryDrainable(batteryEC_kWh_atBattery));

    // Apply Penalties if the energy required from the battery is larger
    // than what the battery can provide.
    if (isBattery_kwh_overMaxOutput)
    {

        cost += 100000.0; // very high cost
    }
    if (!battery.isBatteryRechargable()) {
        cost + 1000.0;
    }
    // Apply penalties if the energy required from the generator is larger
    // than what the generator can provide
    if (powerP > 1.0)
    {
        cost += 100000000.0; // very high cost
    }
    if (std::isnan(batterySOCDeviation))
    {
        batteryDeviation =
            (battery.getBatteryInitialCharge() -
             (battery.getBatteryCurrentCharge() + batteryRecharge_kWh_atBattery)) /
            battery.getBatteryMaxCharge();
    }
    else
    {
        batteryDeviation = batterySOCDeviation / battery.getBatteryMaxCharge();
    }


    // Calculate the deviation from the max efficiency power portion
    double efficiencyDeviation =
        0.025f *
        std::abs(EC::getDriveLineEff(trainSpeed, enginePowerPortion,
                                    powerType, hybridMethod) -
                 EC::getDriveLineEff(trainSpeed, maxEfficientPowerPortion,
                                    powerType, hybridMethod)
                );



    // prefer using battery so reduce the battery energy by a factor of
    // its efficiency
    cost += efficiencyDeviation;
        //std::pow(efficiencyDeviation * 100.0 , 2.0);


    double deviationPenalty = 0.0;

    // undercharge the battery
    if (batteryDeviation < 0.0)
    {
        double underchargePenalty = (1.0f + routeProgress) * 10.0f;
        deviationPenalty = underchargePenalty * batteryDeviation;
    }


    // Incorporate the dynamic priority for recharging as the journey progresses
    // cost += (1.0 - routeProgress) * batteryDeviation;
    // cost += rechargePortion * std::pow(batteryDeviation + 0.02, routeProgress);//batteryDeviation * batteryDiviationPenalty;
    cost += deviationPenalty;

    return cost;

}


// double Locomotive::computeHybridsCost(double timeStep,
//                                       double EC_kwh_atWheels,
//                                       double generatorConsumptionPortion,
//                                       double rechargePortion,
//                                       double routeProgress,
//                                       double batterySOCDeviation)
// {
    // double trainSpeed = 0; //TODO
    // double enginePowerPortion = 1.0; //TODO
    // double virtualPower_W = 1.0;
    // double positiveVPower = 1.0; // no need to calculate it

    // double notChargingBasePenalty = 1.0;
    // double batteryDiviationPenalty = 1.0;
    // double efficiencyDeviationPenalty = 1.0;
    // double underchargePenaltyBase = 1.0;
    // double overchargePenaltyBase = 1.0;

    // // Calculate the efficiency of the generator at the current power portion
    // std::pair<double, double> maxEffRange =
    //     EC::getMaxEffeciencyRange(this->powerType);
    // double maxEfficientPowerPortion =
    //     (maxEffRange.first + maxEffRange.second) / 2.0;

    // double EC_kWh_atDC = 0.0;

    // while(true) {
    //     EC_kWh_atDC =
    //         convertECFromWheelToDCBus(EC_kwh_atWheels,
    //                                   enginePowerPortion,
    //                                   trainSpeed);

    //     // Calculate energy for engine
    //     double engineEC_kWh_atDC =
    //         EC_kWh_atDC * generatorConsumptionPortion;
    //     double engineEC_kWh_atEngine =
    //         convertECFromDCBusToEngine(engineEC_kWh_atDC,
    //                                    virtualPower_W,
    //                                    trainSpeed);

    //     double accuratePowerPortion =
    //         std::min(1.0,
    //                  engineEC_kWh_atEngine /
    //                      (maxPower * (timeStep / 3600.0)));

    //     // check if the calculate EC is from correct power portion
    //     if (Utils::isValueInRange(enginePowerPortion,
    //                               accuratePowerPortion - 0.025,
    //                               accuratePowerPortion + 0.025))
    //     {
    //         break;
    //     }

    //     // Assign the correct power portion and recalculate
    //     enginePowerPortion = accuratePowerPortion;
    // }

    // double batteryEC_kWh_atDC =
    //     EC_kWh_atDC * (1.0 - generatorConsumptionPortion);
    // double batteryEC_kWh_atBattery =
    //     convertECFromDCToBattery(batteryEC_kWh_atDC);

    // bool isBattery_kwh_overMaxOutput =
    //     (batteryEC_kWh_atBattery > battery.getBatteryMaxDischarge(timeStep) ||
    //      !battery.isBatteryDrainable(batteryEC_kWh_atBattery));

    // double batteryRecharge_kWh_atBattery =
    //     rechargePortion * battery.getBatteryMaxRecharge(timeStep);
    // double batteryRecharge_kWh_atDC =
    //     convertECFromBatteryToDC(batteryRecharge_kWh_atBattery);
    // double batteryRecharge_kWh_atEngine =
    //     convertECFromDCBusToEngine(batteryRecharge_kWh_atDC,
    //                                positiveVPower,
    //                                trainSpeed);

    // // Calculate energy recharge at the wheel
    // double generatorPowerPortionForRecharge =
    //     (batteryRecharge_kWh_atEngine) /
    //     (this->maxPower * (timeStep/3600.0));

    // // calculate the efficiency of the generator when providing
    // // energy to both battery and driveline
    // double totalPowerPortionAtGenerator =
    //     std::min(1.0 , enginePowerPortion +
    //                       generatorPowerPortionForRecharge);
    // double generatorEffWhenCharging =
    //     EC::getGeneratorEff(this->powerType, totalPowerPortionAtGenerator);

    // // // Calculate the generator energy as a main source
    // // // and generator energy for recharge
    // // double generatorEnergy_kWh_atTank =
    // //     generatorEnergy_kWh *
    // //     EC::getBatteryEff(this->powerType) / generatorEffWhenCharging;
    // // double generatorEnergy_kWh_forRecharge =
    // //     batteryRecharge_kWh / generatorEffWhenCharging;

    // // // Calculate the normalize generator energy consumption
    // // // and the normalized battery consumption at the tank
    // // double generatorEnergyPortionAtTank =
    // //     generatorEnergy_kWh_atTank / (this->maxPower * (timeStep/3600.0));
    // // // the battery effeciency here is only to
    // // // give higher priority since it will reduce the EC
    // // double rechargeEneryPortionAtTank =
    // //     generatorEnergy_kWh_forRecharge *
    // //     EC::getBatteryEff(this->powerType) /
    // //     (this->maxPower * (timeStep/3600.0));
    // // double batteryEnergyPortion =
    // //     (batteryEnergy_kWh * EC::getBatteryEff(this->powerType)) /
    // //     battery.getBatteryMaxDischarge(timeStep);

    // // Calculate the deviation from the max efficiency power portion
    // double efficiencyDeviation =
    //     std::abs(totalPowerPortionAtGenerator - maxEfficientPowerPortion);




    // // Initial cost based on energy consumption
    // // prefer using battery so reduce the battery energy by a factor of
    // // its efficiency
    // double cost =
    //     // 2.0 * generatorEnergyPortionAtTank +                                    // energy at the generator with double cost
    //     // batteryEnergyPortion -                                                  // energy at the battery
    //     // rechargeEneryPortionAtTank +                                            // energy for recharge
    //     efficiencyDeviationPenalty * std::pow(efficiencyDeviation * 100.0 , 2.0);

    // // Apply Penalties if the energy required from the battery is larger
    // // than what the battery can provide.
    // if (isBattery_kwh_overMaxOutput)
    // {
    //     cost += 100000.0; // very high cost
    // }
    // // Apply penalties if the energy required from the generator is larger
    // // than what the generator can provide
    // if (enginePowerPortion +
    //         generatorPowerPortionForRecharge > 1.0)
    // {
    //     cost += 100000000.0; // very high cost
    // }

    // // Apply penalties based on recharge strategy and progress along the route
    // // if (rechargePortion < 1.0) {
    // //     cost += (1.0 - rechargePortion);               // Apply penalty if not fully recharging
    // // }

    // double batteryDeviation = 0.0;
    // if (std::isnan(batterySOCDeviation))
    // {
    //     batteryDeviation =
    //         (battery.getBatteryInitialCharge() -
    //          (battery.getBatteryCurrentCharge() + batteryRecharge_kWh_atBattery)) /
    //         battery.getBatteryMaxCharge();
    // }
    // else
    // {
    //     batteryDeviation = batterySOCDeviation / battery.getBatteryMaxCharge();
    // }

    // double deviationPenalty = 0.0;

    // // overcharging the battery
    // if (batteryDeviation > 0.0)
    // {
    //     double overchargePenalty = overchargePenaltyBase * std::pow((1 + routeProgress), 2);
    //     double overchargeSOC =
    //         (batteryDeviation > (battery.getBatteryRechargeSOCUpperBound() -
    //                              battery.getBatteryInitialCharge()))?
    //             batteryDeviation : 0.0;
    //     deviationPenalty = overchargePenalty * overchargeSOC;

    // }
    // // undercharge the battery
    // else if (batteryDeviation < 0.0)
    // {
    //     double underchargePenalty = underchargePenaltyBase  * std::pow((1 + routeProgress), 2);
    //     double underchargeSOC =
    //         (batteryDeviation < (battery.getBatteryInitialCharge() -
    //                              battery.getBatteryRechargeSOCLowerBound()))?
    //             batteryDeviation : 0.0;
    //     deviationPenalty = underchargePenalty * underchargeSOC;
    // }

    // // Incorporate the dynamic priority for recharging as the journey progresses
    // cost += (1.0 - routeProgress) * batteryDeviation * batteryDiviationPenalty;
    // // cost += rechargePortion * std::pow(batteryDeviation + 0.02, routeProgress);//batteryDeviation * batteryDiviationPenalty;
    // cost += deviationPenalty;

    // return cost;
//     return 0.0;
// }


// void Locomotive::generateHybridControlActionsNewLevelForParent(
//     TreeNode<Vector<double>>* parent,
//     const Vector<Vector<double>>& combinations)
// {
//     // add all the combinations to the tree node
//     for (const auto& comb : combinations)
//     {
//         // create a new node that is a child to the current parent
//         // the childNode is owned by the tree so dont worry about managing it
//         TreeNode<Vector<double>>* childNode =
//             new TreeNode<Vector<double>>({comb[0], comb[1]});

//         bool isAdded = hybridControlActions->insertChild(parent, childNode);

//         if (!isAdded) {
//             delete childNode;
//         }
//     }

// }

void
Locomotive::generateHybridControlActions(int discritizationActionSteps,
                                         int forwardHorizon)
{
    // clear the old vector from all its content before generating new data
    hybridControlActionsCombination.clear();

    for(int i = discritizationActionSteps; i >= 0; i--)
    {
        // fraction of energy consumed from generator.
        // The rest is from the battery.
        double m = i * (1.0f/discritizationActionSteps);

        for(int j = 0; j <= discritizationActionSteps; j++)
        {
            // fraction of battery recharge as a portion of the max recharge
            double r = j * (1.0f/discritizationActionSteps);

            hybridControlActionsCombination.push_back({m, r});
        }
    }

}




std::tuple<double, double, double>
Locomotive::getHybridParametersForLowerCost(
    double timeStep,
    double trainSpeed,
    double virtualPower_kWh,
    double EC_kwh_atWheels,
    double routeProgress)
{

    double minCost = std::numeric_limits<double>::max();
    double generatorEnergyPortion = 0.0;
    double bestRechargePortion = 0.0;

    for(int i = discritizationActionStepInIndependentAndMPC; i >= 0; i--)
    {
        // fraction of energy consumed from generator.
        // The rest is from the battery.
        double m = i * (1.0/discritizationActionStepInIndependentAndMPC);

        for(int j = 0; j <= discritizationActionStepInIndependentAndMPC; j++)
        {
            // fraction of battery recharge as a portion of the max recharge
            double r = j * (1.0/discritizationActionStepInIndependentAndMPC);

            // compute the cost giving priority to
            // 1. keeping the battery charged to its initial state
            // 2. balancing consumption from generator or battery
            double cost =
                computeHybridsCost2(timeStep, trainSpeed, virtualPower_kWh,
                                    EC_kwh_atWheels,
                                    hybridEnergyDistribution(m, r),
                                    routeProgress);
            if (cost < minCost)
            {
                minCost = cost;
                generatorEnergyPortion = m;
                bestRechargePortion = r;
            }
        }
    }

    return std::tuple<double, double, double>(minCost,
                                              generatorEnergyPortion,
                                              bestRechargePortion);
}

std::pair<bool, double>
Locomotive::consumeEnergyFromHydridTechnologyOptimizationEnabled(
    double timeStep,
    double trainSpeed,
    double EC_kwh_atWheels,
    double routeProgress,
    double fuelConversionFactor,
    double fuelDensity,
    double LocomotiveVirtualTractivePower,
    std::function<
        std::pair<bool,
                  double>(double,
                          double,
                          double)>
        ConsumeFuelFunc)
{
    double cost, generatorEnergyPortion, bestRechargePortion;


    if (hybridCalcMethodinit == TrainTypes::HybridCalculationMethod::timeStepIndependent)
    {
        std::tie(cost, generatorEnergyPortion, bestRechargePortion) =
            getHybridParametersForLowerCost( timeStep, trainSpeed,
                                            LocomotiveVirtualTractivePower,
                                            EC_kwh_atWheels, routeProgress);

    }
    else if (hybridCalcMethodinit == TrainTypes::HybridCalculationMethod::MPC)
    {

        // energy split at the DC
        generatorEnergyPortion = hybridControlAction.engineConsumePortion;
        bestRechargePortion = hybridControlAction.rechargePortion;
    }
    else {
        throw runtime_error("Optimization algorithm is not defined!");
    }

    double powerPortion =
        getUsedEnginePowerPortionForHybrids(timeStep, trainSpeed,
                                            LocomotiveVirtualTractivePower,
                                            EC_kwh_atWheels,
                                            generatorEnergyPortion,
                                            bestRechargePortion);

    double virtualPower_w = getPowerOfPowerPortionAtWheels_W(powerPortion);

    // virtualPower_w =
    //     getAccuratePowerWithBatteryCase(timeStep, trainSpeed, virtualPower_w,
    //                                     EC_kwh_atWheels, generatorEnergyPortion,
    //                                     bestRechargePortion);

    // consume fuel, result in form of <bool, double>,
    //                                  bool: all energy is consumed,
    //                                  double: rest of energy to consume
    //                                          from other sources
    std::pair<bool, double> consumptionResult;

    double EC_kWh_atDC =
        EC::convertECFromWheelToDCBus(EC_kwh_atWheels,
                                      powerPortion,
                                      trainSpeed);

    double ECFromBattery_kwh_atDC =
        (1.0 - generatorEnergyPortion) * EC_kWh_atDC;
    double ECFromBattery_kWh_atBattery =
        EC::convertECFromDCToBattery(ECFromBattery_kwh_atDC, powerType);
    double ECFromGenerator_kwh_atDC =
        generatorEnergyPortion * EC_kWh_atDC;
    double ECFromGenerator_kWh_atEngine =
        EC::convertECFromDCBusToEngine(
        ECFromGenerator_kwh_atDC, powerPortion, virtualPower_w,
        powerType, hybridMethod);

    hybridUsedGeneratorPowerPortion = powerPortion;

    // recharge the battery by the optimized amount
    rechargeBatteryByFlowPortion(timeStep, trainSpeed, powerPortion,
                                 bestRechargePortion, fuelConversionFactor,
                                 fuelDensity, LocomotiveVirtualTractivePower,
                                 ConsumeFuelFunc);

    // consume electricity from the battery first
    consumptionResult =
        this->consumeElectricity(timeStep,
                                 ECFromBattery_kWh_atBattery);

    consumptionResult.second =
        EC::convertECFromBatteryToEngine(consumptionResult.second,
                                         powerPortion, powerType);
    consumptionResult.second += ECFromGenerator_kWh_atEngine; // add generator required energy

    // consume fuel liquid
    consumptionResult =
        ConsumeFuelFunc(consumptionResult.second,
                        fuelConversionFactor, fuelDensity);

    // if (consumptionResult.second > 0.0){
    //     // if battery can be drained
    //     if (battery.isBatteryDrainable(consumptionResult.second)) {
    //         // consume electricity from the battery only
    //         return this->consumeElectricity(timeStep,
    //                                         consumptionResult.second *
    //                                             EC::getGeneratorEff(
    //                                                 this->powerType,
    //                                                 totalPowerPortion));
    //     }
    //     // require a recharge if the battery cannot be drained
    //     else {
    //         return consumptionResult;
    //     }
    // }

    return consumptionResult;

}

double Locomotive::getMaxPowerPortionUsedForBatteryRecharge(
    double timeStep, double trainSpeed,
    double ECAtBattery, double approxLocomotiveVirtualPower_W)
{
    double BatEC_atDC = EC::convertECFromBatteryToDC(ECAtBattery, powerType);
    double BatEC_atWhl =
        EC::convertECFromDCBusToWheels(BatEC_atDC,
                                       approxLocomotiveVirtualPower_W,
                                       trainSpeed);
    double BatPower_atWhl = EC::convertE_kWh_ToPower_W(BatEC_atWhl, timeStep);
    double BatPowerPortion = getUsedEnginePowerPortion(BatPower_atWhl);

    return BatPowerPortion;
}

std::pair<bool, double>
    Locomotive::consumeEnergyFromHybridTechnology(
                      double timeStep,
                      double trainSpeed,
                      double powerPortionAllEngineCase,
                      double EC_kwh,
                      double fuelConversionFactor,
                      double fuelDensity,
                      double LocomotiveVirtualTractivePower,
                      std::function<
                        std::pair<bool,
                                double>(double,
                                        double,
                                        double)>
                      ConsumeFuelFunc)
{


    // get the max eff range
    auto maxEffData = EC::getMaxEffeciencyRange(this->powerType);
    // std::pair<double, double> maxEffRange = {r.lowBoundary, r.highBoundary};
    // double maxEffPoint = r.highestPoint;

    std::pair<bool, double> consumptionResult;

    // Check if the battery needs a recharge
    bool isBetteryRechargable =
        (battery.isBatteryRechargable() && battery.isRechargeRequired());

    // convert the energy to DC
    double EC_kWh_atDC =
        EC::convertECFromWheelToDCBus(
            EC_kwh, LocomotiveVirtualTractivePower, trainSpeed);

    // a function to operate mainly on the engine and recharge the
    // battery if required.
    auto operateEngineAndRechargeBattery =
        [this, &consumptionResult, &EC_kWh_atDC , &timeStep, &trainSpeed,
         &isBetteryRechargable, &maxEffData,
         &powerPortionAllEngineCase, &LocomotiveVirtualTractivePower,
         &fuelConversionFactor, &fuelDensity, &ConsumeFuelFunc] ()
    {
        double powerPortionPotentialForBatteryRecharge = 0.0f;
        double totalPowerPortionUsed = powerPortionAllEngineCase; // if no energy is coming from battery

        // if recharge battery is required, increase the power portion
        // to account for it
        if (isBetteryRechargable) {
            // the max energy the battery can take
            double maxRechargeEC = battery.getBatteryMaxRecharge(timeStep);

            // if the locomotive is using less than highest boundary of the
            // efficiency region, use the eff region for calculations, if it
            // is higher that it, use the full power.
            double referencePoint =
                (maxEffData.highestPoint - powerPortionAllEngineCase <= 0.0 ) ?
                    (((maxEffData.highBoundary - powerPortionAllEngineCase) <= 0.0) ?
                         1.0 : maxEffData.highBoundary) : (maxEffData.highestPoint - powerPortionAllEngineCase);

            // get the power portion needed for the battery recharge
            powerPortionPotentialForBatteryRecharge =
                std::min(referencePoint - powerPortionAllEngineCase,
                         getMaxPowerPortionUsedForBatteryRecharge(
                             timeStep, trainSpeed, maxRechargeEC,
                             LocomotiveVirtualTractivePower));

            // total power portion of the engine
            totalPowerPortionUsed =
                std::min(1.0,
                         powerPortionAllEngineCase +
                             powerPortionPotentialForBatteryRecharge);

            // update the power portion needed for the battery recharge
            powerPortionPotentialForBatteryRecharge =
                totalPowerPortionUsed - powerPortionAllEngineCase;
        };

        // energy at the engine
        double EC_kWh_atEng =
            EC::convertECFromDCBusToEngine(
                EC_kWh_atDC, totalPowerPortionUsed,
                LocomotiveVirtualTractivePower,
                powerType, hybridMethod);

        // energy required to recharge the battery
        double batteryRechargeEC_kWh =
            isBetteryRechargable ?
                EC::convertPower_W_ToE_kWh(
                    getPowerOfPowerPortionAtWheels_W(
                        powerPortionPotentialForBatteryRecharge),
                    timeStep) : 0.0;

        // total energy at the engine with recharge
        double totalEC_atEn = EC_kWh_atEng + batteryRechargeEC_kWh;

        // total fuel amount for main drive and battery recharge
        double total_fuelAmount =
            EC::convertECToFuel(totalEC_atEn, fuelConversionFactor);
        // fuel amount for main drive
        double main_fuelAmount =
            EC::convertECToFuel(EC_kWh_atEng, fuelConversionFactor);

        // check if the tank can consume
        if (tank.isTankDrainable(total_fuelAmount)) {
            // append to results, the result of fuel consumption of the engine
            consumptionResult =
                ConsumeFuelFunc(totalEC_atEn,
                                fuelConversionFactor,
                                fuelDensity);

            // get the energy recharged to the battery
            double batteryRecharge_kWh_IntoBattery =
                EC::convertECFromEngineToBattery_kWh(batteryRechargeEC_kWh,
                                                     totalPowerPortionUsed,
                                                     powerType, hybridMethod);
            // recharge the battery
            battery.rechargeBatteryForHybrids(timeStep, batteryRecharge_kWh_IntoBattery);
        }
        // the locomotive should consume all the fuel inside the
        // loco first before switching to tanks. plus it takes a bit to switch
        else if (tank.isTankDrainable(main_fuelAmount)) {
            // append to results, the result of fuel consumption of the engine
            consumptionResult =
                ConsumeFuelFunc(EC_kWh_atEng,
                                fuelConversionFactor,
                                fuelDensity);
        }
        else { // if the tank cannot consume and
            // recharging the battery is not required,
            // consume everything from the battery
            if (! isBetteryRechargable) {
                // convert the energy to the battery
                double EC_kWh_atBattery =
                    EC::convertECFromDCToBattery(EC_kWh_atDC, powerType);
                // consume the battery
                auto rslt =
                    battery.consumeBattery(timeStep, EC_kWh_atBattery);

                // if battery did not provide all energy
                if (! rslt.first) {
                    // convert the energy from battery to the engine,
                    // so that the result is consistent
                    double EAtDC = rslt.second / EC::getBatteryEff(powerType);
                    double EAtW =
                        EC::convertECFromDCBusToWheels(
                            EAtDC, LocomotiveVirtualTractivePower, trainSpeed);
                    double powerr = EC::convertE_kWh_ToPower_W(EAtW, timeStep);
                    double pp1 = getUsedEnginePowerPortion(powerr);
                    double EAtE =
                        EC::convertECFromDCBusToEngine(EAtDC, pp1, powerr,
                                                       powerType, hybridMethod);
                    consumptionResult = {rslt.first, EAtE};
                }
                else { // the battery provided everything
                    consumptionResult = {rslt.first, rslt.second};
                }

            }
            else { // recharge battery is also required, no energy in the loco
                // return require energy from tenders
                consumptionResult = {false, EC_kWh_atEng};
            }
        }
    };


    // check if the power portion is within the high eff range.
    // If yes, then operate the engine. If not, use the battery.

    // ultimately, the energy should come from the battery
    // if the battery is in recharge state,
    // in this case, do not consume any energy from the battery
    // and recharge it. The energy will come only from the engine
    // for the main drive and the recharge.
    // in the case of using the battery, if the battery does not
    // provide all the required energy, consume the rest from the engine.
    if ((powerPortionAllEngineCase >= maxEffData.lowBoundary &&
         powerPortionAllEngineCase <= maxEffData.highBoundary) ||
        isBetteryRechargable)
    {
        // consume fuel and recharge battery
        operateEngineAndRechargeBattery();
    }

    // if the engine operates outside the eff range or their is no battery
    // required to be recharged, then consume the battery.
    else {

        double EC_kWh_atBat =
            EC::convertECFromDCToBattery(EC_kWh_atDC, powerType);

        // if the battery can provide energy, consume it from the battery,
        // if not consume from the engine
        if (battery.isBatteryDrainable(EC_kWh_atBat)) {
            // drain the battery. there is a very low likelihood that the battery
            // does not provide all energy since we checked already
            auto result = battery.consumeBattery(timeStep, EC_kWh_atBat);

            if (! result.first) // could not consume everything from the battery
            {
                // return the rest of the E to the DC bus
                double EC_kWh_atDC =
                    EC::convertECFromBatteryToDC(result.second, powerType);

                // return the energy to the wheels for calculating the power portion
                double EC_kWh_AtWheels =
                    EC::convertECFromDCBusToWheels(
                        EC_kWh_atDC, LocomotiveVirtualTractivePower, trainSpeed);

                // calculate the power portion
                double powerAtWheels =
                    EC::convertE_kWh_ToPower_W(EC_kWh_AtWheels, timeStep);
                double totalPowerPortionUsed =
                    getUsedEnginePowerPortion(powerAtWheels);

                // calculate the energy at the engine
                double EC_kWh_atEng =
                    EC::convertECFromDCBusToEngine(
                        EC_kWh_atDC, totalPowerPortionUsed,
                        LocomotiveVirtualTractivePower, powerType, hybridMethod);
                // append to results, the result of fuel consumption
                // of the engine
                consumptionResult = ConsumeFuelFunc(EC_kWh_atEng,
                                                    fuelConversionFactor,
                                                    fuelDensity);
            }
            else {
                consumptionResult = {result.first, result.second};
            }
        }
        else {
            operateEngineAndRechargeBattery();
        }
    }

    return consumptionResult;
}

std::pair<bool, double> Locomotive::consumeFuel(
                            double timeStep,
                            double trainSpeed,
                            double EC_kwh_atWheels, // at the wheels
                            double routeProgress,
                            double LocomotiveVirtualTractivePower,
                            double dieselConversionFactor,
                            double bioDieselConversionFactor,
                            double hydrogenConversionFactor,
                            double dieselDensity,
                            double bioDieselDensity,
                            double hydrogenDensity)
{
    // reset the locomotive energy stats first
    this->energyConsumed = 0.0; // the step energy consumption of 1 tech
    this->energyRegenerated = 0.0; // the step energy regeneration of 1 tech

	// if energy should be consumed
    if (EC_kwh_atWheels > 0.0) {

        // if hybrid locomotive, convert energy to battery first before
        //consuming it
        // double EC_kwh_hybrid = EC_kwh/EC::getGeneratorEff(this->powerType,
           //                       powerPortion);

        double enginePowerPortion =
            getUsedEnginePowerPortion(LocomotiveVirtualTractivePower);
		if (this->powerType == TrainTypes::PowerType::diesel ) {
            double EC_kWh_atDC =
                EC::convertECFromWheelToDCBus(EC_kwh_atWheels,
                                              LocomotiveVirtualTractivePower,
                                              trainSpeed);
            double EC_kWh_atEngine =
                EC::convertECFromDCBusToEngine(EC_kWh_atDC,
                                               enginePowerPortion,
                                               LocomotiveVirtualTractivePower,
                                               powerType, hybridMethod);
            return this->consumeFuelDiesel(EC_kWh_atEngine, dieselConversionFactor,
                                           dieselDensity);
		}
		else if (this->powerType == TrainTypes::PowerType::electric) {
            double EC_kWh_atDC =
                EC::convertECFromWheelToDCBus(EC_kwh_atWheels,
                                              LocomotiveVirtualTractivePower,
                                              trainSpeed);
            double EC_kWh_atEngine =
                EC::convertECFromDCBusToEngine(EC_kWh_atDC,
                                               enginePowerPortion,
                                               LocomotiveVirtualTractivePower,
                                               powerType, hybridMethod);
            return this->consumeElectricity(timeStep, EC_kWh_atEngine);
		}
		else if (this->powerType == TrainTypes::PowerType::biodiesel) {
            double EC_kWh_atDC =
                EC::convertECFromWheelToDCBus(EC_kwh_atWheels,
                                              LocomotiveVirtualTractivePower,
                                              trainSpeed);
            double EC_kWh_atEngine =
                EC::convertECFromDCBusToEngine(EC_kWh_atDC,
                                               enginePowerPortion,
                                               LocomotiveVirtualTractivePower,
                                               powerType,
                                               hybridMethod);
            return this->consumeFuelBioDiesel(EC_kWh_atEngine,
                                              bioDieselConversionFactor,
                                              bioDieselDensity);
		}
		else if (this->powerType == TrainTypes::PowerType::dieselElectric) {
            double EC_kWh_atDC =
                EC::convertECFromWheelToDCBus(EC_kwh_atWheels,
                                              LocomotiveVirtualTractivePower,
                                              trainSpeed);
            double EC_kWh_atEngine =
                EC::convertECFromDCBusToEngine(EC_kWh_atDC,
                                               enginePowerPortion,
                                               LocomotiveVirtualTractivePower,
                                               powerType, hybridMethod);
            return this->consumeFuelDiesel(EC_kWh_atEngine, dieselConversionFactor,
                                               dieselDensity);
		}
		else if (this->powerType == TrainTypes::PowerType::dieselHybrid) {
            if (hybridCalcMethod == TrainTypes::HybridCalculationMethod::MPC ||
                hybridCalcMethod == TrainTypes::HybridCalculationMethod::timeStepIndependent)
            {
                return
                    this->consumeEnergyFromHydridTechnologyOptimizationEnabled(
                        timeStep,
                        trainSpeed,
                        EC_kwh_atWheels,
                        routeProgress,
                        dieselConversionFactor,
                        dieselDensity,
                        LocomotiveVirtualTractivePower,
                        std::bind(
                            &Locomotive::consumeFuelDiesel,
                            this, std::placeholders::_1,
                            std::placeholders::_2,
                            std::placeholders::_3));
            }
            else if (hybridCalcMethod == TrainTypes::HybridCalculationMethod::fast)
            {
                double powerPortion =
                    this->getUsedEnginePowerPortion(LocomotiveVirtualTractivePower);
                return
                    this->consumeEnergyFromHybridTechnology(
                        timeStep,
                        trainSpeed,powerPortion,
                        EC_kwh_atWheels,
                        dieselConversionFactor,
                        dieselDensity,
                        LocomotiveVirtualTractivePower,
                        std::bind(
                            &Locomotive::consumeFuelDiesel,
                            this, std::placeholders::_1,
                            std::placeholders::_2,
                            std::placeholders::_3));
            }
		}
        else if (this->powerType == TrainTypes::PowerType::biodieselHybrid) {
            if (hybridCalcMethod == TrainTypes::HybridCalculationMethod::MPC ||
                hybridCalcMethod == TrainTypes::HybridCalculationMethod::timeStepIndependent)
            {
                return
                    this->consumeEnergyFromHydridTechnologyOptimizationEnabled(
                        timeStep, trainSpeed, EC_kwh_atWheels,
                        routeProgress, bioDieselConversionFactor,
                        bioDieselDensity, LocomotiveVirtualTractivePower,
                        std::bind(&Locomotive::consumeFuelBioDiesel,
                                  this, std::placeholders::_1,
                                  std::placeholders::_2,
                                  std::placeholders::_3));
            }
            else if (hybridCalcMethod == TrainTypes::HybridCalculationMethod::fast)
            {
                double powerPortion =
                    this->getUsedEnginePowerPortion(LocomotiveVirtualTractivePower);
                return
                    this->consumeEnergyFromHybridTechnology(
                        timeStep, trainSpeed,powerPortion, EC_kwh_atWheels,
                        bioDieselConversionFactor,
                        bioDieselDensity, LocomotiveVirtualTractivePower,
                        std::bind(&Locomotive::consumeFuelBioDiesel,
                                  this, std::placeholders::_1,
                                  std::placeholders::_2,
                                  std::placeholders::_3));
            }
        }
		else if (this->powerType == TrainTypes::PowerType::hydrogenHybrid) {
            if (hybridCalcMethod == TrainTypes::HybridCalculationMethod::MPC ||
                hybridCalcMethod == TrainTypes::HybridCalculationMethod::timeStepIndependent)
            {
                return
                    this->consumeEnergyFromHydridTechnologyOptimizationEnabled(
                        timeStep, trainSpeed,
                        EC_kwh_atWheels, routeProgress,
                        hydrogenConversionFactor,hydrogenDensity,
                        LocomotiveVirtualTractivePower,
                        std::bind(&Locomotive::consumeFuelHydrogen,
                                  this, std::placeholders::_1,
                                  std::placeholders::_2,
                                  std::placeholders::_3));
            }
            else if (hybridCalcMethod == TrainTypes::HybridCalculationMethod::fast)
            {
                double powerPortion =
                    this->getUsedEnginePowerPortion(LocomotiveVirtualTractivePower);
                return
                    this->consumeEnergyFromHybridTechnology(
                        timeStep, trainSpeed,
                        powerPortion, EC_kwh_atWheels,
                        hydrogenConversionFactor,hydrogenDensity,
                        LocomotiveVirtualTractivePower,
                        std::bind(&Locomotive::consumeFuelHydrogen,
                                  this, std::placeholders::_1,
                                  std::placeholders::_2,
                                  std::placeholders::_3));
            }
		}

		// if it is something else
        return std::make_pair(false, EC_kwh_atWheels);
	}
	// if energy is generated
    else if (EC_kwh_atWheels < 0.0) {
        double EC_kWh_atDC =
            EC::convertECFromWheelToDCBus(EC_kwh_atWheels,
                                          LocomotiveVirtualTractivePower,
                                          trainSpeed);
        double EC_kWh_atBattery = EC::convertECFromDCToBattery(EC_kWh_atDC,
                                                               powerType);
        // if not all the energy is stored, return the rest
        double restEC = this->refillBattery(timeStep, EC_kWh_atBattery);

        // the energy is stored in the battery
        if (restEC == 0.0) {
            return std::make_pair(true, 0.0);
        }
        // the battery is full and it requires a recharge somewhere else
        else {
            // make sure it is negative so it will be recharged
            return std::make_pair(false, (double)-1.0 * std::abs(restEC));
        }
    }
    // if energy is 0.0
	else {
         // no need to do any further calculations if no energy is
        // feed or consumed
        return std::make_pair(true, 0.0);
	}
}


double Locomotive::getResistance_N(double trainSpeed)
{
    // these calculations depend of US units, so these are the
    // conversions factors from meteric system
    double axialWeight = currentWeight / noOfAxiles;
    double rVal = (4.44822f * 1.10231f / 1000.0f) * currentWeight *
                  (1.5f + (16329.34f/axialWeight) +
                   0.0671f * trainSpeed +
                   ((48862.37f * this->frontalArea * this->dragCoef * (pow(trainSpeed, 2))) / currentWeight) +
                   20.0f * (trackGrade + 0.04 * abs(this->trackCurvature)));
    return rVal;
}

double Locomotive::getNetForce(double &frictionCoef,
    double &trainSpeed, bool &optimize, double &optimumThrottleLevel)
{
    return (this->getTractiveForce_N(frictionCoef, trainSpeed,
                                   optimize, optimumThrottleLevel) -
            this->getResistance_N(trainSpeed));
}


double Locomotive::getMaxProvidedEnergy(double &timeStep,
                                        double locomotiveVirtualPower,
                                        double trainSpeed)
{
    if (TrainTypes::locomotiveBatteryOnly.exist(this->powerType)) {
        if (this->hostLink->hasCatenary) {
            return std::numeric_limits<double>::infinity();
        }
        if (battery.batteryHasCharge()) {
            return battery.getBatteryMaxDischarge(timeStep);
        }
        return 0.0;

    }
    else if (TrainTypes::locomotiveTankOnly.exist(this->powerType)) {
        if (!tank.tankHasFuel()) {
            return 0.0;
        }
        double powerPortion =
            getUsedEnginePowerPortion(locomotiveVirtualPower);
        double EC_atEngine =
            EC::convertPower_W_ToE_kWh(getMaxGrossPowerAtWheel_W() *
                                       throttleLevel *
                                       EC::getDriveLineEff(trainSpeed,
                                                           powerPortion,
                                                           powerType,
                                                           hybridMethod),
                                   timeStep);
        double EC_atWheels =
            EC::convertECFromEnginToDCBus(EC_atEngine,
                                          powerPortion,
                                          locomotiveVirtualPower,
                                          powerType,
                                          hybridMethod);
        return EC_atWheels;
    }
    else if (TrainTypes::locomotiveHybrid.exist(this->powerType)) {
        if (!tank.tankHasFuel() && !battery.batteryHasCharge()) {
            return 0.0;
        }
        double powerPortion =
            getUsedEnginePowerPortion(locomotiveVirtualPower);

        double EC_atEngine =
            EC::convertPower_W_ToE_kWh(getMaxGrossPowerAtWheel_W() *
                                       throttleLevel *
                                       EC::getDriveLineEff(trainSpeed,
                                                           powerPortion,
                                                           powerType,
                                                           hybridMethod),
                                   timeStep);
        double EC_atDCBus =
            EC::convertECFromEnginToDCBus(EC_atEngine, powerPortion,
                                          locomotiveVirtualPower, powerType,
                                          hybridMethod);
        double EC_atWheelsFromEngine =
            EC::convertECFromDCBusToWheels(EC_atDCBus, locomotiveVirtualPower,
                                           trainSpeed);
        double EC_atBattery = this->battery.getBatteryMaxDischarge(timeStep);
        double EC_atDCFromBattery =
            EC::convertECFromBatteryToDC(EC_atBattery, powerType);
        double EC_atWheelsFromBattery =
            EC::convertECFromDCBusToWheels(EC_atDCFromBattery,
                                           locomotiveVirtualPower, trainSpeed);

        double totalEC_atWheels = EC_atWheelsFromEngine + EC_atWheelsFromBattery;
        return totalEC_atWheels;

    }
    return std::numeric_limits<double>::infinity();
}

double Locomotive::canProvideEnergy(double &EC_kwh, double &timeStep)
{
    if (EC_kwh <= 0.0) {
        return 0.0;
    }
    if (TrainTypes::locomotiveBatteryOnly.exist(this->powerType)) {
        if (this->hostLink->hasCatenary) {
            return 0.0;
        }

        if (!battery.isBatteryDrainable(EC_kwh)) {
            return EC_kwh;
        }

        double restEC = EC_kwh - battery.getBatteryMaxDischarge(timeStep);
        if (restEC >= 0.0) {
            return restEC;
        } else {
            return 0.0;
        }
    }
    else if (TrainTypes::locomotiveTankOnly.exist(this->powerType)) {
        if (!tank.isTankDrainable(EC::getFuelFromEC(this->powerType, EC_kwh))) {
            return EC_kwh;
        }
    }
    else if (TrainTypes::locomotiveHybrid.exist(this->powerType)) {
        if ( ! (battery.isBatteryDrainable(EC_kwh) ||
              tank.isTankDrainable(EC::getFuelFromEC(this->powerType,
                                                      EC_kwh)) ) ) {
            return EC_kwh;
        }
    }
    return 0.0;
}

ostream& operator<<(std::ostream& ostr, const Locomotive& loco)
{
    ostr <<
        "Locomotive: Power: " << loco.maxPower <<
        ", transmission eff.: " << loco.engineMechanicalEfficiency <<
        ", length: " << loco.length <<
        ", drag coef: " << loco.dragCoef <<
        ", frontal area: " << loco.frontalArea <<
        ", current weight: " << loco.currentWeight <<
        ", no axles: " << loco.noOfAxiles <<
        ", max speed: " << loco.maxSpeed <<
        ", locomotive type: " << TrainTypes::PowerTypeToStr(loco.powerType) <<
        ", battery capacity: " << loco.battery.getBatteryMaxCharge() <<
        ", current battery status: " << loco.battery.getBatteryStateOfCharge() <<
        ", tank capacity: " << loco.tank.getTankMaxCapacity() <<
        ", current tank status: " << loco.tank.getTankStateOfCapacity() << std::endl;
	return ostr;
}
