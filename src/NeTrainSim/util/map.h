/**
 * @file	~\NeTrainSim\src\util\Map.h.
 *
 * Declares the map class
 */
#ifndef Map_H
#define Map_H

#include <map>
#include "vector.h"
#include <sstream>
#include <string>
#include <type_traits>
#include "utils.h"

/**
 * A map.
 *
 * @author	Ahmed Aredah
 * @date	2/28/2023
 *
 * @tparam	Key  	Type of the key.
 * @tparam	Value	Type of the value.
 */
template <typename Key, typename Value>
class Map : public std::map<Key, Value> {
public:
    /** Type of the map */
    using map_type = std::map<Key, Value>;
    /** Type of the key */
    using key_type = typename map_type::key_type;
    /** Type of the mapped */
    using mapped_type = typename map_type::mapped_type;
    /** Type of the value */
    using value_type = typename map_type::value_type;

    using Base = std::map<Key, Value>;

    // Add a default constructor
    Map() : Base() {}

    /**
     * Constructor that takes an initializer list of key-value pairs.
     *
     * @param  initList  The initializer list of key-value pairs.
     */
    Map(std::initializer_list<std::pair<Key, Value>> initList)
        : Base(initList.begin(), initList.end()) {}

    /**
     * Gets the keys
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	The keys.
     */
    Vector<key_type> get_keys() const {
        Vector<key_type> keys;
        keys.reserve(map_type::size());
        for (const auto& pair : *this) {
            keys.push_back(pair.first);
        }
        return keys;
    }

    /**
     * Gets the values
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	The values.
     */
    Vector<mapped_type> get_values() const {
        Vector<mapped_type> values;
        values.reserve(map_type::size());
        for (const auto& pair : *this) {
            values.push_back(pair.second);
        }
        return values;
    }

    /**
     * Query if 'key' is key
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	key	The key.
     *
     * @returns	True if key, false if not.
     */
    bool is_key(const key_type& key) const {
        return map_type::count(key) > 0;
    }

    /**
     * Query if 'value' is value
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	value	The value.
     *
     * @returns	True if value, false if not.
     */
    bool is_value(const mapped_type& value) const {
        for (const auto& pair : *this) {
            if (pair.second == value) {
                return true;
            }
        }
        return false;
    }

    /**
     * Calculates the sum of all values in the map.
     *
     * @return The sum of all values.
     */
    mapped_type sumValues() const {
        mapped_type total = 0;
        for (const auto& pair : *this) {
            total += pair.second;
        }
        return total;
    }

    /**
     *  Calculates the average of all keys in the map.
     *
     *  @return The average of all keys.
     */
    key_type averageKeys() const {
        key_type total = 0;
        for (const auto& pair : *this) {
            total += pair.first;
        }
        return total / this->size();
    }


    /**
     * Prints this object
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     */
    void print() const {
        for (const auto& pair : *this) {
            std::cout << pair.first << ": " << pair.second << '\n';
        }
    }

    /**
     * Convert this object into a string representation
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	A std::string that represents this object.
     */
    std::string toString() const {
        std::stringstream ss = std::stringstream("");
        bool first = true;
        ss << "{ ";
        for (const auto& pair : *this) {
            if (!first) {
                ss << ", ";
            }
            ss << pair.first << ": ";
            try {
                static_cast<long double>(pair.second);
                ss << Utils::thousandSeparator(pair.second);
            }
            catch (const std::exception& e) {
                ss << pair.second;
            }
            first = false;
        }
        ss << "}";
        return ss.str();
    }

    std::string valuesToString(Vector<key_type>keyOrder = Vector<key_type>(),
                               Vector<key_type>ignore = Vector<key_type>(),
                               std::string delim = ",") {
        std::stringstream ss = std::stringstream("");
        bool isFirst = true;
        if (keyOrder.empty()) {
            for (const auto& pair : *this) {
                if (ignore.exist(pair.first)) {
                    continue;
                }
                if (!isFirst) {
                    ss << delim;
                }
                ss << pair.second;
                isFirst = false;
            }
        }
        else
        {
            for (auto& key: keyOrder) {
                if (this->is_key(key)) {
                    if (ignore.exist(key)) {
                        continue;
                    }
                    if (!isFirst) {
                        ss << delim;
                    }
                    ss << this->at(key);
                    isFirst = false;
                }
                else {
                    throw std::runtime_error("Key: " + key +
                                             " is not found!");
                }
            }
        }
        return ss.str();
    }

};
#endif  // !Map_H
