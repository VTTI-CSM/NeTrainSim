find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Xml Network)
find_package(Qt6Keychain QUIET)

set(RABBITMQ_CONFIG_SOURCES
    main.cpp
    RabbitMQConfigDialog.h
    RabbitMQConfigDialog.cpp
)

add_executable(NeTrainSimRabbitMQConfig ${RABBITMQ_CONFIG_SOURCES})

target_link_libraries(NeTrainSimRabbitMQConfig
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Xml
    Qt6::Network
)

# Conditionally link QtKeychain if available
if(Qt6Keychain_FOUND)
    target_link_libraries(NeTrainSimRabbitMQConfig PRIVATE Qt6Keychain::Qt6Keychain)
    target_compile_definitions(NeTrainSimRabbitMQConfig PRIVATE HAVE_QTKEYCHAIN)
endif()

target_include_directories(NeTrainSimRabbitMQConfig
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

# Installation
install(TARGETS NeTrainSimRabbitMQConfig
    RUNTIME DESTINATION bin
    COMPONENT NeTrainSimServer_COMP
)
