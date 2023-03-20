/**
 * @file	NeTrainSim\src\util\Utils.h.
 *
 * Declares the utilities class
 */
#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <iomanip> // Required for setprecision and fixed
#include <locale> // Required for using thousands_separator
#include <sstream> // Required for stringstream
#include "Vector.h"
#include <fstream>
#include <string>
#include <regex>
#include <chrono>
#include <cmath>

namespace Utils {

    using namespace std;

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
    inline Vector<std::string> split(const std::string& s, char delimiter)
    {
        Vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter))
        {
            tokens.push_back(token);
        }
        return tokens;
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
}


#endif  // !UTILS_H
