add_library(tinyxml2 STATIC 
    tinyxml2.cpp
    tinyxml2.h
)

target_include_directories(tinyxml2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})