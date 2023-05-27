/**
 * @file	~\NeTrainSim\src\trainDefintion\battery.h.
 *
 * Declares the battery class
 */
#ifndef BATTERY_H
#define BATTERY_H
#include <iostream>

/**
 * A battery.
 *
 * @author	Ahmed Aredah
 * @date	3/20/2023
 */
class Battery {
    /***********************************************
    *              variables declaration           *
    ************************************************/
private:
    /** Battery max capacity */
    double batteryMaxCapacity;
    /** Battery initial capacity */
    double batteryInitialCharge;
    /** Battery current charge */
    double batteryCurrentCharge;
    /** Battery State of Charge */
    double batteryStateOfCharge;
    /** The C-Rate value of the battery while discharging.
      *  Cx where x is the c-rate value*/
    double batteryDischargeCRate;
    /** The C-Rate value of the battery while recharging. */
    double batteryRechargeCRate;
    /** The minimum battery depth of discharge = 1 - (SOC/100) */
    double batteryDOD;

    /**
     * The depth of recharge of which when the battery is recharging and it reaches, it stops
     * recharging.
     */
    double batteryRechargeSOCUpperBound;
    /** the depth of discharge of which when the battery reaches, it required a recharge. */
    double batteryRechargeSOCLowerBound;

    /** a boolean indicating the battery needs to recharge.
     *  this happens when the battery reaches the batteryRechargeDOD,
     *  this enablerecharge will turn into true and the battery will
     *  request a recharge till it reaches the batteryMaxSOC. */
    bool enableRecharge = false;

    double batteryCumEnergyConsumed = 0.0;
    double batteryCumEnergyRegenerated = 0.0;
    double batteryCumNetEnergyConsumed = 0.0;

public:

    /**
     * set the battery main properties (works as init)
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	maxCharge			   	the max charge the battery can hold in KWh.
     * @param 	initialChargePercentage	the initial charge percentage that the battery holds once the
     * 									train is loaded to the network.
     * @param 	depthOfDischarge	   	the allowable depth of discharge, the battery can drain to.
     * @param 	batteryCRate		   	the C-Rate of discharge, the recharge C-Rate is taken as 1/2
     * 									x C-Rate.
     * @param 	maxRechargeSOC		   	(Optional) the maximum recharge State of Charge the battery
     * 									reaches when recharging.
     * @param 	minRechargeSOC		   	(Optional) the minimum allowed State of charge when the
     * 									battery is being drained.
     */
    void setBattery(double maxCharge, double initialChargePercentage,
                    double depthOfDischarge, double batteryCRate,
                    double maxRechargeSOC = 0.9, double minRechargeSOC = 0.5);

    /**
     * get the battery max charge
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @returns	the battery max charge in kwh.
     */
    double getBatteryMaxCharge() const;

    /**
     * set the battery max charge in kwh
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	newMaxCharge	the new max charge the battery can hold in kwh.
     */
    void setBatteryMaxCharge(double newMaxCharge);

    /**
     * get the battery initial charge in kwh.
     * 
     * @details gets the battery initial charge of the battery initiale status.
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @returns	the battery initial charge in kwh.
     */
    double getBatteryInitialCharge() const;

    /**
     * set the battery initial charge. this should be set before the battery is put in locomotive/car
     * preferably to use the setbattery()
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	newInitialCharge	the new initial charge the battery has once loaded.
     */
    void setBatteryInitialCharge(double newInitialCharge);

    /**
     * Gets battery current charge
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @returns	The battery current charge.
     */
    double getBatteryCurrentCharge() const;

    /**
     * Consume battery
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	timeStep	  	The time step.
     * @param 	consumedCharge	The consumed charge.
     *
     * @returns	A std::pair&lt;bool,double&gt;
     */
    std::pair<bool, double> consumeBattery(double timeStep, double consumedCharge);

    /**
     * Recharge battery
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	timeStep	The time step.
     * @param 	recharge	The recharge.
     *
     * @returns	A double.
     */
    double rechargeBatteryForHybrids(double timeStep, double recharge);

    double rechargeBatteryByRegeneratedEnergy(double timeStep, double recharge);

    /**
     * Gets battery state of charge
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @returns	The battery state of charge.
     */
    double getBatteryStateOfCharge() const;

    /**
     * Gets battery dod
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @returns	The battery dod.
     */
    double getBatteryDOD() const;

    /**
     * Sets battery dod
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	newBatteryDOD	The new battery dod.
     */
    void setBatteryDOD(double newBatteryDOD);

    /**
     * Gets battery c rate
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @returns	The battery c rate.
     */
    double getBatteryCRate() const;

    /**
     * Sets battery c rate
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	newBatteryCRate	The new battery c rate.
     */
    void setBatteryCRate(double newBatteryCRate);

    /**
     * Query if 'requiredCharge' is battery drainable
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	requiredCharge	The required charge.
     *
     * @returns	True if battery drainable, false if not.
     */
    bool isBatteryDrainable(double requiredCharge);

    /**
     * Query if this  is battery rechargable
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @returns	True if battery rechargable, false if not.
     */
    bool isBatteryRechargable();


    /**
     * Gets battery maximum discharge
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	timeStep	The time step.
     *
     * @returns	The battery maximum discharge.
     */
    double getBatteryMaxDischarge(double timeStep);

    /**
     * Gets battery maximum recharge
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	timeStep	The time step.
     *
     * @returns	The battery maximum recharge.
     */
    double getBatteryMaxRecharge(double timeStep);

    /**
     * Query if this  is recharge required
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @returns	True if recharge required, false if not.
     */
    bool isRechargeRequired() const;

    /**
     * Gets battery recharge soc upper bound
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @returns	The battery recharge soc upper bound.
     */
    double getBatteryRechargeSOCUpperBound() const;

    /**
     * Sets battery recharge soc upper bound
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	newBatteryMaxSOC	The new battery maximum soc.
     */
    void setBatteryRechargeSOCUpperBound(double newBatteryMaxSOC);

    /**
     * Gets battery recharge soc lower bound
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @returns	The battery recharge soc lower bound.
     */
    double getBatteryRechargeSOCLowerBound() const;

    /**
     * Sets battery recharge soc lower bound
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	newBatteryRechargeSOCLowerBound	The new battery recharge soc lower bound.
     */
    void setBatteryRechargeSOCLowerBound(double newBatteryRechargeSOCLowerBound);

    /**
     * @brief get the battery cumulative energy consumption for this battery only
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @return the cumulative energy consumption for this battery
     */
    double getBatteryCumEnergyConsumption();

    /**
     * @brief get the battery cummulative energy regenerated for this battery only
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @return the cumulative energy regenerated for this battery only
     */
    double getBatteryCumEnergyRegenerated();

    /**
     * @brief get the battery cumNetEnergyConsumption
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @return
     */
    double getBatteryCumNetEnergyConsumption();

    bool batteryHasCharge();


    bool IsBatteryExceedingThresholds();
};

#endif // BATTERY_H
