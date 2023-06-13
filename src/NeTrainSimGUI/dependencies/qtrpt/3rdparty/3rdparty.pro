TEMPLATE = subdirs
CONFIG += ordered

include($$PWD/../QtRPT/config.pri)  #Loading config file to get path where place ready library

!contains(DEFINES, NO_BARCODE) {
    SUBDIRS += prj1
    prj1.file = $$PWD/zint-2.4.4/backend_qt4/Zint.pro
}

contains(DEFINES, QXLSX_LIBRARY) {
    SUBDIRS += $$PWD/QtXlsx/QtXlsx.pro
}

contains(DEFINES, QTRPT_LIBRARY) {
    SUBDIRS += $$PWD/../QtRPT
}
