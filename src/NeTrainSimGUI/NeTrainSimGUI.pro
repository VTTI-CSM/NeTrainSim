# This file contains the project configuration and build settings.

# Include the "config.pri" files.
include(../mainconfig.pri)
include(config.pri)

# Add the necessary Qt modules to the project.
QT       += core gui charts widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport charts

CONFIG += c++20
CONFIG += windows


## Include the "QtRpt.pri" file located in the "dependencies/qtrpt/QtRPT" directory.
include($$PWD/../dependencies/QtRptProject/QtRPT/QtRPT.pri)


SOURCES += \
    gui/netrainsimmainwindow.cpp \
    gui/settingswindow.cpp \
    gui/togglebutton.cpp \
    main.cpp \
    ../dependencies/qcustomplot/qcustomplot.cpp \
    ../NeTrainSim/network/readwritenetwork.cpp \
    ../NeTrainSim/util/csvmanager.cpp \
    ../NeTrainSim/traindefinition/trainslist.cpp \
    ../NeTrainSim/util/xmlmanager.cpp \
    ../NeTrainSim/network/netlink.cpp \
    ../NeTrainSim/network/netnode.cpp \
    ../NeTrainSim/network/netsignal.cpp \
    ../NeTrainSim/traindefinition/traincomponent.cpp \
    ../NeTrainSim/traindefinition/car.cpp \
    ../NeTrainSim/traindefinition/locomotive.cpp \
    ../NeTrainSim/traindefinition/train.cpp \
    ../NeTrainSim/traindefinition/battery.cpp \
    ../NeTrainSim/traindefinition/tank.cpp \
    ../NeTrainSim/traindefinition/energyconsumption.cpp \
    ../NeTrainSim/util/logger.cpp \
    ../NeTrainSim/simulator.cpp \
    ../NeTrainSim/network/netsignalgroupcontroller.cpp \
    ../NeTrainSim/network/netsignalgroupcontrollerwithqueuing.cpp \
    gui/customtablewidget.cpp \
    gui/disappearinglabel.cpp \
    gui/aboutwindow.cpp \
    gui/customplot.cpp \
    gui/simulationworker.cpp \
    util/configurationmanager.cpp \
    util/errorhandler.cpp \




HEADERS += \
    ../dependencies/qcustomplot/qcustomplot.h \
    ../NeTrainSim/network/readwritenetwork.h \
    ../NeTrainSim/util/csvmanager.h \
    ../NeTrainSim/traindefinition/trainslist.h \
    ../NeTrainSim/util/xmlmanager.h \
    ../NeTrainSim/network/netlink.h \
    ../NeTrainSim/network/netnode.h \
    ../NeTrainSim/network/netsignal.h \
    ../NeTrainSim/traindefinition/traincomponent.h \
    ../NeTrainSim/traindefinition/car.h \
    ../NeTrainSim/traindefinition/locomotive.h \
    ../NeTrainSim/traindefinition/train.h \
    ../NeTrainSim/traindefinition/battery.h \
    ../NeTrainSim/traindefinition/tank.h \
    ../NeTrainSim/traindefinition/energyconsumption.h \
    ../NeTrainSim/util/logger.h \
    ../NeTrainSim/simulator.h \
    ../NeTrainSim/network/netsignalgroupcontroller.h \
    ../NeTrainSim/network/netsignalgroupcontrollerwithqueuing.h \
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
    gui/settingswindow.h \
    gui/simulationworker.h \
    gui/togglebutton.h \
    util/configurationmanager.h \
    util/errorhandler.h \


FORMS += \
    gui/aboutwindow.ui \
    gui/netrainsimmainwindow.ui \
    gui/settingswindow.ui

TRANSLATIONS += \
    NeTrainSimGUI_en_US.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


# Add additional files to be included in the distribution package.
DISTFILES += \
    ../mainconfig.pri \
    config.pri \
    nts_extension.xml \
    gui/GPL.txt \

# Specify the resource file for the project.
RESOURCES += \
    src.qrc


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


DESTDIR += $$PWD/../NeTrainSimInstaller/packages/com.VTTICSM.NeTrainSimGUI/data


## Copy default INI file to the build directory
#copy_cmd = $$QMAKE_COPY_DIR
#copy_cmd += $$_PRO_FILE_PWD_/config.ini
#copy_cmd += $$OUT_PWD
#QMAKE_POST_LINK += $$copy_cmd
