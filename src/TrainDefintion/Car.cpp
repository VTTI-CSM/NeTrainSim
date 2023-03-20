#include <cmath>
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
    double tenderMaxCapacity_l,
	double tenderInitialCapacity_perc,
    std::string carName) {
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
    if (this->carType == TrainTypes::CarType::cargo){
        this->setBattery(0.0, 0.0, 1.0, EC::DefaultCarBatteryCRate);
        this->SetTank(0.0, 0.0, EC::DefaultCarMinTankDOD);
    }
    else if (TrainTypes::carRechargableTechnologies.exist(this->carType) &&
             this->carType != TrainTypes::CarType::cargo) {
        this->setBattery(batteryMaxCapacity_kwh, batteryInitialCharge_perc,
                         EC::DefaultCarBatteryDOD, EC::DefaultCarBatteryCRate);
        this->SetTank(0.0, 0.0, EC::DefaultCarMinTankDOD);
	}
    else if (TrainTypes::carNonRechargableTechnologies.exist(this->carType) &&
             this->carType != TrainTypes::CarType::cargo) {
        this->setBattery(0.0, 0.0, 1.0, EC::DefaultCarBatteryCRate);
        this->SetTank(tenderMaxCapacity_l, tenderInitialCapacity_perc, EC::DefaultCarMinTankDOD);
	}
    this->hostLink = std::shared_ptr<NetLink>();
	this->trackCurvature = 0;
	this->trackGrade = 0;
};

void Car::setCarCurrentWeight(double newCurrentWeight) {
    if (newCurrentWeight > this->emptyWeight) {
        this->currentWeight = newCurrentWeight;
    }
}

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


std::pair<bool, double> Car::consumeFuel(double timeStep, double trainSpeed,
                                         double EC_kwh, double dieselConversionFactor,
                                         double biodieselConversionFactor,
                                         double hydrogenConversionFactor, double dieselDensity,
                                         double biodieselDensity, double hydrogenDensity) {
    this->energyConsumed = 0.0;
    this->energyRegenerated = 0.0;

    if (EC_kwh > 0.0) {
        if (this->carType == TrainTypes::CarType::dieselTender) {
            return this->consumeFuelDiesel(EC_kwh, dieselConversionFactor, dieselDensity);
        }
        else if (this->carType == TrainTypes::CarType::biodieselTender) {
            return this->consumeFuelBioDiesel(EC_kwh, biodieselConversionFactor, biodieselDensity);
        }
        else if (this->carType == TrainTypes::CarType::batteryTender) {
            return this->consumeElectricity(timeStep, EC_kwh);
        }
        else if (this->carType == TrainTypes::CarType::hydrogenTender) {
            return this->consumeFuelHydrogen(EC_kwh, hydrogenConversionFactor, hydrogenDensity);
        }
        return std::pair(false, EC_kwh);
    }
    return std::pair(false, EC_kwh);  // cannot consume
}

ostream& operator<<(ostream& ostr, Car& car) {
	ostr << "Rail Car:: length: " << car.length << ", drag: " << car.dragCoef << ", frontal area: " << car.frontalArea;
	ostr << ", car type: " << TrainTypes::carTypeToStr(car.carType) << ", current weight: " << car.currentWeight;
	ostr << ", total weight: " << car.emptyWeight << ", no axles: " << car.noOfAxiles << endl;
	return ostr;
};
