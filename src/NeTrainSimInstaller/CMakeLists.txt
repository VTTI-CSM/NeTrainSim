cmake_minimum_required(VERSION 3.24)
project(NeTrainSimInstaller)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")

    set(DESTDIR_NeTrainSim "${CMAKE_CURRENT_SOURCE_DIR}/packages/com.VTTICSM.NeTrainSim/data")
    set(DESTDIR_NeTrainSimGUI "${CMAKE_CURRENT_SOURCE_DIR}/packages/com.VTTICSM.NeTrainSimGUI/data")

    # Define the NeTrainSimInstaller target first
    add_custom_target(NeTrainSimInstaller ALL
        COMMAND ${QT_INSTALL_FRAMEWORK_PATH}/bin/binarycreator.exe --offline-only -c ${CMAKE_CURRENT_SOURCE_DIR}/config/config.xml -p ${CMAKE_CURRENT_SOURCE_DIR}/packages NeTrainSimInstaller
        DEPENDS create_package copy_manuals
    )

    # Add dependencies to ensure NeTrainSimInstaller builds only after other subprojects
    add_dependencies(NeTrainSimInstaller NeTrainSimCore NeTrainSimConsole NeTrainSimGUI)

    # Create the directories before copying files
    add_custom_command(
        OUTPUT ${DESTDIR_NeTrainSim}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DESTDIR_NeTrainSim}
    )

    add_custom_command(
        OUTPUT ${DESTDIR_NeTrainSimGUI}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DESTDIR_NeTrainSimGUI}
    )

    add_custom_target(create_dest_dirs ALL DEPENDS ${DESTDIR_NeTrainSim} ${DESTDIR_NeTrainSimGUI})

    # Copy the executable files from the subprojects
    add_custom_command(
        OUTPUT ${DESTDIR_NeTrainSim}/NeTrainSimConsole.exe
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:NeTrainSimConsole> ${DESTDIR_NeTrainSim}/NeTrainSimConsole.exe
        DEPENDS create_dest_dirs NeTrainSimConsole
    )

    add_custom_command(
        OUTPUT ${DESTDIR_NeTrainSimGUI}/NeTrainSimGUI.exe
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:NeTrainSimGUI> ${DESTDIR_NeTrainSimGUI}/NeTrainSimGUI.exe
        DEPENDS create_dest_dirs NeTrainSimGUI
    )
    add_custom_target(copy_executables ALL DEPENDS ${DESTDIR_NeTrainSim}/NeTrainSimConsole.exe ${DESTDIR_NeTrainSimGUI}/NeTrainSimGUI.exe)

    # Copy the DLL from the NeTrainSim subproject
    add_custom_command(
        OUTPUT ${DESTDIR_NeTrainSim}/NeTrainSimCore.dll
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:NeTrainSimCore> ${DESTDIR_NeTrainSim}/NeTrainSimCore.dll
        DEPENDS create_dest_dirs NeTrainSimCore
    )

    # Get the path to the Container.dll using the Container::Container target.
    get_target_property(Container_DLL_PATH Container::Container LOCATION)

    # Add a custom command to copy the DLL to the desired destination.
    add_custom_command(
        OUTPUT ${DESTDIR_NeTrainSim}/Container.dll
        COMMAND ${CMAKE_COMMAND} -E copy ${Container_DLL_PATH} ${DESTDIR_NeTrainSim}/Container.dll
        DEPENDS create_dest_dirs Container::Container
    )
    add_custom_target(copy_dlls ALL DEPENDS ${DESTDIR_NeTrainSim}/NeTrainSimCore.dll ${DESTDIR_NeTrainSim}/Container.dll)


    # Gather all files from ../data
    file(GLOB_RECURSE DATA_FILES RELATIVE ${CMAKE_SOURCE_DIR}/../data ../data/*)

    # Define the custom commands for copying the files
    foreach(file ${DATA_FILES})
        get_filename_component(file_path ${file} PATH)
        get_filename_component(file_name ${file} NAME)

        add_custom_command(
            OUTPUT ${DESTDIR_NeTrainSimGUI}/${file_name}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${DESTDIR_NeTrainSimGUI}/${file_path}
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/../data/${file} ${DESTDIR_NeTrainSimGUI}/${file_path}/${file_name}
            DEPENDS ${file}
        )

        add_custom_command(
            OUTPUT ${DESTDIR_NeTrainSim}/${file_name}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${DESTDIR_NeTrainSim}/${file_path}
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/../data/${file} ${DESTDIR_NeTrainSim}/${file_path}/${file_name}
            DEPENDS ${file}
        )
    endforeach()

    # Create a custom target that depends on the copied files
    add_custom_target(copy_manuals ALL DEPENDS ${DESTDIR_NeTrainSimGUI} ${DESTDIR_NeTrainSim})


    if(WIN32)
        add_custom_command(
            OUTPUT create_package
            COMMAND ${CMAKE_COMMAND} -E env "C:/Qt/6.4.2/msvc2019_64/bin/windeployqt.exe" --qmldir ../NeTrainSim/ ${DESTDIR_NeTrainSim}
            COMMAND ${CMAKE_COMMAND} -E env "C:/Qt/6.4.2/msvc2019_64/bin/windeployqt.exe" --qmldir ../NeTrainSimGUI/ ${DESTDIR_NeTrainSimGUI}
            DEPENDS copy_manuals
        )

    elseif(APPLE)
        add_custom_command(
            OUTPUT create_package
            COMMAND ${CMAKE_COMMAND} -E env macdeployqt ../NeTrainSim/release/NeTrainSimGUI.app -dmg
            DEPENDS ../NeTrainSimGUI
        )
    elseif(UNIX AND NOT WIN32)
        add_custom_command(
            OUTPUT copy_manuals
            COMMAND ${CMAKE_COMMAND} -E copy_directory ../data ${DESTDIR_NeTrainSim}
            COMMAND ${CMAKE_COMMAND} -E copy_directory ../data ${DESTDIR_NeTrainSimGUI}
            DEPENDS ../data
        )
    endif()

    set(QT_INSTALL_FRAMEWORK_PATH "C:/Qt/Tools/QtInstallerFramework/4.6")

    add_custom_command(TARGET NeTrainSimInstaller
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${DESTDIR_NeTrainSim}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${DESTDIR_NeTrainSimGUI}
    )

endif()

# Additional files
set(OTHER_FILES
    config/installscript.qs
)

foreach(file ${OTHER_FILES})
    configure_file(${file} ${CMAKE_CURRENT_BINARY_DIR}/copy_${file} COPYONLY)
endforeach()
