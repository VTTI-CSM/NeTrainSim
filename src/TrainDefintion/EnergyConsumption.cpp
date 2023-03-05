#include "TrainTypes.h"
#include "EnergyConsumption.h"
#include <cmath>


namespace EC {

    double EC::getDriveLineEff(double trainSpeed, int notchNumberIndex, TrainTypes::PowerType powerType) {
        double u_kmh = trainSpeed * 3.6;
        double wheelToDCBusEff = (u_kmh < 33.0) ? 0.2 + 0.018182 * u_kmh : (u_kmh < 44.0) ? 0.8 + 0.009091 * (u_kmh - 33.0) : 0.9;
        double DCBusToTank = 0.0;

        if (powerType == TrainTypes::PowerType::diesel){
            DCBusToTank = 0.2939 * std::pow(notchNumberIndex,0.2075);
        }
        else if (powerType == TrainTypes::PowerType::dieselElectric){
            DCBusToTank = 0.2939 * std::pow(notchNumberIndex,0.2075);
        }
        else if (powerType == TrainTypes::PowerType::electric){
            DCBusToTank = 0.01 * notchNumberIndex + 0.93;
        }
        else if (powerType == TrainTypes::PowerType::hydrogen){
            DCBusToTank = 0.01 * notchNumberIndex + 0.45;
        }
        else if (powerType == TrainTypes::PowerType::hydrogenHybrid){
            DCBusToTank = 0.01 * notchNumberIndex + 0.45;
        }
        return wheelToDCBusEff * DCBusToTank;
    }

}
