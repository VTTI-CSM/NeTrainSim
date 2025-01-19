/**
 * @file	~\NeTrainSim\src\util\Utils.h.
 *
 * Declares the utilities class
 */
#ifndef UTILS_H
#define UTILS_H

#include <any>
#include <iostream>
#include <iomanip> // Required for setprecision and fixed
#include <locale> // Required for using thousands_separator
#include <qvariant.h>
#include <sstream> // Required for stringstream
#include "util/map.h"
#include "vector.h"
#include <fstream>
#include <string>
#include <regex>
#include <chrono>
#include <cmath>
#include <tuple>
#include <type_traits>
#include <QVector>
#include <type_traits>
#include <QMap>
#include <QString>
#include <map>
#include <string>

namespace Utils {

    using namespace std;


    /**
     * Compute the dot product of a 2x2 matrix and a 2D vector.
     * @param matrix A 2D vector representing a 2x2 matrix.
     * @param v A pair of doubles representing a 2D vector.
     * @return A pair of doubles representing the resulting vector after the dot product operation.
     *
     * @author Ahmed Aredah
     * @date 2/14/2023
     */
    inline std::pair<double, double> dot(Vector<Vector<double>> matrix, std::pair<double, double>& v) {
        // The dot product of a matrix and a vector is calculated as follows:
        // The first element is the sum of the products of the corresponding elements of the first row of the matrix and the vector
        // The second element is the sum of the products of the corresponding elements of the second row of the matrix and the vector
        return std::make_pair(matrix.at(0).at(0) * v.first + matrix.at(0).at(1) * v.second,
                              matrix.at(1).at(0) * v.first + matrix.at(1).at(1) * v.second);
    }

    /**
     * Compute the dot product of two 2D vectors.
     * @param u A pair of doubles representing the first vector.
     * @param v A pair of doubles representing the second vector.
     * @return A double representing the dot product of the two vectors.
     *
     * @author Ahmed Aredah
     * @date 2/14/2023
     */
    inline double dot(std::pair<double, double> &u, std::pair<double, double> &v) {
        // The dot product of two vectors is calculated as (u1*v1 + u2*v2)
        return u.first * v.first + u.second * v.second;
    }

    /**
     * Compute the cross product of two 2D vectors.
     * @param u A pair of doubles representing the first vector.
     * @param v A pair of doubles representing the second vector.
     * @return A double representing the pseudo cross product of the two vectors.
     *
     * @author Ahmed Aredah
     * @date 2/14/2023
     */
    inline double cross(std::pair<double, double> &u, std::pair<double, double> &v) {
        // The cross product of two vectors in 2D (also known as the determinant, or pseudo cross product)
        // is calculated as (u1*v2 - u2*v1)
        return u.first * v.second - u.second * v.first;
    }

    /**
     * Computes the Euclidean distance between two coordinates.
     * @param position1 A pair of doubles representing the first coordinate.
     * @param position2 A pair of doubles representing the second coordinate.
     * @return The Euclidean distance between position1 and position2.
     * @author Ahmed
     * @date 2/14/2023
     */
    inline double getDistanceByTwoCoordinates(const std::pair<double, double>& position1,
                                       const std::pair<double, double>& position2) {
        // Calculate differences in x and y coordinates
        double xDiff = position1.first - position2.first;
        double yDiff = position1.second - position2.second;

        // Compute and return Euclidean distance
        return std::sqrt(xDiff * xDiff + yDiff * yDiff);
    }

    /**
     * Powers
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	base		The base.
     * @param 	exponent	The exponent.
     *
     * @returns	A double.
     */
    inline double power(double base, int exponent) {
        if (exponent == 0) {
            return 1.0;
        }
        else if (exponent < 0) {
            return 1.0 / power(base, -exponent);
        }
        else if (exponent % 2 == 0) {
            double temp = power(base, exponent / 2);
            return temp * temp;
        }
        else {
            return base * power(base, exponent - 1);
        }
    }


    template<typename T>
        requires std::is_arithmetic<T>::value
    /**
     * Convert a plain numeric value to thousand separated value
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	n	A T to process.
     *
     * @returns	A std::string.
     *
     * @tparam	T	Generic type parameter.
     */
    inline std::string thousandSeparator(T n, int decimals = 3) {
        // Get the sign of the number and remove it
        int sign = (n < 0) ? -1 : 1;
        double approx = power((double)10.0, decimals);
        n *= sign;
        // Get the integer part of the number
        long long intPart = (long long)n;
        // Check if the fractional part has any value
        bool hasFracPart = (n - intPart > 0);
        // Get the fractional part of the number and trim it to n decimal places
        double fracPart = round((n - intPart) * (approx)) / (approx);
        // Create a locale object to represent a specific localization
        locale loc("");
        // Set the thousand separator according to the localization
        char separator = use_facet< numpunct<char> >(loc).thousands_sep();
        // Convert the integer part to a string
        string intStr = to_string(intPart);
        // Insert the separator every 3 digits from the end
        for (int i = intStr.length() - 3; i > 0; i -= 3) {
            intStr.insert(i, 1, separator);
        }
        // Convert the fractional part to a string and trim it to n decimal places
        stringstream stream;
        if (hasFracPart) {
            stream << fixed << setprecision(decimals) << fracPart;
        }
        string fracStr = (hasFracPart) ? stream.str().substr(1) : "";
        // Combine the integer and fractional parts into a single string
        string result = intStr + fracStr;
        // Add the sign to the beginning of the string if the number was negative
        if (sign == -1) {
            result = "-" + result;
        }
        return result;
    }


    template<typename T>

    /**
     * Format duration
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	seconds	The seconds.
     *
     * @returns	The formatted duration.
     *
     * @tparam	T	Generic type parameter.
     */
    inline std::string formatDuration(T seconds) {
        int minutes = static_cast<int>(seconds) / 60;
        int hours = minutes / 60;
        int days = hours / 24;
        int remainingSeconds = static_cast<int>(seconds) % 60;
        int remainingMinutes = minutes % 60;
        int remainingHours = hours % 24;
        // Use stringstream to convert integer values to strings
        std::stringstream ss;
        ss << days << ":" << setw(2) << setfill('0') << remainingHours << ":" << setw(2) << setfill('0') << remainingMinutes << ":" << setw(2) << setfill('0') << remainingSeconds;
        std::string result = ss.str();
        return result;
    }

    inline std::string getFilenameWithoutExtension(std::string path) {
        size_t pos = path.find_last_of("/\\");
        string filename = (pos == string::npos) ? path : path.substr(pos + 1);
        size_t dotPos = filename.find_last_of(".");
        if (dotPos != string::npos) {
            filename = filename.substr(0, dotPos);
        }
        return filename;
    }

    inline std::string getPrefix(std::string str) {
        std::string prefix;
        bool skipFirstChar = isupper(str[0]);
        if (skipFirstChar) {
            prefix += str[0];
        }
        for (size_t i = skipFirstChar ? 1 : 0; i < str.length(); i++) {
            char c = str[i];
            if (c == '_' || isalpha(c) || c == ' ') {
                if (isupper(c) || c == ' ' || c == '_') {
                    break;
                }
                prefix += c;
            }
        }
        return prefix;
    }


    /**
     * Splits string to double vector
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	input	 	The input.
     * @param 	delimiter	(Optional) The delimiter.
     *
     * @returns	A Vector&lt;double&gt;
     */
    inline Vector<double> splitStringToDoubleVector(const std::string& input, char delimiter = ',') {
        Vector<double> result;
        std::stringstream ss(input);
        std::string item;
        while (std::getline(ss, item, delimiter)) {
            result.push_back(std::stod(item));
        }
        return result;
    }

    /**
     * The function stringToIntVector takes a string as input and converts it into a Vector<int>
     * by parsing the string for comma-separated integers and pushing each one onto the vector.
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	str	The string.
     *
     * @returns	A Vector&lt;int&gt;
     */
    inline Vector<int> splitStringToIntVector(const std::string& str) {
        Vector<int> intVector;
        std::stringstream ss(str);
        int temp;

        while (ss >> temp) {
            intVector.push_back(temp);
            if (ss.peek() == ',') {
                ss.ignore();
            }
            else if (ss.peek() != ' ') {
                break;
            }
        }

        return intVector;
    }

    /**
     * This function is splitting a given string "s" into substrings using a delimiter "delimiter"
     * and returning the resulting substrings as a vector of strings.
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	s		 	A std::string to process.
     * @param 	delimiter	The delimiter.
     *
     * @returns	A Vector&lt;std::string&gt;
     */
    inline Vector<std::string> split(const std::string& s,
                                     char delimiter,
                                     bool ignoreEmpty = false)
    {
        Vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter))
        {
            if (token.empty() && ignoreEmpty) {
                continue;
            }
            tokens.push_back(token);
        }
        return tokens;
    }

    inline Vector<int> convertStringVectorToIntVector(const std::vector<std::string>& stringVector) {
        Vector<int> intVector;

        for(const auto& str : stringVector) {
            try {
                intVector.push_back(std::stoi(str));
            } catch(const std::exception& e) {
                throw std::runtime_error("Error: unable to convert string " + str + " to integer.");
            }
        }

        return intVector;
    }

    /**
     * This function trim takes in a string str and returns a copy of it with leading and trailing
     * white spaces removed.
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	str	The string.
     *
     * @returns	A std::string.
     */
    inline std::string trim(const std::string& str)
    {
        std::string s = str;
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
            return !std::isspace(ch);
            }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
            }).base(), s.end());
        return s;
    }

    inline bool writeToFile(std::stringstream &s, std::string filename) {
        std::ofstream outputFile(filename);
        if (outputFile.is_open()) {
            outputFile << s.rdbuf();  // Write stringstream contents to file
            outputFile.close();
            return true;
        } else {
            return false;
        }
    }

    template <typename... Args>
    inline std::stringstream convertTupleToStringStream(const std::tuple<Args...>& t, int limit, std::string delim = "\t");

    template <typename Tuple, size_t... Indices>
    inline void addTupleValuesToStreamImpl(const Tuple& t, std::stringstream& ss,
                                           std::index_sequence<Indices...>, int limit, std::string delim);

    template <typename T>
    inline void writeToStream(const T& value, std::stringstream& ss, std::string delim);



    template <typename... Args>
    inline std::stringstream convertTupleToStringStream(const std::tuple<Args...>& t, int limit, std::string delim) {
        std::stringstream ss;
        addTupleValuesToStreamImpl(t, ss, std::index_sequence_for<Args...>(),
                                   limit > 0 ? limit: std::tuple_size<std::tuple<Args...>>::value - std::abs(limit) -1, delim);
        return ss;
    }

    template <typename Tuple, size_t... Indices>
    inline void addTupleValuesToStreamImpl(const Tuple& t,
                                           std::stringstream& ss,
                                           std::index_sequence<Indices...>,
                                           int limit, std::string delim) {
        ((Indices < limit
              ? Utils::writeToStream(std::get<Indices>(t), ss, delim) :
              void()), ...);
        ((Indices == limit
              ? Utils::writeToStream(std::get<Indices>(t), ss, "") :
              void()), ...);
    }

    template <typename T>
    inline void writeToStream(const T& value,
                              std::stringstream& ss,
                              std::string delim) {
        if constexpr (std::is_same_v<T, bool>) {
            ss << (value ? "1" : "0") << delim;
        } else {
            ss << value << delim;
        }
    }

    inline std::string removeLastWord(const std::string& originalString) {

        size_t lastSpaceIndex = originalString.find_last_of(' ');

        if (lastSpaceIndex != std::string::npos) {
            std::string updatedString = originalString.substr(0, lastSpaceIndex);
            return updatedString;
        } else {
            // Return the original string if there is no space
            return originalString;
        }
    }

    inline QVector<double> convertQStringVectorToDouble(const QVector<QString>& stringVector) {
        QVector<double> doubleVector;
        doubleVector.reserve(stringVector.size());

        for (const QString& str : stringVector) {
            bool ok;
            double value = str.toDouble(&ok);
            if (ok) {
                doubleVector.append(value);
            } else {
                // Handle conversion error if necessary
            }
        }

        return doubleVector;
    }

    inline QVector<double> subtractQVector(const QVector<double> l1, const QVector<double> l2) {
        QVector<double> result;
        if (l1.size() != l2.size()) {return result; }

        for (int i = 0; i < l1.size(); i++ ) {
            result.push_back(l1[i] - l2[i]);
        }
        return result;
    }

    inline QVector<double> factorQVector(const QVector<double> l1, const double factor) {
        QVector<double> result;
        if (l1.size() < 1) {return result; }

        for (int i = 0; i < l1.size(); i++ ) {
            result.push_back(l1[i] * factor);
        }
        return result;
    }

    inline QVector<QPair<QString, QString>> splitStringStream(std::stringstream& ss, const std::string& delimiter = ":") {
        QVector<QPair<QString, QString>> result;
        std::string line;

        while (std::getline(ss, line)) {
            std::size_t delimiterPos = line.find(delimiter);
            if (delimiterPos != std::string::npos) {
                QString first =
                    QString::fromStdString(line.substr(0, delimiterPos));
                QString second =
                    QString::fromStdString(line.substr(delimiterPos + delimiter.size()));
                result.push_back(QPair<QString, QString>(first, second));
            } else {
                QString l = QString::fromStdString(line);
                result.push_back(QPair<QString, QString>(l, ""));  // If delimiter is not found, add the entire line with an empty second part
            }
        }

        return result;
    }

    inline std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
        }
        return str;
    }

    template <typename T>
    inline bool isValueInRange(T valueToCheck, T lowerLimit, T upperLimit) {
        if (valueToCheck >= lowerLimit && valueToCheck <= upperLimit) {
            return true;
        } else {
            return false;
        }
    }

    // Function to calculate the Levenshtein distance between two strings
    inline int levenshteinDistance(const std::string& str1,
                                   const std::string& str2) {
        const int len1 = str1.length();
        const int len2 = str2.length();

        std::vector<std::vector<int>> dp(len1 + 1,
                                         std::vector<int>(len2 + 1, 0));

        for (int i = 0; i <= len1; ++i)
            dp[i][0] = i;

        for (int j = 0; j <= len2; ++j)
            dp[0][j] = j;

        for (int i = 1; i <= len1; ++i) {
            for (int j = 1; j <= len2; ++j) {
                int cost = (str1[i - 1] == str2[j - 1]) ? 0 : 1;
                dp[i][j] = std::min({ dp[i - 1][j] + 1,
                                     dp[i][j - 1] + 1,
                                     dp[i - 1][j - 1] + cost });
            }
        }

        return dp[len1][len2];
    }

    // Function to find the closest match in the vector of QStrings
    inline std::string findClosestMatch(const std::string& input,
                                        const std::vector<std::string>& values)
    {
        int minDistance = std::numeric_limits<int>::max();
        std::string closestMatch;

        for (const std::string& value : values) {
            int distance = levenshteinDistance(value, input);
            if (distance < minDistance) {
                minDistance = distance;
                closestMatch = value;
            }
        }

        return closestMatch;
    }

    // Function to find the closest match in the vector of QStrings
    inline QString findClosestMatch(const QString& input,
                                    const std::vector<QString>& values) {
        int minDistance = std::numeric_limits<int>::max();
        QString closestMatch;

        for (const QString& value : values) {
            int distance = levenshteinDistance(value.toStdString(),
                                               input.toStdString());
            if (distance < minDistance) {
                minDistance = distance;
                closestMatch = value;
            }
        }

        return closestMatch;
    }

    inline Map<std::string, std::string> convertToStdMap(const QMap<QString, QString>& qmap) {
        Map<std::string, std::string> stdmap;
        for (auto it = qmap.begin(); it != qmap.end(); ++it) {
            stdmap[std::string(it.key().toStdString())] = std::string(it.value().toStdString());
        }
        return stdmap;
    }

    inline Map<std::string, std::any> convertToStdMap(const QMap<QString, std::any>& qmap) {
        Map<std::string, std::any> stdmap;
        for (auto it = qmap.begin(); it != qmap.end(); ++it) {
            stdmap[std::string(it.key().toStdString())] = it.value();
        }
        return stdmap;
    }

    inline QMap<QString, QString> convertToQMap(const Map<std::string, std::string>& stdmap) {
        QMap<QString, QString> qmap;
        for (const auto& entry : stdmap) {
            qmap[QString::fromStdString(entry.first)] = QString::fromStdString(entry.second);
        }
        return qmap;
    }

    inline QMap<QString, std::any> convertToQMap(const Map<std::string, std::any>& stdmap) {
        QMap<QString, std::any> qmap;
        for (const auto& entry : stdmap) {
            qmap[QString::fromStdString(entry.first)] = entry.second;
        }
        return qmap;
    }

    // Generic conversion from QMap<Key, Value> to std::map<Key, Value>
    template <typename Key, typename Value>
    inline std::map<Key, Value> convertToStdMap(const QMap<Key, Value>& qmap) {
        std::map<Key, Value> stdmap;
        for (auto it = qmap.begin(); it != qmap.end(); ++it) {
            stdmap[it.key()] = it.value();
        }
        return stdmap;
    }

    // Generic conversion from std::map<Key, Value> to QMap<Key, Value>
    template <typename Key, typename Value>
    inline QMap<Key, Value> convertToQMap(const std::map<Key, Value>& stdmap) {
        QMap<Key, Value> qmap;
        for (const auto& entry : stdmap) {
            qmap.insert(entry.first, entry.second);
        }
        return qmap;
    }

    template <typename T>
    inline QVector<T> convertToQVector(const Vector<T>& vec) {
        QVector<T> qvec;
        qvec.reserve(static_cast<int>(vec.size()));

        for (const auto& entry : vec) {
            qvec.append(entry);
        }
        return qvec;
    }

    template <typename T>
    inline Vector<T> convertToStdVector(const QVector<T>& qvec) {
        Vector<T> vec;
        vec.reserve(static_cast<size_t>(qvec.size()));

        for (const auto& entry : qvec) {
            vec.push_back(entry);
        }
        return vec;
    }

    inline QVector<QMap<QString, QString>> convertToQVector(const Vector<Map<std::string, std::string>>& vec) {
        QVector<QMap<QString, QString>> qvec;
        qvec.reserve(static_cast<int>(vec.size()));

        for (const auto& entry : vec) {
            qvec.append(convertToQMap(entry));
        }
        return qvec;
    }

    inline QVector<QMap<QString, std::any>> convertToQVector(const Vector<Map<std::string, std::any>>& vec) {
        QVector<QMap<QString, std::any>> qvec;
        qvec.reserve(static_cast<int>(vec.size()));

        for (const auto& entry : vec) {
            qvec.append(convertToQMap(entry));
        }
        return qvec;
    }

    inline Vector<Map<std::string, std::string>> convertToStdVector(const QVector<QMap<QString, QString>>& qvec) {
        Vector<Map<std::string, std::string>> vec;
        vec.reserve(static_cast<size_t>(qvec.size()));

        for (const auto& entry : qvec) {
            vec.push_back(convertToStdMap(entry));
        }
        return vec;
    }

    inline Vector<Map<std::string, std::any>> convertToStdVector(const QVector<QMap<QString, std::any>>& qvec) {
        Vector<Map<std::string, std::any>> vec;
        vec.reserve(static_cast<size_t>(qvec.size()));

        for (const auto& entry : qvec) {
            vec.push_back(convertToStdMap(entry));
        }
        return vec;
    }

    template <typename Key, typename Value>
    inline QVector<QMap<Key, Value>> convertToQVector(const Vector<Map<Key, Value>>& vec) {
        QVector<QMap<Key, Value>> qvec;
        qvec.reserve(static_cast<int>(vec.size()));

        for (const auto& entry : vec) {
            QMap<Key, Value> qmap;
            for (const auto& pair : entry) {
                qmap.insert(pair.first, pair.second);
            }
            qvec.append(qmap);
        }
        return qvec;
    }

    inline QVector<QMap<QString, QString>> convertToQVectorString(const Vector<Map<std::string, std::string>>& vec) {
        QVector<QMap<QString, QString>> qvec;
        qvec.reserve(static_cast<int>(vec.size()));

        for (const auto& entry : vec) {
            QMap<QString, QString> qmap;
            for (const auto& pair : entry) {
                qmap.insert(QString::fromStdString(pair.first),
                            QString::fromStdString(pair.second));
            }
            qvec.append(qmap);
        }
        return qvec;
    }

    template <typename Key, typename Value>
    inline Vector<std::map<Key, Value>> convertToStdVector(const QVector<QMap<Key, Value>>& qvec) {
        Vector<std::map<Key, Value>> vec;
        vec.reserve(static_cast<size_t>(qvec.size()));

        for (const auto& entry : qvec) {
            std::map<Key, Value> stdmap;
            for (auto it = entry.begin(); it != entry.end(); ++it) {
                stdmap[it.key()] = it.value();
            }
            vec.push_back(stdmap);
        }
        return vec;
    }

    inline std::any convertQVariantToAny(const QVariant& variant) {
        switch (variant.typeId()) {
        case QMetaType::Int:
            return std::any(variant.toInt());
        case QMetaType::Double:
            return std::any(variant.toDouble());
        case QMetaType::Bool:
            return std::any(variant.toBool());
        case QMetaType::QString:
            return std::any(variant.toString().toStdString());
        case QMetaType::QVariantList: {
            QVariantList qList = variant.toList();
            std::vector<std::any> anyList;
            for (const QVariant& qVar : qList) {
                anyList.push_back(convertQVariantToAny(qVar));
            }
            return std::any(anyList);
        }
        case QMetaType::QVariantMap: {
            QVariantMap qMap = variant.toMap();
            std::map<std::string, std::any> anyMap;
            for (auto it = qMap.begin(); it != qMap.end(); ++it) {
                anyMap[it.key().toStdString()] = convertQVariantToAny(it.value());
            }
            return std::any(anyMap);
        }
        default:
            return std::any();
        }
    }

    inline std::map<std::string, std::any>
    convertQMapToStdMap(const QMap<QString, QVariant>& qMap) {
        std::map<std::string, std::any> stdMap;

        for (auto it = qMap.begin(); it != qMap.end(); ++it) {
            std::string key = it.key().toStdString();
            std::any value = convertQVariantToAny(it.value());
            stdMap[key] = value;
        }

        return stdMap;
    }

    inline QVector<QPair<QString, QVector<QPair<double, double>>>>
    convertToQtTrainsCoords(const Vector<std::pair<std::string, Vector<std::pair<double, double>>>>& simulator) {
        QVector<QPair<QString, QVector<QPair<double, double>>>> qtSimulator;
        qtSimulator.reserve(simulator.size()); // Reserve space to avoid multiple reallocations

        for (const auto& outerPair : simulator) {
            // Convert inner std::vector<std::pair<double, double>> to QVector<QPair<double, double>>
            QVector<QPair<double, double>> innerQVector;
            innerQVector.reserve(outerPair.second.size());
            for (const auto& innerPair : outerPair.second) {
                innerQVector.append(QPair<double, double>(innerPair.first, innerPair.second));
            }

            // Convert std::string to QString and add to QVector
            qtSimulator.append(QPair<QString, QVector<QPair<double, double>>>(
                QString::fromStdString(outerPair.first), innerQVector));
        }

        return qtSimulator;
    }

    inline Vector<std::pair<std::string, Vector<std::pair<double, double>>>>
    convertFromQtTrainsCoords(const QVector<QPair<QString, QVector<QPair<double, double>>>>& qtSimulator) {
        Vector<std::pair<std::string, Vector<std::pair<double, double>>>> simulator;
        simulator.reserve(qtSimulator.size()); // Reserve space to avoid multiple reallocations

        for (const auto& outerPair : qtSimulator) {
            // Convert inner QVector<QPair<double, double>> to Vector<std::pair<double, double>>
            Vector<std::pair<double, double>> innerVector;
            innerVector.reserve(outerPair.second.size());
            for (const auto& innerPair : outerPair.second) {
                innerVector.push_back(std::make_pair(innerPair.first, innerPair.second));
            }

            // Convert QString to std::string and add to Vector
            simulator.push_back(std::make_pair(outerPair.first.toStdString(), innerVector));
        }

        return simulator;
    }


}


#endif  // !UTILS_H
