#include "TrainTypes.h"
#include "EnergyConsumption.h"
#include <cmath>


namespace EC {

    double getDriveLineEff(double &trainSpeed, int notchNumberIndex, TrainTypes::PowerType powerType) {
        double wheelToDCBussEff = 0.0;
        if (trainSpeed <= 9.16) {
            wheelToDCBussEff = 0.2 + 0.018182*trainSpeed;
        }
        else if (trainSpeed > 9.16 && trainSpeed <= 12.22) {
            wheelToDCBussEff = 0.8 + 0.009091* (trainSpeed - 9.16);
        }
        else {
            wheelToDCBussEff = 0.9;
        }

        double DCBusToTank = 0.0;
        double notchN = (double) notchNumberIndex;
        switch (powerType) {
        case TrainTypes::PowerType::diesel:
            DCBusToTank = 0.2939 * std::pow(notchN,(double)0.2075);
            break;
        case TrainTypes::PowerType::electric:
            DCBusToTank = 0.01 * notchN + 0.93;
            break;
        case TrainTypes::PowerType::hydrogen:
            DCBusToTank = 0.01 * notchN + 0.45;
            break;
        case TrainTypes::PowerType::dieselElectric:
            DCBusToTank = 0.2939 * std::pow(notchN,(double)0.2075);
            break;
        case TrainTypes::PowerType::dieselHybrid:
            DCBusToTank = 0.2939 * std::pow(notchN,(double)0.2075);
            break;
        case TrainTypes::PowerType::hydrogenHybrid:
            DCBusToTank = 0.01 * notchN + 0.45;
            break;
        case TrainTypes::PowerType::biodieselHybrid:
            DCBusToTank = 0.2939 * std::pow(notchN, (double)0.2075);
            break;
        default:
            break;
        }
        return wheelToDCBussEff * DCBusToTank;
    }

    double getGeneratorEff(TrainTypes::PowerType powerType) {
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
}
