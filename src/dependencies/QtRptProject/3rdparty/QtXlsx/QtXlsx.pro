QT      += core


greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

TARGET = QtXlsx
TEMPLATE = lib
CONFIG += dll

macx{
    CONFIG -= dll
    CONFIG += lib_bundle
    CONFIG += plugin
}

unix{
    CONFIG += plugin
}

CONFIG += c++14
QMAKE_CXXFLAGS += -std=c++11

include($$PWD/../../QtRPT/config.pri)  #Loading config file to get path where place ready library
!exists($$PWD/../../QtRPT/config.pri) {
    message("not found config.pri")
}
include(QtXlsx.pri)

INCLUDEPATH += QtXlsx
DLLDESTDIR = $${DEST_DIRECTORY}
DESTDIR    = $${DEST_DIRECTORY}/lib


QMAKE_DOCS = $$PWD/doc/qtxlsx.qdocconf

QMAKE_TARGET_COMPANY = "Debao Zhang"
QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2013-2014 Debao Zhang <hello@debao.me>"
QMAKE_TARGET_DESCRIPTION = ".Xlsx file wirter for Qt5"

