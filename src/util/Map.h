#ifndef Map_H
#define Map_H

#include <map>
#include "Vector.h"
#include <sstream>
#include <string>
#include <type_traits>
#include "Utils.h"

template <typename Key, typename Value>
class Map : public std::map<Key, Value> {
public:
    using map_type = std::map<Key, Value>;
    using key_type = typename map_type::key_type;
    using mapped_type = typename map_type::mapped_type;
    using value_type = typename map_type::value_type;

    Vector<key_type> get_keys() const {
        Vector<key_type> keys;
        keys.reserve(map_type::size());
        for (const auto& pair : *this) {
            keys.push_back(pair.first);
        }
        return keys;
    }

    Vector<mapped_type> get_values() const {
        Vector<mapped_type> values;
        values.reserve(map_type::size());
        for (const auto& pair : *this) {
            values.push_back(pair.second);
        }
        return values;
    }

    bool is_key(const key_type& key) const {
        return map_type::count(key) > 0;
    }

    bool is_value(const mapped_type& value) const {
        for (const auto& pair : *this) {
            if (pair.second == value) {
                return true;
            }
        }
        return false;
    }

    void print() const {
        for (const auto& pair : *this) {
            std::cout << pair.first << ": " << pair.second << '\n';
        }
    }

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
            catch (std::exception& e) {
                ss << pair.second;
            }
            first = false;
        }
        ss << "}";
        return ss.str();
    }

};
#endif  // !Map_H