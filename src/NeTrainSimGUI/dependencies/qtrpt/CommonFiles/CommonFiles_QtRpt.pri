INCLUDEPATH += $$PWD

include($$PWD/../QtRPT/config.pri)

#greaterThan(QT_MAJOR_VERSION,5): QT += core5compat

!contains(DEFINES,NO_BARCODE) {
    INCLUDEPATH += $$PWD/../3rdparty/zint-2.4.4/backend_qt4
    INCLUDEPATH += $$PWD/../3rdparty/zint-2.4.4/backend

    LIBS += -L$${DEST_DIRECTORY}/lib -lQtZint
}

contains(DEFINES,QXLSX_LIBRARY) {
    INCLUDEPATH += $$PWD/../3rdparty/QtXlsx

    LIBS += -L$${DEST_DIRECTORY}/lib -lQtXlsx
}

SOURCES += $$PWD/CommonClasses.cpp \
           $$PWD/numbertoarabicword.cpp \
           $$PWD/Barcode.cpp

HEADERS += $$PWD/CommonClasses.h \
           $$PWD/numbertoarabicword.h \
           $$PWD/Barcode.h

