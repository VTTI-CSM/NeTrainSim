#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

template <typename T>
class Vector : public std::vector<T> {
public:
    using std::vector<T>::vector;

    // Argmin function returns the index of the smallest element in the vector
    [[nodiscard]] auto argmin() const {
        return std::distance(std::cbegin(*this),
            std::min_element(std::cbegin(*this), std::cend(*this)));
    }

    // Argmax function returns the index of the largest element in the vector
    [[nodiscard]] auto argmax() const {
        return std::distance(std::cbegin(*this),
            std::max_element(std::cbegin(*this), std::cend(*this)));
    }

    // Sort function sorts the elements in the vector in ascending order
    void sort() {
        std::sort(std::begin(*this), std::end(*this));
    }

    // min function returns the smalled element in the vector
    [[nodiscard]] T min() const
    {
        return *std::min_element(std::cbegin(*this), std::cend(*this));
    }

    //max function returns the largest element in the vector
    [[nodiscard]] T max() const
    {
        return *std::max_element(std::cbegin(*this), std::cend(*this));
    }

    // Sum function returns the sum of all elements in the vector
    [[nodiscard]] T sum() const
    {
        return std::accumulate(std::cbegin(*this), std::cend(*this), T{});
    }

    // get the index of an element in the vector
    [[nodiscard]] __int64 index(const T& element) const
    {
        auto it = std::find(std::cbegin(*this), std::cend(*this), element);
        return it != std::cend(*this) ? std::distance(std::cbegin(*this), it) : -1;
    }
    // check if the element exists in the vector
    [[nodiscard]] bool exist(const T& element) const
    {
        return std::find(std::cbegin(*this), std::cend(*this), element) != std::cend(*this);
    }

    // insert another vector to the end of this vector
    void insertToEnd(const std::vector<T>& other_vector) {
        if (other_vector.empty()) {
            return;
        }
        this->reserve(this->size() + other_vector.size());
        this->insert(this->end(), other_vector.begin(), other_vector.end());
    }
    bool removeValue(const T& value) {
        auto iter = std::find(this->begin(), this->end(), value);
        if (iter != this->end()) {
            this->erase(iter);
            return true;
        }
        return false;
    }
};

#endif  // VECTOR_H
