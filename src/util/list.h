/**
 * @file	~\NeTrainSim\src\util\List.h.
 *
 * Declares the list class
 */
#ifndef LIST_H
#define LIST_H

#include <iostream>
#include <list>
#include <algorithm>

/**
 * A list.
 *
 * @author	Ahmed Aredah
 * @date	2/28/2023
 *
 * @tparam	T	Generic type parameter.
 */
template <typename T>
class List : public std::list<T> {
public:
    /** . */
    using std::list<T>::list;

    /**
     * Gets the argmin
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	A list of.
     */
    typename std::list<T>::iterator argmin() {
        return std::min_element(this->begin(), this->end());
    }

    /**
     * Gets the argmax
     *
     * @author	Ahmed Aredah
     * @date	2/28/2023
     *
     * @returns	A list of.
     */
    typename std::list<T>::iterator argmax() {
        return std::max_element(this->begin(), this->end());
    }
};

#endif  // LIST_H
