#ifndef LIST_H
#define LIST_H

#include <iostream>
#include <list>
#include <algorithm>

template <typename T>
class List : public std::list<T> {
public:
    using std::list<T>::list;

    typename std::list<T>::iterator argmin() {
        return std::min_element(this->begin(), this->end());
    }

    typename std::list<T>::iterator argmax() {
        return std::max_element(this->begin(), this->end());
    }
};

#endif  // LIST_H
