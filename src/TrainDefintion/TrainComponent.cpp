#include "TrainComponent.h"
using namespace std;


double TrainComponent::getResistance(double trainSpeed) {
	return 0.0;
}

void TrainComponent::resetTimeStepConsumptions() {
	this->energyConsumed = 0.0;
	this->energyRegenerated = 0.0;
}

void TrainComponent::setCurrentWeight(double newCurrentWeight) {
    if (newCurrentWeight > this->emptyWeight) {
        this->currentWeight = newCurrentWeight;
    }
}

std::pair<bool,double> TrainComponent::consumeFuel(double timeStep, double trainSpeed,
                                                   double EC_kwh,
                                                   double dieselConversionFactor,
                                                   double biodieselConversionFactor,
                                                   double hydrogenConversionFactor,
                                                   double dieselDensity,
                                                   double biodieselDensity,
                                                   double hydrogenDensity) {
    return std::make_pair(false,0.0);
}


std::pair<bool, double> TrainComponent::consumeFuelDiesel(double EC_kwh, double dieselConversionFactor,
                                       double dieselDensity) {
    // tenderCurrentCapacity is in liters in that case
    double consumedQuantity = (EC_kwh * dieselConversionFactor); //convert to liters
    if (this->isTankDrainable(consumedQuantity)) {
        this->energyConsumed = EC_kwh;
        this->cumEnergyConsumed += this->energyConsumed;
        this->consumeTank(consumedQuantity);
        double newWeight = this->currentWeight - consumedQuantity * dieselDensity;
        this->setCurrentWeight(newWeight);
        return std::make_pair(true, 0.0); // returns the tender still has fuel and can provide it to the locomotive
    }
    return std::make_pair(false, EC_kwh); // return the tender is empty now and cannot provide any more fuel
}

std::pair<bool, double> TrainComponent::consumeFuelBioDiesel(double EC_kwh, double bioDieselConversionFactor,
                                          double bioDieselDensity) {
    // tenderCurrentCapacity is in liters in that case
    double consumedQuantity = (EC_kwh * bioDieselConversionFactor); //convert to liters
    if (this->isTankDrainable(consumedQuantity)) {
        this->energyConsumed = EC_kwh;
        this->cumEnergyConsumed += this->energyConsumed;
        this->consumeTank(consumedQuantity);
        double newWeight = this->currentWeight - consumedQuantity * bioDieselDensity;
        this->setCurrentWeight(newWeight);
        return std::make_pair(true, 0.0); // returns the tender still has fuel and can provide it to the locomotive
    }
    return std::make_pair(false, EC_kwh); // return the tender is empty now and cannot provide any more fuel
}

std::pair<bool, double> TrainComponent::consumeElectricity(double timeStep, double EC_kwh) {
    if (! this->hostLink->hasCatenary){
        this->energyConsumed = EC_kwh;
        this->cumEnergyConsumed += this->energyConsumed;
        return this->consumeBattery(timeStep, EC_kwh);
    }
    else {
        this->energyConsumed = EC_kwh;
        this->cumEnergyConsumed += this->energyConsumed;
        this->hostLink->catenaryCumConsumedEnergy += EC_kwh;
        return std::make_pair(true, 0.0);
    }

}


std::pair<bool, double> TrainComponent::consumeFuelHydrogen(double EC_kwh, double hydrogenConversionFactor,
                                         double hydrogenDensity) {
    // tenderCurrentCapacity is in kg in that case
    double consumedQuantity = (EC_kwh * hydrogenConversionFactor); //converts to litters
    if (this->isTankDrainable(consumedQuantity)) {
        this->energyConsumed = EC_kwh;
        this->cumEnergyConsumed += this->energyConsumed;
        this->consumeTank(consumedQuantity);
        double newWeight = this->currentWeight - consumedQuantity * hydrogenDensity;
        this->setCurrentWeight(newWeight);
        return std::make_pair(true, 0.0); // returns the tender still has fuel and can provide it to the locomotive
    }
    return std::make_pair(false, EC_kwh); // return the tender is empty now and cannot provide any more fuel
}

bool TrainComponent::refillBattery(double timeStep, double EC_kwh) {
    double ER = std::abs(EC_kwh);   // because the passed EC_kwh is negative when it is recharge value
    // get how much regenerated energy and pushed to the battery,
    // 0.0 means no energy was pushed to the battery
    this->energyRegenerated = this->rechargeBattery(timeStep, ER);
    this->cumEnergyRegenerated += this->energyRegenerated;
    // if the battery is full, it will return 0.0 recharged to the battery.
    // if the battery recharge all/part of the energy, it will return any other valye
    return (this->energyRegenerated != 0.0);
}

bool TrainComponent::rechargeCatenary(double EC_kwh) {
    if (this->hostLink->hasCatenary){
        this->hostLink->catenaryCumRegeneratedEnergy += std::abs(EC_kwh);
        return true;
    }
    return false;
}


ostream& operator<<(ostream& ostr, TrainComponent& stud) {
	ostr << "";
	return ostr;
}
