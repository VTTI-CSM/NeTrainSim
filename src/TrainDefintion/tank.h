#ifndef TANK_H
#define TANK_H


class Tank {
private:
    /** Fuel cell variables if other fuel types and battery tender max capacity */
    double tankMaxCapacity;
    /** Tender initial capacity */
    double tankInitialCapacity;
    /** Tender current capacity */
    double tankCurrentCapacity;
    /** Tender fuel cell state */
    double tankStateOfCapacity;
    /** depth of discharge. */
    double tankDOD;
public:
    /**
     * @brief set the tank main properties (works as init)
     *
     * @param maxCapacity                   the max capacity the tank can hold in liters
     * @param initialCapacityPercentage     the initial capacity percentage that the tank
     *                                      hold once the train is loaded to the network.
     * @param depthOfDischarge              the allowable depth of discharge, the tank
     *                                      can drain to.
     */
    void SetTank(double maxCapacity, double initialCapacityPercentage, double depthOfDischarge);

    /**
     * Gets tank maximum capacity
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @returns	The tank maximum capacity.
     */
    double getTankMaxCapacity() const;

    /**
     * Sets tank maximum capacity
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	newMaxCapacity	The new maximum capacity.
     */
    void setTankMaxCapacity(double newMaxCapacity);

    /**
     * Gets tank initial capacity
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @returns	The tank initial capacity.
     */
    double getTankInitialCapacity() const;

    /**
     * Sets tank initial capacity
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	newInitialCapacityPercentage	The new initial capacity percentage.
     */
    void setTankInitialCapacity(double newInitialCapacityPercentage);

    /**
     * Gets tank current capacity
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @returns	The tank current capacity.
     */
    double getTankCurrentCapacity() const;

    /**
     * Consume tank fuel
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	consumedAmount	The consumed amount.
     *
     * @returns	A double.
     */
    double consumeTank(double consumedAmount);

    /**
     * Gets tank state of capacity
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @returns	The tank state of capacity.
     */
    double getTankStateOfCapacity() const;

    /**
     * Query if 'consumedAmount' is tank drainable
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	consumedAmount	The consumed amount.
     *
     * @returns	True if tank drainable, false if not.
     */
    bool isTankDrainable(double consumedAmount);

    /**
     * Gets tank dod
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @returns	The tank dod.
     */
    double getTankDOD() const;

    /**
     * Sets tank depth of discharge
     *
     * @author	Ahmed Aredah
     * @date	3/20/2023
     *
     * @param 	newTankDOD	The new tank dod.
     */
    void setTankDOD(double newTankDOD);
};

#endif // TANK_H
