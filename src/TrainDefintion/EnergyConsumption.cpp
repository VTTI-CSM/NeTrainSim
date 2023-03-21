#include "TrainTypes.h"
#include "EnergyConsumption.h"
#include <cmath>
#include "../util/Utils.h"


namespace EC {

    double getDriveLineEff(double &trainSpeed, int notchNumberIndex, double powerAtWheelProportion, TrainTypes::PowerType powerType) {
        double wheelToDCBussEff = 0.0;
        double speed = trainSpeed * 3.6;
        if (speed <= 58.2) {
            wheelToDCBussEff = 0.2 + 0.0261*speed - 0.0003 * std::pow(speed, (double)2.0) + 0.000001 * std::pow(speed, (double)3.0);
        }
        else {
            wheelToDCBussEff = 0.9;
        }

        double DCBusToTank = 0.0;
        double notchN = (double) notchNumberIndex;
        switch (powerType) {
            // for all diesel similar generators, use the same eff
        case TrainTypes::PowerType::diesel:
        case TrainTypes::PowerType::biodiesel:
        case TrainTypes::PowerType::dieselElectric:
            DCBusToTank = -0.0021 * std::pow(notchN, (double)2) + 0.0407 * notchN + 0.2618;
            break;
            // for electric, use an average value of 0.965
        case TrainTypes::PowerType::electric:
            DCBusToTank = 0.965;
            break;
        case TrainTypes::PowerType::dieselHybrid:
            DCBusToTank = 0.2939 * std::pow(notchN,(double)0.2075);
            break;
        case TrainTypes::PowerType::hydrogenHybrid:
            DCBusToTank = -0.09 * std::pow(powerAtWheelProportion, (double)2.0) - 0.0026 * powerAtWheelProportion + 0.5621;
            break;
        case TrainTypes::PowerType::biodieselHybrid:
            DCBusToTank = -0.0045 * std::pow(notchN, (double)2.0) + 0.0649 * notchN + 0.21;
            break;
        default:
            break;
        }
        return wheelToDCBussEff * DCBusToTank;
    }

    double getGeneratorEff(TrainTypes::PowerType powerType) {
        // TODO: change it to new values
        switch (powerType) {
        case TrainTypes::PowerType::dieselHybrid:
            return 0.4524;
        case TrainTypes::PowerType::biodieselHybrid:
            return 0.4524;
        case TrainTypes::PowerType::hydrogenHybrid:
            return 0.53;
       default:
            return 1.0;
        }
    }

    double getRequiredGeneratorPowerForRecharge(double batterySOC) {
        // get the battery SOC index 
        int ind = min(static_cast<int>(ceil(batterySOC * 10.0)) , 8);
        // create a map by the soc index
        double map[] = {1,1,0.8,0.6,0.4,0.2,0.1,0.0};
        return map[ind];
    }

    double getEmissions(double fuelConsumption) {
        // convert from liters to gram
        double fuelConsumption_gPersec = fuelConsumption * (double)1000.0;
        return 3.1119 * fuelConsumption_gPersec + 1.2728;
    }
}
