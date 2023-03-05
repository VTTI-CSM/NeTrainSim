#include "TrainComponent.h"
using namespace std;


double TrainComponent::getResistance(double trainSpeed) {
	return 0.0;
}

void TrainComponent::resetTimeStepConsumptions() {
	this->energyConsumed = 0.0;
	this->energyRegenerated = 0.0;
}

bool TrainComponent::consumeFuel(double EC_kwh, double dieselConversionFactor,
    double hydrogenConversionFactor, double dieselDensity) {
	return false;
}

ostream& operator<<(ostream& ostr, TrainComponent& stud) {
	ostr << "";
	return ostr;
}
