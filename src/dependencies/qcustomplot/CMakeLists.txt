# @copyright (c) 2021-2023 virginia polytechnic institute and state university
#                      Transportation Institute (VTTI). All rights reserved.
#
#
# NeTrainSim is a simulator provided and maintained by VTTI
#
#
# @file CMakeLists.txt for qcustomplot
# @version 0.0.1
# @author Ahmed Aredah
# @date 2-2023




###############################################################################
#                                                                             #
#                     THIS IS A QCustomPlot CMAKELISTS.txt                    #
#                                                                             #
###############################################################################


###############################################################################
#                             PRELIMINARIES                                   #
###############################################################################
# Set project properties
set(PROJECT_NAME qcustomplot)
#Qt version
set(QT_MAJOR_VERSION 5)

# Check for target architecture
if(NOT PROJECT_ARCH_TARGET)
    message(FATAL_ERROR "Cmake variable \"PROJECT_ARCH_TARGET\" is undefined, please set it in your root CMakelist.")
endif()

cmake_minimum_required(VERSION 3.24)
 
#name the project, version, and language
if (WIN32)
    project(${PROJECT_NAME} LANGUAGES CXX)
elseif(UNIX)
    project(${PROJECT_NAME})
endif()


set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)   # Specific to Qt
set(CMAKE_AUTOMOC ON)   # Specific to Qt
set(CMAKE_AUTORCC ON)   # Specific to Qt

if(QT_MAJOR_VERSION EQUAL 5)
    set(CMAKE_PREFIX_PATH $ENV{Qt5_HOME})
elseif(QT_MAJOR_VERSION EQUAL 5)
    set(CMAKE_PREFIX_PATH $ENV{Qt6_HOME})
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Find Qt packages
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(QT NAMES Qt6 Qt5 COMPONENTS PrintSupport REQUIRED)

#find_package(Qt${QT_MAJOR_VERSION} COMPONENTS Widgets REQUIRED PATHS $ENV{Qt${QT_MAJOR_VERSION}_HOME/include/QtCore})
#find_package(Qt${QT_MAJOR_VERSION} COMPONENTS Core REQUIRED PATHS $ENV{Qt${QT_MAJOR_VERSION}_HOME/include/QtCore})
#find_package(Qt${QT_MAJOR_VERSION} COMPONENTS PrintSupport REQUIRED PATHS $ENV{Qt${QT_MAJOR_VERSION}_HOME/include/QtCore})

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS PrintSupport REQUIRED)

# Manage library files
set(PROJECT_HEADERS
    qcustomplot.h
)

set(PROJECT_SOURCES
    qcustomplot.cpp
)

set(PROJECT_FILES ${PROJECT_HEADERS} ${PROJECT_SOURCES})

# Add files to the library
add_library(${PROJECT_NAME} SHARED ${PROJECT_FILES})


# Link needed libraries
# Qt Library
target_link_libraries(${PROJECT_NAME} PUBLIC Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(${PROJECT_NAME} PUBLIC Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(${PROJECT_NAME} PUBLIC Qt${QT_VERSION_MAJOR}::PrintSupport)

# Compile needed definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE QCUSTOMPLOT_COMPILE_LIBRARY)


# Directories to includes
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})