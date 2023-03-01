QT       += core gui charts widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport charts

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += src/gui

include($$PWD/src/dependencies/qtrpt/QtRPT/QtRPT.pri)

SOURCES += \
    main.cpp \
    netrainsim.cpp \
    src/Simulator.cpp \
    src/dependencies/qcustomplot/qcustomplot.cpp \
    src/gui/CustomTableWidget.cpp \
    src/gui/DisappearingLabel.cpp \
    src/network/NetLink.cpp \
    src/network/NetNode.cpp \
    src/network/NetSignal.cpp \
    src/network/NetSignalGroupController.cpp \
    src/trainDefintion/Car.cpp \
    src/trainDefintion/EnergyConsumption.cpp \
    src/trainDefintion/Locomotive.cpp \
    src/trainDefintion/Train.cpp \
    src/trainDefintion/TrainComponent.cpp \
    src/trainDefintion/TrainsList.cpp \
    src/util/Logger.cpp

HEADERS += \
    netrainsim.h \
    src/Simulator.h \
    src/dependencies/qcustomplot/qcustomplot.h \
    src/gui/CheckboxDelegate.h \
    src/gui/CustomProgressBar.h \
    src/gui/CustomTableWidget.h \
    src/gui/DisappearingLabel.h \
    src/gui/IntNumericDelegate.h \
    src/gui/numericdelegate.h \
    src/network/NetLink.h \
    src/network/NetNode.h \
    src/network/NetSignal.h \
    src/network/NetSignalGroupController.h \
    src/network/Network.h \
    src/trainDefintion/Car.h \
    src/trainDefintion/EnergyConsumption.h \
    src/trainDefintion/Locomotive.h \
    src/trainDefintion/Train.h \
    src/trainDefintion/TrainComponent.h \
    src/trainDefintion/TrainTypes.h \
    src/trainDefintion/TrainsList.h \
    src/util/List.h \
    src/util/Logger.h \
    src/util/Map.h \
    src/util/Utils.h \
    src/util/Vector.h

FORMS += \
    netrainsim.ui

TRANSLATIONS += \
    NeTrainSim_en_US.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    config.pri \
    src/dependencies/qcustomplot/GPL.txt \
    src/resources/ECEDB.xml \
    src/resources/icon.ico

RESOURCES += \
    src.qrc

