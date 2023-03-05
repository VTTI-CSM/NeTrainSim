#include <iostream>
#include "car.h"
#include "TrainTypes.h"
#include "EnergyConsumption.h"
#define stringify( name ) #name

using namespace std;

Car::Car(double carLength_m, double carDragCoef, double carFrontalArea_sqm, double carEmptyWeight_t,
	double carCurrentWeight_t, int carNoOfAxiles, int carType,
	double auxiliaryPower_kw,
	double batteryMaxCapacity_kwh,
	double batteryInitialCharge_perc,
	double tenderMaxCapacity_kg_l,
	double tenderInitialCapacity_perc,
	std::string carName)
{
	// <summary> this method is used to construct a rail car 
	// <para> carLength is the total length of the car in meters</para>
	this->name = carName;
	this->length = carLength_m;
	this->dragCoef = carDragCoef;
	this->frontalArea = carFrontalArea_sqm;
	this->emptyWeight = carEmptyWeight_t;
	this->currentWeight = carCurrentWeight_t;
	this->noOfAxiles = carNoOfAxiles;
	this->carType = static_cast<TrainTypes::CarType>(carType);;
	this->auxiliaryPower = auxiliaryPower_kw;

	if (this->carType == TrainTypes::CarType::batteryTender) {
		this->batteryMaxCharge= batteryMaxCapacity_kwh;
		this->batteryInitialCharge = batteryInitialCharge_perc * this->batteryMaxCharge;
		this->batteryCurrentCharge = this->batteryInitialCharge;
		this->batteryStateOfCharge = batteryInitialCharge_perc;

		this->tankMaxCapacity = 0.0; //kg for hydrogen, liters for fuel
		this->tankInitialCapacity = 0.0;
		this->tankCurrentCapacity = 0.0;
		this->tankStateOfCapacity = 0.0;
	}
	else if (this->carType == TrainTypes::CarType::dieselTender || this->carType == TrainTypes::CarType::hydrogenTender) {
		this->tankMaxCapacity = tenderMaxCapacity_kg_l; //kg for hydrogen, liters for fuel
		this->tankInitialCapacity = tenderInitialCapacity_perc * this->tankMaxCapacity;
		this->tankCurrentCapacity = this->tankInitialCapacity;
		this->tankStateOfCapacity = tenderInitialCapacity_perc;

		this->batteryMaxCharge= 0.0;
		this->batteryInitialCharge = 0.0;
		this->batteryCurrentCharge = 0.0;
		this->batteryStateOfCharge = 0.0;
	}
    this->hostLink = std::shared_ptr<NetLink>();
	this->trackCurvature = 0;
	this->trackGrade = 0;
};

double Car::getCargoNetWeight() {
	if (this->carType == TrainTypes::CarType::cargo) {
		return this->currentWeight - this->emptyWeight;
	}
	return 0.0;
}

double Car::getResistance(double trainSpeed) {
	// these calculations depend of US units, so these are the conversions factors from meteric system
	double rVal;
	trainSpeed *= 2.23694;
	rVal = 1.5 + 18 / ((this->currentWeight * 1.10231) / this->noOfAxiles) + 0.03 * trainSpeed
		+ (this->frontalArea * 10.7639) * this->dragCoef * (pow(trainSpeed, 2)) / (this->currentWeight * 1.10231);
	rVal = (rVal) * ((this->currentWeight * 1.10231)) + 20 * (this->currentWeight * 1.10231) * (this->trackGrade);
	rVal += abs(this->trackCurvature) * 20 * 0.04 * (this->currentWeight * 1.10231);
	rVal *= (4.44822);
	return rVal;
}

double Car::getEnergyConsumption(double &timeStep) {
	return this->auxiliaryPower* timeStep* (3600 / 1000);
}

bool Car::consumeDiesel(double EC_kwh, double dieselConversionFactor, double dieselDensity) {
    // tenderCurrentCapacity is in liters in that case
    double consumedQuantity = (EC_kwh * dieselConversionFactor); //convert to liters
    if (this->tankCurrentCapacity >= consumedQuantity && this->tankStateOfCapacity > DefaultCarMinTankSOT) {
        this->energyConsumed = EC_kwh;
        this->cumEnergyConsumed += this->energyConsumed;

        this->tankCurrentCapacity -= consumedQuantity;
        this->tankStateOfCapacity = this->tankCurrentCapacity / this->tankMaxCapacity;
        this->currentWeight -= consumedQuantity * dieselDensity;
        return true; // returns the tender still has fuel and can provide it to the locomotive
    }
    return false; // return the tender is empty now and cannot provide any more fuel
}

bool Car::consumeBattery(double EC_kwh) {
    if (! this->hostLink->hasCatenary) {
        if (this->batteryCurrentCharge >= EC_kwh && this->batteryStateOfCharge > DefaultCarMinBatterySOC) {
            this->energyConsumed = EC_kwh;
            this->cumEnergyConsumed += this->energyConsumed;

            this->batteryCurrentCharge -= EC_kwh;
            this->batteryStateOfCharge = this->batteryStateOfCharge / this->batteryMaxCharge;
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

bool Car::consumeHydrogen(double EC_kwh, double hydrogenConversionFactor) {
    // tenderCurrentCapacity is in kg in that case
    double consumedWeight = (EC_kwh * hydrogenConversionFactor); //converts to kg
    if (this->tankCurrentCapacity >= consumedWeight && this->tankStateOfCapacity > DefaultCarMinTankSOT) {
        this->energyConsumed = EC_kwh;
        this->cumEnergyConsumed += this->energyConsumed;

        this->tankCurrentCapacity -= consumedWeight;
        this->tankStateOfCapacity = this->tankCurrentCapacity / this->tankMaxCapacity;
        this->currentWeight -= consumedWeight;
        return true; // returns the tender still has fuel and can provide it to the locomotive
    }
    return false; // return the tender is empty now and cannot provide any more fuel
}

bool Car::RefillBattery(double EC_kwh) {
    if (this->carType == TrainTypes::CarType::batteryTender) {
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
    return true; // you do not want to process anything more

}

bool Car::consumeFuel(double EC_kwh, double dieselConversionFactor,
	double hydrogenConversionFactor, double dieselDensity) {
    this->energyConsumed = 0.0;
    this->energyRegenerated = 0.0;

    if (EC_kwh > 0.0) {
        if (this->carType == TrainTypes::CarType::dieselTender) {
            return this->consumeDiesel(EC_kwh, dieselConversionFactor, dieselDensity);
        }
        else if (this->carType == TrainTypes::CarType::batteryTender) {
            return this->consumeBattery(EC_kwh);
        }
        else if (this->carType == TrainTypes::CarType::hydrogenTender) {
            return this->consumeHydrogen(EC_kwh, hydrogenConversionFactor);
        }
        return false;
    }
    return false;  // cannot consume

}

ostream& operator<<(ostream& ostr, Car& car) {
	ostr << "Rail Car:: length: " << car.length << ", drag: " << car.dragCoef << ", frontal area: " << car.frontalArea;
	ostr << ", car type: " << TrainTypes::carTypeToStr(car.carType) << ", current weight: " << car.currentWeight;
	ostr << ", total weight: " << car.emptyWeight << ", no axles: " << car.noOfAxiles << endl;
	return ostr;
};
