#include "TrainTypes.h"
#include "EnergyConsumption.h"
#include <cmath>


namespace EC {

    double getDriveLineEff(double &trainSpeed, int notchNumberIndex,
                           double powerAtWheelProportion,
                           TrainTypes::PowerType powerType) {
        double wheelToDCBusEff = 0.0;     // initialize the variable
        double speed = trainSpeed * 3.6;  // convert the m/s speed to km/h
        // get the wheel to DC Bus effeciency
        // check which range the speed is in
        if (speed <= 58.2) {
            wheelToDCBusEff = 0.2 + 0.0261*speed - 0.0003 *
                    std::pow(speed, (double)2.0) +
                    0.000001 * std::pow(speed, (double)3.0);
        }
        else {
            wheelToDCBusEff = 0.9;  // constant efficiency
        }

        double DCBusToTank = 0.0;
        switch (powerType) {
            // for all diesel similar motor, use the same eff
        case TrainTypes::PowerType::diesel:
        case TrainTypes::PowerType::biodiesel:
        case TrainTypes::PowerType::dieselElectric:
            DCBusToTank = -0.24 * std::pow(powerAtWheelProportion, (double)2) + 0.3859 * powerAtWheelProportion + 0.29;
            break;
        // for electric similar motor, use an average value of 0.965
        case TrainTypes::PowerType::electric:
        case TrainTypes::PowerType::dieselHybrid:
        case TrainTypes::PowerType::biodieselHybrid:
        case TrainTypes::PowerType::hydrogenHybrid:
            DCBusToTank = 0.965;
            break;
        default:
            break;
        }
        return wheelToDCBusEff * DCBusToTank;
    }

    double getGeneratorEff(TrainTypes::PowerType powerType, double powerAtWheelProportion) {
        switch (powerType) {
        case TrainTypes::PowerType::dieselHybrid:
        case TrainTypes::PowerType::biodieselHybrid:
            return -0.24 * std::pow(powerAtWheelProportion, (double)2.0) +
                    0.3859 * powerAtWheelProportion + 0.29;
        case TrainTypes::PowerType::hydrogenHybrid:
            return -0.0937 * std::pow(powerAtWheelProportion, (double)2.0) +
                    0.002 * powerAtWheelProportion + 0.5609;
        default:
            return 1.0;  // if powertype should not generate
        }
    }

    double getRequiredGeneratorPowerForRecharge(double batterySOC) {
        // get the battery SOC index 
        int ind = min(static_cast<int>(ceil(batterySOC * 10.0)) , 8);
        // find the appropiate required power percentage
        // by index
        return requiredGeneratorPower[ind];
    }

    double getEmissions(double fuelConsumption) {
        // convert from liters to gram
        double fuelConsumption_gPersec = fuelConsumption * (double)1000.0;
        return 3.1119 * fuelConsumption_gPersec + 1.2728;
    }
}
