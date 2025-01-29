#ifndef THREADSAFEAPIDATAMAP_H
#define THREADSAFEAPIDATAMAP_H


#include <QReadWriteLock>
#include <QMap>
#include <QString>
#include <memory>
#include <stdexcept>
#include "simulator.h"
#include "simulatorworker.h"

/**
* @struct APIData
* @brief Container for network-specific simulation components
*
* @details Holds all components required for a single network simulation,
* including network, simulator, workers, and trains. Each network in the
* simulation has its own APIData instance.
*/
struct NETRAINSIMCORE_EXPORT APIData {
    Network * network = nullptr;                          ///< Network instance for routing and geography
    SimulatorWorker* simulatorWorker = nullptr;           ///< Worker for simulation processing
    Simulator * simulator = nullptr;                      ///< Main simulator instance
    QThread *workerThread = nullptr;                      ///< Dedicated thread for this network
    QMap<QString, std::shared_ptr<Train>> trains;         ///< Map of train ID to train instance
    bool isBusy = false;                                  ///< Indicates if network is currently processing
};

class ThreadSafeAPIDataMap
{
public:
    ThreadSafeAPIDataMap() = default;
    ~ThreadSafeAPIDataMap() = default;

    // Add or update APIData
    void addOrUpdate(const QString& networkName, const APIData& data)
    {
        QWriteLocker locker(&mDataLock);
        mData.insert(networkName, data);
    }

    // Remove APIData
    void remove(const QString& networkName)
    {
        QWriteLocker locker(&mDataLock);
        mData.remove(networkName);
    }

    // Get APIData (returns a copy for thread safety)
    APIData get(const QString& networkName) const
    {
        QReadLocker locker(&mDataLock);
        if (!mData.contains(networkName))
        {
            throw std::runtime_error(
                QString("Network not found in APIData: %1").
                arg(networkName).toStdString());
        }
        return mData.value(networkName);
    }

    // Check if a network exists
    bool contains(const QString& networkName) const
    {
        QReadLocker locker(&mDataLock);
        return mData.contains(networkName);
    }

    // Get all network names
    QList<QString> getNetworkNames() const
    {
        QReadLocker locker(&mDataLock);
        return mData.keys();
    }

    // Set busy state for a network
    void setBusy(const QString& networkName, bool busy)
    {
        QWriteLocker locker(&mDataLock);
        if (mData.contains(networkName)) {
            mData[networkName].isBusy = busy;
        }
    }

    // Check if a network is busy
    bool isBusy(const QString& networkName) const
    {
        QReadLocker locker(&mDataLock);
        return mData.contains(networkName) ? mData[networkName].isBusy : false;
    }

    // Get the simulator for a network
    Simulator* getSimulator(const QString& networkName) const
    {
        QReadLocker locker(&mDataLock);
        if (!mData.contains(networkName))
        {
            throw std::runtime_error(
                QString("Network not found in APIData: %1").arg(networkName)
                    .toStdString());
        }
        return mData[networkName].simulator;
    }

    // Get the network for a network name
    Network*
    getNetwork(const QString& networkName) const
    {
        QReadLocker locker(&mDataLock);
        if (!mData.contains(networkName)) {
            throw std::runtime_error(
                QString("Network not found in APIData: %1")
                    .arg(networkName).toStdString());
        }
        return mData[networkName].network;
    }

    // Add a train to a network
    void addTrain(const QString& networkName,
                 const std::shared_ptr<Train>& train)
    {
        QWriteLocker locker(&mDataLock);
        if (mData.contains(networkName))
        {
            mData[networkName].trains.insert(
                QString::fromStdString(train->trainUserID), train);
        }
    }

    // Get all trains for a network
    QVector<std::shared_ptr<Train>>
    getAllTrains(const QString& networkName) const
    {
        QReadLocker locker(&mDataLock);
        if (!mData.contains(networkName))
        {
            throw std::runtime_error(
                QString("Network not found in APIData: %1")
                    .arg(networkName).toStdString());
        }
        return mData[networkName].trains.values().toVector();
    }

    // Get a train by ID
    std::shared_ptr<Train>
    getTrainByID(const QString& networkName, const QString& trainID) const
    {
        QReadLocker locker(&mDataLock);
        if (!mData.contains(networkName))
        {
            throw std::runtime_error(
                QString("Network not found in APIData: %1")
                    .arg(networkName).toStdString());
        }
        if (mData[networkName].trains.contains(trainID))
        {
            return mData[networkName].trains.value(trainID);
        }
        return nullptr;
    }

    // Clear all data
    void clear()
    {
        QWriteLocker locker(&mDataLock);
        mData.clear();
    }

private:
    QMap<QString, APIData> mData;  // The shared data
    mutable QReadWriteLock mDataLock;  // Protects mData
};

#endif // THREADSAFEAPIDATAMAP_H
