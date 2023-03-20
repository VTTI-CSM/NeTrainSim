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
    void SetTank(double maxCapacity, double initialCapacityPercentage, double depthOfDischarge);

    double getTankMaxCapacity() const;
    void setTankMaxCapacity(double newMaxCapacity);
    double getTankInitialCapacity() const;
    void setTankInitialCapacity(double newInitialCapacityPercentage);
    double getTankCurrentCapacity() const;
    double consumeTank(double consumedAmount);
    double getTankStateOfCapacity() const;
    bool isTankDrainable(double consumedAmount);
    double getTankDOD() const;
    void setTankDOD(double newTankDOD);
};

#endif // TANK_H
