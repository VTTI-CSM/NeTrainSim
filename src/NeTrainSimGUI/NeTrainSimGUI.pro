# This file contains the project configuration and build settings.

# Include the "config.pri" files.
include(../mainconfig.pri)
include(config.pri)

# Add the necessary Qt modules to the project.
QT       += core gui charts widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport charts

CONFIG += c++20
CONFIG += windows

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


# Include the "QtRPT.pri" file located in the "dependencies/qtrpt/QtRPT" directory.
include($$PWD/dependencies/qtrpt/QtRPT/QtRPT.pri)


SOURCES += \
    gui/netrainsimmainwindow.cpp \
    main.cpp \
    dependencies/qcustomplot/qcustomplot.cpp \
    gui/customtablewidget.cpp \
    gui/disappearinglabel.cpp \
    gui/aboutwindow.cpp \
    gui/customplot.cpp \
    gui/simulationworker.cpp \
    util/errorhandler.cpp \
    ../NeTrainSim/simulator.cpp \
    ../NeTrainSim/network/netlink.cpp \
    ../NeTrainSim/network/netnode.cpp \
    ../NeTrainSim/network/netsignal.cpp \
    ../NeTrainSim/network/netsignalgroupcontroller.cpp \
    ../NeTrainSim/network/readwritenetwork.cpp \
    ../NeTrainSim/traindefinition/car.cpp \
    ../NeTrainSim/traindefinition/energyconsumption.cpp \
    ../NeTrainSim/traindefinition/locomotive.cpp \
    ../NeTrainSim/traindefinition/train.cpp \
    ../NeTrainSim/traindefinition/traincomponent.cpp \
    ../NeTrainSim/traindefinition/trainslist.cpp \
    ../NeTrainSim/traindefinition/battery.cpp \
    ../NeTrainSim/traindefinition/tank.cpp \
    ../NeTrainSim/util/logger.cpp \
    ../NeTrainSim/util/xmlmanager.cpp \
    ../NeTrainSim/util/csvmanager.cpp


HEADERS += \
    dependencies/qcustomplot/qcustomplot.h \
    gui/checkboxdelegate.h \
    gui/customprogressbar.h \
    gui/customtablewidget.h \
    gui/disappearinglabel.h \
    gui/intnumericdelegate.h \
    gui/netrainsimmainwindow.h \
    gui/nonemptydelegate.h \
    gui/aboutwindow.h \
    gui/customplot.h \
    gui/numericdelegate.h \
    gui/simulationworker.h \
    util/errorhandler.h \
    ../NeTrainSim/simulator.h \
    ../NeTrainSim/network/netlink.h \
    ../NeTrainSim/network/netnode.h \
    ../NeTrainSim/network/netsignal.h \
    ../NeTrainSim/network/netsignalgroupcontroller.h \
    ../NeTrainSim/network/network.h \
    ../NeTrainSim/network/readwritenetwork.h \
    ../NeTrainSim/traindefinition/car.h \
    ../NeTrainSim/traindefinition/energyconsumption.h \
    ../NeTrainSim/traindefinition/locomotive.h \
    ../NeTrainSim/traindefinition/train.h \
    ../NeTrainSim/traindefinition/traincomponent.h \
    ../NeTrainSim/traindefinition/traintypes.h \
    ../NeTrainSim/traindefinition/trainslist.h \
    ../NeTrainSim/traindefinition/battery.h \
    ../NeTrainSim/traindefinition/tank.h \
    ../NeTrainSim/util/error.h \
    ../NeTrainSim/util/list.h \
    ../NeTrainSim/util/logger.h \
    ../NeTrainSim/util/map.h \
    ../NeTrainSim/util/utils.h \
    ../NeTrainSim/util/vector.h \
    ../NeTrainSim/util/xmlmanager.h \
    ../NeTrainSim/util/csvmanager.h

FORMS += \
    gui/aboutwindow.ui \
    gui/netrainsimmainwindow.ui

TRANSLATIONS += \
    NeTrainSimGUI_en_US.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


# Associate the ".NTS" file extension with headers.
HEADERS += *.NTS

# Add additional files to be included in the distribution package.
DISTFILES += \
    ../mainconfig.pri \
    config.pri \
    nts_extension.xml \
    dependencies/qcustomplot/GPL.txt \

# Specify the resource file for the project.
RESOURCES += \
    src.qrc


# Define the NTS file extension and MIME type
QMAKE_EXTENSION_PLUGIN += nts_extension_plugin
nts_extension_plugin.files = nts_extension.xml
nts_extension_plugin.path = $$[QT_INSTALL_DATA]/mimetypes
nts_extension_plugin.extra = update-mime-database $$[QT_INSTALL_DATA]/mimetypes

# Add the "icon.ico" file as an icon resource for the application.
win32:RC_ICONS += resources/icon.ico
macx:{
    ICON = resources/icon.icns
    QMAKE_INFO_PLIST = resources/Info.plist
}
unix:!macx {
    target.path = /usr/share/applications
    desktopfile.files = NeTrainSimGUI.desktop
    desktopfile.path = $$target.path
    INSTALLS += desktopfile
}
