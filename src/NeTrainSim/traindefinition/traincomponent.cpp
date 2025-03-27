#include "traincomponent.h"
using namespace std;

// if the get getResistance is not implemented in the derived class, return 0.0
double TrainComponent::getResistance(double trainSpeed) {
	return 0.0;
}

void TrainComponent::resetTimeStepConsumptions() {
    // reset the energy consumption for the time step.
	this->energyConsumed = 0.0;
	this->energyRegenerated = 0.0;
    this->cumEnergyConsumed = 0.0; // the step energy consumption of multiple technologies (e.g hybrid)
    this->cumEnergyRegenerated = 0.0; // the step energy regeneration of multiple technologies (e.g hybrid)
    this->carbonDioxideEmission = 0.0;
    this->cumCarbonDioxideEmission = 0.0;
}

void TrainComponent::setCurrentWeight(double newCurrentWeight) {
    // if the new gross weight is greater than the light weight, do not assign it.
    // todo: raise error if the new gross weight < the light weight
	if (newCurrentWeight > this->emptyWeight) {
		this->currentWeight = newCurrentWeight;
    } // end if
}

// consume the fuel required by the locomotive
std::pair<bool,double> TrainComponent::consumeFuel(double timeStep, double trainSpeed,
                                                   double EC_kwh,
                                                   double LocomotiveVirtualTractivePower,
                                                   double dieselConversionFactor,
                                                   double biodieselConversionFactor,
                                                   double hydrogenConversionFactor,
                                                   double dieselDensity,
                                                   double biodieselDensity,
                                                   double hydrogenDensity) {
    // a default value for a rail car of false and 0.0. which indicates the railcar cannot consume fuel
	return std::make_pair(false,0.0);
}


std::pair<bool, double> TrainComponent::consumeFuelDiesel(double EC_kwh, double dieselConversionFactor,
									   double dieselDensity) {
	// tenderCurrentCapacity is in liters in that case
	double consumedQuantity = (EC_kwh * dieselConversionFactor); //convert to liters
    // check if the vehicle still has fuel to consume from
	if (this->isTankDrainable(consumedQuantity)) {
        // update statistics
		this->energyConsumed = EC_kwh;
		this->cumEnergyConsumed += this->energyConsumed;
        this->carbonDioxideEmission = EC::getEmissions(consumedQuantity, "diesel");
        this->cumCarbonDioxideEmission += this->carbonDioxideEmission;

        this->consumeTank(consumedQuantity); // consume the vehicle tank if available
        double newWeight = this->currentWeight - consumedQuantity * dieselDensity; // reduce vehicle weight
		this->setCurrentWeight(newWeight);
		return std::make_pair(true, 0.0); // returns the tender still has fuel and can provide it to the locomotive
    } // end if
	return std::make_pair(false, EC_kwh); // return the tender is empty now and cannot provide any more fuel
}

std::pair<bool, double> TrainComponent::consumeFuelBioDiesel(double EC_kwh, double bioDieselConversionFactor,
										  double bioDieselDensity) {
	// tenderCurrentCapacity is in liters in that case
	double consumedQuantity = (EC_kwh * bioDieselConversionFactor); //convert to liters
    // check if the vehicle still has fuel to consume from
	if (this->isTankDrainable(consumedQuantity)) {
        // update statistics
		this->energyConsumed = EC_kwh;
		this->cumEnergyConsumed += this->energyConsumed;
        this->carbonDioxideEmission = EC::getEmissions(consumedQuantity, "biodiesel");
        this->cumCarbonDioxideEmission += this->carbonDioxideEmission;

        this->consumeTank(consumedQuantity); // consume the vehicle tank if available
        double newWeight = this->currentWeight - consumedQuantity * bioDieselDensity;  // reduce vehicle weight
		this->setCurrentWeight(newWeight);
		return std::make_pair(true, 0.0); // returns the tender still has fuel and can provide it to the locomotive
    } // end if
	return std::make_pair(false, EC_kwh); // return the tender is empty now and cannot provide any more fuel
}

std::pair<bool, double> TrainComponent::consumeElectricity(double timeStep, double EC_kwh) {
    // if the link the vehicle is on does not have catenary, consume electricity from the battery
	if (! this->hostLink->hasCatenary){
        // consume electricity and return its results
        auto out = this->consumeBattery(timeStep, EC_kwh); // true if it could consume electricity, false otherwise
        // if it could consume all electricity
        if (out.first && out.second == 0.0){
            // update stats
            this->energyConsumed = EC_kwh;
            this->cumEnergyConsumed += this->energyConsumed;
        }
        // if it consume part of the electricity
        else if (out.first && out.second > 0.0) {
            // update stats
            this->energyConsumed = EC_kwh - out.second;
            this->cumEnergyConsumed += this->energyConsumed;
        }

        return out;
    } // end if
	else {
        // update stats
		this->energyConsumed = EC_kwh;
		this->cumEnergyConsumed += this->energyConsumed;
        this->hostLink->catenaryCumConsumedEnergy += EC_kwh;
        // return true as all energy required is consumed from the catenary
		return std::make_pair(true, 0.0);
    } // end else

}


std::pair<bool, double> TrainComponent::consumeFuelHydrogen(double EC_kwh, double hydrogenConversionFactor,
										 double hydrogenDensity) {
    // tenderCurrentCapacity is in liters in that case
	double consumedQuantity = (EC_kwh * hydrogenConversionFactor); //converts to litters
    // check if the vehicle still has fuel to consume from
	if (this->isTankDrainable(consumedQuantity)) {
        // update statistics
		this->energyConsumed = EC_kwh;
		this->cumEnergyConsumed += this->energyConsumed;

        // consume the vehicle tank if available
		this->consumeTank(consumedQuantity);
        double newWeight = this->currentWeight - consumedQuantity * hydrogenDensity; // reduce vehicle weight
		this->setCurrentWeight(newWeight);
		return std::make_pair(true, 0.0); // returns the tender still has fuel and can provide it to the locomotive
    } // end if
	return std::make_pair(false, EC_kwh); // return the tender is empty now and cannot provide any more fuel
}

double TrainComponent::refillBattery(double timeStep, double EC_kwh) {
	double ER = std::abs(EC_kwh);   // because the passed EC_kwh is negative when it is recharge value
	// get how much regenerated energy and pushed to the battery,
    // 0.0 means no energy was pushed to the battery
    this->energyRegenerated = this->rechargeBatteryByRegeneratedEnergy(timeStep, ER);
    this->cumEnergyRegenerated += this->energyRegenerated;
    // if the battery is full, it will return 0.0 recharged to the battery.
    // if the battery recharge all/part of the energy, it will return any other valye
    return (ER - this->energyRegenerated);
}

bool TrainComponent::rechargeCatenary(double EC_kwh) {
    // if the link the vehicle is on has catenary, recharge it
	if (this->hostLink->hasCatenary){
        // trasfer regenerated energy to the catenary
		this->hostLink->catenaryCumRegeneratedEnergy += std::abs(EC_kwh);
        // add the total amount of regenerated energy to the train total Regenerated Energy
        this->energyRegenerated = std::abs(EC_kwh);
        this->cumEnergyRegenerated += this->energyRegenerated;
		return true;
    } // end if
	return false;
}


ostream& operator<<(ostream& ostr, TrainComponent& comp) {
    ostr << "Component Name: "<< comp.name << "Gross weight: " << comp.currentWeight;
	return ostr;
}
