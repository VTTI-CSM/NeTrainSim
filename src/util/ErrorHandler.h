#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <string>

namespace ErrorHandler {

/**
     * @brief Displays a notification message.
     *
     * @param msg The notification message to be displayed.
     */
void showNotification(std::string msg);

/**
     * @brief Displays a warning message.
     *
     * @param msg The warning message to be displayed.
     */
void showWarning(std::string msg);

/**
     * @brief Displays an error message.
     *
     * @param msg The error message to be displayed.
     */
void showError(std::string msg);

}

#endif // ERRORHANDLER_H
