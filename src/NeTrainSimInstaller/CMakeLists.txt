cmake_minimum_required(VERSION 3.24)

project(NeTrainSimInstaller VERSION ${NeTrainSim_VERSION})

# ========================================
# Helper function to filter debug DLLs and ensure release versions exist
# ========================================
function(filter_debug_dlls input_dll_list output_dll_list)
    set(filtered_dlls "")
    foreach(dll_path ${${input_dll_list}})
        get_filename_component(dll_name "${dll_path}" NAME)
        # Check if this is a debug DLL (ends with d.dll or _d.dll)
        if(dll_name MATCHES ".*[_]?d\\.dll$")
            # Extract the base name without the debug suffix
            if(dll_name MATCHES ".*_d\\.dll$")
                string(REGEX REPLACE "_d\\.dll$" ".dll" release_dll_name "${dll_name}")
            else()
                string(REGEX REPLACE "d\\.dll$" ".dll" release_dll_name "${dll_name}")
            endif()

            get_filename_component(dll_dir "${dll_path}" DIRECTORY)
            set(release_dll_path "${dll_dir}/${release_dll_name}")
            # Only exclude debug DLL if release version exists
            if(EXISTS "${release_dll_path}")
                message(STATUS "Excluding debug DLL (release version exists): ${dll_name}")
            else()
                message(WARNING "Debug DLL found but no release version: ${dll_name} - including anyway")
                list(APPEND filtered_dlls "${dll_path}")
            endif()
        else()
            # Include non-debug DLLs
            list(APPEND filtered_dlls "${dll_path}")
        endif()
    endforeach()
    set(${output_dll_list} "${filtered_dlls}" PARENT_SCOPE)
endfunction()

# Check if building of installer is enabled
if(BUILD_INSTALLER)

    # Define NeTrainSimInstaller as a custom target
    add_custom_target(NeTrainSimInstaller ALL
        COMMENT "Creating an installer for NeTrainSim."
    )

# Ensure that NeTrainSimCore and NeTrainSimConsole are built first
add_dependencies(NeTrainSimInstaller NeTrainSimCore NeTrainSimConsole)

if(BUILD_GUI)
    add_dependencies(NeTrainSimInstaller NeTrainSimGUI)
endif()

if(BUILD_SERVER)
    add_dependencies(NeTrainSimInstaller NeTrainSimServer NeTrainSimRabbitMQConfig)
endif()

# ========================================
# SECTION B: Install Targets
# ========================================

# NeTrainSimCore shared library
install(TARGETS NeTrainSimCore
    RUNTIME DESTINATION bin
    COMPONENT NeTrainSimCore_COMP
    LIBRARY DESTINATION lib
    COMPONENT NeTrainSimCore_COMP
    ARCHIVE DESTINATION lib/static
    COMPONENT NeTrainSimCore_COMP)

# NeTrainSimConsole executable (always)
install(TARGETS NeTrainSimConsole
    RUNTIME DESTINATION bin
    COMPONENT NeTrainSimConsole_COMP)

# NeTrainSimGUI executable (conditional)
if(BUILD_GUI)
    install(TARGETS NeTrainSimGUI
        RUNTIME DESTINATION bin
        COMPONENT NeTrainSimGUI_COMP)
endif()

# NeTrainSimServer + NeTrainSimRabbitMQConfig (conditional)
if(BUILD_SERVER)
    install(TARGETS NeTrainSimServer
        RUNTIME DESTINATION bin
        COMPONENT NeTrainSimServer_COMP)

    install(TARGETS NeTrainSimRabbitMQConfig
        RUNTIME DESTINATION bin
        COMPONENT NeTrainSimServer_COMP)
endif()

# ========================================
# SECTION C: Windows DLL Bundling
# ========================================
if(WIN32)
    # Get Qt parent directory for plugins and libs
    get_filename_component(QT_PARENT_DIR ${QT_BIN_DIR} DIRECTORY)

    # ========================================
    # VALIDATION: Check critical Qt DLLs exist
    # ========================================
    set(REQUIRED_QT_DLLS
        "${QT_BIN_DIR}/Qt6Core.dll"
        "${QT_BIN_DIR}/Qt6Network.dll"
        "${QT_BIN_DIR}/Qt6Sql.dll"
        "${QT_BIN_DIR}/Qt6Xml.dll"
        "${QT_BIN_DIR}/Qt6Concurrent.dll"
    )

    foreach(QT_DLL ${REQUIRED_QT_DLLS})
        if(NOT EXISTS "${QT_DLL}")
            message(FATAL_ERROR "CRITICAL: Required Qt DLL not found: ${QT_DLL}")
        endif()
    endforeach()

    # ========================================
    # VALIDATION: Check Qt plugins exist
    # ========================================
    if(NOT EXISTS "${QT_PARENT_DIR}/plugins/platforms/")
        message(FATAL_ERROR "CRITICAL: Qt platforms plugin directory not found: ${QT_PARENT_DIR}/plugins/platforms/")
    endif()

    if(NOT EXISTS "${QT_PARENT_DIR}/plugins/imageformats/")
        message(FATAL_ERROR "CRITICAL: Qt imageformats plugin directory not found: ${QT_PARENT_DIR}/plugins/imageformats/")
    endif()

    # ========================================
    # CORE Qt6 DLL files (ESSENTIAL FOR ALL APPS)
    # ========================================
    filter_debug_dlls(REQUIRED_QT_DLLS FILTERED_QT_DLLS)
    install(
        FILES ${FILTERED_QT_DLLS}
        DESTINATION bin
        COMPONENT NeTrainSimCore_COMP
    )

    # ========================================
    # GUI-specific Qt6 DLL files
    # ========================================
    if(BUILD_GUI)
        set(REQUIRED_QT_GUI_DLLS
            "${QT_BIN_DIR}/Qt6Gui.dll"
            "${QT_BIN_DIR}/Qt6Widgets.dll"
        )

        foreach(QT_GUI_DLL ${REQUIRED_QT_GUI_DLLS})
            if(NOT EXISTS "${QT_GUI_DLL}")
                message(FATAL_ERROR "CRITICAL: Required Qt GUI DLL not found: ${QT_GUI_DLL}")
            endif()
        endforeach()

        # Optional GUI DLLs (warn if missing but don't fail)
        set(OPTIONAL_QT_GUI_DLLS
            "${QT_BIN_DIR}/Qt6OpenGL.dll"
            "${QT_BIN_DIR}/Qt6OpenGLWidgets.dll"
            "${QT_BIN_DIR}/Qt6PrintSupport.dll"
            "${QT_BIN_DIR}/Qt6Qml.dll"
            "${QT_BIN_DIR}/Qt6Charts.dll"
        )

        set(ALL_QT_GUI_DLLS ${REQUIRED_QT_GUI_DLLS})
        foreach(OPTIONAL_DLL ${OPTIONAL_QT_GUI_DLLS})
            if(EXISTS "${OPTIONAL_DLL}")
                list(APPEND ALL_QT_GUI_DLLS "${OPTIONAL_DLL}")
            else()
                message(WARNING "Optional Qt GUI DLL not found: ${OPTIONAL_DLL}")
            endif()
        endforeach()

        filter_debug_dlls(ALL_QT_GUI_DLLS FILTERED_QT_GUI_DLLS)
        install(
            FILES ${FILTERED_QT_GUI_DLLS}
            DESTINATION bin
            COMPONENT NeTrainSimGUI_COMP
        )
    endif()

    # ========================================
    # SERVER-specific Qt6 GUI DLLs (for RabbitMQConfig GUI)
    # ========================================
    if(BUILD_SERVER AND NOT BUILD_GUI)
        set(SERVER_QT_GUI_DLLS
            "${QT_BIN_DIR}/Qt6Gui.dll"
            "${QT_BIN_DIR}/Qt6Widgets.dll"
        )
        foreach(QT_GUI_DLL ${SERVER_QT_GUI_DLLS})
            if(NOT EXISTS "${QT_GUI_DLL}")
                message(FATAL_ERROR "CRITICAL: Required Qt GUI DLL for NeTrainSimRabbitMQConfig not found: ${QT_GUI_DLL}")
            endif()
        endforeach()
        filter_debug_dlls(SERVER_QT_GUI_DLLS FILTERED_SERVER_QT_GUI_DLLS)
        install(
            FILES ${FILTERED_SERVER_QT_GUI_DLLS}
            DESTINATION bin
            COMPONENT NeTrainSimServer_COMP
        )
    endif()

    # ========================================
    # CRITICAL: Qt6 PLUGINS (REQUIRED FOR APPS TO START)
    # ========================================
    # Platform plugins (ESSENTIAL)
    install(
        DIRECTORY "${QT_PARENT_DIR}/plugins/platforms/"
        DESTINATION bin/plugins/platforms
        COMPONENT NeTrainSimCore_COMP
        FILES_MATCHING PATTERN "*.dll"
    )

    # Image format plugins (CRITICAL)
    install(
        DIRECTORY "${QT_PARENT_DIR}/plugins/imageformats/"
        DESTINATION bin/plugins/imageformats
        COMPONENT NeTrainSimCore_COMP
        FILES_MATCHING PATTERN "*.dll"
    )

    # SQL drivers (REQUIRED)
    if(NOT EXISTS "${QT_PARENT_DIR}/plugins/sqldrivers/")
        message(FATAL_ERROR "CRITICAL: Qt SQL drivers plugin directory not found: ${QT_PARENT_DIR}/plugins/sqldrivers/")
    endif()
    install(
        DIRECTORY "${QT_PARENT_DIR}/plugins/sqldrivers/"
        DESTINATION bin/plugins/sqldrivers
        COMPONENT NeTrainSimCore_COMP
        FILES_MATCHING PATTERN "*.dll"
    )

    # Optional plugin directories (warn if missing)
    set(OPTIONAL_PLUGIN_DIRS "iconengines" "styles" "tls")
    foreach(PLUGIN_DIR ${OPTIONAL_PLUGIN_DIRS})
        if(EXISTS "${QT_PARENT_DIR}/plugins/${PLUGIN_DIR}/")
            install(
                DIRECTORY "${QT_PARENT_DIR}/plugins/${PLUGIN_DIR}/"
                DESTINATION bin/plugins/${PLUGIN_DIR}
                COMPONENT NeTrainSimCore_COMP
                FILES_MATCHING PATTERN "*.dll"
            )
        else()
            message(WARNING "Optional Qt plugin directory not found: ${QT_PARENT_DIR}/plugins/${PLUGIN_DIR}/")
        endif()
    endforeach()

    # ========================================
    # KDReports DLL (if BUILD_GUI)
    # ========================================
    if(BUILD_GUI)
        if(DEFINED KDREPORTS_DIR)
            if(EXISTS "${KDREPORTS_DIR}/../bin/KDReports.dll")
                install(FILES "${KDREPORTS_DIR}/../bin/KDReports.dll"
                    DESTINATION bin
                    COMPONENT NeTrainSimGUI_COMP)
                message(STATUS "Found KDReports.dll at: ${KDREPORTS_DIR}/../bin/KDReports.dll")
            else()
                # Try alternate known paths
                file(GLOB KDREPORTS_DLLS "${KDREPORTS_DIR}/../../../bin/KDReports*.dll")
                if(KDREPORTS_DLLS)
                    filter_debug_dlls(KDREPORTS_DLLS FILTERED_KDREPORTS_DLLS)
                    install(FILES ${FILTERED_KDREPORTS_DLLS}
                        DESTINATION bin
                        COMPONENT NeTrainSimGUI_COMP)
                else()
                    message(WARNING "KDReports.dll not found. Please verify KDREPORTS_DIR.")
                endif()
            endif()
        endif()
    endif()

    # ========================================
    # SERVER-SPECIFIC DLL FILES (REQUIRED IF BUILD_SERVER=ON)
    # ========================================
    if(BUILD_SERVER)
        # Derive CONTAINER_BIN_DIR if not already set
        if(NOT DEFINED CONTAINER_BIN_DIR AND DEFINED CONTAINER_CMAKE_DIR)
            get_filename_component(CONTAINER_BIN_DIR "${CONTAINER_CMAKE_DIR}/../../bin" ABSOLUTE)
        endif()

        # Container library DLLs
        set(CONTAINER_FOUND FALSE)
        if(DEFINED CONTAINER_BIN_DIR AND EXISTS "${CONTAINER_BIN_DIR}")
            file(GLOB CONTAINER_DLLS
                "${CONTAINER_BIN_DIR}/container*.dll"
                "${CONTAINER_BIN_DIR}/Container*.dll"
            )
            if(CONTAINER_DLLS)
                message(STATUS "Found Container DLLs: ${CONTAINER_DLLS}")
                filter_debug_dlls(CONTAINER_DLLS FILTERED_CONTAINER_DLLS)
                install(
                    FILES ${FILTERED_CONTAINER_DLLS}
                    DESTINATION bin
                    COMPONENT NeTrainSimServer_COMP
                )
                set(CONTAINER_FOUND TRUE)
            endif()
        endif()

        if(NOT CONTAINER_FOUND)
            message(FATAL_ERROR "CRITICAL: Container DLLs not found! CONTAINER_BIN_DIR=${CONTAINER_BIN_DIR}")
        endif()

        # RabbitMQ DLL
        set(RABBITMQ_FOUND FALSE)
        if(DEFINED RABBITMQ_SHRD_LIB_DIR AND EXISTS "${RABBITMQ_SHRD_LIB_DIR}")
            file(GLOB RABBITMQ_DLLS
                "${RABBITMQ_SHRD_LIB_DIR}/rabbitmq*.dll"
                "${RABBITMQ_SHRD_LIB_DIR}/librabbitmq*.dll"
            )
            if(RABBITMQ_DLLS)
                message(STATUS "Found RabbitMQ DLLs: ${RABBITMQ_DLLS}")
                filter_debug_dlls(RABBITMQ_DLLS FILTERED_RABBITMQ_DLLS)
                install(
                    FILES ${FILTERED_RABBITMQ_DLLS}
                    DESTINATION bin
                    COMPONENT NeTrainSimServer_COMP
                )
                set(RABBITMQ_FOUND TRUE)
            endif()
        endif()

        if(NOT RABBITMQ_FOUND)
            message(FATAL_ERROR "CRITICAL: RabbitMQ DLLs not found! RABBITMQ_SHRD_LIB_DIR=${RABBITMQ_SHRD_LIB_DIR}")
        endif()
    endif()

    # ========================================
    # VISUAL C++ RUNTIME LIBRARIES
    # ========================================
    set(VCREDIST_SEARCH_PATHS
        "C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Redist/MSVC"
        "C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Redist/MSVC"
        "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Redist/MSVC"
        "C:/Program Files/Microsoft Visual Studio/2019/Community/VC/Redist/MSVC"
        "C:/Program Files (x86)/Microsoft Visual Studio/2022/Professional/VC/Redist/MSVC"
        "C:/Program Files (x86)/Microsoft Visual Studio/2019/Professional/VC/Redist/MSVC"
    )

    set(VCREDIST_FOUND FALSE)
    set(VCREDIST_INSTALLER_PATH "")
    foreach(VCREDIST_BASE ${VCREDIST_SEARCH_PATHS})
        if(EXISTS "${VCREDIST_BASE}")
            file(GLOB VCREDIST_SUBDIRS "${VCREDIST_BASE}/*")
            if(VCREDIST_SUBDIRS)
                list(GET VCREDIST_SUBDIRS -1 LATEST_VCREDIST)
                set(VCREDIST_PATHS
                    "${LATEST_VCREDIST}/x64/Microsoft.VC143.CRT"
                    "${LATEST_VCREDIST}/x64/Microsoft.VC142.CRT"
                    "${LATEST_VCREDIST}/x64/Microsoft.VC141.CRT"
                )
                # Look for redistributable installer
                file(GLOB VCREDIST_INSTALLER "${LATEST_VCREDIST}/vcredist_x64.exe")
                if(VCREDIST_INSTALLER)
                    set(VCREDIST_INSTALLER_PATH ${VCREDIST_INSTALLER})
                endif()

                foreach(VCREDIST_PATH ${VCREDIST_PATHS})
                    if(EXISTS "${VCREDIST_PATH}")
                        file(GLOB VCREDIST_DLLS "${VCREDIST_PATH}/*.dll")
                        if(VCREDIST_DLLS)
                            message(STATUS "Found VC++ Redistributable DLLs in: ${VCREDIST_PATH}")
                            filter_debug_dlls(VCREDIST_DLLS FILTERED_VCREDIST_DLLS)
                            install(
                                FILES ${FILTERED_VCREDIST_DLLS}
                                DESTINATION bin
                                COMPONENT NeTrainSimCore_COMP
                            )
                            set(VCREDIST_FOUND TRUE)
                            break()
                        endif()
                    endif()
                endforeach()
                if(VCREDIST_FOUND)
                    break()
                endif()
            endif()
        endif()
    endforeach()

    if(NOT VCREDIST_FOUND)
        message(WARNING "Visual C++ Redistributable DLLs not found. Target machines may need VC++ Redist installed.")
    endif()

    # Install VC++ Redistributable installer if found
    if(VCREDIST_INSTALLER_PATH)
        message(STATUS "Found VC++ Redistributable installer: ${VCREDIST_INSTALLER_PATH}")
        install(FILES ${VCREDIST_INSTALLER_PATH}
            DESTINATION .
            COMPONENT NeTrainSimCore_COMP)
    endif()

    # ========================================
    # OPTIONAL: ICU DLLs
    # ========================================
    file(GLOB ICU_DLLS "${QT_BIN_DIR}/icu*.dll")
    if(ICU_DLLS)
        message(STATUS "Found ICU DLLs: ${ICU_DLLS}")
        filter_debug_dlls(ICU_DLLS FILTERED_ICU_DLLS)
        install(
            FILES ${FILTERED_ICU_DLLS}
            DESTINATION bin
            COMPONENT NeTrainSimCore_COMP
        )
    else()
        message(STATUS "ICU DLLs not found (this is usually OK)")
    endif()

    # ========================================
    # SUMMARY MESSAGE
    # ========================================
    message(STATUS "=== Windows DLL Detection Summary ===")
    if(BUILD_SERVER)
        message(STATUS "Container: ${CONTAINER_FOUND}")
        message(STATUS "RabbitMQ: ${RABBITMQ_FOUND}")
    endif()
    message(STATUS "VC++ Redistributable: ${VCREDIST_FOUND}")
    message(STATUS "============================")

# ========================================
# SECTION D: Linux Shared Library Bundling
# ========================================
elseif(UNIX AND NOT APPLE)

    # Collect all library search directories for dependency resolution
    set(_LIB_SEARCH_DIRS "${QT_LIB_DIR}")
    list(APPEND _LIB_SEARCH_DIRS "/usr/local/lib")
    # Add RabbitMQ lib dir if available
    if(DEFINED RABBITMQ_CMAKE_DIR)
        get_filename_component(_RABBITMQ_LIB_DIR "${RABBITMQ_CMAKE_DIR}/../.." ABSOLUTE)
        list(APPEND _LIB_SEARCH_DIRS "${_RABBITMQ_LIB_DIR}")
    endif()
    # Add KDReports lib dir if available
    if(BUILD_GUI AND DEFINED KDREPORTS_DIR)
        get_filename_component(_KDREPORTS_LIB_DIR "${KDREPORTS_DIR}/../../../lib" ABSOLUTE)
        list(APPEND _LIB_SEARCH_DIRS "${_KDREPORTS_LIB_DIR}")
    endif()
    list(REMOVE_DUPLICATES _LIB_SEARCH_DIRS)

    # Build the DIRECTORIES string for file(GET_RUNTIME_DEPENDENCIES)
    set(_DIRS_ARGS "")
    foreach(_dir ${_LIB_SEARCH_DIRS})
        string(APPEND _DIRS_ARGS "                \"${_dir}\"\n")
    endforeach()
    message(STATUS "Library search directories for dependency resolution: ${_LIB_SEARCH_DIRS}")

    # Core dependencies (NeTrainSimConsole + NeTrainSimCore)
    install(CODE "
        file(GET_RUNTIME_DEPENDENCIES
            EXECUTABLES
                \$<TARGET_FILE:NeTrainSimConsole>
            LIBRARIES
                \$<TARGET_FILE:NeTrainSimCore>
            RESOLVED_DEPENDENCIES_VAR _resolved
            UNRESOLVED_DEPENDENCIES_VAR _unresolved
            PRE_EXCLUDE_REGEXES
                \"^libc\\\\.so\" \"^libm\\\\.so\" \"^libpthread\" \"^libdl\\\\.so\"
                \"^librt\\\\.so\" \"^ld-linux\" \"^libstdc\\\\+\\\\+\" \"^libgcc_s\"
                \"^libX\" \"^libxcb\" \"^libGL\" \"^libEGL\" \"^libdrm\"
            DIRECTORIES
${_DIRS_ARGS}        )
        foreach(dep \${_resolved})
            file(INSTALL \${dep} DESTINATION \${CMAKE_INSTALL_PREFIX}/lib
                 FOLLOW_SYMLINK_CHAIN)
        endforeach()
        if(_unresolved)
            message(STATUS \"Unresolved core dependencies (may be system libs): \${_unresolved}\")
        endif()
    " COMPONENT NeTrainSimCore_COMP)

    # GUI dependencies
    if(BUILD_GUI)
        install(CODE "
            file(GET_RUNTIME_DEPENDENCIES
                EXECUTABLES
                    \$<TARGET_FILE:NeTrainSimGUI>
                RESOLVED_DEPENDENCIES_VAR _resolved_gui
                UNRESOLVED_DEPENDENCIES_VAR _unresolved_gui
                PRE_EXCLUDE_REGEXES
                    \"^libc\\\\.so\" \"^libm\\\\.so\" \"^libpthread\" \"^libdl\\\\.so\"
                    \"^librt\\\\.so\" \"^ld-linux\" \"^libstdc\\\\+\\\\+\" \"^libgcc_s\"
                    \"^libX\" \"^libxcb\" \"^libGL\" \"^libEGL\" \"^libdrm\"
                DIRECTORIES
${_DIRS_ARGS}            )
            foreach(dep \${_resolved_gui})
                file(INSTALL \${dep} DESTINATION \${CMAKE_INSTALL_PREFIX}/lib
                     FOLLOW_SYMLINK_CHAIN)
            endforeach()
        " COMPONENT NeTrainSimGUI_COMP)

        # Fix RPATH on GUI component bundled libraries
        if(PATCHELF_EXECUTABLE)
            install(CODE "
                file(GLOB _gui_libs \"\${CMAKE_INSTALL_PREFIX}/lib/*.so*\")
                foreach(_lib \${_gui_libs})
                    if(IS_SYMLINK \"\${_lib}\")
                        continue()
                    endif()
                    execute_process(
                        COMMAND \"${PATCHELF_EXECUTABLE}\" --set-rpath \\$ORIGIN \"\${_lib}\"
                        ERROR_QUIET)
                endforeach()
            " COMPONENT NeTrainSimGUI_COMP)
        endif()
    endif()

    # Server dependencies
    if(BUILD_SERVER)
        install(CODE "
            file(GET_RUNTIME_DEPENDENCIES
                EXECUTABLES
                    \$<TARGET_FILE:NeTrainSimServer>
                    \$<TARGET_FILE:NeTrainSimRabbitMQConfig>
                RESOLVED_DEPENDENCIES_VAR _resolved_srv
                UNRESOLVED_DEPENDENCIES_VAR _unresolved_srv
                PRE_EXCLUDE_REGEXES
                    \"^libc\\\\.so\" \"^libm\\\\.so\" \"^libpthread\" \"^libdl\\\\.so\"
                    \"^librt\\\\.so\" \"^ld-linux\" \"^libstdc\\\\+\\\\+\" \"^libgcc_s\"
                    \"^libX\" \"^libxcb\" \"^libGL\" \"^libEGL\" \"^libdrm\"
                DIRECTORIES
${_DIRS_ARGS}            )
            foreach(dep \${_resolved_srv})
                file(INSTALL \${dep} DESTINATION \${CMAKE_INSTALL_PREFIX}/lib
                     FOLLOW_SYMLINK_CHAIN)
            endforeach()
        " COMPONENT NeTrainSimServer_COMP)

        # Fix RPATH on Server component bundled libraries
        if(PATCHELF_EXECUTABLE)
            install(CODE "
                file(GLOB _srv_libs \"\${CMAKE_INSTALL_PREFIX}/lib/*.so*\")
                foreach(_lib \${_srv_libs})
                    if(IS_SYMLINK \"\${_lib}\")
                        continue()
                    endif()
                    execute_process(
                        COMMAND \"${PATCHELF_EXECUTABLE}\" --set-rpath \\$ORIGIN \"\${_lib}\"
                        ERROR_QUIET)
                endforeach()
            " COMPONENT NeTrainSimServer_COMP)
        endif()
    endif()

    # ========================================
    # SECTION E: Linux Qt Plugin Bundling
    # ========================================
    if(DEFINED QT_PLUGINS_DIR)
        # Platform plugins (essential)
        install(DIRECTORY "${QT_PLUGINS_DIR}/platforms/"
            DESTINATION plugins/platforms
            COMPONENT NeTrainSimCore_COMP
            FILES_MATCHING PATTERN "*.so")

        # Image format plugins
        install(DIRECTORY "${QT_PLUGINS_DIR}/imageformats/"
            DESTINATION plugins/imageformats
            COMPONENT NeTrainSimCore_COMP
            FILES_MATCHING PATTERN "*.so")

        # SQL driver plugins
        if(EXISTS "${QT_PLUGINS_DIR}/sqldrivers/")
            install(DIRECTORY "${QT_PLUGINS_DIR}/sqldrivers/"
                DESTINATION plugins/sqldrivers
                COMPONENT NeTrainSimCore_COMP
                FILES_MATCHING PATTERN "*.so")
        endif()

        # Optional plugins
        foreach(PLUGIN_DIR "tls" "iconengines" "xcbglintegrations" "wayland-shell-integration")
            if(EXISTS "${QT_PLUGINS_DIR}/${PLUGIN_DIR}/")
                install(DIRECTORY "${QT_PLUGINS_DIR}/${PLUGIN_DIR}/"
                    DESTINATION plugins/${PLUGIN_DIR}
                    COMPONENT NeTrainSimCore_COMP
                    FILES_MATCHING PATTERN "*.so")
            endif()
        endforeach()
    else()
        message(WARNING "QT_PLUGINS_DIR not defined - Qt plugins will not be bundled")
    endif()

    # ========================================
    # SECTION F: Linux Qt Plugin Dependency Resolution
    # ========================================
    if(DEFINED QT_PLUGINS_DIR)
        install(CODE "
            # Gather source plugin .so files (not installed copies, to avoid RPATH conflicts)
            file(GLOB_RECURSE _plugin_libs
                \"${QT_PLUGINS_DIR}/platforms/*.so\"
                \"${QT_PLUGINS_DIR}/imageformats/*.so\"
                \"${QT_PLUGINS_DIR}/sqldrivers/*.so\"
                \"${QT_PLUGINS_DIR}/tls/*.so\"
                \"${QT_PLUGINS_DIR}/iconengines/*.so\"
                \"${QT_PLUGINS_DIR}/xcbglintegrations/*.so\"
                \"${QT_PLUGINS_DIR}/wayland-shell-integration/*.so\")

            if(_plugin_libs)
                file(GET_RUNTIME_DEPENDENCIES
                    LIBRARIES \${_plugin_libs}
                    RESOLVED_DEPENDENCIES_VAR _resolved_plugin_deps
                    UNRESOLVED_DEPENDENCIES_VAR _unresolved_plugin_deps
                    PRE_EXCLUDE_REGEXES
                        \"^libc\\\\.so\" \"^libm\\\\.so\" \"^libpthread\" \"^libdl\\\\.so\"
                        \"^librt\\\\.so\" \"^ld-linux\" \"^libstdc\\\\+\\\\+\" \"^libgcc_s\"
                        \"^libX\" \"^libGL\" \"^libEGL\" \"^libdrm\"
                    DIRECTORIES
${_DIRS_ARGS}                )
                foreach(dep \${_resolved_plugin_deps})
                    # Skip if already installed
                    get_filename_component(_depname \"\${dep}\" NAME)
                    if(NOT EXISTS \"\${CMAKE_INSTALL_PREFIX}/lib/\${_depname}\")
                        file(INSTALL \${dep} DESTINATION \${CMAKE_INSTALL_PREFIX}/lib
                             FOLLOW_SYMLINK_CHAIN)
                    endif()
                endforeach()
                if(_unresolved_plugin_deps)
                    message(STATUS \"Unresolved plugin deps (may be system libs): \${_unresolved_plugin_deps}\")
                endif()
            endif()

            # Bundle libxcb-cursor (needed by Qt xcb platform plugin from 6.5+)
            if(NOT EXISTS \"\${CMAKE_INSTALL_PREFIX}/lib/libxcb-cursor.so.0\")
                foreach(_xcb_search_dir /lib/x86_64-linux-gnu /usr/lib/x86_64-linux-gnu /usr/lib)
                    file(GLOB _xcb_cursor_libs \"\${_xcb_search_dir}/libxcb-cursor.so*\")
                    if(_xcb_cursor_libs)
                        foreach(_xcb_lib \${_xcb_cursor_libs})
                            file(INSTALL \${_xcb_lib} DESTINATION \${CMAKE_INSTALL_PREFIX}/lib
                                 FOLLOW_SYMLINK_CHAIN)
                        endforeach()
                        break()
                    endif()
                endforeach()
            endif()
        " COMPONENT NeTrainSimCore_COMP)
    endif()

    # ========================================
    # SECTION G: Linux ICU Libraries (Optional)
    # ========================================
    if(DEFINED QT_LIB_DIR)
        file(GLOB ICU_LIBS "${QT_LIB_DIR}/libicu*.so*")
        if(ICU_LIBS)
            message(STATUS "Found ICU libraries: ${ICU_LIBS}")
            install(FILES ${ICU_LIBS} DESTINATION lib COMPONENT NeTrainSimCore_COMP)
        else()
            message(STATUS "ICU libraries not found (this is usually OK)")
        endif()
    endif()

    # ========================================
    # SECTION H: Linux RPATH Fixing with patchelf
    # ========================================
    find_program(PATCHELF_EXECUTABLE patchelf)
    if(PATCHELF_EXECUTABLE)
        message(STATUS "Found patchelf: ${PATCHELF_EXECUTABLE}")
        install(CODE "
            # Fix RPATH on shared libraries in lib/
            file(GLOB _all_libs \"\${CMAKE_INSTALL_PREFIX}/lib/*.so*\")
            foreach(_lib \${_all_libs})
                if(IS_SYMLINK \"\${_lib}\")
                    continue()
                endif()
                execute_process(
                    COMMAND \"${PATCHELF_EXECUTABLE}\" --set-rpath \\$ORIGIN \"\${_lib}\"
                    ERROR_QUIET)
            endforeach()

            # Fix RPATH on Qt plugins in plugins/<type>/
            file(GLOB_RECURSE _qt_plugins \"\${CMAKE_INSTALL_PREFIX}/plugins/*.so\")
            foreach(_plugin \${_qt_plugins})
                execute_process(
                    COMMAND \"${PATCHELF_EXECUTABLE}\" --set-rpath \\$ORIGIN/../../lib \"\${_plugin}\"
                    ERROR_QUIET)
            endforeach()
        " COMPONENT NeTrainSimCore_COMP)
    else()
        message(WARNING "patchelf not found - bundled .so RPATHs will not be fixed for portability. Install with: sudo apt install patchelf")
    endif()

    # ========================================
    # LINUX: Summary
    # ========================================
    message(STATUS "=== Linux Library Detection Summary ===")
    message(STATUS "QT_LIB_DIR: ${QT_LIB_DIR}")
    message(STATUS "QT_PLUGINS_DIR: ${QT_PLUGINS_DIR}")
    message(STATUS "=======================================")

endif() # WIN32 / UNIX

# ========================================
# SECTION I: Data Files (all platforms)
# ========================================
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/src/data/")
    message(FATAL_ERROR "CRITICAL: Data directory not found: ${CMAKE_SOURCE_DIR}/src/data/")
endif()

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/src/data/
    DESTINATION data
    COMPONENT NeTrainSimCore_COMP
    FILES_MATCHING
    PATTERN "*.pdf"
    PATTERN "*.txt"
    PATTERN "*.xml"
    PATTERN "*.json"
    PATTERN "*.csv"
    PATTERN "*.dat"
    PATTERN "*.NTS"
    PATTERN "*.nts"
)

# ========================================
# SECTION J: qt.conf Generation
# ========================================
if(WIN32)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
        "[Paths]\nPlugins = plugins\n")
elseif(UNIX AND NOT APPLE)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
        "[Paths]\nPrefix = ..\nPlugins = plugins\nLibraries = lib\n")
endif()
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
    DESTINATION bin
    COMPONENT NeTrainSimCore_COMP
)

# ========================================
# SECTION K: CPack & QtIFW Global Config
# ========================================
set(CPACK_GENERATOR "IFW")
set(CPACK_PACKAGE_VENDOR "Virginia Tech Transportation Institute - Center for Sustainable Mobility")
set(CPACK_PACKAGE_DESCRIPTION "An installer for NeTrainSim simulation software.")
set(CPACK_PACKAGE_VERSION_MAJOR "${NeTrainSim_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${NeTrainSim_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${NeTrainSim_VERSION_PATCH}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# ========================================
# SECTION L: Windows QtIFW Config + Scripts
# ========================================
if(WIN32)
    set(CPACK_IFW_PACKAGE_TITLE "NeTrainSim")
    set(CPACK_IFW_PACKAGE_PUBLISHER "Virginia Tech Transportation Institute")
    set(CPACK_IFW_PACKAGE_WIZARD_STYLE "Modern")
    set(CPACK_IFW_PACKAGE_WIZARD_DEFAULT_WIDTH 800)
    set(CPACK_IFW_PACKAGE_WIZARD_DEFAULT_HEIGHT 600)
    set(CPACK_IFW_PACKAGE_ALLOW_NON_ASCII_CHARACTERS ON)
    set(CPACK_IFW_PACKAGE_ALLOW_SPACE_IN_PATH ON)
    set(CPACK_IFW_PACKAGE_WIZARD_SHOW_PAGE_LIST ON)
    set(CPACK_IFW_PACKAGE_COMPRESSION_LEVEL "3")
    set(CPACK_IFW_VERBOSE ON)
    set(CPACK_IFW_PACKAGE_REMOVE_TARGET_DIR ON)
    set(CPACK_IFW_PACKAGE_NAME "NeTrainSim")
    set(CPACK_IFW_PACKAGE_VERSION "${NeTrainSim_VERSION_MAJOR}.${NeTrainSim_VERSION_MINOR}.${NeTrainSim_VERSION_PATCH}")
    set(CPACK_IFW_PACKAGE_MAINTENANCE_TOOL_NAME "NeTrainSimMaintenanceTool")

    if(BUILD_GUI)
        set(CPACK_IFW_PACKAGE_RUN_PROGRAM "@TargetDir@/bin/NeTrainSimGUI.exe")
        set(CPACK_IFW_PACKAGE_RUN_PROGRAM_DESCRIPTION "Launch NeTrainSim GUI now")
    endif()
    set(CPACK_IFW_PACKAGE_RUN_PROGRAM_ARGUMENTS "")

    # Create the UI form for the admin selection page
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/adminselectionform.ui" "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
    <ui version=\"4.0\">
     <class>AdminSelectionForm</class>
     <widget class=\"QWidget\" name=\"AdminSelectionForm\">
      <layout class=\"QVBoxLayout\">
       <item>
        <widget class=\"QLabel\">
         <property name=\"text\">
          <string>&lt;h3&gt;Installation Type&lt;/h3&gt;</string>
         </property>
        </widget>
       </item>
       <item>
        <widget class=\"QRadioButton\" name=\"adminRadioButton\">
         <property name=\"text\">
          <string>Install for all users (Recommended - Requires Administrator)</string>
         </property>
         <property name=\"checked\">
          <bool>true</bool>
         </property>
        </widget>
       </item>
       <item>
        <widget class=\"QLabel\">
         <property name=\"text\">
          <string>• Installs to Program Files\\n• Available to all users\\n• Requires administrator privileges</string>
         </property>
         <property name=\"indent\">
          <number>20</number>
         </property>
        </widget>
       </item>
       <item>
        <widget class=\"QRadioButton\" name=\"userRadioButton\">
         <property name=\"text\">
          <string>Install for current user only</string>
         </property>
        </widget>
       </item>
       <item>
        <widget class=\"QLabel\">
         <property name=\"text\">
          <string>• Installs to user profile\\n• Available only to current user\\n• No administrator privileges required</string>
         </property>
         <property name=\"indent\">
          <number>20</number>
         </property>
        </widget>
       </item>
      </layout>
     </widget>
    </ui>")

    # Create control script with QtIFW admin detection
    set(CPACK_IFW_PACKAGE_CONTROL_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/controlscript.qs")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/controlscript.qs" "
function Controller() {
    installer.setValue('StartMenuDir', 'NeTrainSim');
    installer.setMessageBoxAutomaticAnswer('OverwriteTargetDirectory', QMessageBox.Yes);
    installer.setMessageBoxAutomaticAnswer('stopProcessesForUpdates', QMessageBox.Ignore);

    // Set initial default values
    installer.setValue('InstallationType', 'SystemWide');
    installer.setValue('TargetDir', 'C:/Program Files/NeTrainSim');
}

Controller.prototype.IntroductionPageCallback = function() {
    installer.setValue('InstallationType', 'SystemWide');
}

Controller.prototype.DynamicAdminSelectionFormCallback = function() {
    var page = gui.pageWidgetByObjectName('DynamicAdminSelectionForm');
    if (!page) {
        return;
    }

    var adminButton = page.adminRadioButton;
    var userButton = page.userRadioButton;

    // Set initial state based on current InstallationType
    var currentType = installer.value('InstallationType');
    if (currentType === 'UserOnly' && userButton) {
        userButton.checked = true;
    } else if (adminButton) {
        adminButton.checked = true;
    }

    if (adminButton) {
        adminButton.toggled.connect(function(checked) {
            if (checked) {
                installer.setValue('InstallationType', 'SystemWide');
                installer.setValue('TargetDir', 'C:/Program Files/NeTrainSim');
            }
        });
    }

    if (userButton) {
        userButton.toggled.connect(function(checked) {
            if (checked) {
                installer.setValue('InstallationType', 'UserOnly');
                var userPath = installer.value('HomeDir') + '/AppData/Local/NeTrainSim';
                installer.setValue('TargetDir', userPath);
            }
        });
    }
}

Controller.prototype.TargetDirectoryPageCallback = function() {
    var installType = installer.value('InstallationType');

    if (installType === 'SystemWide') {
        try {
            var isAdmin = false;
            if (typeof installer.hasAdminRights === 'function') {
                isAdmin = installer.hasAdminRights();
            }

            if (!isAdmin) {
                var result = QMessageBox.question('admin_required', 'Administrator Required',
                    'Administrator privileges are required for system-wide installation.\\n\\n' +
                    'Click OK to restart with administrator privileges, or Cancel to install for current user only.',
                    QMessageBox.Ok | QMessageBox.Cancel, QMessageBox.Ok);

                if (result === QMessageBox.Ok) {
                    if (typeof installer.gainAdminRights === 'function') {
                        if (!installer.gainAdminRights()) {
                            QMessageBox.information('elevation_failed', 'Elevation Failed',
                                'Could not obtain administrator privileges.\\n\\nContinuing with user installation.');
                            installer.setValue('InstallationType', 'UserOnly');
                            var userPath = installer.value('HomeDir') + '/AppData/Local/NeTrainSim';
                            installer.setValue('TargetDir', userPath);

                            var widget = gui.currentPageWidget();
                            if (widget && widget.TargetDirectoryLineEdit) {
                                widget.TargetDirectoryLineEdit.setText(userPath);
                            }
                        }
                    }
                } else {
                    installer.setValue('InstallationType', 'UserOnly');
                    var userPath = installer.value('HomeDir') + '/AppData/Local/NeTrainSim';
                    installer.setValue('TargetDir', userPath);

                    var widget = gui.currentPageWidget();
                    if (widget && widget.TargetDirectoryLineEdit) {
                        widget.TargetDirectoryLineEdit.setText(userPath);
                    }
                }
            }
        } catch (e) {
            console.log('Admin rights check failed: ' + e.message);
        }
    }

    // Update the displayed path to match the current selection
    var widget = gui.currentPageWidget();
    if (widget && widget.TargetDirectoryLineEdit) {
        var currentPath = installer.value('TargetDir');
        widget.TargetDirectoryLineEdit.setText(currentPath);
    }
}")

    # Core component script (Windows)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/core_component.qs" "
function Component() {
    installer.addWizardPage(component, 'AdminSelectionForm', QInstaller.TargetDirectory);
}

Component.prototype.createOperations = function() {
    try {
        component.createOperations();

        var installType = installer.value('InstallationType');

        if (installType === 'SystemWide') {
            component.addOperation('RegisterFileType',
                'nts',
                '@TargetDir@/bin/NeTrainSim.exe \"%1\"',
                'NeTrainSim Simulation File',
                'application/x-netrainsim');

            component.addOperation('CreateShortcut',
                '@TargetDir@/bin/NeTrainSim.exe',
                '@AllUsersStartMenuProgramsPath@/NeTrainSim/NeTrainSim Console.lnk',
                'workingDirectory=@TargetDir@',
                'iconPath=@TargetDir@/bin/NeTrainSim.exe',
                'description=NeTrainSim Console Application');

            if (installer.fileExists('@TargetDir@/vcredist_x64.exe')) {
                component.addOperation('Execute',
                    '@TargetDir@/vcredist_x64.exe',
                    '/install', '/quiet', '/norestart',
                    'UNDOEXECUTE',
                    '@TargetDir@/vcredist_x64.exe', '/uninstall', '/quiet');
            }
        } else {
            component.addOperation('RegisterFileType',
                'nts',
                '@TargetDir@/bin/NeTrainSim.exe \"%1\"',
                'NeTrainSim Simulation File',
                'application/x-netrainsim',
                'CurrentUser');

            component.addOperation('CreateShortcut',
                '@TargetDir@/bin/NeTrainSim.exe',
                '@UserStartMenuProgramsPath@/NeTrainSim/NeTrainSim Console.lnk',
                'workingDirectory=@TargetDir@',
                'iconPath=@TargetDir@/bin/NeTrainSim.exe',
                'description=NeTrainSim Console Application');
        }
    } catch (e) {
        console.log('ERROR in createOperations: ' + e.message);
    }
}")

    # Console component script (Windows)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/console_component.qs" "
function Component() {
    // Console Component constructor
}

Component.prototype.createOperations = function() {
    try {
        component.createOperations();
        var installType = installer.value('InstallationType');

        if (installType === 'SystemWide') {
            component.addOperation('CreateShortcut',
                '@TargetDir@/bin/NeTrainSim.exe',
                '@AllUsersStartMenuProgramsPath@/NeTrainSim/NeTrainSim Console.lnk',
                'workingDirectory=@TargetDir@',
                'iconPath=@TargetDir@/bin/NeTrainSim.exe',
                'description=NeTrainSim Console Application');

            component.addOperation('CreateShortcut',
                '@TargetDir@/bin/NeTrainSim.exe',
                '@AllUsersDesktopPath@/NeTrainSim Console.lnk',
                'workingDirectory=@TargetDir@',
                'iconPath=@TargetDir@/bin/NeTrainSim.exe',
                'description=NeTrainSim Console Application');
        } else {
            component.addOperation('CreateShortcut',
                '@TargetDir@/bin/NeTrainSim.exe',
                '@UserStartMenuProgramsPath@/NeTrainSim/NeTrainSim Console.lnk',
                'workingDirectory=@TargetDir@',
                'iconPath=@TargetDir@/bin/NeTrainSim.exe',
                'description=NeTrainSim Console Application');

            component.addOperation('CreateShortcut',
                '@TargetDir@/bin/NeTrainSim.exe',
                '@UserDesktopPath@/NeTrainSim Console.lnk',
                'workingDirectory=@TargetDir@',
                'iconPath=@TargetDir@/bin/NeTrainSim.exe',
                'description=NeTrainSim Console Application');
        }
    } catch (e) {
        console.log('Error in Console createOperations: ' + e.message);
    }
}")

    # GUI component script (Windows)
    if(BUILD_GUI)
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/gui_component.qs" "
function Component() {
    // GUI Component constructor
}

Component.prototype.createOperations = function() {
    try {
        component.createOperations();
        var installType = installer.value('InstallationType');

        if (installType === 'SystemWide') {
            component.addOperation('CreateShortcut',
                '@TargetDir@/bin/NeTrainSimGUI.exe',
                '@AllUsersStartMenuProgramsPath@/NeTrainSim/NeTrainSim GUI.lnk',
                'workingDirectory=@TargetDir@',
                'iconPath=@TargetDir@/bin/NeTrainSimGUI.exe',
                'description=NeTrainSim GUI Application');

            component.addOperation('CreateShortcut',
                '@TargetDir@/bin/NeTrainSimGUI.exe',
                '@AllUsersDesktopPath@/NeTrainSim GUI.lnk',
                'workingDirectory=@TargetDir@',
                'iconPath=@TargetDir@/bin/NeTrainSimGUI.exe',
                'description=NeTrainSim GUI Application');
        } else {
            component.addOperation('CreateShortcut',
                '@TargetDir@/bin/NeTrainSimGUI.exe',
                '@UserStartMenuProgramsPath@/NeTrainSim/NeTrainSim GUI.lnk',
                'workingDirectory=@TargetDir@',
                'iconPath=@TargetDir@/bin/NeTrainSimGUI.exe',
                'description=NeTrainSim GUI Application');

            component.addOperation('CreateShortcut',
                '@TargetDir@/bin/NeTrainSimGUI.exe',
                '@UserDesktopPath@/NeTrainSim GUI.lnk',
                'workingDirectory=@TargetDir@',
                'iconPath=@TargetDir@/bin/NeTrainSimGUI.exe',
                'description=NeTrainSim GUI Application');
        }
    } catch (e) {
        console.log('Error in GUI createOperations: ' + e.message);
    }
}")
    endif()

    # Server component script (Windows)
    if(BUILD_SERVER)
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/server_component.qs" "
function Component() {
    // Server Component constructor
}

Component.prototype.createOperations = function() {
    try {
        component.createOperations();
        var installType = installer.value('InstallationType');

        if (installType === 'SystemWide') {
            component.addOperation('CreateShortcut',
                '@TargetDir@/bin/NeTrainSimServer.exe',
                '@AllUsersStartMenuProgramsPath@/NeTrainSim/NeTrainSim Server.lnk',
                'workingDirectory=@TargetDir@',
                'iconPath=@TargetDir@/bin/NeTrainSimServer.exe',
                'description=NeTrainSim Server Application');

            component.addOperation('CreateShortcut',
                '@TargetDir@/bin/NeTrainSimRabbitMQConfig.exe',
                '@AllUsersStartMenuProgramsPath@/NeTrainSim/RabbitMQ Config.lnk',
                'workingDirectory=@TargetDir@',
                'iconPath=@TargetDir@/bin/NeTrainSimRabbitMQConfig.exe',
                'description=RabbitMQ Connection Configuration');
        } else {
            component.addOperation('CreateShortcut',
                '@TargetDir@/bin/NeTrainSimServer.exe',
                '@UserStartMenuProgramsPath@/NeTrainSim/NeTrainSim Server.lnk',
                'workingDirectory=@TargetDir@',
                'iconPath=@TargetDir@/bin/NeTrainSimServer.exe',
                'description=NeTrainSim Server Application');

            component.addOperation('CreateShortcut',
                '@TargetDir@/bin/NeTrainSimRabbitMQConfig.exe',
                '@UserStartMenuProgramsPath@/NeTrainSim/RabbitMQ Config.lnk',
                'workingDirectory=@TargetDir@',
                'iconPath=@TargetDir@/bin/NeTrainSimRabbitMQConfig.exe',
                'description=RabbitMQ Connection Configuration');
        }
    } catch (e) {
        console.log('Error in Server createOperations: ' + e.message);
    }
}")
    endif()

# ========================================
# SECTION M: Linux QtIFW Config + Scripts
# ========================================
elseif(UNIX AND NOT APPLE)

    set(CPACK_IFW_PACKAGE_TITLE "NeTrainSim")
    set(CPACK_IFW_PACKAGE_PUBLISHER "Virginia Tech Transportation Institute")
    set(CPACK_IFW_PACKAGE_WIZARD_STYLE "Modern")
    set(CPACK_IFW_PACKAGE_WIZARD_DEFAULT_WIDTH 800)
    set(CPACK_IFW_PACKAGE_WIZARD_DEFAULT_HEIGHT 600)
    set(CPACK_IFW_PACKAGE_ALLOW_NON_ASCII_CHARACTERS ON)
    set(CPACK_IFW_PACKAGE_ALLOW_SPACE_IN_PATH ON)
    set(CPACK_IFW_PACKAGE_WIZARD_SHOW_PAGE_LIST ON)
    set(CPACK_IFW_PACKAGE_COMPRESSION_LEVEL "3")
    set(CPACK_IFW_VERBOSE ON)
    set(CPACK_IFW_PACKAGE_REMOVE_TARGET_DIR ON)
    set(CPACK_IFW_PACKAGE_NAME "NeTrainSim")
    set(CPACK_IFW_PACKAGE_VERSION "${NeTrainSim_VERSION_MAJOR}.${NeTrainSim_VERSION_MINOR}.${NeTrainSim_VERSION_PATCH}")
    set(CPACK_IFW_PACKAGE_MAINTENANCE_TOOL_NAME "NeTrainSimMaintenanceTool")
    # Linux-specific: default install to home directory
    set(CPACK_IFW_PACKAGE_TARGET_DIR "@HomeDir@/NeTrainSim")

    # Control Script (no admin selection needed on Linux)
    set(CPACK_IFW_PACKAGE_CONTROL_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/controlscript.qs")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/controlscript.qs" "
function Controller() {
    installer.setMessageBoxAutomaticAnswer('OverwriteTargetDirectory', QMessageBox.Yes);
    installer.setMessageBoxAutomaticAnswer('stopProcessesForUpdates', QMessageBox.Ignore);
}

Controller.prototype.IntroductionPageCallback = function() {
    console.log('NeTrainSim Linux Installer');
}

Controller.prototype.TargetDirectoryPageCallback = function() {
    var widget = gui.currentPageWidget();
    if (widget && widget.TargetDirectoryLineEdit) {
        var currentPath = installer.value('TargetDir');
        widget.TargetDirectoryLineEdit.setText(currentPath);
    }
}")

    # Core Component Script (Linux)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/core_component.qs" "
function Component() {
    // Core Component constructor - no admin selection page on Linux
}

Component.prototype.createOperations = function() {
    try {
        component.createOperations();

        if (installer.value('os') === 'x11') {
            // Create desktop entry for NeTrainSim
            component.addOperation('CreateDesktopEntry',
                'NeTrainSim.desktop',
                'Version=1.0\\nType=Application\\nTerminal=true\\n' +
                'Exec=@TargetDir@/bin/NeTrainSim\\n' +
                'Name=NeTrainSim\\nCategories=Education;Science;\\n' +
                'Comment=Network Train Simulator');

            // Make executables executable
            component.addOperation('Execute', 'chmod', '+x',
                '@TargetDir@/bin/NeTrainSim');
        }
    } catch (e) {
        console.log('Error in Core createOperations: ' + e.message);
    }
}")

    # Console Component Script (Linux)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/console_component.qs" "
function Component() {
    // Console Component constructor
}

Component.prototype.createOperations = function() {
    try {
        component.createOperations();

        if (installer.value('os') === 'x11') {
            component.addOperation('CreateDesktopEntry',
                'NeTrainSimConsole.desktop',
                'Version=1.0\\nType=Application\\nTerminal=true\\n' +
                'Exec=@TargetDir@/bin/NeTrainSim\\n' +
                'Name=NeTrainSim Console\\nCategories=Education;Science;\\n' +
                'Comment=NeTrainSim Console Application');

            component.addOperation('Execute', 'chmod', '+x',
                '@TargetDir@/bin/NeTrainSim');
        }
    } catch (e) {
        console.log('Error in Console createOperations: ' + e.message);
    }
}")

    # GUI Component Script (Linux)
    if(BUILD_GUI)
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/gui_component.qs" "
function Component() {
    // GUI Component constructor
}

Component.prototype.createOperations = function() {
    try {
        component.createOperations();

        if (installer.value('os') === 'x11') {
            component.addOperation('CreateDesktopEntry',
                'NeTrainSimGUI.desktop',
                'Version=1.0\\nType=Application\\nTerminal=false\\n' +
                'Exec=@TargetDir@/bin/NeTrainSimGUI\\n' +
                'Name=NeTrainSim GUI\\nCategories=Education;Science;\\n' +
                'Comment=NeTrainSim GUI Application');

            component.addOperation('Execute', 'chmod', '+x',
                '@TargetDir@/bin/NeTrainSimGUI');
        }
    } catch (e) {
        console.log('Error in GUI createOperations: ' + e.message);
    }
}")
    endif()

    # Server Component Script (Linux)
    if(BUILD_SERVER)
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/server_component.qs" "
function Component() {
    // Server Component constructor
}

Component.prototype.createOperations = function() {
    try {
        component.createOperations();

        if (installer.value('os') === 'x11') {
            component.addOperation('CreateDesktopEntry',
                'NeTrainSimServer.desktop',
                'Version=1.0\\nType=Application\\nTerminal=true\\n' +
                'Exec=@TargetDir@/bin/NeTrainSimServer\\n' +
                'Name=NeTrainSim Server\\nCategories=Education;Science;\\n' +
                'Comment=NeTrainSim Server Application');

            component.addOperation('Execute', 'chmod', '+x',
                '@TargetDir@/bin/NeTrainSimServer');
            component.addOperation('Execute', 'chmod', '+x',
                '@TargetDir@/bin/NeTrainSimRabbitMQConfig');
        }
    } catch (e) {
        console.log('Error in Server createOperations: ' + e.message);
    }
}")
    endif()

endif() # WIN32 / UNIX QtIFW config

# ========================================
# SECTION N: QtIFW Root Detection
# ========================================
if(NOT DEFINED CPACK_IFW_ROOT)
    if(WIN32)
        set(POSSIBLE_IFW_ROOTS
            "C:/Qt/Tools/QtInstallerFramework/4.10"
            "C:/Qt/Tools/QtInstallerFramework/4.1"
            "C:/QtIFW-4.10.0"
            "C:/QtIFW"
            "$ENV{QTIFWDIR}"
        )
    elseif(UNIX)
        set(POSSIBLE_IFW_ROOTS
            "$ENV{HOME}/Qt/Tools/QtInstallerFramework/4.10"
            "$ENV{HOME}/Qt/Tools/QtInstallerFramework/4.8"
            "/opt/Qt/Tools/QtInstallerFramework/4.10"
            "/opt/Qt/Tools/QtInstallerFramework/4.8"
            "/usr/local/bin"
            "$ENV{QTIFWDIR}"
        )
    endif()

    foreach(IFW_PATH ${POSSIBLE_IFW_ROOTS})
        if(EXISTS "${IFW_PATH}/bin/binarycreator" OR EXISTS "${IFW_PATH}/bin/binarycreator.exe")
            set(CPACK_IFW_ROOT "${IFW_PATH}")
            message(STATUS "Found QtIFW at: ${IFW_PATH}")
            break()
        endif()
    endforeach()

    if(NOT DEFINED CPACK_IFW_ROOT)
        message(WARNING "QtIFW not found. Please set CPACK_IFW_ROOT manually.")
        if(WIN32)
            set(CPACK_IFW_ROOT "C:/Qt/Tools/QtInstallerFramework/4.10")
        elseif(UNIX)
            set(CPACK_IFW_ROOT "$ENV{HOME}/Qt/Tools/QtInstallerFramework/4.10")
        endif()
    endif()
endif()

include(CPack)
include(CPackIFW)

# ========================================
# SECTION O: CPack Component Definitions
# ========================================

# Core component
cpack_add_component(NeTrainSimCore_COMP
    DISPLAY_NAME "NeTrainSim Core"
    DESCRIPTION "Core libraries and essential files for NeTrainSim."
    REQUIRED)

# Core IFW config: USER_INTERFACES only needed on Windows (admin selection page)
if(WIN32)
    cpack_ifw_configure_component(NeTrainSimCore_COMP
        ESSENTIAL ON
        FORCED_INSTALLATION ON
        VISIBLE ON
        NAME "netrainsim.core"
        VERSION "${NeTrainSim_VERSION_MAJOR}.${NeTrainSim_VERSION_MINOR}.${NeTrainSim_VERSION_PATCH}"
        SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/core_component.qs"
        USER_INTERFACES "${CMAKE_CURRENT_BINARY_DIR}/adminselectionform.ui"
        LICENSES "GPL-3.0 License" ${CPACK_RESOURCE_FILE_LICENSE}
        SORTING_PRIORITY 100)
else()
    cpack_ifw_configure_component(NeTrainSimCore_COMP
        ESSENTIAL ON
        FORCED_INSTALLATION ON
        VISIBLE ON
        NAME "netrainsim.core"
        VERSION "${NeTrainSim_VERSION_MAJOR}.${NeTrainSim_VERSION_MINOR}.${NeTrainSim_VERSION_PATCH}"
        SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/core_component.qs"
        LICENSES "GPL-3.0 License" ${CPACK_RESOURCE_FILE_LICENSE}
        SORTING_PRIORITY 100)
endif()

# Console component (essential, forced)
cpack_add_component(NeTrainSimConsole_COMP
    DISPLAY_NAME "NeTrainSim Console"
    DESCRIPTION "NeTrainSim console executable."
    REQUIRED)

cpack_ifw_configure_component(NeTrainSimConsole_COMP
    ESSENTIAL ON
    FORCED_INSTALLATION ON
    VISIBLE ON
    NAME "netrainsim.console"
    VERSION "${NeTrainSim_VERSION_MAJOR}.${NeTrainSim_VERSION_MINOR}.${NeTrainSim_VERSION_PATCH}"
    SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/console_component.qs"
    SORTING_PRIORITY 90)

# GUI component (optional)
if(BUILD_GUI)
    cpack_add_component(NeTrainSimGUI_COMP
        DISPLAY_NAME "NeTrainSim GUI"
        DESCRIPTION "NeTrainSim graphical user interface.")

    cpack_ifw_configure_component(NeTrainSimGUI_COMP
        VISIBLE ON
        NAME "netrainsim.gui"
        VERSION "${NeTrainSim_VERSION_MAJOR}.${NeTrainSim_VERSION_MINOR}.${NeTrainSim_VERSION_PATCH}"
        SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/gui_component.qs"
        SORTING_PRIORITY 80)
endif()

# Server component (optional)
if(BUILD_SERVER)
    cpack_add_component(NeTrainSimServer_COMP
        DISPLAY_NAME "NeTrainSim Server"
        DESCRIPTION "NeTrainSim server and RabbitMQ configuration tool.")

    cpack_ifw_configure_component(NeTrainSimServer_COMP
        VISIBLE ON
        NAME "netrainsim.server"
        VERSION "${NeTrainSim_VERSION_MAJOR}.${NeTrainSim_VERSION_MINOR}.${NeTrainSim_VERSION_PATCH}"
        SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/server_component.qs"
        SORTING_PRIORITY 70)
endif()

endif() # BUILD_INSTALLER
