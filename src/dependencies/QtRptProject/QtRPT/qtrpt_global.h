#ifndef QTRPT_GLOBAL_H
#define QTRPT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QTRPT_LIBRARY)
#  define QTRPTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QTRPTSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // QTRPT_GLOBAL_H
