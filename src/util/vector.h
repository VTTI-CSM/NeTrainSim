/**
 * @file	~\NeTrainSim\src\util\Vector.h.
 *
 * Declares the vector class
 */
#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <sstream>

/**
 * A vector.
 *
 * @author	Ahmed Aredah
 * @date	2/28/2023
 *
 * @tparam	T	Generic type parameter.
 */
template <typename T>
class Vector : public std::vector<T> {
public:
    /** . */
    using std::vector<T>::vector;

    /**
     * Argmin function returns the index of the smallest element in the vector
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	An auto.
     */
    [[nodiscard]] auto argmin() const {
        return std::distance(std::cbegin(*this),
            std::min_element(std::cbegin(*this), std::cend(*this)));
    }

    /**
     * Argmax function returns the index of the largest element in the vector
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	An auto.
     */
    [[nodiscard]] auto argmax() const {
        return std::distance(std::cbegin(*this),
            std::max_element(std::cbegin(*this), std::cend(*this)));
    }

    /**
     * Sort function sorts the elements in the vector in ascending order
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     */
    void sort() {
        std::sort(std::begin(*this), std::end(*this));
    }

    /**
     * min function returns the smalled element in the vector
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	The minimum value.
     */
    [[nodiscard]] T min() const
    {
        return *std::min_element(std::cbegin(*this), std::cend(*this));
    }

    /**
     * max function returns the largest element in the vector
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	The maximum value.
     */
    [[nodiscard]] T max() const
    {
        return *std::max_element(std::cbegin(*this), std::cend(*this));
    }

    /**
     * Sum function returns the sum of all elements in the vector
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	A T.
     */
    [[nodiscard]] T sum() const
    {
        return std::accumulate(std::cbegin(*this), std::cend(*this), T{});
    }

    /**
     * get the index of an element in the vector
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	element	The element.
     */
    [[nodiscard]] int index(const T& element) const
    {
        auto it = std::find(std::cbegin(*this), std::cend(*this), element);
        return it != std::cend(*this) ? std::distance(std::cbegin(*this), it) : -1;
    }

    /**
     * check if the element exists in the vector
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	element	The element.
     *
     * @returns	True if it succeeds, false if it fails.
     */
    [[nodiscard]] bool exist(const T& element) const
    {
        return std::find(std::cbegin(*this), std::cend(*this), element) != std::cend(*this);
    }

    /**
     * @brief check if the other vector contains any value in the current vector
     * @param other
     * @return
     */
    [[nodiscard]] bool hasCommonElement(const Vector<T>& other) const {
        for (const auto& elem : other) {
            if (std::find(this->begin(), this->end(), elem) != this->end()) {
                return true;
            }
        }
        return false;
    }


    /**
     * @brief check if the other vector is a subset of the current vector
     * @param other
     * @return
     */
    bool isSubsetOf(const Vector<T>& other) const {
        for (const auto& elem : *this) {
            if (std::find(other.begin(), other.end(), elem) == other.end()) {
                return false;
            }
        }
        return true;
    }

    /**
     * insert another vector to the end of this vector
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	other_vector	The other vector.
     */
    void insertToEnd(const std::vector<T>& other_vector) {
        if (other_vector.empty()) {
            return;
        }
        this->reserve(this->size() + other_vector.size());
        this->insert(this->end(), other_vector.begin(), other_vector.end());
    }

    /**
     * Removes the value described by value
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @param 	value	The value.
     *
     * @returns	True if it succeeds, false if it fails.
     */
    bool removeValue(const T& value) {
        auto iter = std::find(this->begin(), this->end(), value);
        if (iter != this->end()) {
            this->erase(iter);
            return true;
        }
        return false;
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
        ss << "[ ";
        for (const auto& elem : *this) {
            if (!first) {
                ss << ", ";
            }
            ss << elem;
            first = false;
        }
        ss << "]";
        return ss.str();
    }

    std::string toNotFormattedString() const {
        std::stringstream ss = std::stringstream("");
        bool first = true;
        for (const auto& elem : *this) {
            if (!first) {
                ss << ",";
            }
            ss << elem;
            first = false;
        }
        return ss.str();
    }
};

#endif  // VECTOR_H
