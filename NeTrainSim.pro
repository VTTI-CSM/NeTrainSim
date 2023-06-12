# This file contains the project configuration and build settings.

# Include the "config.pri" file.
include(config.pri)

# Add the necessary Qt modules to the project.
QT       += core gui charts widgets

# If the major version of Qt is greater than 4, add the widgets, printsupport, and charts modules.
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport charts

# Check if the "AS_CMD" definition is present.
contains(DEFINES, AS_CMD) {
    # If "AS_CMD" is defined, add the c++20 and console configurations.
    CONFIG += c++20 console
    CONFIG -= app_bundle
    CONFIG -= windows
}
else {
    # If "AS_CMD" is not defined, add the c++20 configuration.
    CONFIG += c++20
    CONFIG += windows
}

# Specify the include path for the "src/gui" directory.
INCLUDEPATH += src/gui

# Include the "QtRPT.pri" file located in the "src/dependencies/qtrpt/QtRPT" directory.
include($$PWD/src/dependencies/qtrpt/QtRPT/QtRPT.pri)


# Add the source files to the project.
SOURCES += \
    main.cpp \
    src/simulator.cpp \
    src/dependencies/qcustomplot/qcustomplot.cpp \
    src/gui/customtablewidget.cpp \
    src/gui/disappearingLabel.cpp \
    src/gui/netrainsim.cpp \
    src/gui/aboutwindow.cpp \
    src/gui/customplot.cpp \
    src/gui/simulationworker.cpp \
    src/network/netlink.cpp \
    src/network/netnode.cpp \
    src/network/netsignal.cpp \
    src/network/netsignalgroupcontroller.cpp \
    src/network/readwritenetwork.cpp \
    src/traindefinition/car.cpp \
    src/traindefinition/energyconsumption.cpp \
    src/traindefinition/locomotive.cpp \
    src/traindefinition/train.cpp \
    src/traindefinition/traincomponent.cpp \
    src/traindefinition/trainslist.cpp \
    src/traindefinition/battery.cpp \
    src/traindefinition/tank.cpp \
    src/util/errorhandler.cpp \
    src/util/logger.cpp \
    src/util/xmlmanager.cpp \
    src/util/csvmanager.cpp

# Add the header files to the project.
HEADERS += \
    src/simulator.h \
    src/dependencies/qcustomplot/qcustomplot.h \
    src/gui/checkboxdelegate.h \
    src/gui/customprogressbar.h \
    src/gui/customtablewidget.h \
    src/gui/disappearinglabel.h \
    src/gui/intnumericdelegate.h \
    src/gui/netrainsim.h \
    src/gui/nonemptydelegate.h \
    src/gui/aboutwindow.h \
    src/gui/customplot.h \
    src/gui/numericdelegate.h \
    src/gui/simulationworker.h \
    src/network/netlink.h \
    src/network/netnode.h \
    src/network/netsignal.h \
    src/network/netsignalgroupcontroller.h \
    src/network/network.h \
    src/network/readwritenetwork.h \
    src/traindefinition/car.h \
    src/traindefinition/energyconsumption.h \
    src/traindefinition/locomotive.h \
    src/traindefinition/train.h \
    src/traindefinition/traincomponent.h \
    src/traindefinition/traintypes.h \
    src/traindefinition/trainslist.h \
    src/traindefinition/battery.h \
    src/traindefinition/tank.h \
    src/util/error.h \
    src/util/errorHandler.h \
    src/util/list.h \
    src/util/logger.h \
    src/util/map.h \
    src/util/utils.h \
    src/util/vector.h \
    src/util/xmlmanager.h \
    src/util/csvmanager.h

# Add the form files to the project.
FORMS += \
    src/gui/netrainsim.ui \
    src/gui/aboutwindow.ui

# Add the translation files to the project.
TRANSLATIONS += \
    neTrainSim_en_US.ts

# Configure lrelease for translation processing.
CONFIG += lrelease
CONFIG += embed_translations

# Associate the ".NTS" file extension with headers.
HEADERS += *.NTS


# Specify the target installation path based on the platform.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Add additional files to be included in the distribution package.
DISTFILES += \
    config.pri \
    nts_extension.xml \
    src/dependencies/qcustomplot/GPL.txt \
    src/resources/ECEDB.xml \
    #src/resources/icon.ico

# Specify the resource file for the project.
RESOURCES += \
    src.qrc

# Define the NTS file extension and MIME type
QMAKE_EXTENSION_PLUGIN += nts_extension_plugin
nts_extension_plugin.files = nts_extension.xml
nts_extension_plugin.path = $$[QT_INSTALL_DATA]/mimetypes
nts_extension_plugin.extra = update-mime-database $$[QT_INSTALL_DATA]/mimetypes

# Add the "icon.ico" file as an icon resource for the application.
win32:RC_ICONS += src/resources/icon.ico
macx:{
    ICON = src/resources/icon.icns
    QMAKE_INFO_PLIST = src/resources/Info.plist
}
unix:!macx {
    target.path = /usr/share/applications
    desktopfile.files = NeTrainSimGUI.desktop
    desktopfile.path = $$target.path
    INSTALLS += desktopfile
}

