# Define the project name (NeTrainSim) and
# the programming language used (CXX for C++)
set(NETRAINSIM_SERVER_NAME "NeTrainSimServer" CACHE STRING "NeTrainSimServer name" FORCE)
project(${NETRAINSIM_SERVER_NAME} VERSION ${NeTrainSim_VERSION} LANGUAGES CXX)

# Define and find the required libraries for the project
# Find Qt version 6 and include the Core, Concurent, Xml, Network components
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Concurrent Xml Network Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Concurrent Xml Network Sql)


# Include directories for NeTrainSimServer
# These include directories should point to where the headers are installed or located
include_directories(${CMAKE_SOURCE_DIR}/src/NeTrainSim ${CMAKE_BINARY_DIR}/include)

# Generate the VersionConfig.h file from a template
configure_file(${CMAKE_SOURCE_DIR}/src/NeTrainSimServer/VersionConfig.h.in
               ${CMAKE_BINARY_DIR}/include/VersionConfig.h @ONLY)

# Define the executable target for the project,
# listing the required source and header files
add_executable(${NETRAINSIM_SERVER_NAME}
    SimulationServer.h simulationserver.cpp
    main.cpp
)

# Ensure that NeTrainSimCore is built first by specifying it as a dependency
add_dependencies(${NETRAINSIM_SERVER_NAME} ${NETRAINSIM_CORE_NAME})

# Link required libraries to the executable target
target_link_libraries(${NETRAINSIM_SERVER_NAME}
    NeTrainSimCore
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Sql
    Container::Container
    rabbitmq::rabbitmq
)

# Include Container library
target_include_directories(${NETRAINSIM_CORE_NAME} PRIVATE ${CONTAINER_INCLUDE_DIRS})
target_compile_definitions(${NETRAINSIM_CORE_NAME} PRIVATE BUILD_SERVER_ENABLED)

# Set compiler options for different build types using generator expressions
target_compile_options(${NETRAINSIM_SERVER_NAME} PRIVATE
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

# Ensure the NeTrainSimCore and container DLLs are copied to the output directory
add_custom_command(TARGET ${NETRAINSIM_SERVER_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${NETRAINSIM_CORE_NAME}> $<TARGET_FILE_DIR:${NETRAINSIM_SERVER_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Container::Container> $<TARGET_FILE_DIR:${NETRAINSIM_SERVER_NAME}>  # Copy container.dll
)

# Ensure the NeTrainSim DLL is copied to the output directory
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(WIN32)
        # Copy Windows DLLs in Release mode
        add_custom_command(TARGET ${NETRAINSIM_SERVER_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CONTAINER_BIN_DIR}/container.dll"
                "$<TARGET_FILE_DIR:${NETRAINSIM_SERVER_NAME}>/container.dll"
        )
        add_custom_command(TARGET ${NETRAINSIM_SERVER_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${RABBITMQ_BIN_DIR}/rabbitmq.4.dll"
                "$<TARGET_FILE_DIR:${NETRAINSIM_SERVER_NAME}>/rabbitmq.4.dll"
        )
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(WIN32)
        # Copy Windows debug DLLs
        add_custom_command(TARGET ${NETRAINSIM_SERVER_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CONTAINER_BIN_DIR}/containerd.dll"
                "$<TARGET_FILE_DIR:${NETRAINSIM_SERVER_NAME}>/containerd.dll"
        )
        add_custom_command(TARGET ${NETRAINSIM_SERVER_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${RABBITMQ_BIN_DIR}/rabbitmq.4.dll"
                "$<TARGET_FILE_DIR:${NETRAINSIM_SERVER_NAME}>/rabbitmq.4.dll"
        )
    endif()
endif()


include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Include the GNU installation directories,
# for compatibility with GNU install conventions
include(GNUInstallDirs)

# Install the compiled target (the executable)
# to the specified directories
install(TARGETS ${NeTrainSim_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
)
