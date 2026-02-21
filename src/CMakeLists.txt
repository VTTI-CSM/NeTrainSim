# Indicate that this directory contains additional CMakeLists.txt that should be processed
cmake_minimum_required(VERSION 3.24)

# Set the variable 'QCustomPlot_LIB_TARGET_NAME' with the value "qcustomplotLib".
# This can be used as the name of a library target in the project
set(QCustomPlot_LIB_TARGET_NAME "qcustomplotLib")

# Core simulation engine DLL - build this first
add_subdirectory(NeTrainSim)

# Executable program
add_subdirectory(NeTrainSimConsole)

# GUI components
if (BUILD_GUI)
    add_subdirectory(NeTrainSimGUI)
endif()

# Server
if (BUILD_SERVER)
    add_subdirectory(NeTrainSimServer)
    add_subdirectory(NeTrainSimRabbitMQConfig)
endif()

# Installer - add when BUILD_INSTALLER is ON
if(BUILD_INSTALLER)
    add_subdirectory(NeTrainSimInstaller)

    # Add dependencies for the installer
    add_dependencies(NeTrainSimInstaller NeTrainSimCore NeTrainSimConsole)

    if (BUILD_GUI)
        add_dependencies(NeTrainSimInstaller NeTrainSimGUI)
    endif()

    if (BUILD_SERVER)
        add_dependencies(NeTrainSimInstaller NeTrainSimServer NeTrainSimRabbitMQConfig)
    endif()
endif()

