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
    gui/importshpwindow.cpp \
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
    util/shapefilereader.cpp




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
    gui/comboboxdelegate.h \
    gui/customprogressbar.h \
    gui/customtablewidget.h \
    gui/disappearinglabel.h \
    gui/importshpwindow.h \
    gui/intnumericdelegate.h \
    gui/netrainsimmainwindow.h \
    gui/nonemptydelegate.h \
    gui/aboutwindow.h \
    gui/customplot.h \
    gui/numericdelegate.h \
    gui/settingswindow.h \
    gui/simulationworker.h \
    gui/textboxdelegate.h \
    gui/togglebutton.h \
    util/configurationmanager.h \
    util/errorhandler.h \
    util/shapefilereader.h


FORMS += \
    gui/aboutwindow.ui \
    gui/importshpwindow.ui \
    gui/netrainsimmainwindow.ui \
    gui/settingswindow.ui
    gui/importshpwindow.ui




win32{
#   make sure you compile the dependencies first
    INCLUDEPATH += ../dependencies/shpelib
    LIBS += -L$$PWD/../dependencies/shpelib -lshapelib

#   change the include path to the location of the PROJ library installation.
    INCLUDEPATH += C:\OSGeo4W\include
    LIBS += -LC:\OSGeo4W\lib -lproj
}

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

!CONFIG(debug, debug|release) {
    DESTDIR += $$PWD/../NeTrainSimInstaller/packages/com.VTTICSM.NeTrainSimGUI/data
}

# Determine if the configuration is debug or release and set the appropriate build subdirectory
CONFIG(debug, debug|release) {
    build_subdir = debug

    # Append _d to the DLL filename if it's a debug build
    proj_dll_suffix =
} else {
    build_subdir = release
    proj_dll_suffix =
}

# Define the PROJ DLL file paths and copy command
proj_dll_dir = $$PWD/../dependencies/PROJ/bin/proj_9_3$$join(proj_dll_suffix,,,).dll
proj_dll_outdir = $$OUT_PWD/$$build_subdir/proj_9_3$$join(proj_dll_suffix,,,).dll
message($$proj_dll_dir)
# Define the PROJ Share folder paths and copy command
proj_share_dir = $$PWD/../dependencies/PROJ/share
proj_share_outdir = $$OUT_PWD/$$build_subdir/share

# Windows-specific path transformations and copy command
win32 {
    proj_dll_dir ~= s,/,\\,g
    proj_dll_outdir ~= s,/,\\,g
    proj_share_dir ~= s,/,\\,g
    proj_share_outdir ~= s,/,\\,g
}

# Define the Config file paths and copy command
config_dir = $$PWD/config.ini
config_out = $$OUT_PWD/$$build_subdir/config.ini

# Windows-specific path transformations for the Config file
win32 {
    config_dir ~= s,/,\\,g
    config_out ~= s,/,\\,g
}

win32: copyprojdata.commands = xcopy /E /I /Y $$shell_quote($$proj_share_dir) $$shell_quote($$proj_share_outdir)
unix: copyprojdata.commands = cp -r $$shell_quote($$proj_share_dir) $$shell_quote($$proj_share_outdir)
copyprojdll.commands = $(COPY_FILE) $$shell_quote($$proj_dll_dir) $$shell_quote($$proj_dll_outdir)
copydata.commands = $(COPY_FILE) $$shell_quote($$config_dir) $$shell_quote($$config_out)

# Add the copy commands to the extra targets so they will be executed as part of the build
QMAKE_EXTRA_TARGETS += copyprojdll copyprojdata copydata
PRE_TARGETDEPS += copyprojdll copyprojdata copydata
