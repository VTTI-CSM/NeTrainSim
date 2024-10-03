set(NeTrainSim_GUI_NAME "NeTrainSimGUI" CACHE STRING "Project GUI name" FORCE)
project(${NeTrainSim_GUI_NAME} VERSION ${NeTrainSim_VERSION} LANGUAGES CXX)

# Add the qcustomplot library target
add_library(${QCustomPlot_LIB_TARGET_NAME})

# Add the base include directory for the QtRPT headers
include_directories(${QTRPT_INCLUDE_DIR})

# Specifically add the QtRPT subdirectory where qtrptnamespace.h is located
include_directories(${QTRPT_INCLUDE_DIR}/QtRPT)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets concurrent PrintSupport Xml Sql Qml Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGLWidgets concurrent PrintSupport Xml Sql Qml Charts)

# Define source and header files
set(PROJECT_H_FILES
    ${CMAKE_SOURCE_DIR}/src/dependencies/qcustomplot/qcustomplot.h
    gui/checkboxdelegate.h
    gui/comboboxdelegate.h
    gui/customprogressbar.h
    gui/customtablewidget.h
    gui/disappearinglabel.h
    gui/intnumericdelegate.h
    gui/netrainsimmainwindow.h
    gui/nonemptydelegate.h
    gui/aboutwindow.h
    gui/customplot.h
    gui/numericdelegate.h
    gui/settingswindow.h
    gui/simulationworker.h
    gui/textboxdelegate.h
    gui/togglebutton.h
    util/configurationmanager.h
    util/errorhandler.h
)

set(PROJECT_UI_FILES
    gui/aboutwindow.ui
    gui/netrainsimmainwindow.ui
    gui/settingswindow.ui
)

set(PROJECT_SOURCES
    ${CMAKE_SOURCE_DIR}/src/dependencies/qcustomplot/qcustomplot.cpp
    gui/netrainsimmainwindow.cpp
    gui/settingswindow.cpp
    gui/togglebutton.cpp
    main.cpp
    gui/customtablewidget.cpp
    gui/disappearinglabel.cpp
    gui/aboutwindow.cpp
    gui/customplot.cpp
    gui/simulationworker.cpp
    util/configurationmanager.cpp
    util/errorhandler.cpp
)

# set configuration file
configure_file(VersionConfig.h.in VersionConfig.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${NeTrainSim_GUI_NAME}
        MANUAL_FINALIZATION

        ${PROJECT_H_FILES}
        ${PROJECT_UI_FILES}
        ${PROJECT_SOURCES}
        src.qrc

        # guiclasses/osgwidget.h guiclasses/osgwidget.cpp
    )
else()
    if(ANDROID)
        add_library(${NeTrainSim_GUI_NAME} SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(${NeTrainSim_GUI_NAME}
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_sources(${NeTrainSim_GUI_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/dependencies/qcustomplot/qcustomplot.cpp)

target_include_directories(${NeTrainSim_GUI_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/dependencies/qcustomplot)

# Ensure that NeTrainSim is built first by specifying it as a dependency
add_dependencies(${NeTrainSim_GUI_NAME} ${NETRAINSIM_CORE_NAME})

# Include necessary directories
target_include_directories(${NeTrainSim_GUI_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src/NeTrainSim
    ${CMAKE_SOURCE_DIR}/src/NeTrainSimConsole # Include the source files
    ${CMAKE_SOURCE_DIR}/src/NeTrainSimGUI     # Include the source files
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Link necessary libraries
target_link_libraries(${NeTrainSim_GUI_NAME} PRIVATE
    NeTrainSimCore
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Charts
)

# Link the appropriate library based on the build configuration and platform
target_link_libraries(${NeTrainSim_GUI_NAME} PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<PLATFORM_ID:Windows>>:${QTRPT_LIB_DIR}/QtRPTd.lib>
    $<$<AND:$<CONFIG:Release>,$<PLATFORM_ID:Windows>>:${QTRPT_LIB_DIR}/QtRPT.lib>
    $<$<AND:$<CONFIG:Debug>,$<PLATFORM_ID:Linux>>:${QTRPT_LIB_DIR}/libQtRPTd.so>
    $<$<AND:$<CONFIG:Release>,$<PLATFORM_ID:Linux>>:${QTRPT_LIB_DIR}/libQtRPT.so>
    $<$<AND:$<CONFIG:Debug>,$<PLATFORM_ID:Darwin>>:${QTRPT_LIB_DIR}/libQtRPTd.dylib>
    $<$<AND:$<CONFIG:Release>,$<PLATFORM_ID:Darwin>>:${QTRPT_LIB_DIR}/libQtRPT.dylib>
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.NeTrainSimGUI)
endif()


include(GNUInstallDirs)
install(TARGETS ${NeTrainSim_GUI_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

target_compile_options(${NeTrainSim_GUI_NAME} PRIVATE
    # MSVC-specific flags
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4
        $<$<CONFIG:Debug>:/Od /Zi> # Debug flags
        $<$<CONFIG:Release>:/Ox>   # Release flags
    >
    # GCC and Clang-specific flags
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:
        -Wall
        $<$<CONFIG:Debug>:-O0 -g>  # Debug flags
        $<$<CONFIG:Release>:-O3>   # Release flags
    >
)

# Ensure the NeTrainSim DLL is copied to the output directory
add_custom_command(TARGET ${NeTrainSim_GUI_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${NETRAINSIM_CORE_NAME}> $<TARGET_FILE_DIR:${NeTrainSim_GUI_NAME}>
)

# Conditionally include Container library only if BUILD_SERVER is ON
if(BUILD_SERVER)
    target_link_libraries(${NeTrainSim_GUI_NAME} PRIVATE Container::Container)
    target_include_directories(${NeTrainSim_GUI_NAME} PRIVATE ${CONTAINER_INCLUDE_DIRS})
    target_compile_definitions(${NeTrainSim_GUI_NAME} PRIVATE BUILD_SERVER_ENABLED)

    # Ensure the NeTrainSimCore and container DLLs are copied to the output directory
    add_custom_command(TARGET ${NeTrainSim_GUI_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Container::Container> $<TARGET_FILE_DIR:${NeTrainSim_GUI_NAME}>  # Copy container.dll
    )
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${NeTrainSim_GUI_NAME})
endif()
