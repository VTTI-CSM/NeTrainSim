#pragma once

#include <QtCore/qglobal.h>

#if defined(NETRAINSIMCORE_LIBRARY)
    #define NETRAINSIMCORE_EXPORT Q_DECL_EXPORT
#else
    #define NETRAINSIMCORE_EXPORT Q_DECL_IMPORT
#endif

