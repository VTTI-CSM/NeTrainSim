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
        case TrainTypes::PowerType::diesel:
        case TrainTypes::PowerType::biodiesel:
        case TrainTypes::PowerType::dieselElectric:
            DCBusToTank = -0.0021 * std::pow(notchN, (double)2) + 0.0407 * notchN + 0.2618;
            break;
        case TrainTypes::PowerType::electric:
            DCBusToTank = 0.95;
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
        int ind = min(static_cast<int>(ceil(batterySOC * 10.0)) , 8);
        double map[] = {1,1,0.8,0.6,0.4,0.2,0.1,0.0};
        return map[ind];
    }

    tuple<double,double,double,double> getEmissions(double fuelConsumption) {
        double fuelConsumption_gPersec = fuelConsumption * (double)1000.0;
        double hc =  0.00003 * Utils::power(fuelConsumption_gPersec,2) -
                0.0002 * fuelConsumption_gPersec + 0.0422;
        double co = 0.000006 * Utils::power(fuelConsumption_gPersec,3) -
                0.0004 * Utils::power(fuelConsumption_gPersec,2) +
                0.008 * fuelConsumption_gPersec + 0.0468;
        double NOX = 0.000005 * Utils::power(fuelConsumption_gPersec,3) +
                0.0008 * Utils::power(fuelConsumption_gPersec,2) +
                0.0293 * fuelConsumption_gPersec + 0.0933;
        double co2 = 3.1119 * fuelConsumption_gPersec + 1.2728;
        return std::make_tuple(hc, co, NOX, co2);
    }
}
