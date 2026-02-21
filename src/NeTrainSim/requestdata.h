#ifndef REQUESTDATA_H
#define REQUESTDATA_H

#include <QMap>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>

template<typename T>
struct RequestData {
    /**
     * @brief Thread-safe method to add data to the buffer for a
     *        specific network.
     * @param network The network identifier.
     * @param data The data to store.
     */
    void addUpdateData(const QString& network, const T& data) {
        QMutexLocker locker(&mutex);
        dataBuffer[network] = data;
    }

    /**
     * @brief Thread-safe method to retrieve data for a specific network.
     * @param network The network identifier.
     * @return The data associated with the network.
     */
    T getData(const QString& network) const {
        QMutexLocker locker(&mutex);
        return dataBuffer.value(network);   // Returns default-constructed T
            // if not found
    }

    /**
     * @brief Thread-safe method to get the list of keys (network identifiers)
     *        in the dataBuffer.
     * @return A QList<QString> containing all keys in the dataBuffer.
     */
    QList<QString> getDataBufferKeys() const {
        QMutexLocker locker(&mutex);
        return dataBuffer.keys();
    }

    /**
     * @brief Thread-safe method to get a copy of the dataBuffer.
     * @return A copy of the dataBuffer.
     */
    QMap<QString, T> getDataBuffer() const {
        QMutexLocker locker(&mutex);
        return dataBuffer; // Returns a copy of the dataBuffer
    }

    /**
     * @brief Thread-safe method to clear the dataBuffer.
     */
    void clearDataBuffer() {
        QMutexLocker locker(&mutex);
        dataBuffer.clear();
    }

    bool isDataBufferEmpty() {
        QMutexLocker locker(&mutex);
        return dataBuffer.isEmpty();
    }

    int incrementAndGetCompleted() {
        QMutexLocker locker(&mutex);
        return ++completedRequests;
    }

    /**
     * @brief Thread-safe method to increment the completed requests counter.
     */
    void incrementCompletedRequests() {
        QMutexLocker locker(&mutex);
        completedRequests++;
    }

    /**
     * @brief Thread-safe method to get the current count of completed requests.
     * @return The number of completed requests.
     */
    int getCompletedRequests() const {
        QMutexLocker locker(&mutex);
        return completedRequests;
    }

    /**
     * @brief Thread-safe method to reset the completed requests counter.
     */
    void resetCompletedRequests() {
        QMutexLocker locker(&mutex);
        completedRequests = 0;
    }

    /**
     * @brief Thread-safe method to set the list of requested networks.
     * @param networks The list of requested networks.
     */
    void setRequestedNetworks(const QVector<QString>& networks) {
        QMutexLocker locker(&mutex);
        requestedNetworkProcess = networks;
    }

    /**
     * @brief Thread-safe method to add a network to the requested network list.
     * @param network The network identifier.
     */
    void addRequestedNetwork(const QString& network) {
        QMutexLocker locker(&mutex);
        requestedNetworkProcess.append(network);
    }

    /**
     * @brief Thread-safe method to get the list of requested networks.
     * @return The list of requested networks.
     */
    QVector<QString> getRequestedNetworks() const {
        QMutexLocker locker(&mutex);
        return requestedNetworkProcess;
    }

    /**
     * @brief Thread-safe method to clear the list of requested networks.
     */
    void clearRequestedNetworks() {
        QMutexLocker locker(&mutex);
        requestedNetworkProcess.clear();
    }

    int requestedNetworkCount() const {
        QMutexLocker locker(&mutex);
        return requestedNetworkProcess.size();
    }

    void clearAll() {
        QMutexLocker locker(&mutex);
        dataBuffer.clear();
        completedRequests = 0;
        requestedNetworkProcess.clear();
    }

    int getRequestedCount() const {
        QMutexLocker locker(&mutex);
        return requestedNetworkProcess.size();
    }

    void removeData(const QString& network) {
        QMutexLocker locker(&mutex);
        dataBuffer.remove(network);
        requestedNetworkProcess.removeOne(network);
    }

private:
    QMap<QString, T> dataBuffer;  ///< Stores operation results for each network
    int completedRequests = 0;    ///< Counter for completed network operations
    QVector<QString> requestedNetworkProcess; ///< List of networks involved
    mutable QMutex mutex; ///< Mutex to protect access to the data members
};

#endif // REQUESTDATA_H
