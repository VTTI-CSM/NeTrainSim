# Define the minimum required CMake version
cmake_minimum_required(VERSION 3.24)

# Define the project name and version
project(NeTrainSimInstaller VERSION ${NeTrainSim_VERSION} LANGUAGES NONE)

# Include the GNU install directories for standardized paths
include(GNUInstallDirs)

# Options for optional components
option(INSTALL_CONSOLE "Install NeTrainSimConsole" ON)
option(INSTALL_GUI "Install NeTrainSimGUI" ON)
option(INSTALL_SERVER "Install NeTrainSimServer" ON)

# Ensure NeTrainSimCore is always installed
set(INSTALL_CORE ON)

# Install NeTrainSimCore (mandatory)
install(TARGETS NeTrainSimCore
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
)

# Conditionally install NeTrainSimConsole
if(INSTALL_CONSOLE)
    install(TARGETS NeTrainSimConsole
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
    )
endif()

# Conditionally install NeTrainSimGUI
if(INSTALL_GUI)
    install(TARGETS NeTrainSimGUI
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
    )

    # Include KDReports libraries
    if(WIN32)
        if(NOT EXISTS "${KDREPORTS_DIR}/../bin/KDReports.dll")
            message(WARNING "KDReports.dll not found at ${KDREPORTS_DIR}/../bin. Please verify the path.")
        else()
            install(FILES "${KDREPORTS_DIR}/../bin/KDReports.dll"
                DESTINATION ${CMAKE_INSTALL_BINDIR}
            )
        endif()
    elseif(APPLE)
        file(GLOB KDREPORTS_LIBRARIES "${KDREPORTS_DIR}/../../../lib/libkdreports-qt6.dylib")

        if(KDREPORTS_LIBRARIES)
            install(FILES ${KDREPORTS_LIBRARIES}
                DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
        else()
            message(WARNING "KDReports.dylib not found at ${KDREPORTS_DIR}/../../../lib. Please verify the path.")
        endif()
    elseif(UNIX)
        file(GLOB KDREPORTS_LIBRARIES "${KDREPORTS_DIR}/../../../lib/libkdreports-qt6.so*")

        if(KDREPORTS_LIBRARIES)
            foreach(LIB_FILE ${KDREPORTS_LIBRARIES})
                message(STATUS "Found KDReports library: ${LIB_FILE}")
            endforeach()

            install(FILES ${KDREPORTS_LIBRARIES}
                DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
        else()
            message(WARNING "No KDReports library files found in ${KDREPORTS_DIR}/../../../lib. Please verify the path.")
        endif()
    endif()
endif()

# Conditionally install NeTrainSimServer
if(INSTALL_SERVER)
    install(TARGETS NeTrainSimServer
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
    )

    # Include Container libraries
    if(WIN32)
        if(NOT EXISTS "${CONTAINER_BIN_DIR}/Container.dll")
            message(WARNING "Container.dll not found at ${CONTAINER_BIN_DIR}. Please verify the path.")
        else()
            install(FILES "${CONTAINER_BIN_DIR}/Container.dll"
                DESTINATION ${CMAKE_INSTALL_BINDIR}
            )
        endif()
    elseif(APPLE)
        if(NOT EXISTS "${CONTAINER_BIN_DIR}/libContainer.dylib")
            message(WARNING "libContainer.dylib not found at ${CONTAINER_BIN_DIR}. Please verify the path.")
        else()
            install(FILES "${CONTAINER_BIN_DIR}/libContainer.dylib"
                DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
        endif()
    elseif(UNIX)
        if(NOT EXISTS "${CONTAINER_BIN_DIR}/libContainer.so")
            message(WARNING "libContainer.so not found at ${CONTAINER_BIN_DIR}. Please verify the path.")
        else()
            install(FILES "${CONTAINER_BIN_DIR}/libContainer.so"
                DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
        endif()
    endif()

    # Include RabbitMQ-C libraries
    if(WIN32)
        if(NOT EXISTS "${RABBITMQ_SHRD_LIB_DIR}/rabbitmq.4.dll")
            message(WARNING "rabbitmq.4.dll not found at ${RABBITMQ_BIN_DIR}. Please verify the path.")
        else()
            install(FILES "${RABBITMQ_SHRD_LIB_DIR}/rabbitmq.4.dll"
                DESTINATION ${CMAKE_INSTALL_BINDIR}
            )
        endif()
    elseif(APPLE)
        # Dynamically find the RabbitMQ dynamic library
        file(GLOB RABBITMQ_LIBRARIES "${RABBITMQ_SHRD_LIB_DIR}/librabbitmq*.dylib")

        if(RABBITMQ_LIBRARIES)
            install(FILES ${RABBITMQ_LIBRARIES}
                DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
        else()
            message(WARNING "No RabbitMQ libraries found in ${RABBITMQ_SHRD_LIB_DIR}. Please verify the path.")
        endif()
    elseif(UNIX)
        # Dynamically find all RabbitMQ shared libraries
        file(GLOB RABBITMQ_LIBRARIES "${RABBITMQ_SHRD_LIB_DIR}/librabbitmq.so*")

        if(RABBITMQ_LIBRARIES)
            foreach(LIB_FILE ${RABBITMQ_LIBRARIES})
                message(STATUS "Found RabbitMQ library: ${LIB_FILE}")
            endforeach()

            # Install all discovered RabbitMQ shared libraries
            install(FILES ${RABBITMQ_LIBRARIES}
                DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
        else()
            message(WARNING "No RabbitMQ libraries found in ${RABBITMQ_SHRD_LIB_DIR}. Please verify the path.")
        endif()
    endif()
endif()


# Define a custom target for the installer
add_custom_target(NeTrainSimInstaller ALL
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package
)

# Set CPack configurations
set(CPACK_PACKAGE_NAME "NeTrainSim")
set(CPACK_PACKAGE_VERSION ${NeTrainSim_VERSION})
set(CPACK_PACKAGE_CONTACT "ahmedaredah@vt.edu")
set(CPACK_PACKAGE_VENDOR "Virginia Tech Transportation Institute")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Installer for NeTrainSim and its components.")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "NeTrainSim-${NeTrainSim_VERSION}")

# Define which components are available for installation
set(CPACK_COMPONENTS_ALL Core Console GUI Server)

# Define mandatory and optional components
set(CPACK_COMPONENT_CORE_DISPLAY_NAME "Core (Mandatory)")
set(CPACK_COMPONENT_CORE_DESCRIPTION "The core component of NeTrainSim.")
set(CPACK_COMPONENT_CORE_REQUIRED TRUE)

set(CPACK_COMPONENT_CONSOLE_DISPLAY_NAME "Console")
set(CPACK_COMPONENT_CONSOLE_DESCRIPTION "The console component for NeTrainSim.")
set(CPACK_COMPONENT_CONSOLE_DEPENDS Core)

set(CPACK_COMPONENT_GUI_DISPLAY_NAME "GUI")
set(CPACK_COMPONENT_GUI_DESCRIPTION "The graphical user interface for NeTrainSim.")
set(CPACK_COMPONENT_GUI_DEPENDS Core)

set(CPACK_COMPONENT_SERVER_DISPLAY_NAME "Server")
set(CPACK_COMPONENT_SERVER_DESCRIPTION "The server component for NeTrainSim.")
set(CPACK_COMPONENT_SERVER_DEPENDS Core)

# Platform-specific CPack configurations
if(WIN32)
    # Add NSIS generator for Windows
    set(CPACK_GENERATOR "TGZ;NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "NeTrainSim Installer")
    set(CPACK_NSIS_PACKAGE_NAME "NeTrainSim")
    set(CPACK_NSIS_CONTACT "ahmedaredah@vt.edu")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
elseif(APPLE)
    # Add DragNDrop generator for macOS
    set(CPACK_GENERATOR "TGZ;DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "NeTrainSim Installer")
    # set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/resources/background.png") # Optional background image
else()
    # Add DEB, RPM, and TGZ generators for Linux
    set(CPACK_GENERATOR "TGZ;DEB;RPM")

    # Configuration for .deb packages
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "ahmedaredah@vt.edu")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-base-dev, libstdc++6")

    # Configuration for .rpm packages
    set(CPACK_RPM_PACKAGE_REQUIRES "qt6-base, libstdc++")
endif()

# Include CPack for packaging
include(CPack)
