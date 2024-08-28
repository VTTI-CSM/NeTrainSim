#include "traintypes.h"
#include "energyconsumption.h"
#include <cmath>


namespace EC {

    double getDriveLineEff(double &trainSpeed,
                           double powerAtWheelProportion,
                           TrainTypes::PowerType powerType,
                           TrainTypes::LocomotivePowerMethod hybridMethod) {
        double wheelToDCBusEff = getWheelToDCBusEff(trainSpeed);
        double DCBusToTank = getDCBusToTankEff(powerAtWheelProportion, powerType, hybridMethod);
        return wheelToDCBusEff * DCBusToTank;
    }

    double getDCBusToTankEff(double enginePowerProportion,
                             TrainTypes::PowerType powerType,
                             TrainTypes::LocomotivePowerMethod hybridMethod) {
        (void) hybridMethod;
        double DCBusToTank = 0.0;
        switch (powerType) {
            // for all diesel similar motor, use the same eff
        case TrainTypes::PowerType::diesel:
        case TrainTypes::PowerType::biodiesel:
        case TrainTypes::PowerType::dieselElectric:
            DCBusToTank = -0.24 * std::pow(enginePowerProportion, (double)2.0f) + 0.3859 * enginePowerProportion + 0.29;
            break;
        // for electric similar motor, use an average value of 0.965
        case TrainTypes::PowerType::electric:
            DCBusToTank = 0.965;
            break;
        case TrainTypes::PowerType::dieselHybrid:
        case TrainTypes::PowerType::biodieselHybrid:
        case TrainTypes::PowerType::hydrogenHybrid:
            DCBusToTank = getGeneratorEff(powerType, enginePowerProportion);
            if (hybridMethod == TrainTypes::LocomotivePowerMethod::series) {
                double batteryEff = getBatteryEff(powerType);
                DCBusToTank *= batteryEff * batteryEff; // in and out eff
            }
            break;
        default:
            DCBusToTank = 1.0f;
            throw runtime_error("Power type is not implemented!");
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
        case TrainTypes::PowerType::electric:
            return 0.965;  // it is a decay func but this is for simplicity for now
        default:
            return 1.0; // if powertype does not have a battery
        }
    }

    maxEfficiencyRange
    getMaxEffeciencyRange(TrainTypes::PowerType powerType) {
        switch (powerType) {
        case TrainTypes::PowerType::dieselHybrid:
        case TrainTypes::PowerType::biodieselHybrid:
            return maxEfficiencyRange(0.7,0.9, 0.8);

        case TrainTypes::PowerType::hydrogenHybrid:
            return maxEfficiencyRange(0.0,0.5, 0.0);
        default:
            return maxEfficiencyRange(0.0,1.0, 1.0);
        }
    }

    double getRequiredGeneratorPowerPortionForBatteryRecharge(double batterySOC) {
        // get the battery SOC index
        int ind = std::min(static_cast<size_t>(ceil(batterySOC * 10.0)) ,
                           std::size(requiredGeneratorPowerPortionToRechargeBattery) - 1);
        // find the appropiate required power percentage
        // by index
        return requiredGeneratorPowerPortionToRechargeBattery[ind];
    }

    double getEmissions(double fuelConsumption_inLiters, TrainTypes::PowerType powerType) {
        // convert from liters to gram
        switch (powerType)
        {
        case TrainTypes::PowerType::diesel:
        case TrainTypes::PowerType::dieselElectric:
        case TrainTypes::PowerType::dieselHybrid:
            return 2559.5 * fuelConsumption_inLiters;
        case TrainTypes::PowerType::biodiesel:
        case TrainTypes::PowerType::biodieselHybrid:
            return 2226.7 * fuelConsumption_inLiters;
        default:
            return 0.0;
        }
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


    // Converts the energy that is at the battery to energy at DC bus
    // the energy is in kWh
    double convertECFromBatteryToDC(double& LocomotiveEC_kWh_atBattery,
                                    TrainTypes::PowerType powerType)
    {
        // calculate the eff of conversion
        double eff = EC::getBatteryEff(powerType);
        return LocomotiveEC_kWh_atBattery * eff;
    }

    // Converts the energy that is the output of the engine to energy at
    // the battery. This works regardless of the battery connection.
    // the energy is in kWh
    double convertECFromEngineOutputToBattery_kWh(
        double& EC_kWh_atEngine,
        TrainTypes::PowerType powerType)
    {
        double eff = EC::getBatteryEff(powerType);
        // eff *= EC::getDCBusToTankEff(powerPortion, this->powerType,
        //                              this->hybridMethod);
        return EC_kWh_atEngine * eff;
    }

    // Converts the energy that is at the engine to energy at
    // the battery. This works regardless of the battery connection.
    // This accounts for the efficiency of the battery and the engine.
    // the energy is in kWh.
    double convertECFromEngineToBattery_kWh(
        double& EC_kWh_atEngine, double engineUsedPowerPortion,
        TrainTypes::PowerType powerType,
        TrainTypes::LocomotivePowerMethod hybridMethod)
    {
        double eff = EC::getBatteryEff(powerType);
        eff *= EC::getDCBusToTankEff(engineUsedPowerPortion, powerType,
                                     hybridMethod);
        return EC_kWh_atEngine * eff;
    }

    // Converts the energy from the battery to the energy the engine must provide.
    // This accounts for the efficiency of the battery and the engine.
    // the energy is in kWh
    double convertECFromBatteryToEngine(
        double& LocomotiveEC_kWh_atBattery,
        double& powerPortion,
        TrainTypes::PowerType powerType)
    {
        double eff = EC::getBatteryEff(powerType); // the eff of the battery
        eff *= EC::getGeneratorEff(powerType, powerPortion); // eff of the engine in case of hybrid loco
        return LocomotiveEC_kWh_atBattery / eff;
    }

    // Converts the energy from DC bus to the battery.
    // This accounts for the efficiency of the battery only.
    // the energy is in kWh.
    double convertECFromDCToBattery(double& LocomotiveEC_kWh_atDC,
                                    TrainTypes::PowerType powerType)
    {
        double eff = EC::getBatteryEff(powerType); // the eff of the battery
        return LocomotiveEC_kWh_atDC / eff;
    }

    // Converts the energy from engine to the DC bus.
    // This accounts for the efficiency of the engine and battery
    // if the locomotive has one.
    // the energy is in kWh.
    double convertECFromEnginToDCBus(double& LocomotiveEC_kWh_atEngine,
                                     double& enginePowerPortion,
                                     double& approxLocomotiveVirtualTractivePower_W,
                                     TrainTypes::PowerType powerType,
                                     TrainTypes::LocomotivePowerMethod hybridMethod)
    {
        // calculate the eff of the engine and battery
        double eff =
            EC::getDCBusToTankEff(enginePowerPortion,
                                  powerType, hybridMethod);

        if (approxLocomotiveVirtualTractivePower_W == 0) {
            return LocomotiveEC_kWh_atEngine;
        }
        else if(approxLocomotiveVirtualTractivePower_W > 0) {
            double EC_atDCBus = LocomotiveEC_kWh_atEngine * eff;
            return EC_atDCBus;
        }
        else
        {
            double EC_atDCBus = LocomotiveEC_kWh_atEngine / eff;
            return EC_atDCBus;
        }
    }

    // Converts the energy from DC bus to engine.
    // This accounts for the efficiency of the engine and battery
    // if the locomotive has one.
    // the energy is in kWh.
    double convertECFromDCBusToEngine(double& LocomotiveEC_kWh_atDCBus,
                                      double& enginePowerPortion,
                                      double& approxLocomotiveVirtualTractivePower_W,TrainTypes::PowerType powerType,
                                      TrainTypes::LocomotivePowerMethod hybridMethod)
    {
        double eff =
            EC::getDCBusToTankEff(enginePowerPortion, powerType,
                                  hybridMethod);

        if (approxLocomotiveVirtualTractivePower_W == 0) {
            return LocomotiveEC_kWh_atDCBus;
        }
        else if(approxLocomotiveVirtualTractivePower_W > 0) {
            double EC_atEngine = LocomotiveEC_kWh_atDCBus / eff;
            return EC_atEngine;
        }
        else
        {
            double EC_atEngine = LocomotiveEC_kWh_atDCBus * eff;
            return EC_atEngine;
        }
    }

    // Converts the energy from the DC bus to the wheels.
    // This accounts for the efficiency of the DC bus.
    // the energy is in kWh.
    double convertECFromDCBusToWheels(double& LocomotiveEC_kWh_atDC,
                                                  double& approxLocomotiveVirtualTractivePower_W,
                                                  double &trainSpeed)
    {
        // the eff at the wheel to DC bus
        double eff = EC::getWheelToDCBusEff(trainSpeed);

        if (approxLocomotiveVirtualTractivePower_W == 0) {
            return LocomotiveEC_kWh_atDC;
        }
        else if(approxLocomotiveVirtualTractivePower_W > 0) {

            double EC_atWheels = LocomotiveEC_kWh_atDC * eff;
            return EC_atWheels;
        }
        else
        {
            double EC_atWheels = LocomotiveEC_kWh_atDC / eff;
            return EC_atWheels;
        }
    }

    // Converts the energy from the wheels to the DC bus.
    // This accounts for the efficiency of the DC bus.
    // the energy is in kWh.
    double convertECFromWheelToDCBus(double& LocomotiveEC_kWh_atWheels,
                                     double& approxLocomotiveVirtualTractivePowerAtWheel_W,
                                     double &trainSpeed)
    {
        // the eff at the wheel to DC bus
        double eff = EC::getWheelToDCBusEff(trainSpeed);

        if (approxLocomotiveVirtualTractivePowerAtWheel_W == 0) {
            return LocomotiveEC_kWh_atWheels;
        }
        else if(approxLocomotiveVirtualTractivePowerAtWheel_W > 0) {

            double EC_atDCBus = LocomotiveEC_kWh_atWheels / eff;
            return EC_atDCBus;
        }
        else
        {
            double EC_atDCBus = LocomotiveEC_kWh_atWheels * eff;
            return EC_atDCBus;
        }
    }

    double convertECToFuel(double EC_kwh, double dieselConversionFactor)
    {
        return (EC_kwh * dieselConversionFactor);
    }

    double convertE_kWh_ToPower_W(double EC_kWh, double timeStep) {
        return (EC_kWh * 1000.0f * (3600.0f / timeStep));
    }

    double convertPower_W_ToE_kWh(double power, double timeStep) {
        return ((power / 1000.0f) * (timeStep / 3600.0f));
    }
}
