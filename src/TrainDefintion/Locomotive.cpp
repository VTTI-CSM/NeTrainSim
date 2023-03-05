#include <iostream>
#include "Locomotive.h"
#include <math.h>
#include <algorithm> 
#include "../util/Vector.h"
#include "EnergyConsumption.h"
#include "TrainTypes.h"
#include <cstdlib>

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
	double tankInitialCapacity_perc) {

	this->name = locomotiveName;
	this->maxPower = locomotiveMaxPower_kw;
	this->transmissionEfficiency = locomotiveTransmissionEfficiency;
	this->length = locomotiveLength_m;
	this->dragCoef = locomotiveDragCoef;
	this->frontalArea = locomotiveFrontalArea_sqm;
	this->currentWeight = locomotiveWeight_t;
    this->emptyWeight = DefaultLocomotiveEmptyWeight;
    if (this->emptyWeight > this->currentWeight) {
        this->emptyWeight = this->currentWeight;
    }
	this->noOfAxiles = locomotiveNoOfAxiles;
	this->powerType = TrainTypes::iToPowerType(locomotivePowerType);
	this->maxSpeed = locomotiveMaxSpeed_mps;
	this->Nmax = totalNotches;
	this->maxLocNotch = locomotiveMaxAchievableNotch;
	this->auxiliaryPower = locomotiveAuxiliaryPower_kw;

    // electric has only battery
	if (this->powerType == TrainTypes::PowerType::electric) {
		this->batteryMaxCharge = batteryMaxCharge_kwh;
		this->batteryInitialCharge = batteryInitialCharge_perc * this->batteryMaxCharge;
		this->batteryCurrentCharge = this->batteryInitialCharge;
		this->batteryStateOfCharge = batteryInitialCharge_perc;

		this->tankMaxCapacity = 0.0; //kg for hydrogen and liters for fuel
		this->tankInitialCapacity = 0.0;
		this->tankCurrentCapacity = 0.0;
		this->tankStateOfCapacity = 0.0;
	}
    // diesel and hydrogen have only tanks
    else if (this->powerType == TrainTypes::PowerType::diesel || this->powerType == TrainTypes::PowerType::hydrogen){
		this->batteryMaxCharge = 0.0;
		this->batteryInitialCharge = 0.0;
		this->batteryCurrentCharge = 0.0;
		this->batteryStateOfCharge = 0.0;

		this->tankMaxCapacity = tankMaxCapacity_; //kg for hydrogen and liters for fuel
		this->tankInitialCapacity = tankInitialCapacity_perc * this->tankMaxCapacity;
		this->tankCurrentCapacity = this->tankInitialCapacity;
		this->tankStateOfCapacity = tankInitialCapacity_perc;
	}
    // hybrid locomotives have both source of energy
    else {
        this->batteryMaxCharge = batteryMaxCharge_kwh;
        this->batteryInitialCharge = batteryInitialCharge_perc * this->batteryMaxCharge;
        this->batteryCurrentCharge = this->batteryInitialCharge;
        this->batteryStateOfCharge = batteryInitialCharge_perc;

        this->tankMaxCapacity = tankMaxCapacity_; //kg for hydrogen and liters for fuel
        this->tankInitialCapacity = tankInitialCapacity_perc * this->tankMaxCapacity;
        this->tankCurrentCapacity = this->tankInitialCapacity;
        this->tankStateOfCapacity = tankInitialCapacity_perc;
    }


	this->trackCurvature = 0;
	this->trackGrade = 0;
    this->hostLink = std::shared_ptr<NetLink>();
	this->discritizedLamda.push_back(0.0);
	this->throttleLevels = this->defineThrottleLevels();
};	




double Locomotive::getHyperbolicThrottleCoef(double & trainSpeed) {
	double dv, um;
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
		lamdaD = pow((double)N / (double)this->Nmax, 2);
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
	//if the minimum is too high, we set it to the highest possible value (Restrict max notch to #)
	if (crntLocNotch > this->maxLocNotch) {
		crntLocNotch = this->maxLocNotch;
		lamdaDiscretized = this->discritizedLamda[crntLocNotch];
	}
	return lamdaDiscretized;
}

double Locomotive::getThrottleLevel(double & trainSpeed, double& TrainAcceleration, bool &optimize, double &optimumThrottleLevel) {
	double currentThrottleLevel = 0;
    double throttleL = 0;
	if (TrainAcceleration < 0) {
		throttleL = this->throttleLevels[0];
	}
	else {
		throttleL = getDiscretizedThrottleCoef(trainSpeed);
	};

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
		this->currentLocNotch = this->discritizedLamda.index(lamdaDiscretized);
		// readjust its value if it is higher than what the locomotive have
		if (this->currentLocNotch > this->maxLocNotch) { this->currentLocNotch = this->maxLocNotch; }
	}

};

Vector<double> Locomotive::defineThrottleLevels() {
	double lamdaD;
	int index;
	Vector<double> lamdaDlst(this->Nmax,0);
	//for each notch, calculate the throttle level
	for (int N = 1; N <= this->Nmax; N++) {
		lamdaD = pow(N / this->Nmax, 2);
		index = N - 1;
		lamdaDlst[index] = lamdaD;
	};
	return lamdaDlst;
}

double Locomotive::getTractiveForce(double &frictionCoef, double &trainSpeed, double &trainAcceleration,
								 bool &optimize, double &optimumThrottleLevel) {
	if (!this->isLocOn) {
		return 0;
	}
	double f1,f = 0;
	f1 = frictionCoef * this->currentWeight * 1000 * this->g;
	if (trainSpeed == 0) {
		return f1;
	}
	else {
		f = min((1000 * this->transmissionEfficiency * this->getThrottleLevel(trainSpeed, trainAcceleration, optimize, optimumThrottleLevel)
			* (this->maxPower) / trainSpeed), f1);
		return f;
	};
}

double Locomotive::getSharedVirtualTractivePower(double &trainSpeed, double& trainAcceleration, 
	double& sharedWeight, double& sharedResistance) {

	return ((sharedWeight * trainAcceleration) + sharedResistance) * trainSpeed * this->isLocOn;
}

double Locomotive::getEnergyConsumption(double& LocomotiveVirtualTractivePower, double &trainSpeed, 
                                            double& trainAcceleration, double &timeStep) {
	// if the locomotive is turned off already, do not consume anything
	if (!this->isLocOn) {
		return 0.0;
	}
	// else calculate how much to consume
	double tractivePower = LocomotiveVirtualTractivePower;
	double unitConversionFactor = timeStep / static_cast<unsigned int>(3600 * 1000);
	if (tractivePower == 0) {
		return this->auxiliaryPower * unitConversionFactor;
	}
	else if(tractivePower > 0) {
        return (((tractivePower / EC::getDriveLineEff(trainSpeed, this->currentLocNotch, this->powerType)) +
			this->auxiliaryPower) * unitConversionFactor);
	}
	else {
		if (this->powerType == TrainTypes::PowerType::diesel || this->powerType == TrainTypes::PowerType::hydrogen) {
			return 0.0;
		}
		else {
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
            return ((tractivePower * regenerativeEff * EC::getDriveLineEff(trainSpeed, this->currentLocNotch, this->powerType) +
				this->auxiliaryPower) * unitConversionFactor);
		}		
	}
}

bool Locomotive::consumeFuelDiesel(double EC_kwh, double dieselConversionFactor, double dieselDensity) {
    // tenderCurrentCapacity is in liters in that case
    double consumedQuantity = (EC_kwh * dieselConversionFactor); //convert to liters
    if (this->tankCurrentCapacity >= consumedQuantity && this->tankStateOfCapacity > DefaultLocomotiveMinTankSOT) {
        this->energyConsumed = EC_kwh;
        this->cumEnergyConsumed += this->energyConsumed;

        this->tankCurrentCapacity -= consumedQuantity;
        this->tankStateOfCapacity = this->tankCurrentCapacity / this->tankMaxCapacity;
        this->currentWeight -= consumedQuantity * dieselDensity;
        this->currentWeight = (this->currentWeight <= this->emptyWeight) ? this->emptyWeight : this->currentWeight;
        return true; // returns the tender still has fuel and can provide it to the locomotive
    }
    return false; // return the tender is empty now and cannot provide any more fuel
}

bool Locomotive::consumeBattery(double EC_kwh) {
    if (!this->hostLink->hasCatenary){
        if (this->batteryCurrentCharge >= EC_kwh && this->batteryStateOfCharge > DefaultLocomotiveMinBatterySOC) {
            this->energyConsumed = EC_kwh;
            this->cumEnergyConsumed += this->energyConsumed;

            this->batteryCurrentCharge -= EC_kwh;
            this->batteryStateOfCharge = this->batteryCurrentCharge / this->batteryMaxCharge;
            return true; // returns the tender still has fuel and can provide it to the locomotive
        }
        return false; // return the tender is empty now and cannot provide any more energy
    }
    else {
        this->energyConsumed = EC_kwh;
        this->cumEnergyConsumed += this->energyConsumed;
        this->hostLink->catenaryCumConsumedEnergy += EC_kwh;
        return true;
    }

}

bool Locomotive::consumeFuelHydrogen(double EC_kwh, double hydrogenConversionFactor) {
    // tenderCurrentCapacity is in kg in that case
    double consumedWeight = (EC_kwh * hydrogenConversionFactor); //converts to kg
    if (this->tankCurrentCapacity >= consumedWeight && this->tankStateOfCapacity > DefaultLocomotiveMinTankSOT) {
        this->energyConsumed = EC_kwh;
        this->cumEnergyConsumed += this->energyConsumed;

        this->tankCurrentCapacity -= consumedWeight;
        this->tankStateOfCapacity = this->tankCurrentCapacity / this->tankMaxCapacity;
        this->currentWeight -= consumedWeight;
        this->currentWeight = (this->currentWeight <= this->emptyWeight) ? this->emptyWeight : this->currentWeight;
        return true; // returns the tender still has fuel and can provide it to the locomotive
    }
    return false; // return the tender is empty now and cannot provide any more fuel
}

bool Locomotive::refillBattery(double EC_kwh) {
    double ER = std::abs(EC_kwh);   // because the passed EC_kwh is negative when it is recharge value

    if (this->batteryCurrentCharge < this->batteryMaxCharge) {
        this->energyRegenerated = ER;
        this->cumEnergyRegenerated += this->energyRegenerated;

        this->batteryCurrentCharge += ER;
        this->batteryStateOfCharge = this->batteryCurrentCharge / this->batteryMaxCharge;
        return true; // return the battery is rechargable
    }
    else {
        return false; // return the battery is full
    }

}

bool Locomotive::rechageCatenary(double EC_kwh) {
    if (this->hostLink->hasCatenary){
        this->hostLink->catenaryCumRegeneratedEnergy += std::abs(EC_kwh);
        return true;
    }
    return false;
}

bool Locomotive::consumeFuel(double EC_kwh, double dieselConversionFactor,
	double hydrogenConversionFactor, double dieselDensity) {
    // reset energy stats first
    this->energyConsumed = 0.0;
    this->energyRegenerated = 0.0;

    // if energy should be consumed
    if (EC_kwh > 0.0) {
        if (this->powerType == TrainTypes::PowerType::diesel ) {
            return this->consumeFuelDiesel(EC_kwh, dieselConversionFactor, dieselDensity);
        }
        else if (this->powerType == TrainTypes::PowerType::dieselElectric) {
            if (! this->consumeFuelDiesel(EC_kwh, dieselConversionFactor, dieselDensity)) {
                return this->consumeBattery(EC_kwh);
            }
            return true;
        }
        else if (this->powerType == TrainTypes::PowerType::electric) {
            return this->consumeBattery(EC_kwh);
		}
        else if (this->powerType == TrainTypes::PowerType::hydrogen) {
            return this->consumeFuelHydrogen(EC_kwh, hydrogenConversionFactor);
		}
        else if (this->powerType == TrainTypes::PowerType::hydrogenHybrid) {
            if (! this->consumeFuelHydrogen(EC_kwh, hydrogenConversionFactor)) {
                return this->consumeBattery(EC_kwh);
            }
            return true;
        }
        // if it is something else
		return false;
	}
    // if energy is generated
    else if (EC_kwh < 0.0) {
        return this->refillBattery(EC_kwh);
	}
    else {
        return true;  // no need to do any further calculations if no energy is feed or consumed
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

double Locomotive::getNetForce(double &frictionCoef, double &trainAcceleration,
	double &trainSpeed, bool &optimize, double &optimumThrottleLevel) {
	return (this->getTractiveForce(frictionCoef, trainAcceleration, trainSpeed, optimize, optimumThrottleLevel) - this->getResistance(trainSpeed));
}

ostream& operator<<(std::ostream& ostr, const Locomotive& loco) {
	ostr << "Locomotive: Power: " << loco.maxPower << ", transmission eff.: " << loco.transmissionEfficiency;
	ostr << ", length: " << loco.length << ", drag coef: " << loco.dragCoef << ", frontal area: " << loco.frontalArea;
	ostr << ", current weight: " << loco.currentWeight << ", no axles: " << loco.noOfAxiles;
	ostr << ", max speed: " << loco.maxSpeed << ", locomotive type: " << TrainTypes::PowerTypeToStr(loco.powerType);
	ostr << ", battery capacity: " << loco.batteryMaxCharge << ", current battery status: " << loco.batteryStateOfCharge;
	ostr << ", tank capacity: " << loco.tankMaxCapacity << ", current tank status: " << loco.tankStateOfCapacity << std::endl;
	return ostr;
}
