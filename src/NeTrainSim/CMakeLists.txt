# Define the project name (ShipNetSimCore) and the programming language used (CXX for C++)
set(NETRAINSIM_CORE_NAME "NeTrainSimCore" CACHE STRING "Project CORE name" FORCE)
project(${NETRAINSIM_CORE_NAME} VERSION ${NeTrainSim_VERSION} LANGUAGES CXX)

# Define and find the required libraries for the project
# Find Qt version 6 and include the Core, Concurrent, Xml, Network components
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Concurrent Xml Network Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Concurrent Xml Network Sql)

# Define the shared library target for the project,
# listing the required source and header files
add_library(${NETRAINSIM_CORE_NAME} SHARED
    simulatorapi.cpp
    simulator.cpp
    network/netlink.cpp
    network/netnode.cpp
    network/netsignal.cpp
    network/netsignalgroupcontrollerwithqueuing.cpp
    network/netsignalgroupcontroller.cpp
    network/readwritenetwork.cpp
    traindefinition/car.cpp
    traindefinition/energyconsumption.cpp
    traindefinition/locomotive.cpp
    traindefinition/train.cpp
    traindefinition/traincomponent.cpp
    traindefinition/trainslist.cpp
    traindefinition/battery.cpp
    traindefinition/tank.cpp
    util/jsonmanager.cpp
    util/logger.cpp
    util/xmlmanager.cpp
    util/csvmanager.cpp
    simulatorworker.cpp

    export.h
    simulatorapi.h
    simulator.h
    network/netlink.h
    network/netnode.h
    network/netsignal.h
    network/netsignalgroupcontrollerwithqueuing.h
    network/netsignalgroupcontroller.h
    network/network.h
    network/readwritenetwork.h
    traindefinition/trainscommon.h
    traindefinition/car.h
    traindefinition/energyconsumption.h
    traindefinition/locomotive.h
    traindefinition/train.h
    traindefinition/traincomponent.h
    traindefinition/traintypes.h
    traindefinition/trainslist.h
    traindefinition/battery.h
    traindefinition/tank.h
    util/error.h
    util/jsonmanager.h
    util/list.h
    util/logger.h
    util/map.h
    util/utils.h
    util/vector.h
    util/xmlmanager.h
    util/csvmanager.h
    simulatorworker.h

)

# Define SHIPNETSIM_LIBRARY during compilation to activate symbol export
target_compile_definitions(${NETRAINSIM_CORE_NAME} PRIVATE NETRAINSIMCORE_LIBRARY)

# Include directories and link libraries are inherited from the parent scope
target_include_directories(${NETRAINSIM_CORE_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src/NeTrainSim
    ${CMAKE_BINARY_DIR}
)

# Link the Boost and Qt libraries to the shared library target
target_link_libraries(${NETRAINSIM_CORE_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Sql
)

# Conditionally include Container library only if BUILD_SERVER is ON
if(BUILD_SERVER)
    target_link_libraries(${NETRAINSIM_CORE_NAME} PRIVATE Container::Container)
    target_include_directories(${NETRAINSIM_CORE_NAME} PRIVATE ${CONTAINER_INCLUDE_DIRS})
    target_compile_definitions(${NETRAINSIM_CORE_NAME} PRIVATE BUILD_SERVER_ENABLED)

    # Ensure the NeTrainSimCore and container DLLs are copied to the output directory
    add_custom_command(TARGET ${NETRAINSIM_CORE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Container::Container> $<TARGET_FILE_DIR:${NETRAINSIM_CORE_NAME}>  # Copy container.dll
    )
endif()

# Set compiler options for different build types using generator expressions
target_compile_options(${NETRAINSIM_CORE_NAME} PRIVATE
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

# Set configuration file
configure_file(VersionConfig.h.in VersionConfig.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Include the GNU installation directories,
# for compatibility with GNU install conventions
include(GNUInstallDirs)

# Install the compiled target (the shared library)
# to the specified directories
install(TARGETS ${NETRAINSIM_CORE_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
)

# Install header files
install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/NeTrainSim/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/NeTrainSim
    FILES_MATCHING PATTERN "*.h")
