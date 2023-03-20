#ifndef BATTERY_H
#define BATTERY_H
#include <iostream>

class Battery {
private:
    /** Battery max capacity */
    double batteryMaxCapacity;
    /** Battery initial capacity */
    double batteryInitialCharge;
    /** Battery current charge */
    double batteryCurrentCharge;
    /** Battery State of Charge*/
    double batteryStateOfCharge;
    /** The C-Rate value of the battery while discharging.
     *  Cx where x is the c-rate value*/
    double batteryDischargeCRate;
    /** The C-Rate value of the battery while recharging. */
    double batteryRechargeCRate;
    /** The minimum battery depth of discharge = 1 - (SOC/100) */
    double batteryDOD;
    /** The depth of recharge of which when the battery is recharging and it reaches, it stops recharging. */
    double batteryRechargeSOCUpperBound;
    /** the depth of discharge of which when the battery reaches, it required a recharge. */
    double batteryRechargeSOCLowerBound;

    /** a boolean indicating the battery needs to recharge.
     *  this happens when the battery reaches the batteryRechargeDOD,
     *  this enablerecharge will turn into true and the battery will
     *  request a recharge till it reaches the batteryMaxSOC. */
    bool enableRecharge = false;

public:
    /**
     * @brief set the battery main properties (works as init)
     *
     * @param maxCharge                 the max charge the battery can hold in KWh
     * @param initialChargePercentage   the initial charge percentage that the battery
     *                                  holds once the train is loaded
     * @param depthOfDischarge          the allowable depth of discharge, the battery
     *                                  can drain
     * @param batteryCRate              the C-Rate of discharge, the recharge C-Rate is
     *                                  taken as 1/2 x C-Rate
     * @param maxRechargeSOC            the maximum recharge State of Charge the battery
     *                                  reaches when recharging
     * @param minRechargeSOC            the minimum allowed State of charge when the
     *                                  battery is being drained
     */
    void setBattery(double maxCharge, double initialChargePercentage,
                    double depthOfDischarge, double batteryCRate,
                    double maxRechargeSOC = 0.9, double minRechargeSOC = 0.5);
    /**
     * @brief get the battery max charge
     * @return the battery max charge in kwh
     */
    double getBatteryMaxCharge() const;

    /**
     * @brief set the battery max charge in kwh
     * @param newMaxCharge  the new max charge the battery can hold in kwh
     */
    void setBatteryMaxCharge(double newMaxCharge);

    /**
     * @brief get the battery initial charge in kwh
     *
     * @details gets the battery initial charge of the battery initiale status.
     * @return the battery initial charge in kwh
     */
    double getBatteryInitialCharge() const;

    /**
     * @brief set the battery initial charge. this should be set before the
     *        battery is put in locomotive/car
     * @details preferably to use the @macro setbattery()
     * @param newInitialCharge the new initial charge the battery has once loaded
     */
    void setBatteryInitialCharge(double newInitialCharge);
    double getBatteryCurrentCharge() const;
    std::pair<bool, double> consumeBattery(double timeStep, double consumedCharge);
    double rechargeBattery(double timeStep, double recharge);
    double getBatteryStateOfCharge() const;
    double getBatteryDOD() const;
    void setBatteryDOD(double newBatteryDOD);
    double getBatteryCRate() const;
    void setBatteryCRate(double newBatteryCRate);
    bool isBatteryDrainable(double requiredCharge);
    bool isBatteryRechargable();
    double getBatteryMaxDischarge(double timeStep);
    double getBatteryMaxRecharge(double timeStep);
    bool isRechargeRequired() const;
    double getBatteryRechargeSOCUpperBound() const;
    void setBatteryRechargeSOCUpperBound(double newBatteryMaxSOC);
    double getBatteryRechargeSOCLowerBound() const;
    void setBatteryRechargeSOCLowerBound(double newBatteryRechargeSOCLowerBound);
};

#endif // BATTERY_H
