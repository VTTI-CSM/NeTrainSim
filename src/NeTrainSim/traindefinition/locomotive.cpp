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
        TrainTypes::LocomotivePowerMethod theHybridMethod)
{
    // assign the values to the variables
	this->name = locomotiveName;
    this->maxPower = locomotiveMaxPower_kw;  // in kw
    this->transmissionEfficiency = locomotiveTransmissionEfficiency; // unitless
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
        this->setBattery(batteryMaxCharge_kwh, batteryInitialCharge_perc,
                         EC::DefaultLocomotiveBatteryDOD, batteryCRate);
        this->SetTank(0.0, 0.0, EC::DefaultLocomotiveMinTankDOD);

    }//end if battery tech
	// diesel and hydrogen have only tanks
	else if (TrainTypes::locomotiveTankOnly.exist(this->powerType)) {
        this->setBattery(0.0, 0.0, 1.0, batteryCRate);
        this->SetTank(tankMaxCapacity_, tankInitialCapacity_perc,
                      EC::DefaultLocomotiveMinTankDOD);
    } // end else if tank tech
	// hybrid locomotives have both source of energy
	else {
		double maxRSOC = 0.0;
		double minRSOC = 0.0;
        if (this->powerType == TrainTypes::PowerType::dieselHybrid ||
                this->powerType == TrainTypes::PowerType::biodieselHybrid) {
			maxRSOC = EC::DefaultLocomotiveBatteryRechargeMaxSOC_Diesel;
			minRSOC = EC::DefaultLocomotiveBatteryRechargeMinSOC_Diesel;
        } //end if diesel or biodiesel
		else {
			maxRSOC = EC::DefaultLocomotiveBatteryRechargeMaxSOC_Other;
			minRSOC = EC::DefaultLocomotiveBatteryRechargeMinSOC_Other;
        } // end else technologies
        this->setBattery(batteryMaxCharge_kwh, batteryInitialCharge_perc,
                         EC::DefaultLocomotiveBatteryDOD,
                         batteryCRate, maxRSOC, minRSOC);
        this->SetTank(tankMaxCapacity_, tankInitialCapacity_perc,
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
            fuelWeight = this->getTankInitialCapacity() *
                         EC::DefaultDieselDensity;
        }
        // if the tender is for biodiesel
        else if (this->powerType == TrainTypes::PowerType::biodiesel ||
                 this->powerType == TrainTypes::PowerType::biodieselHybrid) {
            fuelWeight = this->getTankInitialCapacity() *
                         EC::DefaultBioDieselDensity;
        }
        else if (this->powerType == TrainTypes::PowerType::hydrogenHybrid) {
            fuelWeight = this->getTankInitialCapacity() *
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
        this->hybridMethod = TrainTypes::LocomotivePowerMethod::series;
    }
    else if (TrainTypes::locomotiveHybrid.exist(this->powerType)) {
        this->hybridMethod = theHybridMethod;
    }
    else {
        this->hybridMethod = TrainTypes::LocomotivePowerMethod::notApplicable;
    }

};	


double Locomotive::getHyperbolicThrottleCoef(double & trainSpeed)
{
	double dv, um;
    // ratio of current train speed by the max loco speed
	dv = trainSpeed / this->maxSpeed;
    double lambda = (double)1.0 / (1.0 + exp(-7.82605 * (dv - 0.42606)));

    //double lambda = 1.0 - (1.0 / (-2.5576+(3.5637/(1.0-dv))-3.7146*dv));

    if (lambda < 0.0){
        return 0.0;
    }
    else if (lambda > 1.0) {
        return 1.0;
    }

    return lambda;

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
    int lowerNotch = this->currentLocNotch - 1;
    double lowerNotchLambda = this->discritizedLamda[lowerNotch];
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

double Locomotive::getTractiveForce(double &frictionCoef,
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
		this->maxTractiveForce = f1;
		return f1;
	}
    else {
        f = min((this->locPowerReductionFactor * 1000.0 *
                 this->transmissionEfficiency *
                 this->getThrottleLevel(trainSpeed, optimize,
                                        optimumThrottleLevel) *
                 (EC::getLocomotivePowerReductionFactor(this->powerType) *
                    this->maxPower) / trainSpeed), f1);
		this->maxTractiveForce = f;
		return f;
	};
}

double Locomotive::getSharedVirtualTractivePower(double &trainSpeed,
                                                 double& trainAcceleration,
                                                 double& sharedWeight,
                                                 double& sharedResistance)
{
	if (! this->isLocOn) {
		return 0.0;
	}
    return ((sharedWeight * trainAcceleration) + sharedResistance) *
           trainSpeed;
}

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
            //if virtual deceleration is not zero, calculate eff
            // from virtual deceleration
            if (virtualAcceleration != 0) {
                regenerativeEff = 1 / (exp(EC::gamma /
                                           abs(virtualAcceleration)));
            }
            else {
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

double Locomotive::getEnergyConsumptionAtDCBus(
                        double &LocomotiveVirtualTractivePower,
                        double &trainAcceleration,
                        double &trainSpeed,
                        double &timeStep)
{
    // if the locomotive is turned off already, do not consume anything
    if (!this->isLocOn) {
        return 0.0;
    }
    // else calculate how much to consume in watt (kg * m^3/s^3)

    // the conversion from KW to kwh
    double unitConversionFactor = timeStep / (double)(3600.0 * 1000.0);


    if(LocomotiveVirtualTractivePower >= 0) {
        double eff = EC::getWheelToDCBusEff(trainSpeed);
        double EC =
            (((LocomotiveVirtualTractivePower +
                       this->auxiliaryPower ) *
                      unitConversionFactor) / eff );
        return EC;
    }
    else {
        // get regenerative eff
        double regenerativeEff =
            this->getRegenerativeEffeciency(LocomotiveVirtualTractivePower,
                                            trainAcceleration, trainSpeed);

        return (((LocomotiveVirtualTractivePower ) * regenerativeEff *
                 EC::getWheelToDCBusEff(trainSpeed)) *
                unitConversionFactor);

    }
}

double Locomotive::getEnergyConsumptionAtTank(
                                    double &LocomotiveVirtualTractivePower,
                                    double &trainSpeed,
                                    double EnergyConsumptionAtDCBus)
{

    double powerPortion =
        this->getUsedPowerPortion(trainSpeed,
                                  LocomotiveVirtualTractivePower);

    if (EnergyConsumptionAtDCBus >= 0.0) {
        return EnergyConsumptionAtDCBus /
               EC::getDCBusToTankEff(std::abs(powerPortion),
                                     this->powerType, this->hybridMethod);
    }
    else {
        return EnergyConsumptionAtDCBus *
               EC::getDCBusToTankEff(std::abs(powerPortion),
                                     this->powerType,
                                     this->hybridMethod);
    }
}



double Locomotive::getEnergyConsumption(double& LocomotiveVirtualTractivePower,
                                        double& trainAcceleration,
                                        double &trainSpeed,
                                        double &timeStep)
{
	// if the locomotive is turned off already, do not consume anything
	if (!this->isLocOn) {
		return 0.0;
	}
	// else calculate how much to consume in watt (kg * m^3/s^3)
	double tractivePower = LocomotiveVirtualTractivePower;

    // the conversion from KW to kwh
    double unitConversionFactor = timeStep / (double)(3600.0 * 1000.0);

    double powerPortion =
        this->getUsedPowerPortion(trainSpeed,
                                  LocomotiveVirtualTractivePower);

	if (tractivePower == 0) {
		return this->auxiliaryPower * unitConversionFactor;
	}
	else if(tractivePower > 0) {
        double eff = EC::getDriveLineEff(trainSpeed, this->currentLocNotch,
                                         powerPortion ,this->powerType,
                                         this->hybridMethod);
        double EC = (((tractivePower + this->auxiliaryPower ) *
                      unitConversionFactor) / eff );
		return EC;
	}
    else {
        // get regenerative eff
        double regenerativeEff =
            this->getRegenerativeEffeciency(tractivePower,
                                            trainAcceleration, trainSpeed);

        return ((tractivePower * regenerativeEff + this->auxiliaryPower) *
                EC::getDriveLineEff(trainSpeed,
                                    this->currentLocNotch,
                                    std::abs(powerPortion),
                                    this->powerType,
                                    this->hybridMethod) *
                unitConversionFactor);

	}
}

double Locomotive::getUsedPowerPortion(double trainSpeed,
                                       double LocomotiveVirtualTractivePower)
{
    // the max power of the locomotive in watt
    double maxPower = this->maxPower * (double)1000.0; // Kw to watt
    // power portion watt/watt (unitless)
    // it is limited because at the begining of the deceleration, the
    // power is very high
    return std::min((LocomotiveVirtualTractivePower) / maxPower, 1.0);
}

double Locomotive::getMaxRechargeEnergy(double timeStep, double trainSpeed,
                                        double LocomotiveVirtualTractivePower)
{
    // get locomotive power portion required to recharge the locomotive
    double generatorMaxRequired =
        EC::getRequiredGeneratorPowerForRecharge(getBatteryStateOfCharge());

    // get the actual power portion used at this time step
    double locomotiveUsedPower =
        this->getUsedPowerPortion(trainSpeed,
                                  LocomotiveVirtualTractivePower);
    // get the feasible range of the locomotive power portion that
    // could be used to recharge the battery
    double locomotiveFeasiblePower = min(1.0 - locomotiveUsedPower,
                                         generatorMaxRequired);
    // portion of the max locomotive power
    return (this->maxTractiveForce * trainSpeed * timeStep / ((double)3600.0)) *
            locomotiveFeasiblePower;
}

void Locomotive::rechargeBatteryByMaxFlow(double timeStep, double trainSpeed,
                                          double powerPortion,
                                          double fuelConversionFactor,
                                          double fuelDensity,
                                          double LocomotiveVirtualTractivePower,
                                          std::function<
                                              std::pair<bool, double>(double,
                                                                      double,
                                                                      double)>
                                                              ConsumeFuelFunc)
{

    // the max Energy the locomotive can regenerate
    double maxLocoRecharge =
        this->getMaxRechargeEnergy(timeStep, trainSpeed,
                                   LocomotiveVirtualTractivePower);
    // get the max recharge current that the battery can take
    double requiredE = this->getBatteryMaxRecharge(timeStep);
    // the amount that should be recharged to the battery
    double minE = std::max(std::min(maxLocoRecharge, requiredE), 0.0);
    if (minE > 0.0) {
        // get the fuel consumption
        // E/ Generator EFF/ Battery EFF
        double minEC =
            minE / EC::getGeneratorEff(this->powerType, powerPortion) /
                       EC::getBatteryEff(this->powerType);

        // if the locomotive has fuel, consume the required amound of energy
        // from Diesel
        // --> since portion of the energy will be lost when stored in the
        //     battery, use the effeciency of the generator
        if (ConsumeFuelFunc(minEC,
                            fuelConversionFactor,
                            fuelDensity).first) {
            this->cumEnergyConsumed -= minE;
            // recharge battery
            this->rechargeBatteryForHybrids(timeStep, minE);
        }
    }
}

std::pair<bool, double>
    Locomotive::consumeEnergyFromHybridTechnology(
                      double timeStep,
                      double trainSpeed,
                      double powerPortion,
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
    // provided EC_kwh = EC * driveline eff -> (0.965) in case of series, (1)
    // in case of parellel

    // get the max eff range
    std::pair<double, double> maxEffRange =
        EC::getMaxEffeciencyRange(this->powerType);

    std::pair<bool, double> consumptionResult;

    // if the locomotive is operating inside the max eff range, consume fuel,
    // O.W consume battery
    if (powerPortion > maxEffRange.first && powerPortion < maxEffRange.second) {
        // consume fuel, result in form of <bool, double>,
        //                                  bool: all energy is consumed,
        //                                  double: rest of energy to consume
        //                                          from other sources
        consumptionResult =
            ConsumeFuelFunc(EC_kwh *
                             EC::getBatteryEff(this->powerType) /
                             EC::getGeneratorEff(this->powerType, powerPortion),
                                            fuelConversionFactor, fuelDensity);
        // if all energy required is consumed, recharge the battery
        // for later use.
        if (consumptionResult.first && consumptionResult.second == 0.0) {
            if (this->isBatteryRechargable()) {
                // recharge battery with power
                this->rechargeBatteryByMaxFlow(timeStep, trainSpeed,
                                               powerPortion,
                                               fuelConversionFactor,
                                               fuelDensity,
                                               LocomotiveVirtualTractivePower,
                                               ConsumeFuelFunc);
            }
            return std::make_pair(true, 0.0);
        }
        // if there is some energy could not be consumed from the diesel
        else if (consumptionResult.second > 0.0){
            // if battery can be drained
            if (this->isBatteryDrainable(consumptionResult.second)) {
                // consume electricity from the battery only
                return this->consumeElectricity(timeStep,
                                                consumptionResult.second *
                                                    EC::getGeneratorEff(
                                                        this->powerType,
                                                          powerPortion));
            }
            // require a recharge if the battery cannot be drained
            else {
                return consumptionResult;
            }
        }

    }
    // if the locomotive is operating outside the max eff range, consume battery
    else {
        // consume electricity from the battery
        consumptionResult = this->consumeElectricity(timeStep, EC_kwh);

        // if it did not consume any energy from the battery or portion of
        // the energy is consumed,
        // get rest of energy from the generator
        if (consumptionResult.second > 0.0) {
            // if hybrid locomotive, convert energy to battery first
            // before consuming it
            double EC_kwh_hybrid =
                consumptionResult.second *
                                   EC::getBatteryEff(this->powerType)/
                                   EC::getGeneratorEff(this->powerType,
                                                       powerPortion);

            // if no fuel, return <false, EC_kwh_hybrid>
            // the locomotive will be turned off in this case
            consumptionResult = ConsumeFuelFunc(EC_kwh_hybrid,
                                   fuelConversionFactor,
                                   fuelDensity);
        }
        // if no further energy is required
        else {
            consumptionResult = std::make_pair(true, 0.0);
        }
    }
    // check if recharge is required
    if (this->IsBatteryExceedingThresholds()) {
        this->rechargeBatteryByMaxFlow(timeStep, trainSpeed,
                                       powerPortion, fuelConversionFactor,
                                       fuelDensity,
                                       LocomotiveVirtualTractivePower,
                                       ConsumeFuelFunc);
    }
    return consumptionResult;
}

std::pair<bool, double> Locomotive::consumeFuel(
                            double timeStep,
                            double trainSpeed,
                            double EC_kwh,
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
	if (EC_kwh > 0.0) {
        double powerPortion =
            this->getUsedPowerPortion(trainSpeed,
                                      LocomotiveVirtualTractivePower);

        this->usedPowerPortion = powerPortion;

        // if hybrid locomotive, convert energy to battery first before
        //consuming it
        // double EC_kwh_hybrid = EC_kwh/EC::getGeneratorEff(this->powerType,
           //                       powerPortion);

		if (this->powerType == TrainTypes::PowerType::diesel ) {
            return this->consumeFuelDiesel(EC_kwh, dieselConversionFactor,
                                           dieselDensity);
		}
		else if (this->powerType == TrainTypes::PowerType::electric) {
			return this->consumeElectricity(timeStep, EC_kwh);
		}
		else if (this->powerType == TrainTypes::PowerType::biodiesel) {
            return this->consumeFuelBioDiesel(EC_kwh,
                                              bioDieselConversionFactor,
                                              bioDieselDensity);
		}
		else if (this->powerType == TrainTypes::PowerType::dieselElectric) {
            return this->consumeFuelDiesel(EC_kwh, dieselConversionFactor,
                                               dieselDensity);
		}
		else if (this->powerType == TrainTypes::PowerType::dieselHybrid) {
            return
                this->consumeEnergyFromHybridTechnology(
                                        timeStep,
                                        trainSpeed,powerPortion,
                                        EC_kwh,
                                        dieselConversionFactor,
                                        dieselDensity,
                                        LocomotiveVirtualTractivePower,
                                        std::bind(
                                               &Locomotive::consumeFuelDiesel,
                                               this, std::placeholders::_1,
                                               std::placeholders::_2,
                                               std::placeholders::_3));
		}
        else if (this->powerType == TrainTypes::PowerType::biodieselHybrid) {
            return
                this->consumeEnergyFromHybridTechnology(
                           timeStep, trainSpeed,powerPortion, EC_kwh,
                           bioDieselConversionFactor,bioDieselDensity,
                           LocomotiveVirtualTractivePower,
                           std::bind(&Locomotive::consumeFuelBioDiesel,
                                     this, std::placeholders::_1,
                                     std::placeholders::_2,
                                     std::placeholders::_3));
        }
		else if (this->powerType == TrainTypes::PowerType::hydrogenHybrid) {
            return
                this->consumeEnergyFromHybridTechnology(
                            timeStep, trainSpeed,
                            powerPortion, EC_kwh,
                            hydrogenConversionFactor,hydrogenDensity,
                            LocomotiveVirtualTractivePower,
                            std::bind(&Locomotive::consumeFuelHydrogen,
                                     this, std::placeholders::_1,
                                     std::placeholders::_2,
                                     std::placeholders::_3));
		}

		// if it is something else
		return std::make_pair(false, EC_kwh);
	}
	// if energy is generated
	else if (EC_kwh < 0.0) {
        // if not all the energy is stored, return the rest
        double restEC = this->refillBattery(timeStep, EC_kwh);

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


double Locomotive::getResistance(double trainSpeed)
{
    // these calculations depend of US units, so these are the
    // conversions factors from meteric system
    double rVal = 0.0;
    double speed = trainSpeed * 2.23694;
    rVal = 1.5 + 18 / ((this->currentWeight * 1.10231) /
                this->noOfAxiles) + 0.03 * speed
        + (this->frontalArea * 10.7639) * this->dragCoef * (pow(speed, 2)) /
                 (this->currentWeight * 1.10231);
    rVal = (rVal) *
               ((this->currentWeight * 1.10231)) + 20 *
               (this->currentWeight * 1.10231) * (this->trackGrade);
    rVal += abs(this->trackCurvature) * 20 * 0.04 *
               (this->currentWeight * 1.10231);
    rVal *= (4.44822);
    return rVal;
}

double Locomotive::getNetForce(double &frictionCoef,
    double &trainSpeed, bool &optimize, double &optimumThrottleLevel)
{
    return (this->getTractiveForce(frictionCoef, trainSpeed,
                                   optimize, optimumThrottleLevel) -
            this->getResistance(trainSpeed));
}


double Locomotive::getMaxProvidedEnergy(double &timeStep)
{
    if (TrainTypes::locomotiveBatteryOnly.exist(this->powerType)) {
        if (this->hostLink->hasCatenary) {
            return std::numeric_limits<double>::infinity();
        }
        if (this->batteryHasCharge()) {
            return this->getBatteryMaxDischarge(timeStep);
        }
        return 0.0;

    }
    else if (TrainTypes::locomotiveTankOnly.exist(this->powerType)) {
        if (!this->tankHasFuel()) {
            return 0.0;
        }
    }
    else if (TrainTypes::locomotiveHybrid.exist(this->powerType)) {
        if (!this->tankHasFuel() && !this->batteryHasCharge()) {
            return 0.0;
        }
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

        if (!isBatteryDrainable(EC_kwh)) {
            return EC_kwh;
        }

        double restEC = EC_kwh - this->getBatteryMaxDischarge(timeStep);
        if (restEC >= 0.0) {
            return restEC;
        } else {
            return 0.0;
        }
    }
    else if (TrainTypes::locomotiveTankOnly.exist(this->powerType)) {
        if (!isTankDrainable(EC::getFuelFromEC(this->powerType, EC_kwh))) {
            return EC_kwh;
        }
    }
    else if (TrainTypes::locomotiveHybrid.exist(this->powerType)) {
        if ( ! (this->isBatteryDrainable(EC_kwh) ||
              this->isTankDrainable(EC::getFuelFromEC(this->powerType,
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
        ", transmission eff.: " << loco.transmissionEfficiency <<
        ", length: " << loco.length <<
        ", drag coef: " << loco.dragCoef <<
        ", frontal area: " << loco.frontalArea <<
        ", current weight: " << loco.currentWeight <<
        ", no axles: " << loco.noOfAxiles <<
        ", max speed: " << loco.maxSpeed <<
        ", locomotive type: " << TrainTypes::PowerTypeToStr(loco.powerType) <<
        ", battery capacity: " << loco.getBatteryMaxCharge() <<
        ", current battery status: " << loco.getBatteryStateOfCharge() <<
        ", tank capacity: " << loco.getTankMaxCapacity() <<
        ", current tank status: " << loco.getTankStateOfCapacity() << std::endl;
	return ostr;
}
