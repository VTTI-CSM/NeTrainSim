/**
 * @file battery.h
 * @brief This file declares the Battery class.
 *        The Battery class represents a battery used in a train or locomotive.
 *        It stores information about the battery's capacity, charge level, state of charge,
 *        discharge rate, recharge rate, depth of discharge, and other properties.
 *        The Battery class provides methods for setting and retrieving battery properties,
 *        consuming and recharging the battery, and checking battery status.
 *        The Battery class is intended to be used in a train simulation system.
 *        Note: The implementation of the class is not provided in this declaration file.
 *              It should be implemented separately in a corresponding source file.
 * @author Ahmed Aredah
 * @date 3/20/2023
 */

#ifndef BATTERY_H
#define BATTERY_H

#include <iostream>

class Battery {
private:
    double batteryMaxCapacity;               // Battery max capacity
    double batteryInitialCharge;             // Battery initial capacity
    double batteryCurrentCharge;             // Battery current charge
    double batteryStateOfCharge;             // Battery State of Charge
    double batteryDischargeCRate;            // The C-Rate value of the battery while discharging
    double batteryRechargeCRate;             // The C-Rate value of the battery while recharging
    double batteryDOD;                       // The minimum battery depth of discharge
    double batteryRechargeSOCUpperBound;     // The depth of recharge at which the battery stops recharging
    double batteryRechargeSOCLowerBound;     // The depth of discharge at which the battery requires a recharge
    bool enableRecharge = false;                     // Indicates whether the battery needs to recharge
    double batteryCumEnergyConsumed = 0.0;         // Cumulative energy consumed by the battery
    double batteryCumEnergyRegenerated = 0.0;      // Cumulative energy regenerated by the battery
    double batteryCumNetEnergyConsumed = 0.0;      // Cumulative net energy consumed by the battery

public:
    /**
     * @brief Sets the battery properties.
     * @param maxCharge The maximum charge the battery can hold in kWh.
     * @param initialChargePercentage The initial charge percentage of the battery.
     * @param depthOfDischarge The allowable depth of discharge for the battery.
     * @param batteryCRate The C-Rate of discharge for the battery.
     * @param maxRechargeSOC The maximum State of Charge the battery reaches when recharging (optional).
     * @param minRechargeSOC The minimum State of Charge the battery reaches when being drained (optional).
     */
    void setBattery(double maxCharge, double initialChargePercentage, double depthOfDischarge,
                    double batteryCRate, double maxRechargeSOC = 0.9, double minRechargeSOC = 0.5);

    /**
     * @brief Gets the maximum charge of the battery.
     * @return The maximum charge of the battery in kWh.
     */
    double getBatteryMaxCharge() const;

    /**
     * @brief Sets the maximum charge of the battery.
     * @param newMaxCharge The new maximum charge of the battery in kWh.
     */
    void setBatteryMaxCharge(double newMaxCharge);

    /**
     * @brief Gets the initial charge of the battery.
     * @return The initial charge of the battery in kWh.
     */
    double getBatteryInitialCharge() const;

    /**
     * @brief Sets the initial charge of the battery.
     * @param newInitialCharge The new initial charge of the battery in kWh.
     */
    void setBatteryInitialCharge(double newInitialCharge);

    /**
     * @brief Gets the current charge of the battery.
     * @return The current charge of the battery in kWh.
     */
    double getBatteryCurrentCharge() const;

    /**
     * @brief Consumes charge from the battery.
     * @param timeStep The time step in seconds.
     * @param consumedCharge The amount of charge to be consumed.
     * @return A pair containing a boolean indicating whether the consumption was successful
     *         and the actual amount of charge consumed.
     */
    std::pair<bool, double> consumeBattery(double timeStep, double consumedCharge);

    /**
     * @brief Recharges the battery for hybrid vehicles.
     * @param timeStep The time step in seconds.
     * @param recharge The amount of charge to be recharged.
     * @return The actual amount of charge recharged.
     */
    double rechargeBatteryForHybrids(double timeStep, double recharge);

    /**
     * @brief Recharges the battery by regenerated energy.
     * @param timeStep The time step in seconds.
     * @param recharge The amount of charge to be recharged.
     * @return The actual amount of charge recharged.
     */
    double rechargeBatteryByRegeneratedEnergy(double timeStep, double recharge);

    /**
     * @brief Gets the state of charge of the battery.
     * @return The state of charge of the battery as a percentage.
     */
    double getBatteryStateOfCharge() const;

    /**
     * @brief Gets the depth of discharge of the battery.
     * @return The depth of discharge of the battery as a percentage.
     */
    double getBatteryDOD() const;

    /**
     * @brief Sets the depth of discharge of the battery.
     * @param newBatteryDOD The new depth of discharge of the battery as a percentage.
     */
    void setBatteryDOD(double newBatteryDOD);

    /**
     * @brief Gets the C-Rate of discharge for the battery.
     * @return The C-Rate of discharge for the battery.
     */
    double getBatteryCRate() const;

    /**
     * @brief Sets the C-Rate of discharge for the battery.
     * @param newBatteryCRate The new C-Rate of discharge for the battery.
     */
    void setBatteryCRate(double newBatteryCRate);

    /**
     * @brief Checks if the battery can be drained by the required charge.
     * @param requiredCharge The required charge to be drained.
     * @return True if the battery can be drained, false otherwise.
     */
    bool isBatteryDrainable(double requiredCharge);

    /**
     * @brief Checks if the battery can be recharged.
     * @return True if the battery can be recharged, false otherwise.
     */
    bool isBatteryRechargable();

    /**
     * @brief Gets the maximum discharge of the battery.
     * @param timeStep The time step in seconds.
     * @return The maximum discharge of the battery in kWh.
     */
    double getBatteryMaxDischarge(double timeStep);

    /**
     * @brief Gets the maximum recharge of the battery.
     * @param timeStep The time step in seconds.
     * @return The maximum recharge of the battery in kWh.
     */
    double getBatteryMaxRecharge(double timeStep);

    /**
     * @brief Checks if the battery requires a recharge.
     * @return True if a recharge is required, false otherwise.
     */
    bool isRechargeRequired() const;

    /**
     * @brief Gets the upper bound of the battery's recharge state of charge.
     * @return The upper bound of the recharge state of charge.
     */
    double getBatteryRechargeSOCUpperBound() const;

    /**
     * @brief Sets the upper bound of the battery's recharge state of charge.
     * @param newBatteryMaxSOC The new upper bound of the recharge state of charge.
     */
    void setBatteryRechargeSOCUpperBound(double newBatteryMaxSOC);

    /**
     * @brief Gets the lower bound of the battery's recharge state of charge.
     * @return The lower bound of the recharge state of charge.
     */
    double getBatteryRechargeSOCLowerBound() const;

    /**
     * @brief Sets the lower bound of the battery's recharge state of charge.
     * @param newBatteryRechargeSOCLowerBound The new lower bound of the recharge state of charge.
     */
    void setBatteryRechargeSOCLowerBound(double newBatteryRechargeSOCLowerBound);

    /**
     * @brief Gets the cumulative energy consumption for this battery only.
     * @return The cumulative energy consumption for this battery.
     */
    double getBatteryCumEnergyConsumption();

    /**
     * @brief Gets the cumulative energy regenerated for this battery only.
     * @return The cumulative energy regenerated for this battery.
     */
    double getBatteryCumEnergyRegenerated();

    /**
     * @brief Gets the cumulative net energy consumption for this battery.
     * @return The cumulative net energy consumption for this battery.
     */
    double getBatteryCumNetEnergyConsumption();

    /**
     * @brief Checks if the battery has enough charge.
     * @return True if the battery has enough charge, false otherwise.
     */
    bool batteryHasCharge();

    /**
     * @brief Checks if the battery exceeds certain thresholds.
     * @return True if the battery exceeds the thresholds, false otherwise.
     */
    bool IsBatteryExceedingThresholds();
};

#endif // BATTERY_H
