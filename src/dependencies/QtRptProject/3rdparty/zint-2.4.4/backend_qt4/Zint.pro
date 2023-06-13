DEFINES += NO_PNG

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

#VERSION = 2.4.4

INCLUDEPATH += ../backend
DEFINES +=  _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS ZINT_VERSION=\\\"$$VERSION\\\"
DEFINES += QZINT_LIBRARY
TARGET = QtZint

#!contains(DEFINES, NO_PNG) {
#    SOURCES += ../backend/png.c
#    LIBS += -lpng
#}


win32-msvc* {
    DEFINES += _CRT_SECURE_NO_WARNINGS
    #QMAKE_CFLAGS += /TP /wd4018 /wd4244 /wd4305
    #QMAKE_CXXFLAGS += /TP /wd4018 /wd4244 /wd4305
}

include($$PWD/../../../QtRPT/config.pri)  #Loading config file to get path where place ready library
!exists($$PWD/../../../QtRPT/config.pri) {
    message("not found config.pri")
}

INCLUDEPATH += zint zint/backend zint/backend_qt4
DLLDESTDIR = $${DEST_DIRECTORY}
DESTDIR    = $${DEST_DIRECTORY}/lib

HEADERS += \
    qzint.h \
    qzint_global.h \
    ../backend/aztec.h \
    ../backend/code1.h \
    ../backend/code49.h \
    ../backend/common.h \
    ../backend/composite.h \
    ../backend/dmatrix.h \
    ../backend/font.h \
    ../backend/gb2312.h \
    ../backend/gridmtx.h \
    ../backend/gs1.h \
    ../backend/large.h \
    ../backend/maxicode.h \
    ../backend/maxipng.h \
    ../backend/ms_stdint.h \
    ../backend/pdf417.h \
    ../backend/qr.h \
    ../backend/reedsol.h \
    ../backend/rss.h \
    ../backend/sjis.h \
    ../backend/zint.h

SOURCES += \
    $$PWD/qzint.cpp \
    ../backend/2of5.c \
    ../backend/auspost.c \
    ../backend/aztec.c \
    ../backend/code.c \
    ../backend/code1.c \
    ../backend/code16k.c \
    ../backend/code49.c \
    ../backend/code128.c \
    ../backend/common.c \
    ../backend/composite.c \
    ../backend/dllversion.c \
    ../backend/dmatrix.c \
    ../backend/gridmtx.c \
    ../backend/gs1.c \
    ../backend/imail.c \
    ../backend/large.c \
    ../backend/library.c \
    ../backend/maxicode.c \
    ../backend/medical.c \
    ../backend/pdf417.c \
    ../backend/plessey.c \
    ../backend/png.c \
    ../backend/postal.c \
    ../backend/ps.c \
    ../backend/qr.c \
    ../backend/reedsol.c \
    ../backend/render.c \
    ../backend/rss.c \
    ../backend/svg.c \
    ../backend/telepen.c \
    ../backend/upcean.c

win32 {
    MOC_DIR = tmp-win32
    UI_DIR = tmp-win32
    UI_HEADERS_DIR = tmp-win32
    UI_SOURCES_DIR = tmp-win32
    OBJECTS_DIR = tmp-win32
    RCC_DIR = tmp-win32
}

linux {
    MOC_DIR = tmp-lin64
    UI_DIR = tmp-lin64
    UI_HEADERS_DIR = tmp-lin64
    UI_SOURCES_DIR = tmp-lin64
    OBJECTS_DIR = tmp-lin64
    RCC_DIR = tmp-lin64
}

CONFIG -= debug_and_release debug_and_release_target

#DESTDIR = .

#include.path = $$[ZINT_INSTALL_HEADERS]
#include.files = ../backend/zint.h qzint.h

#target.path = $$[ZINT_INSTALL_LIBS]

#INSTALLS += target include

