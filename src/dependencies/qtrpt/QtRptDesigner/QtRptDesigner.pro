#-------------------------------------------------
#
# Project created by QtCreator 2012-03-29T16:09:02
#
#-------------------------------------------------

QT       += core gui xml network charts qml sql

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
    DEFINES += HAVE_QT5
}

TARGET = QtRptDesigner
VERSION = 3.0.0
DEFINES += VERSION=\\\"$$VERSION\\\"
TEMPLATE = app

CONFIG(debug, debug|release) {
    DEST_DIRECTORY = $$PWD/../bin/debug
}
CONFIG(release, debug|release) {
    DEST_DIRECTORY = $$PWD/../bin/release
}

DESTDIR = $${DEST_DIRECTORY}

#DEFINES += QTRPT_LIBRARY

contains(DEFINES, QTRPT_LIBRARY) {
    INCLUDEPATH += $$PWD/../QtRPT/
    LIBS += -L$${DEST_DIRECTORY}/lib -lQtRPT
} else {
    include(../QtRPT/QtRPT.pri)
}

!contains(DEFINES, NO_BARCODE) {
    INCLUDEPATH += $$PWD/../3rdparty/zint-2.4.4/backend_qt4
    INCLUDEPATH += $$PWD/../3rdparty/zint-2.4.4/backend

    LIBS += -L$${DEST_DIRECTORY}/lib -lQtZint
}

include(../CommonFiles/CommonFiles_QtRptDesigner.pri)
include(Graphics/Graphics.pri)
include(SQLDiagram/SQLDiagram.pri)



SOURCES += main.cpp\
    mainwindow.cpp \
    PageSettingDlg.cpp \
    ReportBand.cpp \
    EditFldDlg.cpp \
    aboutDlg.cpp \
    FldPropertyDlg.cpp \
    RepScrollArea.cpp \
    SettingDlg.cpp \
    SqlDesigner.cpp \
    XmlViewModel.cpp \
    ScriptEditor.cpp

HEADERS  += mainwindow.h \
    PageSettingDlg.h \
    ReportBand.h \
    EditFldDlg.h \
    aboutDlg.h \
    FldPropertyDlg.h \
    RepScrollArea.h \
    SettingDlg.h \
    SqlDesigner.h \
    XmlViewModel.h \
    CustomInterface.h \
    ScriptEditor.h

FORMS    += mainwindow.ui \
    PageSettingDlg.ui \
    ReportBand.ui \
    EditFldDlg.ui \
    FldPropertyDlg.ui \
    RepScrollArea.ui \
    SettingDlg.ui \
    SqlDesigner.ui \
    ScriptEditor.ui

RESOURCES += \
    images.qrc

RC_FILE = QtRptDesigner.rc

TRANSLATIONS +=  \
    $$PWD/i18n/QtRprtDesigner_uk_UA.ts \
    $$PWD/i18n/QtRprtDesigner_sr_RS.ts \
    $$PWD/i18n/QtRprtDesigner_sr_RS@latin.ts \
    $$PWD/i18n/QtRprtDesigner_ar.ts \
    $$PWD/i18n/QtRprtDesigner_ka_GE.ts \
    $$PWD/i18n/QtRprtDesigner_pt_PT.ts \
    $$PWD/i18n/QtRprtDesigner_zh_CN.ts \
    $$PWD/i18n/QtRprtDesigner_es_ES.ts \
    $$PWD/i18n/QtRprtDesigner_fr_FR.ts \
    $$PWD/i18n/QtRprtDesigner_de_DE.ts \
    $$PWD/i18n/QtRprtDesigner_nl_BE.ts \
    $$PWD/i18n/QtRprtDesigner_ta_IN.ts \
    $$PWD/i18n/QtRprtDesigner_it_IT.ts

OTHER_FILES += $$TRANSLATIONS

win32 {
    MOC_DIR = tmp-win32
    UI_DIR = tmp-win32
    UI_HEADERS_DIR = tmp-win32
    UI_SOURCES_DIR = tmp-win32
    OBJECTS_DIR = tmp-win32
    RCC_DIR = tmp-win32
}

unix {
    MOC_DIR = tmp-lin64
    UI_DIR = tmp-lin64
    UI_HEADERS_DIR = tmp-lin64
    UI_SOURCES_DIR = tmp-lin64
    OBJECTS_DIR = tmp-lin64
    RCC_DIR = tmp-lin64
}

CONFIG += c++14
QMAKE_CXXFLAGS += -std=c++11

CONFIG += app_bundle
CONFIG -= debug_and_release debug_and_release_target

# Automatically build required translation files (*.qm)
isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}
updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm
PRE_TARGETDEPS += compiler_updateqm_make_all
