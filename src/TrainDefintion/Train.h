//
// Created by Ahmed Aredah
// Version 0.1
//

#ifndef NeTrainSim_Train_h
#define NeTrainSim_Train_h


#include <iostream>
#include "../util/Vector.h"
#include "Car.h"
#include "Locomotive.h"
//#include "../network/NetLink.h"
#include "EnergyConsumption.h"
#include "../util/Map.h"
#include <utility>
#include <variant>

class NetNode;
class NetLink;
using namespace std;

class Train {
private:
    // holds the number of trains in the simulator
    static unsigned int NumberOfTrainsInSimulator;
    // the default desired deceleration rate 
    static constexpr double DefaultDesiredDecelerationRate = 0.2;
    // the default reaction time of the train operator
    static constexpr double DefaultOperatorReactionTime = 1.0;
    // the default switch of the train behaviour if no energy source
    static constexpr bool DefaultStopIfNoEnergy = false;
    // the default switch if the train is running off the grid or on the grid
    static constexpr bool DefaultIsRunningOffGrid = false;
    // the default allowed jerk
    static constexpr double DefaultMaxAllowedJerk = 2.0;
    // the min gap between the current train and the leading train in m
    static constexpr double DefaultMinFollowingGap = 2.0;

public:

#pragma region DefaultTrainCharacteristics
    // the speed of sound in m / s, this is an approximation of the brackes back propagation
    static constexpr double speedOfSound = 343.0;
    // the desired decceleration value
    double d_des;
    // the perception reaction time of the train operator
    double operatorReactionTime;
    // change this to true if you want the train to stop if it runs out of energy
    bool stopTrainIfNoEnergy;
    // change this to true if you want the train to run on off - grid mode
    bool isOffGrid;
    // true if the train has energy, false if dead
    bool isOn;
#pragma endregion

#pragma region generalVariables
    // the name of the train
    string trainUserID;
    // number of cars in the train
    int nCars = 0;
    // number of locomotives in the train
    int nlocs = 0;
    // total length of the train
    double totalLength;
    // holds all cars in the train
    Vector<std::shared_ptr<Car>> cars;
    // holds all locomotives in the train
    Vector<std::shared_ptr<Locomotive>> locomotives;
    // the total weight of the train
    double totalMass;
    // the total empty weight of the train
    double totalEmptyMass = 0;
    // the name of the train
    int id;
    // gravitational acceleration
    const double g = 9.8066;  
    // coefficient of fricition between the trains' wheels and the track
    double coefficientOfFriction = 0.9;
    // max allowable jerk (m/s^3) for the train
    double maxJerk = 2.0;
    // time to fully activate the brakes, considering the network signal speed equals speed of sound
    double T_s;
    // start time of the train to enter the network retrative to the beginning of the simulator
    double trainStartTime;
    // total time spent between the train entering and leaving the network
    double tripTime;
    // if the train is on the network, it is true, false otherwise.
    bool offloaded = false;
    // the predefined path of the train by the simulator node id. It is originally populated by the user node ids.
    // the simulator replaces it by the simulator node ids.
    Vector<int> trainPath;
    // the predefined path of the train by the node reference.
    Vector<std::shared_ptr<NetNode>> trainPathNodes;

    Vector<bool> trainStoppingStations;
    
    // true if the simulator reached its destination, false otherwise.
    bool reachedDestination = false;
    // the total length of the path the train is suppost to be taking.
    double trainTotalPathLength;
    // holds the current coordinates of the tip of the train
    pair<double, double> currentCoordinates;
    // the spanned links the train is on. works as blocks
    Vector<std::shared_ptr<NetLink>> currentLinks;
    // holds the centroid location mapped by the car/loco and relative to the tip of the train
    Map<std::shared_ptr<TrainComponent>, double> WeightCentroids;
    // holds the first link the train is on
    std::shared_ptr<NetLink> currentFirstLink;
    // true if the train is loaded to the simulator, false otherwise 
    bool loaded = false;
    // the previous node ID the tip of the train just passed
    int previousNodeID;
    // the previous node ID the last point of the train just passed
    int LastTrainPointpreviousNodeID;
    // holds the computed distances between two nodes along the train's path
    Vector<Vector<double>> betweenNodesLengths;
    // holds the cummulative distance from the start of the train's path to each and every node in the path
    Vector<double> linksCumLengths;
    // holds the lower speed node ID's the train will have to reduce its speed at
    Vector<Vector<Map<int, double>>> LowerSpeedNodeIDs;
    // the next node the train is targetting
    int nextNodeID;
    // holds both the start and end tips' coordinates of the train
    Vector<pair<double, double>> startEndPoints;
    // the previous links the train spanned before.
    Vector<std::shared_ptr<NetLink>> previousLinks;
    // grade of the links the train is taking and it is mapped by the link ID
    Map<int, double> LinkGradeDirection;
    // holds the arrangement of the train and how locomotives and cars are arranged in that train
    Vector < std::shared_ptr<TrainComponent>> trainVehicles;

    // counts the number of steps the train could not move forward because of the lack of power source
    int NoPowerCountStep;
    // maps the train cars types 
    Map<TrainTypes::CarType, Vector<std::shared_ptr<Car>>> carsTypes;
    // maps the train active cars types
    Map<TrainTypes::CarType, Vector<std::shared_ptr<Car>>> ActiveCarsTypes;
    // maps the train active locomotives types
    Vector<std::shared_ptr<Locomotive>> ActiveLocos;
    //// holds the number of cars that has rechargable capability
    //int numberOfRechargableCars;
    //// holds the number of locomotives that has rechargable capability
    //int numberofRechargableLocos;
#pragma endregion

#pragma region dynamicsVariables
    // travelled distance of the train measured from the front tip of the train
    double travelledDistance;
    // travelled distance of the train measured from the front tip of the train (virtual and does not affect train movement.
    // it is only used for optimization.
    double virtualTraveledDistance;
    // the current speed of the train (at time t)
    double currentSpeed;
    // the previous speed of the train (at time t-1)
    double previousSpeed;
    // the current acceleration of the train (at time t)
    double currentAcceleration;
    // the previous acceleration of the train (at time t-1)
    double previousAcceleration;
    // the current tractive forces the train is using 
    double currentTractiveForce;
    // the current resistance forces on the train 
    double currentResistanceForces;

    // the current used tractive power that the locomotives provides
    double currentUsedTractivePower;
    // 
    Vector<double> currentUsedTractivePowerList;

    // the number of steps ahead the train is looking aheaf for optimization
    int lookAheadStepCounter;
    // the number of steps ahead the train should update its optimization at
    int lookAheadCounterToUpdate;
    // the optimum throttle level that the train should go by to minimize its energy use
    int optimumThrottleLevel;
    // true if the train should optimize its energy consumption. train trajectory will vary here.
    bool optimize;
#pragma endregion

#pragma region EnergyConsumptionAndStatsVariables
    //  total energy consumption (consumed + regenerated) at time step t
    double energyStat;
    // cumulative total energy consumed till time step t
    double cumEnergyStat;
    // total energy consumpted only of the train till time t
    double totalEConsumed;
    // energy regenerated of the train till time t
    double totalERegenerated;
    // total energy consumed till time step t mapped by region
    Map<string, double> cumRegionalConsumedEnergyStat;
    // the time the train is delayed at time step t, relative to min free flow speed of all spanned links
    double delayTimeStat;
    // cumulative total time delayed untill time step t, relative to min free flow speed of all spanned links
    double cumDelayTimeStat;
    // the time the train is delayed at time step t, relative to max free flow speed of all spanned links
    double maxDelayTimeStat;
    // total time delayed untill time step t, relative to max free flow speed of all spanned links
    double cumMaxDelayTimeStat;
    // statistic of the stoppings at time t.
    double stoppedStat;
    // statistic of stoppings untill time t
    double cumStoppedStat;

#pragma endregion

    // this function returns how many trains are loaded in the simulator
    static unsigned int getNumberOfTrainsInSimulator();

    // This constructor initializes a train with the passed parameters
    Train(string id, Vector<int> trainPath, double trainStartTime_sec, double frictionCoeff, 
        Vector<std::shared_ptr<Locomotive>> locomotives, Vector<std::shared_ptr<Car>> cars, bool optimize, 
        double desiredDecelerationRate_mPs = DefaultDesiredDecelerationRate,
        double operatorReactionTime_s = DefaultOperatorReactionTime, 
        bool stopIfNoEnergy = DefaultStopIfNoEnergy, bool isRunnigOffGrid = DefaultIsRunningOffGrid,
        double maxAllowedJerk_mPcs = DefaultMaxAllowedJerk);

    double getMinFollowingTrainGap();
    double getCargoNetWeight();
    Map<TrainTypes::PowerType, int> LocTypeCount();

    // This function returns the centroids of all vehicles in the train
    Map < std::shared_ptr<TrainComponent>, double> getTrainCentroids();
    
    int getActiveLocomotivesNumber();
    double getAverageLocomotivesBatteryStatus();
    double getTrainTotalTorque();
    double getAverageTendersStatus();
    //void setCars(Vector<Car> cars);
    //void setLocomotives(Vector<Locomotive> locomotives);
    void setTrainLength();
    void setTrainWeight();

    void resetTrain();
    void rearrangeTrain();
    void updateGradesCurvatures(Vector<double> &LocsCurvature, Vector<double> &LocsGrade, 
        Vector<double> &CarsCurvature, Vector<double> &CarsGrade);
    void updateGradesCurvatures(const Vector<double> &trainGrade, const Vector<double> &trainCurvature);

    double getTotalTractiveForce(double speed, double acceleration, bool optimize, double optimumThrottleLevel);
    double getTotalResistance(double speed);
    double getAccelerationUpperBound(double speed, double acceleration, double freeFlowSpeed, 
        bool optimize, double optimumThrottleLevel);
    double getSafeGap(double initialGap, double speed, double freeFlowSpeed, double T_s, bool estimate);

    double getNextTimeStepSpeed(double gap, double minGap, double speed, double freeFlowSpeed, 
        double aMax, double T_s, double deltaT);
    double getTimeToCollision(double gap, double minGap, double speed, double leaderSpeed);

    double accelerate(double gap, double mingap, double speed, double acceleration, double leaderSpeed,
        double freeFlowSpeed, double deltaT, bool optimize, double throttleLevel = -1);
    double accelerateConsideringJerk(double acceleration, double previousAcceleration, double jerk, double deltaT);
    double smoothAccelerate(double acceleration, double previousAccelerationValue, double alpha = 0.2);
    double speedUpDown(double previousSpeed, double acceleration, double deltaT, double freeFlowSpeed);
    double adjustAcceleration(double speed, double previousSpeed, double deltaT);
    void checkSuddenAccChange(double previousAcceleration, double currentAcceleration, double deltaT);

    void moveTrain(double timeStep, double freeFlowSpeed, Vector<double>& gapToNextCriticalPoint,
        Vector<bool>& gapToNextCriticalPointType, Vector<double>& leaderSpeed);

    pair<Vector<double>, double> getTractivePower(double speed, double acceleration, double resistanceForces);
    void updateLocNotch();
    void immediateStop(double timeStep);
    void kickForwardADistance(double& distance);
    double getEnergyConsumption(double timeStep);
    void calculateEnergyConsumption(double timeStep, std::string currentRegion);
    bool consumeEnergy(double& timeStep, Vector<double>& usedTractivePower);
    void resetTrainEnergyConsumption();
    bool consumeTendersEnergy(double EC_kwh, TrainTypes::PowerType powerType,
        double dieselConversionFactor = EC::DefaultDieselConversionFactor, 
        double hydrogenConversionFactor = EC::DefaultHydrogenConversionFactor, 
        double dieselDensity = EC::DefaultDieselDensity);
    Vector<std::shared_ptr<Car>>  getActiveTanksOfType(TrainTypes::CarType cartype);
    int getRechargableCarsNumber();
    int getRechargableLocsNumber();

#pragma region StatsCalc
    void calcTrainStats(Vector<double> listOfLinksFreeFlowSpeeds, double MinFreeFlow, double timeStep, std::string currentRegion);
    double getDelayTimeStat(double freeflowSpeed, double timeStep);
    double getMaxDelayTimeStat(Vector<double> listOfLinksFreeFlowSpeeds, double timeStep);
    double getStoppingTimeStat(Vector<double> listOfLinksFreeFlowSpeeds);
#pragma endregion

    friend ostream& operator<<(ostream& ostr, Train& train);

    private:
        double get_acceleration_an11(double u_hat, double speed, double TTC_s, double frictionCoef);
        double get_acceleration_an12(double u_hat, double speed, double T_s, double amax);
        double get_beta1(double an11);
        double get_acceleration_an13(double beta1, double an11, double an12);
        double get_acceleration_an14(double speed, double leaderSpeed, double T_s, double amax, double frictionCoef);
        double get_beta2();
        double get_acceleration_an1(double beta2, double an13, double an14);
        double get_gamma(double speedDiff);
        double get_acceleration_an2(double gap, double minGap, double speed, double leaderSpeed, double T_s, double frictionCoef);

};

#endif // !NeTrainSim_Train_h
