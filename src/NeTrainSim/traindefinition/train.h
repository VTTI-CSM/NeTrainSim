//
// Created by Ahmed Aredah
// Version 0.1
//

#ifndef NeTrainSim_Train_h
#define NeTrainSim_Train_h


#include <iostream>
#include "../util/vector.h"
#include "car.h"
#include "locomotive.h"
#include "../util/map.h"
#include "qobject.h"
#include <utility>
#include <variant>

/**
 * A net node.
 *
 * @author	Ahmed Aredah
 * @date	2/28/2023
 */
class NetNode;

/**
 * A net link.
 *
 * @author	Ahmed Aredah
 * @date	2/28/2023
 */
class NetLink;
using namespace std;

/**
 * A train.
 *
 * @author	Ahmed Aredah
 * @date	2/28/2023
 */
class Train : public QObject {
    Q_OBJECT
    /***********************************************
    *              variables declaration           *
    ************************************************/

private:
    /** Holds the number of trains in the simulator. */
    static unsigned int NumberOfTrainsInSimulator;
    /** (Immutable) the default desired deceleration rate */
    static constexpr double DefaultDesiredDecelerationRate = 0.2;
    /** (Immutable) the default reaction time of the train operator */
    static constexpr double DefaultOperatorReactionTime = 1.0;
    /** (Immutable) the default switch of the train behaviour if no energy source */
    static constexpr bool DefaultStopIfNoEnergy = false;
    /** (Immutable) the default allowed jerk */
    static constexpr double DefaultMaxAllowedJerk = 2.0;
    /** (Immutable) the min gap between the current train and the leading train in m */
    static constexpr double DefaultMinFollowingGap = 2.0;
    /** The default look ahead counter to update the throttle level of the train */
    static constexpr int DefaultLookAheadCounterToUpdate = 1;
    /** The default look ahead number of steps */
    static constexpr int DefaultLookAheadCounter = 1;

public:

    /**
     * (Immutable) the speed of sound in m / s, this is an approximation of the brackes back
     * propagation
     */
    static constexpr double speedOfSound = 343.0;
    /** (Immutable) gravitational acceleration */
    const double g = 9.8066;
    /** The desired decceleration value */
    double d_des;
    /** the perception reaction time of the train operator. */
    double operatorReactionTime;
    /** Total length of the train */
    double totalLength;
    /** The total weight of the train in kg*/
    double totalMass;
    /** The total empty weight of the train */
    double totalEmptyMass = 0;

    /** Coefficient of fricition between the trains' wheels and the track */
    double coefficientOfFriction = 0.9;
    /** Max allowable jerk (m/s^3) for the train */
    double maxJerk = 2.0;
    /** Time to fully activate the brakes, considering the network signal speed equals speed of sound */
    double T_s;
    /** Start time of the train to enter the network retrative to the beginning of the simulator */
    double trainStartTime;
    /** Total time spent between the train entering and leaving the network */
    double tripTime;
    /** The total length of the path the train is suppost to be taking. */
    double trainTotalPathLength;
    /** Travelled distance of the train measured from the front tip of the train */
    double travelledDistance;
    /**
     * Travelled distance of the train measured from the front tip of the train (virtual and does
     * not affect train movement. it is only used for optimization.
     */
    double virtualTravelledDistance;
    /** The current speed of the train (at time t) */
    double currentSpeed;
    /** The previous speed of the train (at time t-1) */
    double previousSpeed;
    /** The average journey speed of the train from t = 0 to t */
    double averageSpeed;
    /** The current acceleration of the train (at time t) */
    double currentAcceleration;
    /** The previous acceleration of the train (at time t-1) */
    double previousAcceleration;
    /** The average journey acceleration of the train from t = 0 to t */
    double averageAcceleration;
    /** The current tractive forces the train is using in Newton.*/
    double currentTractiveForce;
    /** The current resistance forces on the train in Newton*/
    double currentResistanceForces;
    /** The current used tractive power that the locomotives provides in kw*/
    double currentUsedTractivePower_W;
    /** The cummulative used tractive power (work) that the locomotives provide in kw*/
    double cumUsedTractivePower_W;
    /** The optimum throttle level that the train should go by to minimize its energy use */
    double optimumThrottleLevel;
    /** Total energy consumption (consumed + regenerated) at time step t */
    double energyStat;
    /** Cumulative total energy consumed till time step t */
    double cumEnergyStat;
    /** Total energy consumpted only of the train till time t */
    double totalEConsumed;
    /** Energy regenerated of the train till time t */
    double totalERegenerated;
    /** The time the train is delayed at time step t, relative to min free flow speed of all spanned links. */
    double delayTimeStat;
    /** Cumulative total time delayed untill time step t, relative to min free flow speed of all spanned links. */
    double cumDelayTimeStat;
    /** The time the train is delayed at time step t, relative to max free flow speed of all spanned links. */
    double maxDelayTimeStat;
    /** Total time delayed untill time step t, relative to max free flow speed of all spanned links. */
    double cumMaxDelayTimeStat;
    /** Statistic of the stoppings at time t. */
    double stoppedStat;
    /** Statistic of stoppings untill time t. */
    double cumStoppedStat;
    /** holds the waited time at any depot */
    double waitedTimeAtNode;

    /** holds the weight of speed priority*/
    double optimizeForSpeedNormalizedWeight;
    /** holds the optimum throttle levels*/
    Vector<double> optimumThrottleLevels;

    /** Holds the current coordinates of the tip of the train */
    pair<double, double> currentCoordinates;


    /** Holds the first link the train is on */
    std::shared_ptr<NetLink> currentFirstLink;
    /** Holds the centroid location mapped by the car/loco and relative to the tip of the train */
    Map<std::shared_ptr<TrainComponent>, double> WeightCentroids;
    /** Grade of the links the train is taking and it is mapped by the link ID */
    Map<int, double> LinkGradeDirection;
    /** Maps the train cars types */
    Map<TrainTypes::CarType, Vector<std::shared_ptr<Car>>> carsTypes;
    /** Maps the train active cars types */
    Map<TrainTypes::CarType, Vector<std::shared_ptr<Car>>> ActiveCarsTypes;
    /** Total energy consumed till time step t mapped by region */
    Map<string, double> cumRegionalConsumedEnergyStat;


    /** Holds all cars in the train */
    Vector<std::shared_ptr<Car>> cars;
    /** Holds all locomotives in the train */
    Vector<std::shared_ptr<Locomotive>> locomotives;
    /** The predefined path of the train by the simulator node id. It is originally populated by the
     * user node ids. the simulator replaces it by the simulator node ids. */
    Vector<int> trainPath;
    /** The predefined path of the train by the node reference. */
    Vector<std::shared_ptr<NetNode>> trainPathNodes;
    /** The spanned links the train is on. works as blocks */
    Vector<std::shared_ptr<NetLink>> currentLinks;
    /** Holds the computed distances between two nodes along the train's path */
    Vector<Vector<double>> betweenNodesLengths;
    /** Holds the cummulative distance from the start of the train's path to each and every node in
     * the path. */
    Vector<double> linksCumLengths;
    /** Holds the lower speed node ID's the train will have to reduce its speed at */
    Vector<Vector<Map<int, double>>> LowerSpeedNodeIDs;
    /** Holds both the start and end tips' coordinates of the train */
    Vector<pair<double, double>> startEndPoints;
    /** The previous links the train spanned before. */
    Vector<std::shared_ptr<NetLink>> previousLinks;
    /** holds the arrangement of the train and how locomotives and cars are arranged in that train. */
    Vector < std::shared_ptr<TrainComponent>> trainVehicles;
    /** Maps the train active locomotives types */
    Vector<std::shared_ptr<Locomotive>> ActiveLocos;
    /** The current used tractive power list */
    Vector<double> currentUsedTractivePowerList_W;
    /** The throttle levels that the train will go by. */
    Vector<double> throttleLevels;

    /** The name of the train */
    string trainUserID;

    /** The train stopping stations */
    Vector<bool> trainStoppingStations;


    /** Number of cars in the train */
    int nCars = 0;
    /** Number of locomotives in the train */
    int nlocs = 0;
    /** The name of the train */
    int id;
    /** The previous node ID the tip of the train just passed */
    int previousNodeID;
    /** The previous node ID the last point of the train just passed */
    int LastTrainPointpreviousNodeID;
    /** The next node the train is targetting */
    int nextNodeID;
    /** Counts the number of steps the train could not move forward because of the lack of power
     * source. */
    int NoPowerCountStep;
    /** The number of steps ahead the train is looking aheaf for optimization */
    int lookAheadStepCounter;
    int mem_lookAheadStepCounter;
    /** The number of steps ahead the train should update its optimization at */
    int lookAheadCounterToUpdate;
    int mem_lookAheadCounterToUpdate;

    // **************************************************************
    // ************** For hybrid locomotives only *******************
    // **************************************************************
    TrainTypes::HybridCalculationMethod defaultHybridLocoType =
        TrainTypes::HybridCalculationMethod::fast;
    int forwardHorizonStepsInMPC = 3; // # steps forward in the MPC optimization
    int discritizationCount = 20;
    // **************************************************************
    // **************************************************************
    // **************************************************************


    /** Change this to true if you want the train to stop if it runs out of energy */
    bool stopTrainIfNoEnergy;
    /** True if the train has energy, false if dead */
    bool isOn;
    /** If the train is on the network, it is true, false otherwise. */
    bool offloaded = false;
    /** True if the simulator reached its destination, false otherwise. */
    bool reachedDestination = false;
    /** True if the train ran out of energy. */
    bool outOfEnergy = false;
    /** True if the train is loaded to the simulator, false otherwise */
    bool loaded = false;
    /** True if the train should optimize its energy consumption. train trajectory will vary here. */
    bool optimize;


    /**
     * \brief This constructor initializes a train with the passed parameters
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param   simulatorID                 simulatorID A unique identifier for the node, it should start by 0 and increment by 1
     * @param 	id						   	The identifier.
     * @param 	trainPath				   	Full pathname of the train file.
     * @param 	trainStartTime_sec		   	The train start time security.
     * @param 	frictionCoeff			   	The friction coeff.
     * @param 	locomotives				   	The locomotives.
     * @param 	cars					   	The cars.
     * @param 	optimize				   	True to optimize.
     * @param 	desiredDecelerationRate_mPs	(Optional) The desired deceleration rate m ps.
     * @param 	operatorReactionTime_s	   	(Optional) The operator reaction time s.
     * @param 	stopIfNoEnergy			   	(Optional) True to stop if no energy.
     * @param 	isRunnigOffGrid			   	(Optional) True if is runnig off grid, false if not.
     * @param 	maxAllowedJerk_mPcs		   	(Optional) The maximum allowed jerk m pcs.
     */
    Train(int simulatorID, string id, Vector<int> trainPath, double trainStartTime_sec, double frictionCoeff,
          Vector<std::shared_ptr<Locomotive>> locomotives, Vector<std::shared_ptr<Car>> cars, bool optimize,
          double desiredDecelerationRate_mPs = DefaultDesiredDecelerationRate,
          double operatorReactionTime_s = DefaultOperatorReactionTime,
          bool stopIfNoEnergy = DefaultStopIfNoEnergy,
          double maxAllowedJerk_mPcs = DefaultMaxAllowedJerk,
          double optimization_k = 0.0,
          int runOptimizationEvery = DefaultLookAheadCounterToUpdate,
          int optimizationLookaheadSteps = DefaultLookAheadCounter);

    ~Train();

    /**
     * @brief setOptimization   enable or disable the single train trajectory optimization
     * @param enable    bool value to enable optimization if true, false O.W.
     * @param optimizationSpeedImportanceNormalizedWeight double value to set the importance of speed compared to the energy consumption [0,1].
     */
    void setOptimization(bool enable = false,
                         double optimizationSpeedImportanceNormalizedWeight = 1.0,
                         int runOptimizationEvery = DefaultLookAheadCounterToUpdate,
                         int optimizationLookaheadSteps = DefaultLookAheadCounter);

    void setTrainSimulatorID(int newID);

    /**
     * @brief recharge all train cars batteries.
     * @param EC_kwh
     * @return
     */
    bool rechargeCarsBatteries(double timeStep, double EC_kwh, std::shared_ptr<Locomotive> &loco);

    /**
     * @brief getTrainConsumedTank
     * @return number of litters consumed from the tank.
     */
    Map<string, double> getTrainConsumedTank();

    /**
     * @brief set the train path.
     * @param path the new path of the simulator node ids.
     */
    void setTrainPath(Vector<int> path);
    /**
     * this function returns how many trains are loaded in the simulator
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	The number of trains in simulator.
     */
    static unsigned int getNumberOfTrainsInSimulator();


    /**
     * \brief Gets minimum following train gap
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	The minimum following train gap.
     */
    double getMinFollowingTrainGap();

    /**
     * @brief set the current links the train is spanning
     * @param newLinks
     */
    void setTrainsCurrentLinks(Vector<std::shared_ptr<NetLink> > newLinks);
    /**
     * \brief Gets cargo net weight
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	The cargo net weight.
     */
    double getCargoNetWeight();

    /**
     * \brief Locomotive type count
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	A Map&lt;TrainTypes::PowerType,int&gt;
     */
    Map<TrainTypes::PowerType, int> LocTypeCount();

    /**
     * @brief car type count
     *
     * @author	Ahmed Aredah
     * @date    4/6/2023
     *
     * @returns	A Map&lt;TrainTypes::CarTypes,int&gt;
     */
    Map<TrainTypes::CarType, int> carTypeCount();
    /**
     * \brief This function returns the centroids of all vehicles in the train
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	The train centroids.
     */
    Map < std::shared_ptr<TrainComponent>, double> getTrainCentroids();

    /**
     * \brief Gets active locomotives number
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	The active locomotives number.
     */
    int getActiveLocomotivesNumber();

    /**
     * @brief Gets the battery energy consumed in kWh.
     *
     * @details Gets the battery energy consumed only.
     *          it does not report the energy regenerated.
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @return  the energy consumed in kWh.
     */
    double getBatteryEnergyConsumed();
    /**
     * @brief Gets the battery energy regenerated in kWh.
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @return  the energy regenerated in kWh.
     */
    double getBatteryEnergyRegenerated();

    /**
     * @brief Gets the battery energy consumed in kWh.
     *
     * @details Gets the battery energy consumed and regenerated.
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @return  the energy consumed in kWh.
     */
    double getBatteryNetEnergyConsumed();

    /**
     * \brief Gets average locomotives battery status
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	The average locomotives battery status.
     */
    double getAverageLocomotivesBatteryStatus();

    /**
     * @brief Gets average locomotives tank status
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @return The average locomotives tank status.
     */
    double getAverageLocomotiveTankStatus();

    /**
     * @brief Gets average tenders tank status
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @return The average tenders tank status.
     */
    double getAverageTendersTankStatus();

    /**
     * @brief Gets average tenders battery status
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @return The average tenders battery status.
     */
    double getAverageTendersBatteryStatus();


    /**
     * \brief Gets train total torque in tons x meters.
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	The train total torque.
     */
    double getTrainTotalTorque();

    /**
     * \brief Gets average tenders status
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	The average tenders status.
     */
    double getAverageTendersStatus();
    //void setCars(Vector<Car> cars);
    //void setLocomotives(Vector<Locomotive> locomotives);

    /**
     * \brief Sets train length
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     */
    void setTrainLength();

    /**
     * \brief Sets train weight
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     */
    void setTrainWeight();

    /**
     * \brief Resets the train parameters
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     */
    void resetTrain();

    /**
     * \brief Rearrange train
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     */
    void rearrangeTrain();

    /**
     * \brief Updates the grades curvatures
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param [in,out]	LocsCurvature	The locs curvature.
     * @param [in,out]	LocsGrade	 	The locs grade.
     * @param [in,out]	CarsCurvature	The cars curvature.
     * @param [in,out]	CarsGrade	 	The cars grade.
     */
    void updateGradesCurvatures(Vector<double> &LocsCurvature, Vector<double> &LocsGrade, 
        Vector<double> &CarsCurvature, Vector<double> &CarsGrade);

    /**
     * \brief Updates the grades curvatures
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	trainGrade	  	The train grade.
     * @param 	trainCurvature	The train curvature.
     */
    void updateGradesCurvatures(const Vector<double> &trainGrade, const Vector<double> &trainCurvature);

    /**
     * \brief Gets total tractive force
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	speed					The speed.
     * @param 	acceleration			The acceleration.
     * @param 	optimize				True to optimize.
     * @param 	optimumThrottleLevel	The optimum throttle level.
     *
     * @returns	The total tractive force.
     */
    double getTotalTractiveForce(double speed, double acceleration, bool optimize, double optimumThrottleLevel);

    /**
     * \brief Gets total resistance
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	speed	The speed.
     *
     * @returns	The total resistance.
     */
    double getTotalResistance(double speed);

    /**
     * \brief Gets acceleration upper bound.
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	speed					The speed.
     * @param 	acceleration			The acceleration.
     * @param 	freeFlowSpeed			The free flow speed.
     * @param 	optimize				True to optimize.
     * @param 	optimumThrottleLevel	The optimum throttle level.
     *
     * @returns	The acceleration upper bound.
     */
    double getAccelerationUpperBound(double speed, double acceleration, double freeFlowSpeed, 
        bool optimize, double optimumThrottleLevel);

    /**
     * \brief Gets safe gap.
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	initialGap   	The initial gap.
     * @param 	speed		 	The speed.
     * @param 	freeFlowSpeed	The free flow speed.
     * @param 	T_s			 	The s.
     * @param 	estimate	 	True to estimate.
     *
     * @returns	The safe gap.
     */
    double getSafeGap(double initialGap, double speed, double freeFlowSpeed, double T_s, bool estimate);

    /**
     * \brief Gets the next time step speed
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	gap			 	The gap.
     * @param 	minGap		 	The minimum gap.
     * @param 	speed		 	The speed.
     * @param 	freeFlowSpeed	The free flow speed.
     * @param 	aMax		 	The maximum.
     * @param 	T_s			 	The s.
     * @param 	deltaT		 	The delta t.
     *
     * @returns	The next time step speed.
     */
    double getNextTimeStepSpeed(double gap, double minGap, double speed, double freeFlowSpeed, 
        double aMax, double T_s, double deltaT);

    /**
     * \brief Gets time to collision
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	gap		   	The gap.
     * @param 	minGap	   	The minimum gap.
     * @param 	speed	   	The speed.
     * @param 	leaderSpeed	The leader speed.
     *
     * @returns	The time to collision.
     */
    double getTimeToCollision(double gap, double minGap, double speed, double leaderSpeed);

    /**
     * \brief Gets the acceleration of the train.
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	gap			 	The gap.
     * @param 	mingap		 	The mingap.
     * @param 	speed		 	The speed.
     * @param 	acceleration 	The acceleration.
     * @param 	leaderSpeed  	The leader speed.
     * @param 	freeFlowSpeed	The free flow speed.
     * @param 	deltaT		 	The delta t.
     * @param 	optimize	 	True to optimize.
     * @param 	throttleLevel	(Optional) The throttle level.
     *
     * @returns	A double.
     */
    double accelerate(double gap, double mingap, double speed, double acceleration, double leaderSpeed,
        double freeFlowSpeed, double deltaT, bool optimize, double throttleLevel = -1);

    /**
     * \brief Accelerate considering jerk
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	acceleration			The acceleration.
     * @param 	previousAcceleration	The previous acceleration.
     * @param 	jerk					The jerk.
     * @param 	deltaT					The delta t.
     *
     * @returns	A double.
     */
    double accelerateConsideringJerk(double acceleration, double previousAcceleration, double jerk, double deltaT);

    /**
     * \brief Smooth the acceleration.
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	acceleration			 	The acceleration.
     * @param 	previousAccelerationValue	The previous acceleration value.
     * @param 	alpha					 	(Optional) The alpha.
     *
     * @returns	A double.
     */
    double smoothAccelerate(double acceleration, double previousAccelerationValue, double alpha = 0.2);

    /**
     * \brief Gets the speed of the train based on the acceleration.
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	previousSpeed	The previous speed.
     * @param 	acceleration 	The acceleration.
     * @param 	deltaT		 	The delta t.
     * @param 	freeFlowSpeed	The free flow speed.
     *
     * @returns	A double.
     */
    double speedUpDown(double previousSpeed, double acceleration, double deltaT, double freeFlowSpeed);

    /**
     * \brief Adjust acceleration
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	speed		 	The speed.
     * @param 	previousSpeed	The previous speed.
     * @param 	deltaT		 	The delta t.
     *
     * @returns	A double.
     */
    double adjustAcceleration(double speed, double previousSpeed, double deltaT);

    /**
     * \brief Check sudden accumulate change
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	previousAcceleration	The previous acceleration.
     * @param 	currentAcceleration 	The current acceleration.
     * @param 	deltaT					The delta t.
     */
    void checkSuddenAccChange(double previousAcceleration, double currentAcceleration, double deltaT);

    /**
     * @brief getStepDynamics
     * @param timeStep
     * @param freeFlowSpeed
     * @param gapToNextCriticalPoint
     * @param gapToNextCriticalPointType
     * @param leaderSpeed
     * @return
     */
    double getStepAcceleration(double timeStep, double freeFlowSpeed, Vector<double>& gapToNextCriticalPoint,
                                         Vector<bool> &gapToNextCriticalPointType, Vector<double>& leaderSpeed);

    /**
     * \brief Move train forward
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 		  	timeStep				  	The time step.
     * @param 		  	freeFlowSpeed			  	The free flow speed.
     * @param [in,out]	gapToNextCriticalPoint	  	The gap to next critical point.
     * @param [in,out]	gapToNextCriticalPointType	Type of the gap to next critical point.
     * @param [in,out]	leaderSpeed				  	The leader speed.
     */
    void moveTrain(double timeStep, double freeFlowSpeed, Vector<double>& gapToNextCriticalPoint,
        Vector<bool>& gapToNextCriticalPointType, Vector<double>& leaderSpeed);

    /**
     * \brief Gets tractive power
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	speed				The speed.
     * @param 	acceleration		The acceleration.
     * @param 	resistanceForces	The resistance forces.
     *
     * @returns	The tractive power.
     */
    pair<Vector<double>, double> getTractivePower(double speed, double acceleration, double resistanceForces);

    /**
     * \brief Updates the location notch
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     */
    void updateLocNotch();

    /**
     * \brief Immediate stop
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	timeStep	The time step.
     */
    void immediateStop(double timeStep);

    /**
     * \brief Kick forward a distance
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param [in,out]	distance	The distance.
     */
    void kickForwardADistance(double& distance);

    /**
     * \brief Gets energy consumption
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	timeStep	The time step.
     *
     * @returns	The energy consumption.
     */
    double getEnergyConsumption(double timeStep);

    /**
     * \brief Calculates the energy consumption
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	timeStep	 	The time step.
     * @param 	currentRegion	The current region.
     */
    void calculateEnergyConsumption(double timeStep, std::string currentRegion);

    /**
     * \brief Gets total energy consumption
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param [in,out]	timeStep		 	The time step.
     * @param [in,out]  StepSpeed           The train speed.
     * @param [in,out]  stepAcceleration    The train acceleration.
     * @param [in,out]	usedTractivePower	The used tractive power.
     *
     * @returns	The total energy consumption.
     */
    double getTotalEnergyConsumption(double& timeStep, double& stepSpeed, double& stepAcceleration, Vector<double>& usedTractivePower);

    bool isMPCOptimizationNeeded();

    void
    whatCostIfConsumeEnergyWithHeuristic(
        double& timeStep, double trainSpeed,
        Vector<double>& usedTractivePower,
        int stepCounter);

    /**
     * @brief Calculates the cost if we consume energy consumption at this time step.
     * @param timeStep The time step in seconds
     * @param trainSpeed the current train speed
     * @param usedTractivePower the used tractive power of all the train
     * @param stepCounter is the forward step counter [int]
     * @return the min cost associated with the current hybrid configuration.
     */
    // void whatCostIfConsumeEnergy(double& timeStep, double trainSpeed,
    //                     Vector<double>& usedTractivePower,
    //                     int stepCounter);
    /**
     * \brief Consume energy
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param [in,out]	timeStep		 	The time step.
     * @param [in,out]	usedTractivePower	The used tractive power in Watt.
     *
     * @returns	True if it succeeds, false if it fails.
     */
    bool consumeEnergy(double& timeStep, double trainSpeed, Vector<double>& usedTractivePower_W);

    /**
     * \brief Resets the train energy consumption
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     */
    void resetTrainEnergyConsumption();

    /**
     * \brief Consume tenders energy
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	EC_kwh						The ec kwh.
     * @param 	powerType					Type of the power.
     * @param 	dieselConversionFactor  	(Optional) The diesel conversion factor.
     * @param 	hydrogenConversionFactor	(Optional) The hydrogen conversion factor.
     * @param 	dieselDensity				(Optional) The diesel density.
     *
     * @returns	True if it succeeds, false if it fails.
     */
    std::pair<bool, double> consumeTendersEnergy(double timeStep, double trainSpeed, double EC_kwh, TrainTypes::PowerType powerType,
                              double dieselConversionFactor = EC::DefaultDieselConversionFactor,
                              double hydrogenConversionFactor = EC::DefaultHydrogenConversionFactor,
                              double dieselDensity = EC::DefaultDieselDensity);

    /**
     * \brief Gets active tanks of type
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	cartype	The cartype.
     *
     * @returns	The active tanks of type.
     */
    Vector<std::shared_ptr<Car>>  getActiveTanksOfType(TrainTypes::CarType cartype);

    /**
     * \brief Gets rechargable cars number
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	The rechargable cars number.
     */
    int getRechargableCarsNumber();

    /**
     * \brief Gets rechargable locs number
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	The rechargable locs number.
     */
    int getRechargableLocsNumber();

    double getRouteProgress();
    /**
     * \brief This function adopts the A Star optimization to get the optimum throttle level.
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	prevSpeed			  	The previous speed.
     * @param 	currentSpeed		  	The current speed.
     * @param 	currentAcceleration   	The current acceleration.
     * @param 	prevThrottle		  	The previous throttle.
     * @param 	vector_grade		  	The vector grade.
     * @param 	vector_curvature	  	The vector curvature.
     * @param 	freeSpeed_ms		  	The free speed in milliseconds.
     * @param 	timeStep			  	The time step.
     * @param 	u_leader			  	The leader.
     * @param 	gapToNextCriticalPoint	The gap to next critical point.
     *
     * @returns	A tuple&lt;double,double,double&gt;
     */
    tuple<double, double, double> AStarOptimization(double prevSpeed, double currentSpeed, double currentAcceleration,
                                                         double prevThrottle, Vector<double> vector_grade,
                                                         Vector<double> vector_curvature, double freeSpeed_ms,
                                                         double timeStep, Vector<double> u_leader,
                                                         Vector<double> gapToNextCriticalPoint);

    /**
     * \brief The heuristic function for the A-Star algorithm
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	distanceToEnd   	The distance to end.
     * @param 	stepAcceleration	The step acceleration.
     * @param 	stepSpeed			The step speed.
     * @param 	timeStep			The time step.
     * @param 	resistance			The resistance.
     * @param 	currentSpeed		The current speed.
     *
     * @returns	A double.
     */
    double heuristicFunction(double distanceToEnd, double stepAcceleration, double stepSpeed,
                                    double timeStep, double resistance, double currentSpeed, double prevSpeed);

    /**
     * \brief Picks the optimal throttle level considering the A-Star optimization
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	throttleLevels				The throttle levels.
     * @param 	lookAheadCounterToUpdate	The look ahead counter to update.
     *
     * @returns	A double.
     */
    double pickOptimalThrottleLevelAStar(Vector<double> throttleLevels, int lookAheadCounterToUpdate);

// ##################################################################
// #                  start: statistics calculations                #
// ##################################################################
    /**
     * \brief Calculates the train statistics
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param   forwardLinksData            The list of ahead topography of the train.
     * @param 	listOfLinksFreeFlowSpeeds	The list of links free flow speeds.
     * @param 	MinFreeFlow				 	The minimum free flow.
     * @param 	timeStep				 	The time step.
     * @param 	currentRegion			 	The current region.
     */
    void calcTrainStatsWithHybridLocosOptimizationOn(
        Vector<std::tuple<Vector<double>, Vector<double>, Vector<double>,
                          Vector<std::shared_ptr<NetLink>>>> forwardLinksData,
        Vector<double> listOfLinksFreeFlowSpeeds, double MinFreeFlow,
        double timeStep, std::string currentRegion);
    /**
     * \brief Calculates the train statistics
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	listOfLinksFreeFlowSpeeds	The list of links free flow speeds.
     * @param 	MinFreeFlow				 	The minimum free flow.
     * @param 	timeStep				 	The time step.
     * @param 	currentRegion			 	The current region.
     */
    void calcTrainStatsAndConsumeEnergy(Vector<double> listOfLinksFreeFlowSpeeds, double MinFreeFlow, double timeStep, std::string currentRegion);

    /**
     * \brief Finds the average of the given arguments
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	previousAverage	   	The previous average.
     * @param 	currentTimeStepData	Information describing the current time step.
     * @param 	timeStep		   	The time step.
     *
     * @returns	The calculated average.
     */
    double calculateAverage(double previousAverage, double currentTimeStepData, double timeStep);

    /**
     * \brief Gets delay time stat
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	freeflowSpeed	The freeflow speed.
     * @param 	timeStep	 	The time step.
     *
     * @returns	The delay time stat.
     */
    double getDelayTimeStat(double freeflowSpeed, double timeStep);

    /**
     * \brief Gets maximum delay time stat
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	listOfLinksFreeFlowSpeeds	The list of links free flow speeds.
     * @param 	timeStep				 	The time step.
     *
     * @returns	The maximum delay time stat.
     */
    double getMaxDelayTimeStat(Vector<double> listOfLinksFreeFlowSpeeds, double timeStep);

    /**
     * \brief Gets stopping time stat
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	listOfLinksFreeFlowSpeeds	The list of links free flow speeds.
     *
     * @returns	The stopping time stat.
     */
    double getStoppingTimeStat(Vector<double> listOfLinksFreeFlowSpeeds);

    /**
     * @brief reset train look ahead parameters
     */
    void resetTrainLookAhead();

    /**
     * @brief getMaxProvidedEnergy
     * @param timeStep
     * @return
     */
    std::pair<double, Map<TrainTypes::PowerType, double>> getMaxProvidedEnergy(double &timeStep);

    /**
     * @brief check if the train can provide the required energy to move forward
     * @param EC
     * @param timeStep
     * @return
     */
    bool canProvideEnergy(double &EC, double &timeStep);

    /**
     * @brief reducePower
     * @param reductionFactor
     */
    void reducePower(double &reductionFactor);

    /**
     * @brief resetPowerRestriction
     */
    void resetPowerRestriction();

// ##################################################################
// #                    end: statistics calculations                #
// ##################################################################

    /**
     * \brief Stream insertion operator
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param [in,out]	ostr 	The ostr.
     * @param [in,out]	train	The train.
     *
     * @returns	The shifted result.
     */
    friend ostream& operator<<(ostream& ostr, Train& train);

    private:

        /**
         * \brief Gets acceleration an 11
         *
         * @author	Ahmed Aredah
         * @date	2/28/2023
         *
         * @param 	u_hat			The hat.
         * @param 	speed			The speed.
         * @param 	TTC_s			The ttc s.
         * @param 	frictionCoef	The friction coef.
         *
         * @returns	The acceleration an 11.
         */
        double get_acceleration_an11(double u_hat, double speed, double TTC_s, double frictionCoef);

        /**
         * \brief Gets acceleration an 12
         *
         * @author	Ahmed Aredah
         * @date	2/28/2023
         *
         * @param 	u_hat	The hat.
         * @param 	speed	The speed.
         * @param 	T_s  	The s.
         * @param 	amax 	The amax.
         *
         * @returns	The acceleration an 12.
         */
        double get_acceleration_an12(double u_hat, double speed, double T_s, double amax);

        /**
         * \brief Gets beta 1
         *
         * @author	Ahmed Aredah
         * @date	2/28/2023
         *
         * @param 	an11	an 11.
         *
         * @returns	The beta 1.
         */
        double get_beta1(double an11);

        /**
         * \brief Gets acceleration an 13
         *
         * @author	Ahmed Aredah
         * @date	2/28/2023
         *
         * @param 	beta1	The first beta.
         * @param 	an11 	an 11.
         * @param 	an12 	an 12.
         *
         * @returns	The acceleration an 13.
         */
        double get_acceleration_an13(double beta1, double an11, double an12);

        /**
         * \brief Gets acceleration an 14
         *
         * @author	Ahmed Aredah
         * @date	2/28/2023
         *
         * @param 	speed			The speed.
         * @param 	leaderSpeed 	The leader speed.
         * @param 	T_s				The s.
         * @param 	amax			The amax.
         * @param 	frictionCoef	The friction coef.
         *
         * @returns	The acceleration an 14.
         */
        double get_acceleration_an14(double speed, double leaderSpeed, double T_s, double amax, double frictionCoef);

        /**
         * \brief Gets beta 2
         *
         * @author	Ahmed Aredah
         * @date	2/28/2023
         *
         * @returns	The beta 2.
         */
        double get_beta2();

        /**
         * \brief Gets acceleration an 1
         *
         * @author	Ahmed Aredah
         * @date	2/28/2023
         *
         * @param 	beta2	The second beta.
         * @param 	an13 	an 13.
         * @param 	an14 	an 14.
         *
         * @returns	The acceleration an1.
         */
        double get_acceleration_an1(double beta2, double an13, double an14);

        /**
         * \brief Gets gamma
         *
         * @author	Ahmed Aredah
         * @date	2/28/2023
         *
         * @param 	speedDiff	The speed difference.
         *
         * @returns	The gamma.
         */
        double get_gamma(double speedDiff);

        /**
         * @brief getMaxProvidedEnergyFromLocomotivesOnly
         * @param timeStep
         * @return
         */
        Map<TrainTypes::PowerType, double> getMaxProvidedEnergyFromLocomotivesOnly(double &timeStep);

        /**
         * @brief getMaxProvidedEnergyFromTendersOnly
         * @param EC
         * @param timeStep
         * @return
         */
        Map<TrainTypes::PowerType, double> getMaxProvidedEnergyFromTendersOnly(Map<TrainTypes::PowerType, double> EC,
                                                                               double &timeStep);
        /**
         * @brief canProvideEnergyFromLocomotives
         * @param EC
         * @param timeStep
         * @return
         */
        Map<TrainTypes::CarType, double> canProvideEnergyFromLocomotivesOnly(double &EC, double &timeStep);

        /**
         * @brief canProvideEnergyFromTendersOnly
         * @param EC
         * @param timeStep
         * @return
         */
        bool canProvideEnergyFromTendersOnly(Map<TrainTypes::CarType, double> &EC, double &timeStep);

        /**
         * \brief Gets acceleration an 2
         *
         * @author	Ahmed Aredah
         * @date	2/28/2023
         *
         * @param 	gap				The gap.
         * @param 	minGap			The minimum gap.
         * @param 	speed			The speed.
         * @param 	leaderSpeed 	The leader speed.
         * @param 	T_s				The s.
         * @param 	frictionCoef	The friction coef.
         *
         * @returns	The acceleration an2.
         */
        double get_acceleration_an2(double gap, double minGap, double speed, double leaderSpeed, double T_s, double frictionCoef);

    public:
    signals:

        /**
         * @brief report a sudden acceleration.
         * @details this is emitted when the train's acceleration is larger
         * than the jerk
         * @param msg is the warning message
         */
        void suddenAccelerationOccurred(std::string msg);

        /**
         * @brief report the trains is very slow or stopped
         * @details this is emitted when the train's speed is very
         * slow either because the resistance is high or because the
         * distance in front of the train is very small
         * @param msg
         */
        void slowSpeedOrStopped(std::string msg);

};

#endif // !NeTrainSim_Train_h
