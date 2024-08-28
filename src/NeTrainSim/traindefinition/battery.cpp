#include "battery.h"
#include <iostream>

double Battery::getBatteryMaxCharge() const {
    return this->batteryMaxCapacity;
}

void Battery::setBatteryMaxCharge(double newMaxCharge) {
    batteryMaxCapacity = newMaxCharge;
}

double Battery::getBatteryInitialCharge() const {
    return this->batteryInitialCharge;
}

void Battery::setBatteryInitialCharge(double newInitialChargePercentage) {
    this->batteryInitialCharge = batteryMaxCapacity * newInitialChargePercentage;
}

double Battery::getBatteryCurrentCharge() const {
    return this->batteryCurrentCharge;
}

std::pair<bool, double> Battery::consumeBattery(double timeStep, double consumedCharge) {
    // check if the battery is drainable,
    if (! isBatteryDrainable(consumedCharge)) { return std::make_pair(false, consumedCharge); }
    double batteryMax_kwh = getBatteryMaxDischarge(timeStep);
    if (consumedCharge > batteryMax_kwh) {
        double EC_extra_kwh = consumedCharge - batteryMax_kwh;
        this->batteryCumEnergyConsumed += batteryMax_kwh;
        this->batteryCumNetEnergyConsumed += batteryMax_kwh;
        batteryCurrentCharge -= batteryMax_kwh;
        batteryStateOfCharge = batteryCurrentCharge / batteryMaxCapacity;
        return std::make_pair(true, EC_extra_kwh);
    }
    this->batteryCumEnergyConsumed += consumedCharge;
    this->batteryCumNetEnergyConsumed += consumedCharge;
    batteryCurrentCharge -= consumedCharge;
    batteryStateOfCharge = batteryCurrentCharge / batteryMaxCapacity;
    return std::make_pair(true, 0.0);
}

double Battery::rechargeBatteryForHybrids(double timeStep, double recharge) {
    if (! isBatteryRechargable()) { return 0.0; }
    double batteryMax_kwh = getBatteryMaxRecharge(timeStep);
    if (recharge > batteryMax_kwh) {
        this->batteryCumEnergyConsumed -= batteryMax_kwh;
        this->batteryCumNetEnergyConsumed -= batteryMax_kwh;
        batteryCurrentCharge += batteryMax_kwh;
        batteryStateOfCharge = batteryCurrentCharge / batteryMaxCapacity;
        return batteryMax_kwh;
    }
    this->batteryCumEnergyConsumed -= recharge;
    this->batteryCumNetEnergyConsumed -= recharge;
    batteryCurrentCharge += recharge;
    batteryStateOfCharge = batteryCurrentCharge / batteryMaxCapacity;
    return recharge;
}

double Battery::rechargeBatteryByRegeneratedEnergy(double timeStep, double recharge) {
    if (! isBatteryRechargable()) { return 0.0; }
    double batteryMax_kwh = getBatteryMaxRecharge(timeStep);
    if (recharge > batteryMax_kwh) {
        this->batteryCumEnergyRegenerated += batteryMax_kwh;
        this->batteryCumNetEnergyConsumed -= batteryMax_kwh;
        batteryCurrentCharge += batteryMax_kwh;
        batteryStateOfCharge = batteryCurrentCharge / batteryMaxCapacity;
        return batteryMax_kwh;
    }
    this->batteryCumEnergyRegenerated += recharge;
    this->batteryCumNetEnergyConsumed -= recharge;
    batteryCurrentCharge += recharge;
    batteryStateOfCharge = batteryCurrentCharge / batteryMaxCapacity;
    return recharge;
}

double Battery::getBatteryStateOfCharge() const {
    return batteryStateOfCharge;
}

double Battery::getBatteryDOD() const {
    return batteryDOD;
}

void Battery::setBatteryDOD(double newBatteryDOD) {
    if (newBatteryDOD<=1 && newBatteryDOD>0.0){
        batteryDOD = newBatteryDOD;
    }
    else {
        throw std::invalid_argument(
                    "the Depth of Discharge must be between 0.0 and "
                    "1.0. 0.0: no discharge is allowed, 1.0: full "
                    "discharge is allowed");
    }
}

double Battery::getBatteryCRate() const {
    return batteryDischargeCRate;
}

void Battery::setBatteryCRate(double newBatteryCRate) {
    batteryDischargeCRate = newBatteryCRate;
    batteryRechargeCRate = 0.5 * newBatteryCRate;
}

bool Battery::isBatteryDrainable(double requiredCharge) {
    // check if the battery reaches the low level of charge,
    // enable the recharge request
    this->isBatteryExceedingThresholds();
    return (requiredCharge <= this->batteryCurrentCharge && batteryStateOfCharge > (1.0- batteryDOD));
}

bool Battery::isBatteryRechargable() {
    // check if the battery reaches the max level of charge,
    // disable the recharge request
    this->isBatteryExceedingThresholds();
    return (batteryStateOfCharge <= batteryRechargeSOCUpperBound);
}


bool Battery::isBatteryExceedingThresholds(){
    if (this->batteryStateOfCharge >= this->batteryRechargeSOCUpperBound) {
        this->enableRecharge = false;
    }
    else if (this->batteryStateOfCharge <= this->batteryRechargeSOCLowerBound) {
        this->enableRecharge = true;
    }
    return this->enableRecharge;
}

bool Battery::isBatteryOutsideBoundingThresholds() {
    return this->batteryStateOfCharge >= this->batteryRechargeSOCUpperBound ||
           this->batteryStateOfCharge <= this->batteryRechargeSOCLowerBound;
}


double Battery::getBatteryMaxDischarge(double timeStep) {
    // returns the max discharge in kWh
    return (batteryMaxCapacity * batteryDischargeCRate) * timeStep / (double)3600.0;
}

double Battery::getBatteryMaxRecharge(double timeStep){
    // returns the max recharge in kWh
    return (batteryMaxCapacity * batteryRechargeCRate) * timeStep / (double)3600.0;
}
bool Battery::isRechargeRequired() const {
    return enableRecharge;
}

double Battery::getBatteryRechargeSOCUpperBound() const {
    return batteryRechargeSOCUpperBound;
}

void Battery::setBatteryRechargeSOCUpperBound(double newBatteryRechargeSOCUpperBound) {
    if (newBatteryRechargeSOCUpperBound < (1 - batteryDOD)) {
        batteryRechargeSOCUpperBound = 1 - batteryDOD;
    }
    else if (newBatteryRechargeSOCUpperBound > batteryDOD) {
        batteryRechargeSOCUpperBound = batteryDOD;
    }
    else{
        batteryRechargeSOCUpperBound = newBatteryRechargeSOCUpperBound;
    }

    if (batteryRechargeSOCUpperBound < batteryRechargeSOCLowerBound) {
        batteryRechargeSOCUpperBound = batteryRechargeSOCLowerBound;
    }
}

double Battery::getBatteryRechargeSOCLowerBound() const {
    return batteryRechargeSOCLowerBound;
}

void Battery::setBatteryRechargeSOCLowerBound(double newBatteryRechargeSOCLowerBound) {
    if (newBatteryRechargeSOCLowerBound < (1 - batteryDOD)) {
        batteryRechargeSOCLowerBound = 1 - batteryDOD;
    }
    else if (newBatteryRechargeSOCLowerBound > batteryDOD) {
        batteryRechargeSOCLowerBound = batteryDOD;
    }
    else{
        batteryRechargeSOCLowerBound = newBatteryRechargeSOCLowerBound;
    }
}

double Battery::getBatteryCumEnergyConsumption() {
    return this->batteryCumEnergyConsumed;
}

double Battery::getBatteryCumEnergyRegenerated() {
    return this->batteryCumEnergyRegenerated;
}

double Battery::getBatteryCumNetEnergyConsumption() {
    return this->batteryCumNetEnergyConsumed;
}

bool Battery::batteryHasCharge() {
    return batteryStateOfCharge > (1.0- batteryDOD);
}

void Battery::setBattery(double maxCharge,
                         double initialChargePercentage,
                         double depthOfDischarge,
                         double batteryCRate,
                         double maxRechargeSOC,
                         double minRechargeSOC) {
    this->setBatteryMaxCharge(maxCharge);
    this->setBatteryInitialCharge(initialChargePercentage);
    this->batteryCurrentCharge = this->batteryInitialCharge;
    this->batteryStateOfCharge = initialChargePercentage;
    this->setBatteryDOD(depthOfDischarge);
    this->setBatteryCRate(batteryCRate);
    this->setBatteryRechargeSOCLowerBound(minRechargeSOC);
    this->setBatteryRechargeSOCUpperBound(maxRechargeSOC);
}
