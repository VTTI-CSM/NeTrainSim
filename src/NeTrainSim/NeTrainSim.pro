# This file contains the project configuration and build settings.

# Include the "config.pri" files.
include(../mainconfig.pri)
include(config.pri)


QT += core xml

#CONFIG += c++20 cmdline
CONFIG += c++20 console
CONFIG -= app_bundle
CONFIG -= windows

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        simulator.cpp \
        network/netlink.cpp \
        network/netnode.cpp \
        network/netsignal.cpp \
        network/netsignalgroupcontroller.cpp \
        network/readwritenetwork.cpp \
        traindefinition/car.cpp \
        traindefinition/energyconsumption.cpp \
        traindefinition/locomotive.cpp \
        traindefinition/train.cpp \
        traindefinition/traincomponent.cpp \
        traindefinition/trainslist.cpp \
        traindefinition/battery.cpp \
        traindefinition/tank.cpp \
        util/errorhandler.cpp \
        util/logger.cpp \
        util/xmlmanager.cpp \
        util/csvmanager.cpp

# Add the header files to the project.
HEADERS += \
    simulator.h \
    network/netlink.h \
    network/netnode.h \
    network/netsignal.h \
    network/netsignalgroupcontroller.h \
    network/network.h \
    network/readwritenetwork.h \
    traindefinition/car.h \
    traindefinition/energyconsumption.h \
    traindefinition/locomotive.h \
    traindefinition/train.h \
    traindefinition/traincomponent.h \
    traindefinition/traintypes.h \
    traindefinition/trainslist.h \
    traindefinition/battery.h \
    traindefinition/tank.h \
    util/error.h \
    util/errorHandler.h \
    util/list.h \
    util/logger.h \
    util/map.h \
    util/utils.h \
    util/vector.h \
    util/xmlmanager.h \
    util/csvmanager.h


TRANSLATIONS += \
    NeTrainSim_en_US.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


# Add additional files to be included in the distribution package.
DISTFILES += \
    ../mainconfig.pri \
    config.pri
