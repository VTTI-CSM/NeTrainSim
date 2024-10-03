# Define the project name (NeTrainSim) and
# the programming language used (CXX for C++)
Set(NeTrainSimConsole_NAME "NeTrainSimConsole")
project(${NeTrainSimConsole_NAME} VERSION ${NeTrainSim_VERSION} LANGUAGES CXX)

# Define and find the required libraries for the project
# Find Qt version 6 and include the Core, Concurent, Xml, Network components
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Concurrent Xml Network Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Concurrent Xml Network Sql)

# Include directories for NeTrainSimCore
# These include directories should point to where the headers are installed or located
include_directories(${CMAKE_SOURCE_DIR}/src/NeTrainSim ${CMAKE_BINARY_DIR}/include)

# Generate the VersionConfig.h file from a template
configure_file(${CMAKE_SOURCE_DIR}/src/NeTrainSim/VersionConfig.h.in
               ${CMAKE_BINARY_DIR}/include/VersionConfig.h @ONLY)

# Define the executable target for the project,
# listing the required source and header files
add_executable(${NeTrainSimConsole_NAME}
    errorHandler.h errorhandler.cpp
    main.cpp)

# Ensure that NeTrainSimCore is built first by specifying it as a dependency
add_dependencies(${NeTrainSimConsole_NAME} ${NETRAINSIM_CORE_NAME})

# Link required libraries to the executable target
target_link_libraries(${NeTrainSimConsole_NAME} PRIVATE
    NeTrainSimCore
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Sql
)

# Set compiler options for different build types using generator expressions
target_compile_options(${NeTrainSimConsole_NAME} PRIVATE
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

# Ensure the NeTrainSimCore DLL is copied to the output directory
add_custom_command(TARGET ${NeTrainSimConsole_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying NeTrainSimCore DLL from $<TARGET_FILE:NeTrainSimCore> to $<TARGET_FILE_DIR:${NeTrainSimConsole_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${NETRAINSIM_CORE_NAME}> $<TARGET_FILE_DIR:${NeTrainSimConsole_NAME}>
)

# Conditionally include Container library only if BUILD_SERVER is ON
if(BUILD_SERVER)
    target_link_libraries(${NeTrainSimConsole_NAME} PRIVATE Container::Container)
    target_include_directories(${NeTrainSimConsole_NAME} PRIVATE ${CONTAINER_INCLUDE_DIRS})
    target_compile_definitions(${NeTrainSimConsole_NAME} PRIVATE BUILD_SERVER_ENABLED)

    # Ensure the NeTrainSimCore and container DLLs are copied to the output directory
    add_custom_command(TARGET ${NeTrainSimConsole_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Container::Container> $<TARGET_FILE_DIR:${NeTrainSimConsole_NAME}>  # Copy container.dll
    )
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Include the GNU installation directories,
# for compatibility with GNU install conventions
include(GNUInstallDirs)

# Install the compiled target (the executable)
# to the specified directories
install(TARGETS ${NeTrainSimConsole_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
)
