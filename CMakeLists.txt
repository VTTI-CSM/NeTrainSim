# min required cmake version
cmake_minimum_required(VERSION 3.24)

# Define the project name (NeTrainSim) and
# the programming language used (CXX for C++)
set(NeTrainSim_VERSION "0.1.3" CACHE STRING "Project version" FORCE)
set(NETRAINSIM_NAME "NeTrainSim" CACHE STRING "Project name" FORCE)
set(NETRAINSIM_VENDOR "(C) 2022-2023 Virginia Tech Transportation Institute - Center for Sustainable Mobility." CACHE STRING "Project vendor" FORCE)
# Get the current date and time
string(TIMESTAMP BUILD_DATE "%Y-%m-%d %H:%M:%S")
# Set the BUILD_DATE variable
set(BUILD_DATE ${BUILD_DATE} CACHE STRING "Project build time" FORCE)

# Extract major, minor, and patch version from NeTrainSim_VERSION
string(REPLACE "." ";" VERSION_LIST ${NeTrainSim_VERSION})
list(GET VERSION_LIST 0 NeTrainSim_VERSION_MAJOR)
list(GET VERSION_LIST 1 NeTrainSim_VERSION_MINOR)
list(GET VERSION_LIST 2 NeTrainSim_VERSION_PATCH)

project(${NETRAINSIM_NAME} VERSION ${NeTrainSim_VERSION} LANGUAGES CXX)

if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release;MinSizeRel;RelWithDebInfo" CACHE STRING "" FORCE)
endif()
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Compiler settings
# Set the C++20 standard to be used for compiling
set(CMAKE_CXX_STANDARD 23)
# Ensure that the selected C++ standard is a
# requirement for the compiler
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Enable Qt's Automatic User Interface Compiler (UIC)
set(CMAKE_AUTOUIC ON)
# Enable Qt's Meta-Object Compiler (MOC) which allows
# the use of Qt features such as signals and slots
set(CMAKE_AUTOMOC ON)
# Enable Qt's Resource Compiler (RCC) for compiling
# resource files into binary format
set(CMAKE_AUTORCC ON)

# Platform-specific compiler flags
if(MSVC)
    # MSVC-specific flags
    add_compile_options(/W4 /MP)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(/Od /Zi)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O2)
    endif()
else()
    # GCC/Clang-specific flags
    add_compile_options(-Wall)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-O0 -g)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3)
    endif()
endif()

# Add definitions based on build type
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

# -------------------------------------------------------
# --------------------- OPTIONS -------------------------
# -------------------------------------------------------

# Option to build GUI components
option(BUILD_GUI "Build the GUI components" ON)
# Cache the option so it's stored between runs
set(BUILD_GUI ${BUILD_GUI} CACHE BOOL "Build the GUI components" FORCE)

# Option to build Server components
option(BUILD_SERVER "Build the SERVER components" ON)
# Cache the option so it's stored between runs
set(BUILD_SERVER ${BUILD_SERVER} CACHE BOOL "Build the SERVER components" FORCE)

# -------------------------------------------------------
# --------------------- Libraries -----------------------
# --------------- Define Default Paths ------------------
# -------------------------------------------------------

if(BUILD_GUI)
    if(WIN32)
        set(KDREPORTS_DIR "C:/Program Files/KDAB/KDReports/cmake" CACHE PATH "Path to KDReports CMake directory")
    elseif(APPLE)
        set(KDREPORTS_DIR "/usr/local/KDAB/KDReports-2.3.95/lib/cmake/KDReports-qt6" CACHE PATH "Path to KDReports CMake directory")
    elseif(UNIX)
        set(KDREPORTS_DIR "/usr/local/KDAB/KDReports-2.3.95/lib/cmake/KDReports-qt6" CACHE PATH "Path to KDReports CMake directory")
    endif()

    # Use the KDReports CMake package
    find_package(KDReports-qt6 REQUIRED CONFIG PATHS ${KDREPORTS_DIR})
endif()

if(BUILD_SERVER)
    if(WIN32)
        # Windows-specific paths
        set(CONTAINER_SEARCH_PATHS "C:/Program Files/Container/cmake" CACHE PATH "Default path to container's library")
        set(RABBITMQ_CMAKE_DIR "C:/Program Files/rabbitmq-c/lib/cmake/rabbitmq-c" CACHE PATH "Default path to RabbitMQ-C library on Windows")
    elseif(APPLE)
        # macOS-specific paths
        set(CONTAINER_SEARCH_PATHS "/usr/local/lib/cmake/Container" CACHE PATH "Default path to container's library on macOS")
        set(RABBITMQ_CMAKE_DIR "/usr/local/lib/rabbitmq-c/cmake" CACHE PATH "Default path to RabbitMQ-C library on macOS")
    elseif(UNIX)
        # Linux-specific paths
        set(CONTAINER_SEARCH_PATHS "/usr/local/lib/cmake/Container" CACHE PATH "Default path to container's library on Linux")
        set(RABBITMQ_CMAKE_DIR "/usr/local/lib/cmake/rabbitmq-c" CACHE PATH "Default path to RabbitMQ-C library on Linux")
    else()
        message(FATAL_ERROR "Unsupported platform. Please set paths for CONTAINER_CMAKE_DIR and RABBITMQ_CMAKE_DIR manually.")
    endif()


    # Find the installed Container library
    set(CONTAINER_CMAKE_DIR "${CONTAINER_SEARCH_PATHS}" CACHE PATH "Path to Container library's CMake files")

    find_package(Container REQUIRED PATHS ${CONTAINER_CMAKE_DIR} NO_DEFAULT_PATH)

    if (NOT Container_FOUND)
        message(FATAL_ERROR "Container not found. Please specify the correct path to the Container Library cmake installation.")
    endif()

    # Check if the directory exists
    if(NOT EXISTS "${CONTAINER_LIB_DIR}")
        message(FATAL_ERROR "The specified CONTAINER_LIB_DIR does not exist: ${CONTAINER_LIB_DIR}")
    endif()

    find_package(RabbitMQ-C REQUIRED CONFIG PATHS ${RABBITMQ_CMAKE_DIR})

    if (NOT RabbitMQ-C_FOUND)
        message(FATAL_ERROR "RabbitMQ-C not found. Please specify the correct path to the RabbitMQ-C cmake installation.")
    endif()

    # Set and cache the path to the RabbitMQ bin directory using RABBITMQ_CMAKE_DIR
    if(WIN32)
        # For Windows, use /bin
        set(RABBITMQ_SHRD_LIB_DIR "${RABBITMQ_CMAKE_DIR}/../../../bin" CACHE PATH "Path to the RabbitMQ-C library's bin directory")
    elseif(UNIX AND NOT APPLE)
        # For Linux, use /lib or /lib64 (adjust based on your setup)
        set(RABBITMQ_SHRD_LIB_DIR "${RABBITMQ_CMAKE_DIR}/../../" CACHE PATH "Path to the RabbitMQ-C library's bin directory")
    elseif(APPLE)
        # For macOS, use /lib or /lib64 (adjust based on your setup)
        set(RABBITMQ_SHRD_LIB_DIR "${RABBITMQ_CMAKE_DIR}/../../" CACHE PATH "Path to the RabbitMQ-C library's bin directory")
    endif()
endif()


# -------------------------------------------------------
# ---------------- RULES AND SUB PROJECTS ---------------
# -------------------------------------------------------

# include src directory
add_subdirectory(src)

# Installation rules
install(TARGETS NeTrainSimCore RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})


