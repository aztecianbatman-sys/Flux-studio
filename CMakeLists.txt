cmake_minimum_required(VERSION 3.21)
project(FluxStudio VERSION 0.8.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets OpenGLWidgets Svg)

qt_add_executable(FluxStudio
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow_extra.cpp
    src/canvaswidget.cpp
    src/canvaswidget.h
    src/fluxcanvasengine.cpp
    src/fluxcanvasengine.h
    src/fluxdocument.cpp
    src/fluxdocument.h
    src/fluxbrush.cpp
    src/fluxbrush.h
    src/brusheditor.cpp
    src/brusheditor.h
    src/fluxselection.cpp
    src/fluxselection.h
    src/fluxlayertree.cpp
    src/fluxlayertree.h
    src/fluxwheel.cpp
    src/fluxwheel.h
    src/fluxanimation.cpp
    src/fluxanimation.h
    src/fluxtimelinewidget.cpp
    src/fluxtimelinewidget.h
    src/fluxcompositor.cpp
    src/fluxcompositor.h
    src/fluxcompositorwidget.cpp
    src/fluxcompositorwidget.h
    src/fluxmedia.cpp
    src/fluxmedia.h
    src/fluxexport.cpp
    src/fluxexport.h
    src/fluxrenderqueue.cpp
    src/fluxrenderqueue.h
    src/fluxprojectmanager.cpp
    src/fluxprojectmanager.h
    src/fluxrecovery.cpp
    src/fluxrecovery.h
    src/fluxcommandpalette.cpp
    src/fluxcommandpalette.h
    src/fluxtabletprofiles.cpp
    src/fluxtabletprofiles.h
    src/fluxperformance.cpp
    src/fluxperformance.h
    src/fluxworkflow.cpp
    src/fluxworkflow.h
    src/fluxprojectpackage.cpp
    src/fluxprojectpackage.h
    src/fluxproductiondock.cpp
    src/fluxproductiondock.h
)

target_link_libraries(FluxStudio PRIVATE Qt6::Widgets Qt6::OpenGLWidgets Qt6::Svg)

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
