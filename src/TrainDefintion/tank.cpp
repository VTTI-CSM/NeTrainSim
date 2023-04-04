#include "tank.h"
#include <iostream>

double Tank::getTankMaxCapacity() const {
    return this->tankMaxCapacity;
}

void Tank::setTankMaxCapacity(double newMaxCapacity) {
    this->tankMaxCapacity = newMaxCapacity;
}

double Tank::getTankInitialCapacity() const {
    return tankInitialCapacity;
}

void Tank::setTankInitialCapacity(double newInitialCapacityPercentage) {
    tankInitialCapacity = tankMaxCapacity * newInitialCapacityPercentage;
}

double Tank::getTankCurrentCapacity() const {
    return tankCurrentCapacity;
}

double Tank::consumeTank(double consumedAmount) {
    if (! isTankDrainable(consumedAmount)) { return consumedAmount; }
    tankCurrentCapacity -= consumedAmount;
    tankStateOfCapacity = tankCurrentCapacity / tankMaxCapacity;
    return 0.0;
}

double Tank::getTankStateOfCapacity() const {
    return tankStateOfCapacity;
}

bool Tank::isTankDrainable(double consumedAmount) {
    return (consumedAmount <= this->tankCurrentCapacity && tankStateOfCapacity > (1.0- tankDOD));
}

double Tank::getTankDOD() const {
    return tankDOD;
}

void Tank::setTankDOD(double newTankDOD) {
    if (newTankDOD<=1 && newTankDOD>0.0){
        tankDOD = newTankDOD;
    }
    else {
        throw std::invalid_argument(
                    "the Depth of Discharge must be between 0.0 and "
                    "1.0. 0.0: no discharge is allowed, 1.0: full "
                    "discharge is allowed");
    }
}

void Tank::SetTank(double maxCapacity, double initialCapacityPercentage, double depthOfDischarge){
    this->setTankMaxCapacity(maxCapacity);
    this->setTankInitialCapacity(initialCapacityPercentage);
    tankCurrentCapacity = tankInitialCapacity;

    this->tankStateOfCapacity = initialCapacityPercentage;
    this->setTankDOD(depthOfDischarge);
}

