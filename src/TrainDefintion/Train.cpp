#include <iostream>
#include "../util/Vector.h"
#include "train.h"
#include "../network//NetNode.h"
#include "../network/NetLink.h"
#include <variant>
#include "TrainTypes.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include "../util/Logger.h"
#include "../util/Error.h"
#define stringify( name ) #name
using namespace std;


unsigned int Train::NumberOfTrainsInSimulator = 0;

Train::Train(string id, Vector<int> trainPath, double trainStartTime_sec, double frictionCoeff,
    Vector<std::shared_ptr<Locomotive>> locomotives, Vector<std::shared_ptr<Car>> cars, bool optimize, 
    double desiredDecelerationRate_mPs, double operatorReactionTime_s, bool stopIfNoEnergy,
    double maxAllowedJerk_mPcs) {

    this->d_des = desiredDecelerationRate_mPs;
    this->operatorReactionTime = operatorReactionTime_s;
    this->stopTrainIfNoEnergy = stopIfNoEnergy;
    this->maxJerk = maxAllowedJerk_mPcs;
    this->id = Train::NumberOfTrainsInSimulator;
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

    if (this->optimize){
        double nMax = 8;
        for (int n = 0; n <= nMax ; n++){
            this->throttleLevels.push_back(std::pow( ( ((double)n) / nMax), 2.0));
        }
        this->lookAheadCounterToUpdate = DefaultLookAheadCounterToUpdate;
        this->lookAheadStepCounter = DefaultLookAheadCounter;
    }

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
};

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
        std::cerr << "All locomotives of train (" << this->id << ") are out of energy" << std::endl;
    }
    return this->ActiveLocos.size();
}

double Train::getMinFollowingTrainGap() {
    return DefaultMinFollowingGap;
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

double Train::getTrainConsumedTank() {
    double consumption = 0.0;
    for (auto &loco: this->locomotives) {
        consumption += loco->getTankInitialCapacity() - loco->getTankCurrentCapacity();
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
    double totalForce = 0;
    // loop over all locomotives
    for (Vector< std::shared_ptr<Locomotive>>::iterator it = this->locomotives.begin(); it != this->locomotives.end(); ++it) {
        totalForce += it->get()->getTractiveForce(this->coefficientOfFriction, speed,
            optimize, optimumThrottleLevel);
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
    //cout<<"----------------" << "speed: " << speed << "leader speed: " << leaderSpeed << " gap: " << gap <<  endl;
//    if (throttleLevel == -1) {
//        throttleLevel = this->optimumThrottleLevel;
//    };

    //get the maximum acceleration that the train can go by
    double amax = this->getAccelerationUpperBound(speed, acceleration, freeFlowSpeed, optimize, throttleLevel);
    //std::cout << "amax: " << amax << endl;
    if ((gap > this->getSafeGap(mingap, speed, freeFlowSpeed, this->T_s, false)) && (amax > 0)) {
        if (speed < freeFlowSpeed) {
            return amax;
        }
        else if ( speed == freeFlowSpeed) {
            return 0.0;
        }
    }
    double u_hat = this->getNextTimeStepSpeed(gap, mingap, speed, freeFlowSpeed, amax, this->T_s, deltaT);
    //std::cout << "u_hat: " << u_hat << endl;
    double TTC_s = this->getTimeToCollision(gap, mingap, speed, leaderSpeed);
    //std::cout << "TTC_s: " <<TTC_s << endl;
    double an11 = this->get_acceleration_an11(u_hat, speed, TTC_s, this->coefficientOfFriction);
    //std::cout << "an11: " <<an11 << endl;
    double an12 = this->get_acceleration_an12(u_hat, speed, this->T_s, amax);
    //std::cout << "an12: " <<an12 << endl;
    double beta1 = this->get_beta1(an11);
    //std::cout << "beta1: " <<beta1 << endl;
    double an13 = this->get_acceleration_an13(beta1, an11, an12);
    //std::cout << "an13: " <<an13<< endl;
    double an14 = this->get_acceleration_an14(speed, leaderSpeed, this->T_s, amax, this->coefficientOfFriction);
    //std::cout << "an14: " <<an14 << endl;
    double beta2 = this->get_beta2();
    //std::cout << "beta2: " <<beta2 << endl;
    double an1 = this->get_acceleration_an1(beta2, an13, an14);
    //std::cout << "an1: " <<an1 << endl;
    double du = speed - leaderSpeed;
    //std::cout << "du: " << du << endl;
    double gamma = this->get_gamma(du);
    //std::cout << "gamma: " <<gamma << endl;
    double an2 = this->get_acceleration_an2(gap, mingap, speed, leaderSpeed, this->T_s, this->coefficientOfFriction );
    //std::cout << "an2: " <<an2 << endl;
    double a = an1 * (1.0 - gamma) - gamma * an2;
    //std::cout << "a: " <<a << endl;
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
    if ((currentAcceleration - previousAcceleration) / deltaT > this->maxJerk) {
        throw std::invalid_argument("sudden acceleration change!");
    }
}

void Train::moveTrain(double timeStep, double freeFlowSpeed, Vector<double>& gapToNextCriticalPoint,
    Vector<bool> &gapToNextCriticalPointType, Vector<double>& leaderSpeed) {

    // decrease the given quota of the future number of steps covered for virtual simulation to update
    if (this->optimize) {
        this->lookAheadCounterToUpdate -= 1;
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


    if (nonsmoothedAcceleration < 0.0 && this->currentSpeed <= 0.001 && gapToNextCriticalPoint.back() > 50) {
        if (this->NoPowerCountStep < 5) {
            stringstream message;
            message << "Train " << this->id
                << " Slad is short or Resistance is larger than train tractive force at distance "
                << travelledDistance << "!\n";
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
    this->currentAcceleration = jerkedAcceleration;
    this->previousSpeed = this->currentSpeed;
    this->currentSpeed = this->speedUpDown(this->previousSpeed, this->currentAcceleration, timeStep, freeFlowSpeed);
    this->currentAcceleration = this->adjustAcceleration(this->currentSpeed, this->previousSpeed, timeStep);
    this->checkSuddenAccChange(this->previousAcceleration, this->currentAcceleration, timeStep);
    this->travelledDistance += this->currentSpeed * timeStep;
       

    // update the throttle level of the train
    this->updateLocNotch();
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

std::tuple<double, double, double> Train::AStarOptimization(double prevSpeed, double currentSpeed, double currentAcceleration,double prevThrottle,
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
            if (gapToNextCriticalPoint.size() > 0){
                Vector<double> allAcc = Vector<double>();
                // loop through all the gaps to next train/station
                for (int i = 0; i < gapToNextCriticalPoint.size(); i ++){
                    // get all accelerations and append them to the acceleration vector
                    allAcc.push_back(this->accelerate(gapToNextCriticalPoint[i], 0, currentSpeed, currentAcceleration,
                                                      u_leader[i],freeSpeed_ms, timeStep, true, throttleLevel));
                }
                // get the min acceleration
                stepAcceleration = allAcc.min();
            }
            else{
                stepAcceleration = this->accelerate(std::numeric_limits<double>::infinity(), 0, currentSpeed, currentAcceleration,
                                         0.0, freeSpeed_ms, timeStep, true, throttleLevel);
            }
            // get speed after acceleration, jerk is not considered here, since it will be automatically considered
            // in the true calculations of the acceleration
            double stepSpeed = this->speedUpDown(prevSpeed, stepAcceleration, timeStep, freeSpeed_ms);
            // get the energy for the train if that particular throttle level is used till the end of the look ahead
            double energy = this->heuristicFunction(gapToNextCriticalPoint.back(), stepAcceleration, stepSpeed,
                                                    timeStep, resistance, currentSpeed);
            // append the step values to their corresponding vectors
            accelerationVec.push_back(stepAcceleration);
            speedVec.push_back(stepSpeed);
            throttleVec.push_back(throttleLevel);
            energyVec.push_back(energy);
        }
    }

    if (energyVec.size() == 0){
        return std::make_tuple(currentSpeed, currentAcceleration, prevThrottle);
    }
    // get the minimum heuristic energy and the corresponding throttle level
    int minI = energyVec.argmin();
    // return the values corresponding to the min energy for the next step analysis
    return std::make_tuple(speedVec[minI], accelerationVec[minI], throttleVec[minI]);
}


double Train::heuristicFunction(double distanceToEnd, double stepAcceleration, double stepSpeed,
                                double timeStep, double resistance, double currentSpeed) {
    if (distanceToEnd == 0) {
        distanceToEnd = this->lookAheadStepCounter * timeStep * currentSpeed;
    }
    double stepTime = distanceToEnd / max(stepSpeed, 0.0001);

    pair<Vector<double>, double> out = this->getTractivePower(stepSpeed, stepAcceleration, resistance);

    return this->getTotalEnergyConsumption(stepTime, out.first);
}

double Train::pickOptimalThrottleLevelAStar(Vector<double> throttleLevels, int lookAheadCounterToUpdate) {
    int end = ( lookAheadCounterToUpdate <= throttleLevels.size()) ? lookAheadCounterToUpdate : throttleLevels.size();
    if (end == 0){
        return this->optimumThrottleLevel;
    }
    this->optimumThrottleLevel = *std::max_element(throttleLevels.begin(), throttleLevels.begin() + end);
    return this->optimumThrottleLevel;
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
                virtualPower = loco->getSharedVirtualTractivePower(this->currentSpeed, this->currentAcceleration,
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

double Train::getTotalEnergyConsumption(double& timeStep, Vector<double>& usedTractivePower) {
    if (usedTractivePower.empty()){ return 0.0; }
    double energy = 0.0;
    double averageSpeed = (this->currentSpeed + this->previousSpeed) / (double)2.0;
    for (int i =0; i < this->ActiveLocos.size(); i++){
        energy += this->ActiveLocos.at(i)->getEnergyConsumption(usedTractivePower.at(i),
                                             this->currentAcceleration, averageSpeed, timeStep);
    }
    return energy;
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
            // reset the power reduction restriction
            this->ActiveLocos.at(i)->resetPowerRestriction();
            // calculate the amount of energy consumption 
            double averageSpeed = (this->currentSpeed + this->previousSpeed) / (double)2.0;
            EC_kwh = this->ActiveLocos.at(i)->getEnergyConsumption(usedTractivePower.at(i), 
                                                        this->currentAcceleration, averageSpeed, timeStep);

            // consume/recharge fuel from/to the locomotive if it still has fuel or can be rechargable
            auto out = this->ActiveLocos.at(i)->consumeFuel(timeStep, trainSpeed, EC_kwh);
            //bool fuelConsumed = out.first;
            double restEC = out.second;

            // if it is energy consumption and fuel was not consumed from the locomotive, consume it by the tenders
            if (restEC > 0.0) {
                //consume it equally from the tenders with similar fuel type
                bool fuelConsumedFromTender = this->consumeTendersEnergy(timeStep,
                                                                         trainSpeed,
                                                                         restEC,
                                                                         this->ActiveLocos.at(i)->powerType);
                // When not all the energy is consumed by the tender
                // reduce the current notch to a lower position,
                // since there is no power source can provide all required energy
                if (!fuelConsumedFromTender && restEC != EC_kwh) {
                    this->ActiveLocos.at(i)->reducePower();
                }
                if (!fuelConsumedFromTender && restEC == EC_kwh) {
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

bool Train::consumeTendersEnergy(double timeStep, double trainSpeed,
                                 double EC_kwh, TrainTypes::PowerType powerType,
                                 double dieselConversionFactor,
                                 double hydrogenConversionFactor,
                                 double dieselDensity) {

    bool consumed = false;
    int count = this->ActiveCarsTypes[TrainTypes::powerToCarMap.at(powerType)].size();
    double ECD = 0.0;
    if (count > 0) { ECD = EC_kwh / count; }
    else { return consumed; }


    for (auto& car : this->ActiveCarsTypes[TrainTypes::powerToCarMap.at(powerType)]) {
        // if the tender/battery still has energy to draw from, consume it
        if (car->getBatteryCurrentCharge() > 0 || car->getTankCurrentCapacity() > 0) {
            car->consumeFuel(timeStep, trainSpeed, ECD, dieselConversionFactor,
                             hydrogenConversionFactor, dieselDensity);
            consumed = true;
        }
        // remove the car from the active list
        else {
            this->ActiveCarsTypes[TrainTypes::powerToCarMap.at(powerType)].removeValue(car);
        }
    }
    return consumed;
}

bool Train::rechargeCarsBatteries(double timeStep, double EC_kwh, std::shared_ptr<Locomotive> &loco) {
    bool consumed = false;
    int count = this->carsTypes[TrainTypes::CarType::batteryTender].size();
    double ECD = 0.0;
    // if the count is > 0, there are cars to rechange
    if (count > 0) { ECD = EC_kwh / count; }
    else { return false; }  // no cars to recharge. this is redundant

    // refill all cars by that shared portion
    for (auto& car : this->carsTypes[TrainTypes::CarType::batteryTender]) {
        if (! car->refillBattery(timeStep, ECD)) {
            loco->rechargeCatenary(ECD);
            consumed = true;
        }
    }
    return consumed;
}


void Train::calculateEnergyConsumption(double timeStep, std::string currentRegion) {
    double NEC = 0.0;
    double NER = 0.0;
    for (auto& vehicle : this->trainVehicles) {
        NEC += vehicle->energyConsumed;
        NER += std::abs(vehicle->energyRegenerated);
    }

    this->energyStat = NEC - NER;
    this->cumEnergyStat += this->energyStat;
    this->totalEConsumed += NEC;
    this->totalERegenerated += NER;

    if (this->cumRegionalConsumedEnergyStat.count(currentRegion) > 0) {
        this->cumRegionalConsumedEnergyStat[currentRegion] = 
            this->cumRegionalConsumedEnergyStat[currentRegion] + this->energyStat;
    }
    else {
        this->cumRegionalConsumedEnergyStat[currentRegion] = this->energyStat;
    }
}

// ##################################################################
// #                   end: train energy consumption                #
// ##################################################################


void Train::resetTrainLookAhead(){
    this->lookAheadCounterToUpdate = DefaultLookAheadCounterToUpdate;
    this->lookAheadStepCounter = DefaultLookAheadCounter;
}

void Train::resetTrain() {
    this->betweenNodesLengths.clear();
    for (Vector< std::shared_ptr<TrainComponent>>::iterator it = this->trainVehicles.begin(); it != this->trainVehicles.end(); ++it) {
        it->get()->trackCurvature = 0.0;
        it->get()->trackGrade = 0.0;
        it->get()->energyConsumed = 0.0;
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
    //this->currentUsedTractivePower = 0.0;
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
