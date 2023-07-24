TEMPLATE = aux

QT_INSTALL_FRAMEWORK_PATH = C:\Qt\Tools\QtInstallerFramework\4.6

include(../mainconfig.pri)
include(create_xml.pri)


!CONFIG(debug, debug|release) {
    DESTDIR_NeTrainSim    = packages/com.VTTICSM.NeTrainSim/data
    DESTDIR_NeTrainSimGUI = packages/com.VTTICSM.NeTrainSimGUI/data

    win32 {
        create_package.commands = $$quote("C:\Qt\6.4.2\msvc2019_64\bin\windeployqt.exe" --qmldir ../NeTrainSim/ $${DESTDIR_NeTrainSim})
        create_package.commands += $$quote("C:\Qt\6.4.2\msvc2019_64\bin\windeployqt.exe" --qmldir ../NeTrainSimGUI/ $${DESTDIR_NeTrainSimGUI})
    }

    macx {
        create_package.commands = $$quote(macdeployqt $$PWD/../NeTrainSim/release/NeTrainSimGUI.app -dmg)
    }

    QMAKE_EXTRA_TARGETS += create_package
    PRE_TARGETDEPS  += create_package

    DISTFILES += \
    config/config.xml.in \
        packages/com.VTTICSM.NeTrainSim/meta/installscript.qs \
    packages/com.VTTICSM.NeTrainSim/meta/package.xml.in \
        packages/com.VTTICSM.NeTrainSimGUI/meta/installscript.qs \
        ../data/* \
    packages/com.VTTICSM.NeTrainSimGUI/meta/package.xml.in

    # copy the manual and sample project to the installer
    win32 {
        create_package.commands = $$quote(cmd /C "xcopy /E /I /Y \"..\\data\" \"$${DESTDIR_NeTrainSimGUI}\" && xcopy /E /I /Y \"..\\data\" \"$${DESTDIR_NeTrainSim}\"")
        create_package.depends = $$PWD/../data
    }
    unix:!win32 {
        create_package.commands = $$quote(cp -R $$PWD/../data/ $${DESTDIR_NeTrainSim})
        create_package.commands += $$quote(cp -R $$PWD/../data/ $${DESTDIR_NeTrainSimGUI})
        create_package.depends = $$PWD/../data
    }


    INSTALLER = NeTrainSimInstaller


    INPUT = $$PWD/config/config.xml $$PWD/packages

    NeTrainSimInstaller.input = INPUT
    NeTrainSimInstaller.output = $$INSTALLER
    NeTrainSimInstaller.commands = $$QT_INSTALL_FRAMEWORK_PATH/bin/binarycreator.exe --offline-only -c config/config.xml -p packages ${QMAKE_FILE_OUT}
    win32 {
        NeTrainSimInstaller.clean_commands += $$quote(del ..\NeTrainSim\NeTrainSimInstaller\packages\com.VTTICSM.NeTrainSim\data /Q)
        NeTrainSimInstaller.clean_commands += & for /d %%x in (..\NeTrainSim\NeTrainSimInstaller\packages\com.VTTICSM.NeTrainSim\data\*) do rd /s /q "%%x"

        NeTrainSimInstaller.clean_commands += $$quote(del ..\NeTrainSim\NeTrainSimInstaller\packages\com.VTTICSM.NeTrainSimGUI\data /Q)
        NeTrainSimInstaller.clean_commands += & for /d %%x in (..\NeTrainSim\NeTrainSimInstaller\packages\com.VTTICSM.NeTrainSimGUI\data\*) do rd /s /q "%%x"
    }


    QMAKE_EXTRA_COMPILERS += NeTrainSimInstaller

}

OTHER_FILES += \
    config/installscript.qs
