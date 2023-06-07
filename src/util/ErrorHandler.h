#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H
#include <string>

namespace ErrorHandler {

void showNotification(std::string msg);
void showWarning(std::string msg);
void showError(std::string msg);

#ifdef AS_CMD

#endif

#ifndef AS_CMD

#endif
}

#endif // ERRORHANDLER_H
