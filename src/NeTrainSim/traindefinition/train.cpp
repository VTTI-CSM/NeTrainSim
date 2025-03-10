#include <iostream>
#include "../util/vector.h"
#include "qdebug.h"
#include "train.h"
#include "../network//netnode.h"
#include "../network/netlink.h"
#include <variant>
#include "traintypes.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include "../util/logger.h"
#include "../util/error.h"
#include "util/utils.h"

#define stringify( name ) #name
using namespace std;


unsigned int Train::NumberOfTrainsInSimulator = 0;

Train::Train(int simulatorID, string id, Vector<int> trainPath,
             double trainStartTime_sec, double frictionCoeff,
             Vector<std::shared_ptr<Locomotive>> locomotives,
             Vector<std::shared_ptr<Car>> cars, bool optimize,
             double desiredDecelerationRate_mPs,
             double operatorReactionTime_s, bool stopIfNoEnergy,
             double maxAllowedJerk_mPcs, double optimization_k,
             int runOptimizationEvery,
             int optimizationLookaheadSteps) : QObject(nullptr) {

    this->d_des = desiredDecelerationRate_mPs;
    this->operatorReactionTime = operatorReactionTime_s;
    this->stopTrainIfNoEnergy = stopIfNoEnergy;
    this->maxJerk = maxAllowedJerk_mPcs;
    this->id = simulatorID;
    this->trainUserID = id;
    this->trainPath = trainPath;
    this->trainPathNodes = Vector<std::shared_ptr<NetNode>>();
    this->trainStartTime = trainStartTime_sec;
    this->coefficientOfFriction = frictionCoeff;
    this->nCars = static_cast<int>(cars.size());
    this->cars = cars;
    this->optimize = optimize;
    this->nlocs = static_cast<int>(locomotives.size());
    this->locomotives = locomotives;
    this->isOn = true;
    this->waitedTimeAtNode = 0.0;

    this->rearrangeTrain(); //rearrange order of each loco and car

    this->setTrainLength();
    this->setTrainWeight();
    this->resetTrain();

    this->WeightCentroids = this->getTrainCentroids();
    int sizeOfPath = this->trainPath.size();
    this->betweenNodesLengths = Vector<Vector<double>>(sizeOfPath, Vector<double>(sizeOfPath));
    this->LowerSpeedNodeIDs = Vector<Vector<Map<int, double>>>(sizeOfPath, 
        Vector<Map<int,double>>(sizeOfPath, Map<int,double>()));
    Train::NumberOfTrainsInSimulator++;
    this->T_s = this->operatorReactionTime + (this->totalLength / this->speedOfSound);
    
    for (auto& car : this->cars) {
        this->carsTypes[car->carType].push_back(car);
        this->ActiveCarsTypes[car->carType].push_back(car);
    }
    for (auto& loco : this->locomotives) {
        this->ActiveLocos.push_back(loco);
    }

    setOptimization(optimize, optimization_k,
                    runOptimizationEvery, optimizationLookaheadSteps );
};

Train::~Train(){
    Train::NumberOfTrainsInSimulator--;
}

void Train::moveObjectToThread(QThread *thread)
{
    // Move Simulator object itself to the thread
    this->moveToThread(thread);
}

void Train::setOptimization(bool enable,
                            double optimizationSpeedImportanceNormalizedWeight,
                            int runOptimizationEvery,
                            int optimizationLookaheadSteps)
{
    optimize = enable;
    optimizeForSpeedNormalizedWeight =
        optimizationSpeedImportanceNormalizedWeight;

    // make sure the throttle levels are empty first before
    // populating it
    if (throttleLevels.size() > 0 ) { throttleLevels.clear(); }
    for (int n = 0; n <= this->locomotives.front()->Nmax ; n++){
        this->throttleLevels.push_back(
            std::pow( ( ((double)n) / this->locomotives.front()->Nmax),
                     2.0));
    }
    this->lookAheadCounterToUpdate = runOptimizationEvery;
    this->mem_lookAheadCounterToUpdate = runOptimizationEvery;
    this->lookAheadStepCounter = optimizationLookaheadSteps;
    this->mem_lookAheadStepCounter = optimizationLookaheadSteps;
}

void Train::setTrainSimulatorID(int newID){
    this->id = newID;
}

void Train::setTrainPath(Vector<int> path) {
    this->trainPath = path;
    int sizeOfPath = this->trainPath.size();
    this->LowerSpeedNodeIDs = Vector<Vector<Map<int, double>>>(sizeOfPath,
        Vector<Map<int,double>>(sizeOfPath, Map<int,double>()));
}

// ##################################################################
// #                        start: utilities                        #
// ##################################################################

int Train::getActiveLocomotivesNumber() {
    this->ActiveLocos.erase(std::remove_if(this->ActiveLocos.begin(), this->ActiveLocos.end(),
        [](std::shared_ptr<Locomotive>& loco) {
            return !loco->isLocOn;
        }), this->ActiveLocos.end());

    if (this->ActiveLocos.size() == 0){
        this->outOfEnergy = true;
        //ErrorHandler::showNotification("All locomotives of train (" + std::to_string(this->id) + ") are out of energy");
    }
    return this->ActiveLocos.size();
}

double Train::getMinFollowingTrainGap() {
    return DefaultMinFollowingGap;
}

double Train::getBatteryEnergyConsumed() {
    double total = 0.0;
    for (auto &vehicle:this->trainVehicles) {
        total += vehicle->getBatteryCumEnergyConsumption();
    }
    return total;
}

double Train::getBatteryEnergyRegenerated() {
    double total = 0.0;
    for (auto &vehicle:this->trainVehicles) {
        total += vehicle->getBatteryCumEnergyRegenerated();
    }
    return total;
}

double Train::getBatteryNetEnergyConsumed() {
    double total = 0.0;
    for (auto &vehicle:this->trainVehicles) {
        total += vehicle->getBatteryCumNetEnergyConsumption();
    }
    return total;
}

double Train::getAverageLocomotivesBatteryStatus() {
    double sum = 0;
    for (auto& loco : this->locomotives) {
        sum += loco->getBatteryStateOfCharge();
    }
    return sum / (double)this->nlocs;
}

double Train::getAverageLocomotiveTankStatus() {
    double sum = 0;
    for (auto& loco : this->locomotives) {
        sum += loco->getTankStateOfCapacity();
    }
    return sum / (double)this->nlocs;
}

double Train::getAverageTendersTankStatus() {
    double sum = 0.0;
    int count = 0;
    for (auto& car : this->cars) {
        if (TrainTypes::carNonRechargableTechnologies.exist(car->carType)) {
            sum += car->getTankStateOfCapacity();
            count ++;
        }
    }
    if (count > 0){
        return sum / (double)count;
    }
    return 0.0;
}

double Train::getAverageTendersBatteryStatus() {
    double sum = 0.0;
    int count = 0;
    for (auto& car : this->cars) {
        if (TrainTypes::carRechargableTechnologies.exist(car->carType)) {
            sum += car->getBatteryStateOfCharge();
            count ++;
        }
    }
    if (count > 0){
        return sum / (double)count;
    }
    return 0.0;
}

double Train::getTrainTotalTorque() {
    return this->getCargoNetWeight() * (this->travelledDistance /(double)1000.0);
}

Map<std::string, double> Train::getTrainConsumedTank() {
    Map<std::string, double> consumption;
    //double consumption = 0.0;
    for (auto &loco: this->locomotives) {
        // check if the locomotive of type no fuel
        if (TrainTypes::getFuelTypeFromPowerType(loco->powerType) == TrainTypes::FuelType::noFuel) {
                continue;
        }
        // add the consumed fuel amount
        if (consumption.is_key(TrainTypes::fuelTypeToStr(TrainTypes::getFuelTypeFromPowerType(loco->powerType)))) {
            consumption[TrainTypes::fuelTypeToStr(TrainTypes::getFuelTypeFromPowerType(loco->powerType))] += loco->getTankCumConsumedFuel();
        }
        else {
            consumption[TrainTypes::fuelTypeToStr(TrainTypes::getFuelTypeFromPowerType(loco->powerType))] = loco->getTankCumConsumedFuel();
        }
    }
    return consumption;
}

unsigned int Train::getNumberOfTrainsInSimulator() {
    return Train::NumberOfTrainsInSimulator;
}

//void Train::setCars(Vector<Car> cars) {
//    this->cars = cars;
//    this->nCars = static_cast<int>(cars.size());
//    this->setTrainLength();
//    this->setTrainWeight();
//}
//void Train::setLocomotives(Vector<Locomotive> locomotives) {
//    this->locomotives = locomotives;
//    this->nlocs = static_cast<int>(locomotives.size());
//    this->setTrainLength();
//    this->setTrainWeight();
//}


Map < std::shared_ptr<TrainComponent>, double> Train::getTrainCentroids() {
    Map < std::shared_ptr<TrainComponent>, double> mapper;
    double cumLength = 0;
    for (auto& vehicle : this->trainVehicles) {
        cumLength += vehicle.get()->length;
        mapper.emplace_hint(mapper.end(), std::ref(vehicle), cumLength);
    }

    for (const auto& [vehilce, value] : mapper) {
        mapper[vehilce] -= vehilce->length / 2.0;
    }
    return mapper;
}

double Train::getCargoNetWeight() {
    double w = 0.0;
    for (auto& car : this->cars) {
        w += car->getCargoNetWeight();
    }
    return w;
}

Map<TrainTypes::PowerType, int> Train::LocTypeCount() {
    Map<TrainTypes::PowerType, int> typesCount;
    for (auto& loco : this->locomotives) {
        if (typesCount.count(loco->powerType) > 0) {
            typesCount[loco->powerType] += 1;
        }
        else {
            typesCount[loco->powerType] = 1;
        }
    }
    return typesCount;
}

Map<TrainTypes::CarType, int> Train::carTypeCount() {
    Map<TrainTypes::CarType, int> typesCount;
    for (auto& car : this->cars) {
        if (typesCount.count(car->carType) > 0) {
            typesCount[car->carType] += 1;
        }
        else {
            typesCount[car->carType] = 1;
        }
    }
    return typesCount;
}

void Train::rearrangeTrain() {
    this->trainVehicles.clear();
    int size = this->nlocs + this->nCars;
    this->trainVehicles.reserve(size);

    if (this->nlocs == 0) {
        throw std::runtime_error(std::string("Error: ") +
                                     std::to_string(static_cast<int>(Error::trainDoesNotHaveLocos)) +
                                     "\nThe Train does not have locomotives!");
    }
    
    if (this->nlocs == 1 || this->nCars == 0) {
        for (auto &loco : this->locomotives) { this->trainVehicles.push_back(loco); }
        if (this->cars.size() > 0) {
            for (auto& car : this->cars) { this->trainVehicles.push_back(car); }
        }

    }
    else if ((this->nlocs > 1 && this->nlocs < 7) || (this->nCars < 2)) {
        int locoSize = this->nlocs / 2;
        int firstLocoSize = this->nlocs - locoSize;
        for (int i = 0; i < firstLocoSize; i++) { this->trainVehicles.push_back((this->locomotives.at(i))); }
        if (this->cars.size() > 0) {
            for (int i = 0; i < this->cars.size(); i++) { this->trainVehicles.push_back((this->cars.at(i)));}
        }
        for (int i = firstLocoSize; i < this->locomotives.size(); i++) { this->trainVehicles.push_back((this->locomotives.at(i))); }
    }
    else {
        int locoSize = this->nlocs / 3;
        int firstLocoSize = this->nlocs - (2 * locoSize);
        int carSize = this->nCars / 2;
        int firstCarSize = this->nCars - carSize;

        //append first locos
        for (int i = 0; i < firstLocoSize; i++) { this->trainVehicles.push_back((this->locomotives.at(i))); }
        //append first cars
        for (int i = 0; i < firstCarSize; i++) { this->trainVehicles.push_back((this->cars.at(i))); }
        //append second locos
        for (int i = firstLocoSize; i < (firstLocoSize + locoSize); i++) { this->trainVehicles.push_back((this->locomotives.at(i))); }
        //append second cars
        for (int i = firstCarSize; i < firstCarSize + carSize; i++) { this->trainVehicles.push_back((this->cars.at(i))); }
        //append third locos
        for (int i = (firstLocoSize + locoSize); i < this->locomotives.size(); i++) { this->trainVehicles.push_back((this->locomotives.at(i))); }
    }  
}

void Train::setTrainLength() {
    double totalTrainLength = 0;
    // loop over all vehicles
    for (Vector<std::shared_ptr<TrainComponent>>::iterator it = this->trainVehicles.begin(); it != this->trainVehicles.end(); ++it ) {
        totalTrainLength += it->get()->length;

    }
    this->totalLength = totalTrainLength;
};

void Train::setTrainWeight() {
    double totalTrainWeight = 0;
    double totalEmptyWeight = 0;

    // loop over all locomotives
    for (Vector< std::shared_ptr<Locomotive>>::iterator it = this->locomotives.begin(); it != this->locomotives.end(); ++it) {
        totalTrainWeight += it->get()->currentWeight;
    }

    // loop over all cars
    for (Vector< std::shared_ptr<Car>>::iterator it = this->cars.begin(); it != this->cars.end(); ++it) {
        totalTrainWeight += it->get()->currentWeight;
        totalEmptyWeight += it->get()->emptyWeight;
    }
    this->totalMass = totalTrainWeight * 1000; //convert to kg
    this->totalEmptyMass = totalEmptyWeight * 1000;  //convert to kg
};


void Train::updateGradesCurvatures(const Vector<double> &trainGrades, const Vector<double> &trainCurvature) {
    int totalN = this->nlocs + this->nCars;
    if ((trainGrades).size() != totalN ||
        (trainCurvature).size() != totalN) {
        throw std::runtime_error(std::string("Error: ") +
                                     std::to_string(static_cast<int>(Error::trainInvalidGradesCurvature)) +
                                     "\nInvalid Grades or Curvatures values!");
    };
    for (int i = 0; i < totalN; i++) {
        this->trainVehicles[i]->trackCurvature = trainCurvature[i];
        this->trainVehicles[i]->trackGrade = trainGrades[i];
    }
}

Vector<std::shared_ptr<Car>> Train::getActiveTanksOfType(TrainTypes::CarType cartype) {
    Vector<std::shared_ptr<Car>> filteredCars;
    if (!this->carsTypes[cartype].empty()) {
        for (auto& car : this->carsTypes[cartype]) {
            if (car->getBatteryStateOfCharge() > 0.0 || car->getTankCurrentCapacity() > 0.0) {
                filteredCars.push_back(car);
            }
        }
    }
    return filteredCars;
}

void Train::updateLocNotch() {
    // loop over all locomotives
    if (!this->locomotives.empty()) {
        for (Vector< std::shared_ptr<Locomotive>>::iterator it = this->locomotives.begin(); it != this->locomotives.end(); ++it) {
            it->get()->updateLocNotch(this->currentSpeed);
        }
    }
}

// ##################################################################
// #                        end: utilities                          #
// ##################################################################

// ##################################################################
// #                   start: train dynamics                        #
// ##################################################################

double Train::getTotalResistance(double speed) {
    double totalRes = 0.0;
    // loop over all vehicles
    for (Vector< std::shared_ptr<TrainComponent>>::iterator it = this->trainVehicles.begin(); it != this->trainVehicles.end(); ++it) {
        totalRes += it->get()->getResistance(speed);
    }
    this->currentResistanceForces = totalRes;

    return totalRes;
}

double Train::getTotalTractiveForce(double speed, double acceleration, bool optimize, double optimumThrottleLevel) {
    double totalForce = 0.0;
    // loop over all locomotives
    for (Vector< std::shared_ptr<Locomotive>>::iterator it = this->locomotives.begin(); it != this->locomotives.end(); ++it) {
        it->get()->currentTractiveForce = it->get()->getTractiveForce(this->coefficientOfFriction, speed,
                                                            optimize, optimumThrottleLevel);
        totalForce += it->get()->currentTractiveForce;
    }
    this->currentTractiveForce = totalForce;
    return totalForce;
}

double Train::getAccelerationUpperBound(double speed, double acceleration, double freeFlowSpeed, bool optimize, double optimumThrottleLevel) {
    double t = 0.0, r = 0.0, a_max = 0.0;
    t = this->getTotalTractiveForce(speed, acceleration, optimize, optimumThrottleLevel);
    r = this->getTotalResistance(speed);
    a_max = (t - r) / this->totalMass;
    return a_max;
}

double Train::getSafeGap(double initialGap, double speed, double freeFlowSpeed, double T_s, bool estimate) {
    double gap_lad = 0;
    if (! estimate ) {
        gap_lad = initialGap + T_s * speed + (Utils::power(speed, 2) / (2.0 * this->d_des));
    }
    else {
        gap_lad = initialGap + T_s * freeFlowSpeed + (Utils::power(freeFlowSpeed, 2) / (2.0 * this->d_des));
    };
    return gap_lad;
}

double Train::getNextTimeStepSpeed(double gap, double minGap, double speed, double freeFlowSpeed, double aMax, double T_s, double deltaT) {
    double u_hat = min((gap - minGap) / T_s, freeFlowSpeed);
    if (u_hat < speed) {
        u_hat = max(u_hat, speed - this->coefficientOfFriction * deltaT);
    }
    else if (u_hat > speed && u_hat != freeFlowSpeed) {
        u_hat = min(u_hat, speed + aMax * deltaT);
    }
    return u_hat;
}

double Train::getTimeToCollision(double gap, double minGap, double speed, double leaderSpeed) {
    return min((gap - minGap) / max(speed - leaderSpeed, 0.0001), 100.0);
}

double Train::get_acceleration_an11(double u_hat, double speed, double TTC_s, double frictionCoef) {
    double denominator = 0;
    if (TTC_s > 0) {
        denominator = TTC_s;
    }
    else {
        denominator = 0.0001;
    }
    return max(((u_hat - speed) / (denominator)), -frictionCoef * this->g);
}

/**
 * @brief   Calculates the acceleration of a train to reach a predicted speed based on its
 * current speed, maximum acceleration, and time constant.
 *
 * @param u_hat The target speed in meters per second (m/s).
 * @param speed The current speed in meters per second (m/s).
 * @param T_s   The time to break in seconds (s).
 * @param amax  The maximum acceleration in meters per second squared (m/s^2).
 * @return The acceleration in meters per second squared (m/s^2).
 */
double Train::get_acceleration_an12(double u_hat, double speed, double T_s, double amax) {
    T_s = (T_s == 0) ? 0.0001 : T_s;
    return min((u_hat - speed) / T_s, amax);
}

double Train::get_beta1(double an11) {
    if (an11 > 0) {
        return 1.0;
    }
    else {
        return 0.0;
    }
}

double Train::get_acceleration_an13(double beta1, double an11, double an12) {
    return (1.0 - beta1) * an11 + beta1 * an12;
}

double Train:: get_acceleration_an14(double speed, double leaderSpeed, double T_s, double amax, double frictionCoef) {
    return max(min((leaderSpeed - speed) / T_s, amax), -frictionCoef * this->g);
}

double Train::get_beta2() {
    return 1.0;
}

double Train::get_acceleration_an1(double beta2, double an13, double an14) {
    return beta2 * an13 + (1.0 - beta2) * an14;
}

double Train::get_gamma(double speedDiff) {
    if (speedDiff > 0.0) {
        return 1.0;
    }
    else {
        return 0.0;
    }
}

double Train::get_acceleration_an2(double gap, double minGap, double speed, double leaderSpeed,
    double T_s, double frictionCoef){
    double term = 0.0;
    term = Utils::power(Utils::power(speed, 2) - Utils::power(leaderSpeed, 2), 2) / (4.0 * this->d_des);
    term = term / Utils::power(max((gap - minGap), 0.0001), 2);
    return min(term, frictionCoef * this->g);
}
    
double Train::accelerate(double gap, double mingap, double speed, double acceleration, double leaderSpeed, 
    double freeFlowSpeed, double deltaT, bool optimize, double throttleLevel) {

    if (throttleLevel == -1) {
        throttleLevel = this->optimumThrottleLevel;
    };

    //get the maximum acceleration that the train can go by
    double amax = this->getAccelerationUpperBound(speed, acceleration, freeFlowSpeed, optimize, throttleLevel);
    if ((gap > this->getSafeGap(mingap, speed, freeFlowSpeed, this->T_s, false)) && (amax > 0)) {
        if (speed < freeFlowSpeed) {
            return amax;
        }
        else if ( speed == freeFlowSpeed) {
            return 0.0;
        }
    }
    double u_hat = this->getNextTimeStepSpeed(gap, mingap, speed, freeFlowSpeed, amax, this->T_s, deltaT);
    double TTC_s = this->getTimeToCollision(gap, mingap, speed, leaderSpeed);
    double an11 = this->get_acceleration_an11(u_hat, speed, TTC_s, this->coefficientOfFriction);
    double an12 = this->get_acceleration_an12(u_hat, speed, this->T_s, amax);
    double beta1 = this->get_beta1(an11);
    double an13 = this->get_acceleration_an13(beta1, an11, an12);
    double an14 = this->get_acceleration_an14(speed, leaderSpeed, this->T_s, amax, this->coefficientOfFriction);
    double beta2 = this->get_beta2();
    double an1 = this->get_acceleration_an1(beta2, an13, an14);
    double du = speed - leaderSpeed;
    double gamma = this->get_gamma(du);
    double an2 = this->get_acceleration_an2(gap, mingap, speed, leaderSpeed, this->T_s, this->coefficientOfFriction );
    double a = an1 * (1.0 - gamma) - gamma * an2;
    return a;
}

double Train::accelerateConsideringJerk(double acceleration, double previousAcceleration, double jerk, double deltaT ) {
    double an = min(abs(acceleration), abs(previousAcceleration) + jerk * deltaT);
    return an * ((acceleration > 0) ? 1 : -1);
}

double Train::smoothAccelerate(double acceleration, double previousAccelerationValue, double alpha) {
    return alpha * acceleration + (1 - alpha) * previousAccelerationValue;
}

double Train::speedUpDown(double previousSpeed, double acceleration, double deltaT, double freeFlowSpeed) {
    double u_next = min(previousSpeed + acceleration * deltaT, freeFlowSpeed);
    return max(u_next, 0.0);
}

double Train::adjustAcceleration(double speed, double previousSpeed, double deltaT) {
    return ((speed - previousSpeed) / deltaT);
}

void Train::checkSuddenAccChange(double previousAcceleration, double currentAcceleration, double deltaT) {
    if (std::abs((currentAcceleration - previousAcceleration) / deltaT) >
        this->maxJerk) {
        emit suddenAccelerationOccurred(
            "sudden acceleration change!\n Report to the developer!");
    }
}


double Train::getStepAcceleration(double timeStep, double freeFlowSpeed, Vector<double>& gapToNextCriticalPoint,
                                            Vector<bool> &gapToNextCriticalPointType, Vector<double>& leaderSpeed) {

    // decrease the given quota of the future number of steps covered for virtual simulation to update
    if (this->optimize) {
        this->lookAheadCounterToUpdate -= 1;
        // get the optimum throttle level to drive by
        if (optimumThrottleLevels.size() > 0)
        {
            optimumThrottleLevel = optimumThrottleLevels.front();
        }
        if (optimumThrottleLevels.size() > 1)
        {
            // remove the used throttle level from the vector
            optimumThrottleLevels.erase(optimumThrottleLevels.begin());
        }
    }

    // set the min gap to the next train / station
    double minGap = 0.0;
    double GapFollowing = this->getMinFollowingTrainGap();
    Vector<double> allAccelerations;

    for (int i = 0; i < gapToNextCriticalPoint.size(); i++) {
        if (! (gapToNextCriticalPointType)[i]) {
            allAccelerations.push_back(this->accelerate((gapToNextCriticalPoint)[i],
                                                        minGap, this->currentSpeed, this->currentAcceleration,
                                                        (leaderSpeed)[i], freeFlowSpeed, timeStep, this->optimize));
        }
        else {
            allAccelerations.push_back(this->accelerate((gapToNextCriticalPoint)[i],
                                                        GapFollowing, this->currentSpeed, this->currentAcceleration,
                                                        (leaderSpeed)[i], freeFlowSpeed, timeStep, this->optimize));
        }
    }
    // get the minimum acceleration from all the accelerations
    double nonsmoothedAcceleration = allAccelerations.min();

    //restore forces
    if (allAccelerations.size() > 1) {
        int newIndx = allAccelerations.index(nonsmoothedAcceleration);

        if (!(gapToNextCriticalPointType)[newIndx]) {
            this->accelerate((gapToNextCriticalPoint)[newIndx],
                             minGap, this->currentSpeed, this->currentAcceleration,
                             (leaderSpeed)[newIndx], freeFlowSpeed, timeStep, this->optimize);
        }
        else {
            this->accelerate((gapToNextCriticalPoint)[newIndx],
                             GapFollowing, this->currentSpeed, this->currentAcceleration,
                             (leaderSpeed)[newIndx], freeFlowSpeed, timeStep, this->optimize);
        }
    }


    if (nonsmoothedAcceleration < 0.0 && this->currentSpeed <= 0.001 &&
        gapToNextCriticalPoint.back() > 50) {
        if (this->NoPowerCountStep < 5) {
            stringstream message;
            message << "Train " << this->id
                    << " Resistance is "
                    << "larger than train tractive force at distance "
                    << travelledDistance << "(m)!\n";
            emit slowSpeedOrStopped(message.str());
            Logger::Logger::logMessage(Logger::LogLevel::WARNING, message.str());
            NoPowerCountStep++;
        }
    }
    // smooth the acceleration and consider jerk
    double smoothedAcceleration = this->smoothAccelerate(nonsmoothedAcceleration, this->previousAcceleration, 1.0);
    double jerkedAcceleration = this->accelerateConsideringJerk(smoothedAcceleration, this->previousAcceleration,
                                                                this->maxJerk, timeStep);
    if (round(this->currentSpeed*1000)/1000 == 0.0 && jerkedAcceleration < 0) {
        jerkedAcceleration = 0.0;
    }
    return jerkedAcceleration;
}

void Train::moveTrain(double currentSimulationTime, double timeStep, double freeFlowSpeed, Vector<double>& gapToNextCriticalPoint,
                      Vector<bool> &gapToNextCriticalPointType, Vector<double>& leaderSpeed) {

    double jerkedAcceleration = this->getStepAcceleration(timeStep, freeFlowSpeed, gapToNextCriticalPoint,
                                                          gapToNextCriticalPointType, leaderSpeed);
    this->currentAcceleration = jerkedAcceleration;
    this->previousSpeed = this->currentSpeed;
    this->currentSpeed = this->speedUpDown(this->previousSpeed, this->currentAcceleration, timeStep, freeFlowSpeed);
    this->currentAcceleration = this->adjustAcceleration(this->currentSpeed, this->previousSpeed, timeStep);
    this->checkSuddenAccChange(this->previousAcceleration, this->currentAcceleration, timeStep);
    this->travelledDistance += this->currentSpeed * timeStep;
       

    // update the throttle level of the train
    this->updateLocNotch();

    if ((std::round(trainTotalPathLength * 1000.0) / 1000.0) <= (std::round(travelledDistance * 1000.0) / 1000.0)) {
        travelledDistance = trainTotalPathLength;
        reachedDestination = true;
        auto jsonState = getCurrentStateAsJson();
        emit destinationReached(jsonState);
    }
}

void Train::enableWaitingAtTerminalsForDwellTime(bool state) {
    forceStopAtTerminal = state;
}

bool Train::isCurrentlyDwelling() const {
    return dwellStartTime >= 0;
}

void Train::forceTrainToStopFor(double duration, double currentTime) {
    dwellStartTime = currentTime;
    dwellDuration = duration;
}

double Train::getRemainingDwellTime(double currentTime) const {
    if (dwellStartTime < 0) return 0;
    double elapsedTime = currentTime - dwellStartTime;
    double remainingTime = dwellDuration - elapsedTime;
    return (remainingTime > 0) ? remainingTime : 0;
}

// When the train starts moving again, reset the dwell state
void Train::resetDwellState() {
    dwellStartTime = -1;
    dwellDuration = 0;
}

void Train::immediateStop(double timestep){
    this->previousAcceleration = this->currentAcceleration;
    this->previousSpeed = this->currentSpeed;
    this->currentSpeed = 0.0;
    this->currentAcceleration = 0.0;
    this->updateLocNotch();
}

void Train::kickForwardADistance(double& distance) {
    this->previousAcceleration = 0.0;
    this->currentAcceleration = 0.0;
    this->previousSpeed = 0.0;
    this->currentSpeed = 0.0;
    this->travelledDistance += distance;
}

// ##################################################################
// #                   end: train dynamics                          #
// ##################################################################

// ##################################################################
// #                 start: train statistics                        #
// ##################################################################

void Train::calcTrainStats(Vector<double> listOfLinksFreeFlowSpeeds, double MinFreeFlow, double timeStep, 
        std::string currentRegion) {
    std::pair<Vector<double>, double> pwr = this->getTractivePower(this->currentSpeed, this->currentAcceleration,
                                                                    this->currentResistanceForces);
    this->currentUsedTractivePowerList = pwr.first;
    this->currentUsedTractivePower = pwr.second;
    this->cumUsedTractivePower += this->currentUsedTractivePower;

    this->isOn = this->consumeEnergy(timeStep, this->currentSpeed, currentUsedTractivePowerList);
    this->calculateEnergyConsumption(timeStep, currentRegion);

    this->tripTime += timeStep;
    this->delayTimeStat = this->getDelayTimeStat(MinFreeFlow, timeStep);
    this->cumDelayTimeStat += this->delayTimeStat;
    this->maxDelayTimeStat = this->getMaxDelayTimeStat(listOfLinksFreeFlowSpeeds, timeStep);
    this->cumMaxDelayTimeStat += this->maxDelayTimeStat;
    this->stoppedStat = this->getStoppingTimeStat(listOfLinksFreeFlowSpeeds);
    this->cumStoppedStat += this->stoppedStat;
    this->averageSpeed = this->calculateAverage(this->averageSpeed, this->currentSpeed, timeStep);
    this->averageAcceleration = this->calculateAverage(this->currentAcceleration, this->currentAcceleration, timeStep);
}

double Train::calculateAverage(double previousAverage, double currentTimeStepData, double timeStep) {
    double n = this->tripTime / timeStep;
    return (previousAverage * ((n - 1)/n)) + (currentTimeStepData / n);
}

double Train::getMaxDelayTimeStat(Vector<double> listOfLinksFreeFlowSpeeds, double timeStep) {
    double finalMaxDelay = 0.0;
    for (double uf : listOfLinksFreeFlowSpeeds) {
        finalMaxDelay += (1 - (this->currentSpeed / uf)) * timeStep;
    }
    return finalMaxDelay / (this->nCars + this->nlocs);
}

double Train::getDelayTimeStat(double freeflowSpeed, double timeStep) {
    double finalDelay = 0.0;
    finalDelay += (1 - (this->currentSpeed / freeflowSpeed)) * timeStep;
    return finalDelay;
}

double Train::getStoppingTimeStat(Vector<double> listOfLinksFreeFlowSpeeds) {
    double finalStopping = 0.0;
    if (this->previousSpeed > this->currentSpeed) {
        for (double uf : listOfLinksFreeFlowSpeeds) {
            finalStopping += (this->previousSpeed - this->currentSpeed) / uf;
        }
        return (finalStopping) / (this->nCars + this->nlocs);
    }
    else {
        return 0.0;
    }
}

#ifdef BUILD_SERVER_ENABLED
QVector<ContainerCore::Container *> Train::getLoadedContainers() const
{
    QVector<ContainerCore::Container*> containerList;
    for (auto &container : mLoadedContainers.getAllContainers()) {
        containerList.append(container);
    }
    return containerList;
}

void Train::addContainer(ContainerCore::Container* container) {
    if (container) {
        container->setContainerCurrentLocation("Train_" + QString::fromStdString(this->trainUserID));
        mLoadedContainers.addContainer(container->getContainerID(), container);
        emit containersLoaded();
    }
}

void Train::addContainers(QJsonObject json) {
    auto containers =
        ContainerCore::ContainerMap::loadContainersFromJson(json);

    for (auto container : containers) {
        container->setContainerCurrentLocation("Train_" + QString::fromStdString(this->trainUserID));
    }

    mLoadedContainers.addContainers(containers);
    emit containersLoaded();
}

QPair<QString, QVector<ContainerCore::Container *>>
Train::getContainersLeavingAtPort(const QVector<QString>& portNames)
{
    // Early return if no port names provided
    if (portNames.isEmpty()) {
        return {"", QVector<ContainerCore::Container*>()};
    }

    // Check each port until we find containers
    for (const QString& portName : portNames) {
        auto containers =
            mLoadedContainers.dequeueContainersByNextDestination(portName);
        if (!containers.isEmpty()) {
            return {portName, containers};
        }
    }

    return {"", QVector<ContainerCore::Container*>()};
}

QPair<QString, qsizetype>
Train::countContainersLeavingAtPort(const QVector<QString>& portNames)
{
    if (portNames.isEmpty()) {
        return {"", 0};
    }

    qsizetype count = 0;
    // Check each port until we find containers
    for (const QString& portName : portNames) {
        count =
            mLoadedContainers.countContainersByNextDestination(portName);
        if (count != 0) {
            return {portName, count};
        }
    }
    return {"", count};
}


void Train::
    requestUnloadContainersAtTerminal(const QVector<QString> &portNames)
{
    if (isCurrentlyDwelling() || reachedDestination) {

        auto containers =
            getContainersLeavingAtPort(portNames);

        QJsonArray containersJson;
        for (const auto& container : containers.second) {
            containersJson.append(container->toJson());
        }

        emit containersUnloaded(QString::fromStdString(this->trainUserID),
                                containers.first,
                                containersJson);
    }
}
#endif

// ##################################################################
// #                   end: train statistics                        #
// ##################################################################

// ##################################################################
// #                 start: train energy consumption                #
// ##################################################################

int Train::getRechargableCarsNumber() {
    int count = 0;
    for (Vector< std::shared_ptr<Car>>::iterator it = this->cars.begin(); it != this->cars.end(); ++it) {
        if (TrainTypes::carRechargableTechnologies.exist(it->get()->carType)) {
            count++;
        }
    }
    return count;
}

int Train::getRechargableLocsNumber() {
    int count = 0;
    for (Vector< std::shared_ptr<Locomotive>>::iterator it = this->locomotives.begin(); it != this->locomotives.end(); ++it) {
        if (TrainTypes::locomotiveRechargableTechnologies.exist(it->get()->powerType)) {
            count++;
        }
    }
    return count;
}

tuple<double, double, double> Train::AStarOptimization(double prevSpeed, double currentSpeed, double currentAcceleration,double prevThrottle,
                                                    Vector<double> vector_grade, Vector<double> vector_curvature,
                                                    double freeSpeed_ms, double timeStep, Vector<double> u_leader,
                                                    Vector<double> gapToNextCriticalPoint) {
    this->updateGradesCurvatures(vector_grade, vector_curvature);
    double resistance = this->getTotalResistance(currentSpeed);
    Vector<double> speedVec = Vector<double>();
    Vector<double> throttleVec = Vector<double>();
    Vector<double> energyVec = Vector<double>();
    Vector<double> accelerationVec = Vector<double>();


    // loop over all possible throttleLevels
    for (auto throttleLevel: this->throttleLevels){
        // if the throttle level (and resultant force) is less than resistance, then the train is not moving forward
        // then discard this throttle level
        if (resistance < this->getTotalTractiveForce(currentSpeed, currentAcceleration, true, throttleLevel) ||
                throttleLevel == this->throttleLevels.back()){

            double stepAcceleration = 0.0;

            // if there is no gap fed to the function, consider a finite gap.
            // this is only in case the train could not calculate the next step gap.
            if (gapToNextCriticalPoint.size() > 0)
            {
                Vector<double> allAcc = Vector<double>();
                // loop through all the gaps to next train/station
                for (int i = 0; i < gapToNextCriticalPoint.size(); i ++){
                    // get all accelerations and append them to the acceleration vector
                    allAcc.push_back(this->accelerate(gapToNextCriticalPoint[i], 0.0, currentSpeed, currentAcceleration,
                                                      u_leader[i],freeSpeed_ms, timeStep, true, throttleLevel));
                }
                // get the min acceleration
                stepAcceleration = allAcc.min();
            }
            else{
                stepAcceleration = this->accelerate(std::numeric_limits<double>::infinity(), 0.0, currentSpeed, currentAcceleration,
                                         0.0, freeSpeed_ms, timeStep, true, throttleLevel);
            }
            // get speed after acceleration, jerk is not considered here, since it will be automatically considered
            // in the true calculations of the acceleration
            double stepSpeed = this->speedUpDown(prevSpeed, stepAcceleration, timeStep, freeSpeed_ms);
            // get the energy for the train if that particular throttle level is used till the end of the look ahead
            double energy = this->heuristicFunction(gapToNextCriticalPoint.back(), stepAcceleration, stepSpeed,
                                                    timeStep, resistance, currentSpeed, prevSpeed);

            // append the step values to their corresponding vectors
            accelerationVec.push_back(stepAcceleration);
            speedVec.push_back(stepSpeed);
            throttleVec.push_back(throttleLevel);
            energyVec.push_back(energy);
        }
    }

    // if there is no throttle consider (wont happen), return the previous recommendation
    if (energyVec.size() == 0){
        return std::make_tuple(currentSpeed, currentAcceleration, prevThrottle);
    }

    // Normalize energy and speed
    double max_energy = energyVec.max();
    double min_energy = energyVec.min();

    // if both the min and max energy are equal, return the previous recommendation
    if (min_energy == max_energy) {
        return std::make_tuple(currentSpeed, currentAcceleration, prevThrottle);
    }

    double max_speed = speedVec.max();
    double min_speed = speedVec.min();

    // define the normalize function
    auto normalize = [](double value, double min_value, double max_value) -> double {
        //return max_value != min_value ? (value - min_value) / (max_value - min_value) : 0.5;
        if (max_value != min_value) {
            double normalized = (value - min_value) / (max_value - min_value);
            return 0.1 + 0.9 * normalized;  // Scale to range [0.1, 1]
        }
        return 0.5;
    };

    // define the normalize energy and speed lists
    Vector<double> energyLN;
    std::vector<double> speedLN;

    // normalize the energy and speed
    std::transform(energyVec.begin(), energyVec.end(), std::back_inserter(energyLN),
                   [&normalize, &max_energy, &min_energy](double energy) {
                       return normalize(energy, min_energy, max_energy);
                   });

    std::transform(speedVec.begin(), speedVec.end(), std::back_inserter(speedLN),
                   [&normalize, &max_speed, &min_speed](double speed) {
                       return normalize(speed, min_speed, max_speed);
                   });

    //get the weighting
    double weight_energy = (double)1.0 - optimizeForSpeedNormalizedWeight;
    double weight_speed = optimizeForSpeedNormalizedWeight;

    // calculate the weighted normalize energy consumption (goal programming)
    Vector<double> weighted_sums;
    for (size_t i = 0; i < energyLN.size(); ++i) {
        double weighted_sum = weight_energy * energyLN[i] - weight_speed * std::log(speedLN[i] + 1e-6);
        weighted_sums.push_back(weighted_sum);
    }


    // get the minimum weighted value index
    int idx_optimum = weighted_sums.index(weighted_sums.min());

    // return the corresponding low combined energy consumption data
    return std::make_tuple(speedVec[idx_optimum], accelerationVec[idx_optimum], throttleVec[idx_optimum]);

//    // get the minimum heuristic energy and the corresponding throttle level
//    int minI = energyVec.argmin();
//    // return the values corresponding to the min energy for the next step analysis
//    return std::make_tuple(speedVec[minI], accelerationVec[minI], throttleVec[minI]);
}


double Train::heuristicFunction(double distanceToEnd, double stepAcceleration, double stepSpeed,
                                double timeStep, double resistance, double currentSpeed, double prevSpeed) {
    if (distanceToEnd == 0) {
        distanceToEnd = this->lookAheadStepCounter * timeStep * currentSpeed;
    }
    // predict time needed to travel the segment
    double timeInterval = distanceToEnd / max(stepSpeed, 0.0001);
    // get the tractive power to travel a step forward
    pair<Vector<double>, double> out = this->getTractivePower(stepSpeed, stepAcceleration, resistance);
    // get the energy consumption given the timeInterval
    return this->getTotalEnergyConsumption(timeInterval, stepSpeed, stepAcceleration, out.first);
}

double Train::pickOptimalThrottleLevelAStar(Vector<double> throttleLevels, int lookAheadCounterToUpdate) {
    optimumThrottleLevels = throttleLevels;
    return 0.0;
//    int end = ( lookAheadCounterToUpdate <= throttleLevels.size()) ? lookAheadCounterToUpdate : throttleLevels.size();
//    if (end == 0){
//        return this->optimumThrottleLevel;
//    }
//    this->optimumThrottleLevel = *std::max_element(throttleLevels.begin(), throttleLevels.begin() + end);
//    return this->optimumThrottleLevel;
}

pair<Vector<double>, double> Train::getTractivePower(double speed, double acceleration, double resistanceForces) {
    Vector<double> currentVirtualTractivePowerList = {};
    double currentVirtualTractivePower = 0.0;

    // if no speed or acceleration is given, return zeros
    if (speed == 0.0 && acceleration == 0.0) {
        return { currentVirtualTractivePowerList, currentVirtualTractivePower };
    }
    else {
        // get the number of working locomotives
        int n = this->getActiveLocomotivesNumber();
        currentVirtualTractivePowerList.reserve(n);
        // divide the weight and resistance equally to all active locomotives
        double oneLocoWeight = this->totalMass / (double)n;
        double oneLocoResistance = resistanceForces / (double)n;

        // loop over all locomotives
        double virtualPower = 0.0;
        if (!this->ActiveLocos.empty()) {
            for (const auto& loco : this->ActiveLocos) {
                virtualPower = loco->getSharedVirtualTractivePower(speed, acceleration,
                                                                    oneLocoWeight, oneLocoResistance);
                currentVirtualTractivePowerList.push_back(virtualPower);
            }
        }
        currentVirtualTractivePower = currentVirtualTractivePowerList.sum();
        return { currentVirtualTractivePowerList, currentVirtualTractivePower };
    }
}

void Train::resetTrainEnergyConsumption() {
    for (std::shared_ptr<TrainComponent>& vehicle : this->trainVehicles) {
        vehicle->resetTimeStepConsumptions();
    }   
}

double Train::getTotalEnergyConsumption(double& timeStep, double& trainSpeed, double& acceleration, Vector<double>& usedTractivePower) {
    if (usedTractivePower.empty()){ return 0.0; }
    double energy = 0.0;
//    double averageSpeed = (this->currentSpeed + this->previousSpeed) / (double)2.0;
    for (int i =0; i < this->ActiveLocos.size(); i++){
        energy += this->ActiveLocos.at(i)->getEnergyConsumption(usedTractivePower.at(i),
                                             acceleration, trainSpeed, timeStep);
    }
    return energy;
}

void Train::reducePower(double &reductionFactor) {
    for (auto &loco: this->ActiveLocos) {
        loco->reducePower(reductionFactor); //TODO: make this only for the affected locomotives
    }
}

void Train::resetPowerRestriction() {
    for (auto &loco: this->ActiveLocos) {
        loco->resetPowerRestriction();
    }
}

void Train::setTrainsCurrentLinks(Vector<std::shared_ptr<NetLink>> newLinks) {
    // clear the vector
    this->currentLinks = Vector<std::shared_ptr<NetLink>>();
    for (auto &lnk: newLinks) {
        if (! this->currentLinks.exist(lnk)) {
            this->currentLinks.push_back(lnk);
        }
    }
    int j = 0;
    for (int i = 0; i < this->trainVehicles.size(); i++) {
        j = i;
        if (j >= newLinks.size()) {
            j = newLinks.size() - 1;
        }
        this->trainVehicles[i]->hostLink = newLinks[j];
    }
    this->currentFirstLink = newLinks.at(0);
}

bool Train::consumeEnergy(double& timeStep, double trainSpeed, Vector<double>& usedTractivePower) {
    this->resetTrainEnergyConsumption();
    // if the train is off, return
    if (!this->isOn) { return false; }
    int offLocos = 0;   // count the number of locomotives that are off
    double EC_kwh = 0;  // locomotive Energy consumption
    
    // if the train is not moving and it does not consume power, skip.
    if (usedTractivePower.empty()) { return true; }

    // loop over all locomotives
    for(int i =0; i < this->ActiveLocos.size(); i++){
    //for (auto& loco : this->ActiveLocos) {
        // if the locomotive is on, compute the energy consumption
        if (this->ActiveLocos.at(i)->isLocOn) {
            // calculate the amount of energy consumption 
            double averageSpeed = (this->currentSpeed + this->previousSpeed) / (double)2.0;
            double UsedTractiveP = usedTractivePower.at(i);
            EC_kwh = this->ActiveLocos.at(i)->getEnergyConsumption(UsedTractiveP,
                                                        this->currentAcceleration, averageSpeed, timeStep);

            // consume/recharge fuel from/to the locomotive if it still has fuel or can be rechargable
            auto out = this->ActiveLocos.at(i)->consumeFuel(timeStep, trainSpeed, EC_kwh, UsedTractiveP);
            //bool fuelConsumed = out.first;
            double restEC = out.second;

            // if it is energy consumption and fuel was not consumed from the locomotive, consume it by the tenders
            if (restEC > 0.0) {
                //consume it equally from the tenders with similar fuel type
                std::pair<bool, double> fuelConsumedFromTender = this->consumeTendersEnergy(timeStep,
                                                                         trainSpeed,
                                                                         restEC,
                                                                         this->ActiveLocos.at(i)->powerType);
                if (!fuelConsumedFromTender.first && restEC == EC_kwh) {
                    this->ActiveLocos.at(i)->isLocOn = false;
                }
            }
            // if it is energy regenerated and fuel was not recharged to the locomotive, recharge the cars' batteries
            else if (restEC < 0.0) {
                // recharge cars batteries
                this->rechargeCarsBatteries(timeStep, EC_kwh, this->ActiveLocos.at(i));
            }
        }
        // increment the offLocos if the locomotive is off
        else {
            offLocos++;
        }
    }
    return (offLocos != this->locomotives.size());
}

std::pair<bool, double> Train::consumeTendersEnergy(double timeStep, double trainSpeed,
                                 double EC_kwh, TrainTypes::PowerType powerType,
                                 double dieselConversionFactor,
                                 double hydrogenConversionFactor,
                                 double dieselDensity) {

    // bool consumed = false;
    double notConsumed = 0.0;
    int count = this->ActiveCarsTypes[TrainTypes::powerToCarMap.at(powerType)].size();
    double ECD = 0.0;
    if (count > 0) { ECD = EC_kwh / count; }
    else { return std::make_pair(false, EC_kwh); }


    for (auto& car : this->ActiveCarsTypes[TrainTypes::powerToCarMap.at(powerType)]) {
        // if the tender/battery still has energy to draw from, consume it
        if (car->getBatteryCurrentCharge() > 0 || car->getTankCurrentCapacity() > 0) {
            std::pair<bool, double> out = car->consumeFuel(timeStep, trainSpeed,
                                                                 ECD, 0.0 ,dieselConversionFactor,
                                                                 hydrogenConversionFactor, dieselDensity);
            notConsumed += out.second;
            // consumed = true;
        }
        // remove the car from the active list
        else {
            this->ActiveCarsTypes[TrainTypes::powerToCarMap.at(powerType)].removeValue(car);
            notConsumed += ECD;
        }
    }
    return std::make_pair( !(EC_kwh == notConsumed), notConsumed);
    // return consumed;
}

bool Train::rechargeCarsBatteries(double timeStep, double EC_kwh, std::shared_ptr<Locomotive> &loco) {
    bool consumed = false;
    int count = this->carsTypes[TrainTypes::CarType::batteryTender].size();
    double ECD = 0.0;
    // if the count is > 0, there are cars to rechange
    if (count > 0) { ECD = EC_kwh / count; }
    else {
        return loco->rechargeCatenary(EC_kwh); // recharge the catenary if the train does not have tenders
    }
    // refill all cars by that shared portion
    for (auto& car : this->carsTypes[TrainTypes::CarType::batteryTender]) {
        double restEC = car->refillBattery(timeStep, ECD);
        if ( restEC > 0.0) {
            loco->rechargeCatenary(restEC);
            consumed = true;
        }
    }
    // if the train does not have tenders or the tenders are full, feed to the catenary
    if (! consumed) {
        return loco->rechargeCatenary(EC_kwh);
    }
    return consumed;
}


void Train::calculateEnergyConsumption(double timeStep, std::string currentRegion) {
    double NEC = 0.0;
    double NER = 0.0;
    double NCE = 0.0;
    for (auto& vehicle : this->trainVehicles) {
        NEC += vehicle->cumEnergyConsumed;
        NER += std::abs(vehicle->cumEnergyRegenerated);
        NCE += vehicle->cumCarbonDioxideEmission;
    }

    this->energyStat = NEC - NER;
    this->cumEnergyStat += this->energyStat;
    this->totalEConsumed += NEC;
    this->totalERegenerated += NER;
    this->totalCarbonDioxideEmitted += NCE;

    if (this->cumRegionalConsumedEnergyStat.count(currentRegion) > 0) {
        this->cumRegionalConsumedEnergyStat[currentRegion] = 
            this->cumRegionalConsumedEnergyStat[currentRegion] + this->energyStat;
    }
    else {
        this->cumRegionalConsumedEnergyStat[currentRegion] = this->energyStat;
    }
}

Map<TrainTypes::PowerType, double> Train::getMaxProvidedEnergyFromLocomotivesOnly(double &timeStep) {
    Map<TrainTypes::PowerType, double> trainAllEC;
    if (this->ActiveLocos.size() < 1) { return trainAllEC; }
    for (auto& loco: this->ActiveLocos) {
        double locoEC = loco->getMaxProvidedEnergy(timeStep);
        // add value to locoRestEC
        if (trainAllEC.is_key(loco->powerType)) { trainAllEC[loco->powerType] = locoEC; }
        else { trainAllEC[loco->powerType] += locoEC; }
    }
    return trainAllEC;
}


Map<TrainTypes::PowerType, double> Train::getMaxProvidedEnergyFromTendersOnly(Map<TrainTypes::PowerType, double> EC,
                                                                              double &timeStep) {
    for (auto& locoType: EC.get_keys()) {
        TrainTypes::CarType tenderType = TrainTypes::powerToCarMap.at(locoType);
        for (auto& tender: this->ActiveCarsTypes[tenderType]) {
            EC[locoType] += tender->getMaxProvidedEnergy(timeStep);
        }
    }
    return EC;
}

std::pair<double, Map<TrainTypes::PowerType, double>> Train::getMaxProvidedEnergy(double &timeStep) {
    // define a var to hold the result
    Map<TrainTypes::PowerType, double> locoMaxEC = this->getMaxProvidedEnergyFromLocomotivesOnly(timeStep);

    if (locoMaxEC.get_keys().size() == 0) {
        return std::make_pair(0.0, Map<TrainTypes::PowerType, double>());
    }

    auto out = this->getMaxProvidedEnergyFromTendersOnly(locoMaxEC, timeStep);

    // sum all values in the map
    double res = std::accumulate(out.begin(), out.end(), 0.0,
                                 [](const double prev_sum, const std::pair<TrainTypes::PowerType, double> &entry) {
                                     return prev_sum + entry.second;
                                 });

    return std::make_pair(res, out);
}

Map<TrainTypes::CarType, double> Train::canProvideEnergyFromLocomotivesOnly(double &EC, double &timeStep) {
    // define a var to hold the result
    Map<TrainTypes::CarType, double> locoRestEC;

    if (this->ActiveLocos.size() < 1) { return locoRestEC; }
    // get the required energy by each locomotive
    double dividedEC = EC / ((double)this->ActiveLocos.size());
    // check all active locomotives, if any cannot provide, return not the rest of the energy required to get later from tenders
    for (auto& loco: this->ActiveLocos) {
        double locoRest = loco->canProvideEnergy(dividedEC, timeStep);  // this is only checked for electric trains only
        TrainTypes::CarType tenderType = TrainTypes::powerToCarMap.at(loco->powerType);

        // add value to locoRestEC
        if (locoRestEC.is_key(tenderType)) { locoRestEC[tenderType] = locoRest; }
        else { locoRestEC[tenderType] += locoRest; }
    }

    return locoRestEC;
}

bool Train::canProvideEnergyFromTendersOnly(Map<TrainTypes::CarType, double> &EC, double &timeStep) {
    bool result = true;

    for (auto& tenderT: EC.get_keys()) {
        double dividedEC_tenders = EC[tenderT] / this->ActiveCarsTypes[tenderT].size();
        for (auto& tender: this->ActiveCarsTypes[tenderT]) {
            result = result && tender->canProvideEnergy(dividedEC_tenders, timeStep);
//            result *= tender->canProvideEnergy(dividedEC_tenders, timeStep);
        }
    }

    return result;
}
bool Train::canProvideEnergy(double &EC, double &timeStep) {
    // define a var to hold the result
    Map<TrainTypes::CarType, double> locoRestEC = this->canProvideEnergyFromLocomotivesOnly(EC, timeStep);

    if (locoRestEC.get_keys().size() == 0) { return false; }

    if ( locoRestEC.get_values().sum() == 0.0) { // if no tenders
        return true; //return true if all energy can be provided and false o.w
    }
    return this->canProvideEnergyFromTendersOnly(locoRestEC, timeStep);

}

QJsonObject Train::getCurrentStateAsJson() {
    QJsonObject jsonState;

    Map<std::string, double> consumedTankACC;
    const auto& consumedTank = getTrainConsumedTank();
    for (const auto& kvp : consumedTank) {
        consumedTankACC[kvp.first] += kvp.second;
    }

    // Add basic information
    jsonState["trainUserID"] = QString::fromStdString(trainUserID);
    jsonState["totalLength"] = totalLength;
    jsonState["totalMass"] = totalMass / 1000.0;  // Convert kg to tons
    jsonState["travelledDistance"] = travelledDistance;
    jsonState["cumEnergyStat"] = cumEnergyStat;
    jsonState["isLoaded"] = loaded;
    jsonState["isOn"] = isOn;
    jsonState["outOfEnergy"] = outOfEnergy;
    jsonState["reachedDestination"] = reachedDestination;

    // Add cumulative statistics
    jsonState["totalEnergyConsumed"] = totalEConsumed;
    jsonState["totalEnergyRegenerated"] = totalERegenerated;
    jsonState["totalCarbonDioxideEmitted"] = totalCarbonDioxideEmitted / 1000.00; // kg;
    jsonState["cumulativeDelayTimeStat"] = cumDelayTimeStat;
    jsonState["cumulativeMaxDelayTimeStat"] = cumMaxDelayTimeStat;
    jsonState["cumulativeStoppedStat"] = cumStoppedStat;
    jsonState["tripTime"] = tripTime;
    QJsonObject fuel;
    for (const auto& kvp : consumedTank) {
        fuel.insert(QString::fromStdString(kvp.first), kvp.second);
    }
    jsonState["totalFuelConsumed"] = fuel;

    // Add train state
    jsonState["currentSpeed"] = currentSpeed;
    jsonState["currentAcceleration"] = currentAcceleration;
    jsonState["currentTractiveForce"] = currentTractiveForce;
    jsonState["currentResistanceForces"] = currentResistanceForces;
    jsonState["currentUsedTractivePower"] = currentUsedTractivePower;

#ifdef BUILD_SERVER_ENABLED
    jsonState["containersCount"] =
        mLoadedContainers.getAllContainers().count();
#endif

    return jsonState;
}

void Train::requestCurrentStateAsJson() {
    auto out = getCurrentStateAsJson();
    emit trainStateAvailable(out);
}

// ##################################################################
// #                   end: train energy consumption                #
// ##################################################################


void Train::resetTrainLookAhead(){
    this->lookAheadCounterToUpdate = mem_lookAheadCounterToUpdate;
    this->lookAheadStepCounter = mem_lookAheadStepCounter;
}

void Train::resetTrain() {
    this->betweenNodesLengths.clear();
    for (Vector< std::shared_ptr<TrainComponent>>::iterator it = this->trainVehicles.begin(); it != this->trainVehicles.end(); ++it) {
        it->get()->trackCurvature = 0.0;
        it->get()->trackGrade = 0.0;
        it->get()->energyConsumed = 0.0;
        it->get()->cumEnergyConsumed = 0.0;
        it->get()->carbonDioxideEmission = 0.0;
        it->get()->cumCarbonDioxideEmission = 0.0;
    };

    this->loaded = false;
    this->travelledDistance = 0.0;
    this->virtualTravelledDistance = 0.0;
    
    this->tripTime = 0.0;
    this->NoPowerCountStep = 0;
    this->energyStat = 0.0;
    this->cumEnergyStat = 0.0;
    this->totalEConsumed = 0.0;
    this->totalERegenerated = 0.0;
    this->cumDelayTimeStat = 0.0;
    this->cumMaxDelayTimeStat = 0.0;
    this->cumStoppedStat = 0.0;
    this->cumRegionalConsumedEnergyStat.clear();
    this->currentAcceleration = 0.0;
    this->previousAcceleration = 0.0;
    this->currentResistanceForces = 0.0;
    this->currentSpeed = 0.0;
    this->previousSpeed = 0.0;
    this->currentTractiveForce = 0.0;
    this->cumUsedTractivePower = 0.0;
    this->currentUsedTractivePower = 0.0;
    this->delayTimeStat = 0.0;
    this->trainTotalPathLength = 0.0;
    this->currentCoordinates = { 0.0, 0.0 };
    this->lookAheadStepCounter = 1;
    this->lookAheadCounterToUpdate = 1;
    this->optimumThrottleLevel = 1;
    this->maxDelayTimeStat = 0.0;
    this->stoppedStat = 0.0;

    //this->LastTrainPointpreviousNodeID = -1;
    //this->previousNodeID = -1;

}

std::ostream& operator<<(std::ostream &ostr, Train & train) {
    ostr << "Freight Train:: ID: " << train.trainUserID << ", length (m): " << train.totalLength;
    ostr << ", total weight (t): " << train.totalMass/1000; //convert to tons
    ostr << ", # locomotives: " << train.nlocs << ", # cars: " << train.nCars;
    ostr << ", travelled distance (m):"<< train.travelledDistance << std::endl;
    return ostr;
};
