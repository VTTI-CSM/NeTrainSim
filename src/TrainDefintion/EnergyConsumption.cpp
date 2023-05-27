#include "TrainTypes.h"
#include "EnergyConsumption.h"
#include <cmath>


namespace EC {

    double getDriveLineEff(double &trainSpeed, int notchNumberIndex,
                           double powerAtWheelProportion,
                           TrainTypes::PowerType powerType,
                           TrainTypes::LocomotivePowerMethod hybridMethod) {

        double wheelToDCBusEff = getWheelToDCBusEff(trainSpeed);
        double DCBusToTank = getDCBusToTankEff(powerAtWheelProportion, powerType, hybridMethod);

        return wheelToDCBusEff * DCBusToTank;
    }

    double getDCBusToTankEff(double powerAtWheelProportion,
                             TrainTypes::PowerType powerType,
                             TrainTypes::LocomotivePowerMethod hybridMethod) {
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
            DCBusToTank = 0.965;
            break;
        case TrainTypes::PowerType::dieselHybrid:
        case TrainTypes::PowerType::biodieselHybrid:
        case TrainTypes::PowerType::hydrogenHybrid:
            switch (hybridMethod) {
            // energy has to pass through the battery.
            // in this case, the effeciency is only the eff of the chemical-to-electricity conversion
            case TrainTypes::LocomotivePowerMethod::series:
            case TrainTypes::LocomotivePowerMethod::notApplicable: //default
                DCBusToTank = 0.965;
                break;
            case TrainTypes::LocomotivePowerMethod::parallel:
                DCBusToTank = 1.0;
                break;
            }

        default:
            break;
        }
        return DCBusToTank;
    }

    double getWheelToDCBusEff(double &trainSpeed) {
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
        return wheelToDCBusEff;
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

    double getBatteryEff(TrainTypes::PowerType powerType) {
        switch (powerType) {
        case TrainTypes::PowerType::dieselHybrid:
        case TrainTypes::PowerType::biodieselHybrid:
        case TrainTypes::PowerType::hydrogenHybrid:
            return 0.965;
        default:
            return 1.0; // if powertype does not have a battery
        }
    }

    std::pair<double,double> getMaxEffeciencyRange(TrainTypes::PowerType powerType) {
        switch (powerType) {
        case TrainTypes::PowerType::dieselHybrid:
        case TrainTypes::PowerType::biodieselHybrid:
            return std::make_pair(0.7,0.9);

        case TrainTypes::PowerType::hydrogenHybrid:
            return std::make_pair(0.0,0.5);
        default:
            return std::make_pair(0.0,1.0);
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

    double getLocomotivePowerReductionFactor(TrainTypes::PowerType powerType) {
        //return 1.0;
        switch (powerType) {
        case TrainTypes::PowerType::dieselHybrid:
            return EC::DefaultLocomotivePowerReduction_DieselHybrid;
        case TrainTypes::PowerType::biodieselHybrid:
            return EC::DefaultLocomotivePowerReduction_BioDieselHybrid;
        case TrainTypes::PowerType::hydrogenHybrid:
            return EC::DefaultLocomotivePowerReduction_HydrogenHybrid;
        default:
            return (double)1.0;
        }
    }

    double getFuelFromEC(TrainTypes::PowerType powerType, double &EC_KWh){
        return EC_KWh * getFuelConversionFactor(powerType);
    }

    double getFuelFromEC(TrainTypes::CarType carType, double &EC_KWh) {
        return EC_KWh * getFuelConversionFactor(carType);
    }

    double getFuelConversionFactor(TrainTypes::PowerType powerType) {
        return fuelConversionFactor_powerTypes[powerType];
    }

    double getFuelConversionFactor(TrainTypes::CarType carType) {
        return fuelConversionFactor_carTypes[carType];
    }

}
