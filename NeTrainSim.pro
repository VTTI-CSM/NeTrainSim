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
    src/Simulator.cpp \
    src/dependencies/qcustomplot/qcustomplot.cpp \
    src/gui/CustomTableWidget.cpp \
    src/gui/DisappearingLabel.cpp \
    src/gui/Netrainsim.cpp \
    src/gui/aboutwindow.cpp \
    src/gui/customplot.cpp \
    src/gui/simulationworker.cpp \
    src/network/NetLink.cpp \
    src/network/NetNode.cpp \
    src/network/NetSignal.cpp \
    src/network/NetSignalGroupController.cpp \
    src/network/ReadWriteNetwork.cpp \
    src/trainDefintion/Car.cpp \
    src/trainDefintion/EnergyConsumption.cpp \
    src/trainDefintion/Locomotive.cpp \
    src/trainDefintion/Train.cpp \
    src/trainDefintion/TrainComponent.cpp \
    src/trainDefintion/TrainsList.cpp \
    src/trainDefintion/battery.cpp \
    src/trainDefintion/tank.cpp \
    src/util/ErrorHandler.cpp \
    src/util/Logger.cpp \
    src/util/XMLManager.cpp \
    src/util/csvmanager.cpp

# Add the header files to the project.
HEADERS += \
    src/Simulator.h \
    src/dependencies/qcustomplot/qcustomplot.h \
    src/gui/CheckboxDelegate.h \
    src/gui/CustomProgressBar.h \
    src/gui/CustomTableWidget.h \
    src/gui/DisappearingLabel.h \
    src/gui/IntNumericDelegate.h \
    src/gui/Netrainsim.h \
    src/gui/NonEmptyDelegate.h \
    src/gui/aboutwindow.h \
    src/gui/customplot.h \
    src/gui/numericdelegate.h \
    src/gui/simulationworker.h \
    src/network/NetLink.h \
    src/network/NetNode.h \
    src/network/NetSignal.h \
    src/network/NetSignalGroupController.h \
    src/network/Network.h \
    src/network/ReadWriteNetwork.h \
    src/trainDefintion/Car.h \
    src/trainDefintion/EnergyConsumption.h \
    src/trainDefintion/Locomotive.h \
    src/trainDefintion/Train.h \
    src/trainDefintion/TrainComponent.h \
    src/trainDefintion/TrainTypes.h \
    src/trainDefintion/TrainsList.h \
    src/trainDefintion/battery.h \
    src/trainDefintion/tank.h \
    src/util/Error.h \
    src/util/ErrorHandler.h \
    src/util/List.h \
    src/util/Logger.h \
    src/util/Map.h \
    src/util/Utils.h \
    src/util/Vector.h \
    src/util/XMLManager.h \
    src/util/csvmanager.h

# Add the form files to the project.
FORMS += \
    src/gui/Netrainsim.ui \
    src/gui/aboutwindow.ui

# Add the translation files to the project.
TRANSLATIONS += \
    NeTrainSim_en_US.ts
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
    src/resources/icon.ico

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

