cmake_minimum_required(VERSION 3.16)

project(Vega VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Svg)

qt_add_resources(SOURCES Resources.qrc)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Backend/OpenXLSX/)

set(PROJECT_SOURCES
    ${SOURCES}
    UI/main.cpp
    UI/MainWidget.cpp
    UI/MainWidget.h
    UI/ScheduleTab.cpp
    UI/ScheduleTab.h
    UI/SettingsTab.cpp
    UI/SettingsTab.h
    UI/CustomElements.cpp
    UI/CustomElements.h
    UI/CommonFunctions.cpp
    UI/CommonFunctions.h
    Backend/Parser.cpp
    Backend/Parser.h
)

qt_add_executable(Vega
    ${PROJECT_SOURCES}
    Backend/Parser.cpp
)

target_link_libraries(Vega PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Svg
    OpenXLSX::OpenXLSX
)

qt_finalize_executable(Vega)

if(EMSCRIPTEN)
    target_link_options(Vega PRIVATE -sNO_DISABLE_EXCEPTION_CATCHING)
endif()