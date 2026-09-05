cmake_minimum_required(VERSION 3.21)
project(FluxStudio VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Svg)

qt_add_executable(FluxStudio
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/splashscreen.cpp
    src/splashscreen.h
    src/canvaswidget.cpp
    src/canvaswidget.h
    src/fluxwheel.cpp
    src/fluxwheel.h
    resources.qrc
)

target_link_libraries(FluxStudio PRIVATE Qt6::Widgets Qt6::Svg)

if(MSVC)
    target_compile_options(FluxStudio PRIVATE /W4 /permissive-)
else()
    target_compile_options(FluxStudio PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(FluxStudio PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    OUTPUT_NAME "Flux Studio"
)

install(TARGETS FluxStudio BUNDLE DESTINATION . RUNTIME DESTINATION bin)
