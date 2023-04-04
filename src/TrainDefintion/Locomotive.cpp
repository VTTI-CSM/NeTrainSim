//
// Created by Ahmed Aredah
// Version 0.0.1
//

#include <iostream>
#include "Locomotive.h"
#include <math.h>
#include <algorithm> 
#include "../util/Vector.h"
#include "EnergyConsumption.h"
#include "TrainTypes.h"
#include <cstdlib>
#include "../util/Utils.h"
#define stringify( name ) #name
using namespace std;


Locomotive::Locomotive(
		double locomotiveMaxPower_kw,
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
		double batteryCRate) {
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
    this->powerType = TrainTypes::iToPowerType(locomotivePowerType); // convert from digits to enum type
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
                batteryInitialCharge_perc =EC::DefaultLocomotiveBatteryRechargeMinSOC_Diesel;
            }
            else {
                batteryInitialCharge_perc = EC::DefaultLocomotiveBatteryInitialCharge_DieselHybrid;
            }
        }
        else if (this->powerType == TrainTypes::PowerType::biodieselHybrid) {
            if (EC::DefaultLocomotiveBatteryInitialCharge_BioDieselHybrid == 0.0) {
                batteryInitialCharge_perc = EC::DefaultLocomotiveBatteryRechargeMinSOC_Diesel;
            }
            else {
                batteryInitialCharge_perc = EC::DefaultLocomotiveBatteryInitialCharge_BioDieselHybrid;
            }
        }
        else if (this->powerType == TrainTypes::PowerType::electric) {
            if (EC::DefaultLocomotiveBatteryInitialCharge_Electric == 0.0) {
                batteryInitialCharge_perc = EC::DefaultLocomotiveBatteryRechargeMinSOC_Other;
            }
            else {
                batteryInitialCharge_perc = EC::DefaultLocomotiveBatteryInitialCharge_Electric;
            }
        }
        else if (this->powerType == TrainTypes::PowerType::hydrogenHybrid) {
            if (EC::DefaultLocomotiveBatteryInitialCharge_HydrogenHybrid == 0.0) {
                batteryInitialCharge_perc = EC::DefaultLocomotiveBatteryRechargeMinSOC_Other;
            }
            else {
                batteryInitialCharge_perc = EC::DefaultLocomotiveBatteryInitialCharge_HydrogenHybrid;
            }
        }
    }

    // set the battery max charge if no value is passed
    if (std::isnan(batteryMaxCharge_kwh)) {
        if (this->powerType == TrainTypes::PowerType::dieselHybrid) {
            batteryMaxCharge_kwh = EC::DefaultLocomotiveBatteryMaxCharge_DieselHybrid;
        }
        else if (this->powerType == TrainTypes::PowerType::biodieselHybrid) {
            batteryMaxCharge_kwh = EC::DefaultLocomotiveBatteryMaxCharge_BioDieselHybrid;
        }
        else if (this->powerType == TrainTypes::PowerType::electric) {
            batteryMaxCharge_kwh = EC::DefaultLocomotiveBatteryMaxCharge_Electric;
        }
        else if (this->powerType == TrainTypes::PowerType::hydrogenHybrid) {
            batteryMaxCharge_kwh = EC::DefaultLocomotiveBatteryMaxCharge_HydogenHybrid;
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
		this->SetTank(tankMaxCapacity_, tankInitialCapacity_perc, EC::DefaultLocomotiveMinTankDOD);
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
						 EC::DefaultLocomotiveBatteryDOD, batteryCRate, maxRSOC, minRSOC);
		this->SetTank(tankMaxCapacity_, tankInitialCapacity_perc, EC::DefaultLocomotiveMinTankDOD);
    } //end else

    // if the locomotive has fuel, calculate the weight of the fuel
    if (TrainTypes::locomotiveTankOnly.exist(this->powerType) ||
            TrainTypes::locomotiveHybrid.exist(this->powerType)) {
        double fuelWeight = 0.0; // in ton
        // if the tender is for diesel
        if (this->powerType == TrainTypes::PowerType::diesel ||
                this->powerType == TrainTypes::PowerType::dieselElectric ||
                this->powerType == TrainTypes::PowerType::dieselHybrid) {
            fuelWeight = this->getTankInitialCapacity() * EC::DefaultDieselDensity;
        }
        // if the tender is for biodiesel
        else if (this->powerType == TrainTypes::PowerType::biodiesel ||
                 this->powerType == TrainTypes::PowerType::biodieselHybrid) {
            fuelWeight = this->getTankInitialCapacity() * EC::DefaultBioDieselDensity;
        }
        else if (this->powerType == TrainTypes::PowerType::hydrogenHybrid) {
            fuelWeight = this->getTankInitialCapacity() * EC::DefaultHydrogenDensity;
        }
        // check if the fuel weight provided is not correct, update it.
        // maybe the use accounts for other weights on the locomotive.
        // and the gross locomotive weight is higher than the empty weight + fuel weight
        if ((this->currentWeight - this->emptyWeight) < fuelWeight) {
            this->currentWeight = this->emptyWeight + fuelWeight;
        }
    }

	this->trackCurvature = 0;
	this->trackGrade = 0;
    this->hostLink = std::shared_ptr<NetLink>(); // assign empty placeholder
    this->discritizedLamda.push_back(0.0); // add idle throttle level
    this->throttleLevels = this->defineThrottleLevels(); // define all the throttle levels
};	


double Locomotive::getHyperbolicThrottleCoef(double & trainSpeed) {
	double dv, um;
    // ratio of current train speed by the max loco speed
	dv = trainSpeed / this->maxSpeed;
	um = 0.05 * this->maxSpeed;        //critical speed
	//if the ratio is greater than 1, we set it to 1
	if (dv >= 1.0) {
		return 1.0;
	}
	// if the ratio is less than 0.05, we set it to the result of the throttle function
	if (trainSpeed <= um) {
		return abs((dv) / ((0.001) + (0.05 / (1 - dv)) + (0.030 * (dv))));    // these values are derived in an excel sheet
	}
	else {
		return 1.0;          // straight out to full throttle if exceeded the critical speed
	}
};

double Locomotive::getlamdaDiscretized(double &lamda) {
	__int64 minI;
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

double Locomotive::getDiscretizedThrottleCoef(double &trainSpeed) {
	double lmda, lamdaDiscretized;
	if (this->maxLocNotch == 0) {
		this->maxLocNotch = this->Nmax;
	}
	if (this->maxLocNotch > this->Nmax) {
		this->maxLocNotch = this->Nmax;
	}
	if (reducedPowerNotch == 0 ){
		reducedPowerNotch = this->Nmax;
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
    //if the minimum is too high, set it to the highest possible value (Restrict max notch to #)
	if (crntLocNotch > this->maxLocNotch) {
		crntLocNotch = this->maxLocNotch;
		lamdaDiscretized = this->discritizedLamda[crntLocNotch];
	}
	if (crntLocNotch > reducedPowerNotch) {
		lamdaDiscretized = this->discritizedLamda[reducedPowerNotch];
	}
	return lamdaDiscretized;
}

double Locomotive::getThrottleLevel(double & trainSpeed, bool &optimize, double &optimumThrottleLevel) {
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

void Locomotive::updateLocNotch(double &trainSpeed) {
	if (trainSpeed == 0.0 || !this->isLocOn) { this->currentLocNotch = 0; }
	else {
		// get the discretized Throttle Level and compare it to the list
		double lamdaDiscretized = this->getDiscretizedThrottleCoef(trainSpeed);
		// the index of it is the current Loc Notch
		this->currentLocNotch = this->throttleLevels.index(lamdaDiscretized);
		// readjust its value if it is higher than what the locomotive have
		if (this->currentLocNotch > this->maxLocNotch) { this->currentLocNotch = this->maxLocNotch; }
	}

}

void Locomotive::reducePower() {
	if (this->currentLocNotch > 2) {
		this->reducedPowerNotch = this->currentLocNotch - 1;
	}
	else {
		this->isLocOn = false;
	}
}

void Locomotive::resetPowerRestriction() {
	this->reducedPowerNotch = 0;
}

Vector<double> Locomotive::defineThrottleLevels() {
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

double Locomotive::getTractiveForce(double &frictionCoef, double &trainSpeed,
								 bool &optimize, double &optimumThrottleLevel) {
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
		f = min((1000 * this->transmissionEfficiency * this->getThrottleLevel(trainSpeed, optimize, optimumThrottleLevel)
			* (this->maxPower) / trainSpeed), f1);
		this->maxTractiveForce = f;
		return f;
	};
}

double Locomotive::getSharedVirtualTractivePower(double &trainSpeed, double& trainAcceleration, 
	double& sharedWeight, double& sharedResistance) {
	if (! this->isLocOn) {
		return 0.0;
	}

	return ((sharedWeight * trainAcceleration) + sharedResistance) * trainSpeed;

}

double Locomotive::getEnergyConsumption(double& LocomotiveVirtualTractivePower,
										double& trainAcceleration,
										double &trainSpeed, double &timeStep) {
	// if the locomotive is turned off already, do not consume anything
	if (!this->isLocOn) {
		return 0.0;
	}
	// else calculate how much to consume in watt (kg * m^3/s^3)
	double tractivePower = LocomotiveVirtualTractivePower;

	// the conversion from watt to kwh
	double unitConversionFactor = timeStep / (double)(3600.0 * 1000.0);
	// the max power of the locomotive in watt
    double maxPower = this->maxPower * (double)1000.0; // Kw to watt
	// power portion watt/watt (unitless)
	// it is limited because at the begining of the deceleration, the power is very high
    double powerPortion = std::min(tractivePower / maxPower, 1.0);

    //double powerPortion = (double) this->currentLocNotch / (double)this->maxLocNotch;

	if (tractivePower == 0) {
		return this->auxiliaryPower * unitConversionFactor;
	}
	else if(tractivePower > 0) {
		double eff = EC::getDriveLineEff(trainSpeed, this->currentLocNotch, powerPortion ,this->powerType);
		double EC = (((tractivePower/ eff ) + this->auxiliaryPower ) * unitConversionFactor);
		return EC;
	}
	else {
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
				double virtualAcceleration = tractivePower / (trainSpeed * this->currentWeight);
				// calculate regenerative eff
				//if virtual deceleration is not zero, calculate eff from virtual deceleration
				if (virtualAcceleration != 0) {
					regenerativeEff = 1 / (exp(EC::gamma / abs(virtualAcceleration)));
				}
				else {
					regenerativeEff = 0.0;
				}
			}
			return ((tractivePower * regenerativeEff * EC::getDriveLineEff(trainSpeed,
																		   this->currentLocNotch,
																		   std::abs(powerPortion),
																		   this->powerType) +
					 this->auxiliaryPower) * unitConversionFactor);
		}
		// if it is diesel-electric, do no regenerate electricity
		else {
			return 0.0;
		}
	}
}

double Locomotive::getMaxRechargeEnergy(double timeStep, double trainSpeed) {
	return (this->maxTractiveForce * trainSpeed * timeStep / ((double)3600.0)) *
			EC::getRequiredGeneratorPowerForRecharge(getBatteryStateOfCharge());
}

std::pair<bool, double> Locomotive::consumeFuel(double timeStep, double trainSpeed,
                                                double EC_kwh,
                                                double LocomotiveVirtualTractivePower,
												double dieselConversionFactor,
												double bioDieselConversionFactor,
												double hydrogenConversionFactor,
												double dieselDensity,
												double bioDieselDensity,
                                                double hydrogenDensity) {
    // reset the locomotive energy stats first
    this->energyConsumed = 0.0; // the step energy consumption of 1 tech
    this->energyRegenerated = 0.0; // the step energy regeneration of 1 tech
    this->cumEnergyConsumed = 0.0; // the step energy consumption of multiple technologies (e.g hybrid)
    this->cumEnergyRegenerated = 0.0; // the step energy regeneration of multiple technologies (e.g hybrid)

	// if energy should be consumed
	if (EC_kwh > 0.0) {
        // the max power of the locomotive in watt
        double maxPower = this->maxPower * (double)1000.0; // Kw to watt
        // power portion watt/watt (unitless)
        // it is limited because at the begining of the deceleration, the power is very high
        double powerPortion = std::min(LocomotiveVirtualTractivePower / maxPower, 1.0);
		// if hybrid locomotive, convert energy to battery first before consuming it
        double EC_kwh_hybrid = EC_kwh/EC::getGeneratorEff(this->powerType, powerPortion);

		if (this->powerType == TrainTypes::PowerType::diesel ) {
			return this->consumeFuelDiesel(EC_kwh, dieselConversionFactor, dieselDensity);
		}
		else if (this->powerType == TrainTypes::PowerType::electric) {
			return this->consumeElectricity(timeStep, EC_kwh);
		}
		else if (this->powerType == TrainTypes::PowerType::biodiesel) {
			return this->consumeFuelBioDiesel(EC_kwh, bioDieselConversionFactor, bioDieselDensity);
		}
		else if (this->powerType == TrainTypes::PowerType::dieselElectric) {
			return this->consumeFuelDiesel(EC_kwh, dieselConversionFactor, dieselDensity);
		}
		else if (this->powerType == TrainTypes::PowerType::dieselHybrid) {
			// consume electricity from the battery
			auto EC = this->consumeElectricity(timeStep, EC_kwh);
			// check if the battery reached a low level and it needs a recharge
//			if (this->isRechargeRequired()) {
//				// the max Energy the locomotive can regenerate
//				double maxLocoRecharge = this->getMaxRechargeEnergy(timeStep, trainSpeed);
//				// get the max recharge current that the battery can take
//				double requiredE = this->getBatteryMaxRecharge(timeStep);
//				// the amount that should be recharged to the battery
//				double minE = std::min(maxLocoRecharge, requiredE);
//				// if the locomotive has fuel, consume the required amound of energy from Diesel
//				// --> since portion of the energy will be lost when stored in the battery, use
//				//     the effeciency of the generator
//                if (this->consumeFuelDiesel(minE / EC::getGeneratorEff(this->powerType, powerPortion),
//											dieselConversionFactor, dieselDensity).first) {
//					// recharge battery
//					this->rechargeBattery(timeStep, minE);
//				}
//			}
			// if it did not consume any energy from the battery, get all energy from the generator
			if (!EC.first) {

                return this->consumeFuelDiesel(EC_kwh_hybrid,
                                               dieselConversionFactor,
											   dieselDensity);
			}
			// if it consumed portion of the energy by the battery but also there is an excessive energy required,
			// get the excessive energy from the generator
			else if (EC.first && EC.second > 0.0) {
                return this->consumeFuelDiesel(EC.second / EC::getGeneratorEff(this->powerType, powerPortion),
											   dieselConversionFactor, dieselDensity);
			}
			// if it consumed all the required energy from the battery, return true
			return std::make_pair(true, 0.0);
		}
		else if (this->powerType == TrainTypes::PowerType::hydrogenHybrid) {
			auto EC = this->consumeElectricity(timeStep, EC_kwh);
			// check if the battery reached a low level and it needs a recharge
//			if (this->isRechargeRequired()) {
//				// the max Energy the locomotive can regenerate
//				double maxLocoRecharge = this->getMaxRechargeEnergy(timeStep, trainSpeed);
//				// get the max recharge current that the battery can take
//				double requiredE = this->getBatteryMaxRecharge(timeStep);
//				// the amount that should be recharged to the battery
//				double minE = std::min(maxLocoRecharge, requiredE);
//				// if the locomotive has fuel, consume the required amound of energy from Diesel
//				// --> since portion of the energy will be lost when stored in the battery, use
//				//     the effeciency of the generator
//                if (this->consumeFuelHydrogen(minE / EC::getGeneratorEff(this->powerType, powerPortion),
//											  hydrogenConversionFactor, hydrogenDensity).first) {
//					this->rechargeBattery(timeStep, minE);
//				}

//			}
			// if it did not consume any energy from the battery, get all energy from the generator
			if (!EC.first) {
				return this->consumeFuelHydrogen(EC_kwh_hybrid, hydrogenConversionFactor,
												 hydrogenDensity);
			}
			// if it consumed portion of the energy by the battery but also there is an excessive energy required,
			// get the excessive energy from the generator
			else if (EC.first && EC.second > 0.0) {
                return this->consumeFuelHydrogen(EC.second/EC::getGeneratorEff(this->powerType, powerPortion),
											   hydrogenConversionFactor, hydrogenDensity);
			}
			// if it consumed all the required energy from the battery, return true
			return std::make_pair(true, 0.0);
		}
		else if (this->powerType == TrainTypes::PowerType::biodieselHybrid) {

			auto EC = this->consumeElectricity(timeStep, EC_kwh);
			// check if the battery reached a low level and it needs a recharge
//			if (this->isRechargeRequired()) {
//				// the max Energy the locomotive can regenerate
//				double maxLocoRecharge = this->getMaxRechargeEnergy(timeStep, trainSpeed);
//				// get the max recharge current that the battery can take
//				double requiredE = this->getBatteryMaxRecharge(timeStep);
//				// the amount that should be recharged to the battery
//				double minE = std::min(maxLocoRecharge, requiredE);
//				// if the locomotive has fuel, consume the required amound of energy from Diesel
//				// --> since portion of the energy will be lost when stored in the battery, use
//				//     the effeciency of the generator
//                if (this->consumeFuelBioDiesel(minE / EC::getGeneratorEff(this->powerType, powerPortion),
//										   bioDieselConversionFactor, bioDieselDensity).first) {
//					this->rechargeBattery(timeStep, minE);
//				}

//			}
			// if it did not consume any energy from the battery, get all energy from the generator
			if (!EC.first) {
				return this->consumeFuelBioDiesel(EC_kwh_hybrid, bioDieselConversionFactor,
												 bioDieselDensity);
			}
			// if it consumed portion of the energy by the battery but also there is an excessive energy required,
			// get the excessive energy from the generator
			else if (EC.first && EC.second > 0.0) {
                return this->consumeFuelBioDiesel(EC.second/EC::getGeneratorEff(this->powerType, powerPortion),
											   bioDieselConversionFactor, bioDieselDensity);
			}
			// if it consumed all the required energy from the battery, return true
			return std::make_pair(true, 0.0);
		}

		// if it is something else
		return std::make_pair(false, EC_kwh);
	}
	// if energy is generated
	else if (EC_kwh < 0.0) {
        // if not all the energy is stored, return the rest
        if (this->refillBattery(timeStep, EC_kwh)) {
            return std::make_pair(true, 0.0);
        }
        else {
            return std::make_pair(false, EC_kwh);
        }
    }
    // if energy is 0.0
	else {
		return std::make_pair(true, 0.0);  // no need to do any further calculations if no energy is feed or consumed
	}
}


double Locomotive::getResistance(double trainSpeed) {
	// these calculations depend of US units, so these are the conversions factors from meteric system
	double rVal = 0.0;
	double speed = trainSpeed * 2.23694;
	rVal = 1.5 + 18 / ((this->currentWeight * 1.10231) / this->noOfAxiles) + 0.03 * speed
		+ (this->frontalArea * 10.7639) * this->dragCoef * (pow(speed, 2)) / (this->currentWeight * 1.10231);
	rVal = (rVal) * ((this->currentWeight * 1.10231)) + 20 * (this->currentWeight * 1.10231) * (this->trackGrade);
	rVal += abs(this->trackCurvature) * 20 * 0.04 * (this->currentWeight * 1.10231);
	rVal *= (4.44822);
	return rVal;
}

double Locomotive::getNetForce(double &frictionCoef,
	double &trainSpeed, bool &optimize, double &optimumThrottleLevel) {
	return (this->getTractiveForce(frictionCoef, trainSpeed, optimize, optimumThrottleLevel) - this->getResistance(trainSpeed));
}

ostream& operator<<(std::ostream& ostr, const Locomotive& loco) {
	ostr << "Locomotive: Power: " << loco.maxPower << ", transmission eff.: " << loco.transmissionEfficiency;
	ostr << ", length: " << loco.length << ", drag coef: " << loco.dragCoef << ", frontal area: " << loco.frontalArea;
	ostr << ", current weight: " << loco.currentWeight << ", no axles: " << loco.noOfAxiles;
	ostr << ", max speed: " << loco.maxSpeed << ", locomotive type: " << TrainTypes::PowerTypeToStr(loco.powerType);
	ostr << ", battery capacity: " << loco.getBatteryMaxCharge() << ", current battery status: " << loco.getBatteryStateOfCharge();
	ostr << ", tank capacity: " << loco.getTankMaxCapacity() << ", current tank status: " << loco.getTankStateOfCapacity() << std::endl;
	return ostr;
}
