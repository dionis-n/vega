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

# OpenXLSX 0.3.2 местами использует uint16_t/uint64_t без явного <cstdint>.
# Под emscripten/clang заголовок подтягивается транзитивно, а под нативным
# GCC 13+ (в т.ч. в CI на ubuntu) сборка падает. Принудительно подключаем
# <cstdint> во все единицы трансляции OpenXLSX, чтобы либа собиралась везде.
if(MSVC)
    target_compile_options(OpenXLSX PRIVATE /FIcstdint)
else()
    target_compile_options(OpenXLSX PRIVATE -include cstdint)
endif()

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)
enable_testing()

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
    Backend/ParserHelpers.cpp
    Backend/ParserHelpers.h
)

qt_add_executable(Vega
    ${PROJECT_SOURCES}
)

target_link_libraries(Vega PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Svg
    OpenXLSX::OpenXLSX
)

set_target_properties(Vega PROPERTIES
    QT_WASM_INITIAL_MEMORY 256MB
)

qt_add_executable(VegaTests
    Tests/test_parser.cpp
    Backend/ParserHelpers.cpp
    Backend/ParserHelpers.h
    Backend/Parser.cpp
    Backend/Parser.h
)

target_include_directories(VegaTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Parser.cpp использует OpenXLSX, чтобы тесты могли прогоняться по реальному
# .xlsx-расписанию, переданному через --schedule=<файл>.
target_link_libraries(VegaTests PRIVATE
    GTest::gtest
    Qt6::Core
    OpenXLSX::OpenXLSX
)

if(EMSCRIPTEN)
    target_link_options(Vega PRIVATE -sNO_DISABLE_EXCEPTION_CATCHING)

    # Тестам тоже нужен Embind и отключение «глотания» исключений, которые использует GTest.
    target_link_options(VegaTests PRIVATE
        --bind
        -sNO_DISABLE_EXCEPTION_CATCHING
    )
endif()

qt_finalize_executable(Vega)
qt_finalize_executable(VegaTests)

include(GoogleTest)
gtest_discover_tests(VegaTests)

add_custom_command(TARGET Vega POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/Resources/VegaIcon/Vega.ico" "$<TARGET_FILE_DIR:Vega>/favicon.ico"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/Resources/VegaIcon/Vega.ico" "$<TARGET_FILE_DIR:Vega>/Resources/VegaIcon/Vega.ico"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/Resources/VegaIcon/icon-192.png" "$<TARGET_FILE_DIR:Vega>/Resources/VegaIcon/icon-192.png"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/Resources/VegaIcon/icon-144.png" "$<TARGET_FILE_DIR:Vega>/Resources/VegaIcon/icon-144.png"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/Resources/VegaIcon/icon-512.png" "$<TARGET_FILE_DIR:Vega>/Resources/VegaIcon/icon-512.png"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/Resources/VegaIcon/screenshot-mobile.png" "$<TARGET_FILE_DIR:Vega>/Resources/VegaIcon/screenshot-mobile.png"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/Resources/VegaIcon/screenshot-desktop.png" "$<TARGET_FILE_DIR:Vega>/Resources/VegaIcon/screenshot-desktop.png"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/Resources/VegaIcon/manifest.json" "$<TARGET_FILE_DIR:Vega>/manifest.json"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/Resources/VegaIcon/service-worker.js" "$<TARGET_FILE_DIR:Vega>/service-worker.js"
)

add_custom_command(TARGET Vega POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Patching Vega.html..."
    COMMAND ${CMAKE_COMMAND} -DHTML_FILE=$<TARGET_FILE_DIR:Vega>/Vega.html -P "${CMAKE_CURRENT_SOURCE_DIR}/patch_html.cmake"
)