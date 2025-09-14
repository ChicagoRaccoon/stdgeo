cmake_minimum_required(VERSION 3.16)

project(stdgeo-gui VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)

qt6_standard_project_setup()

set(SOURCES
    main.cpp
    mainwindow.cpp
    openglwidget.cpp
)

set(HEADERS
    mainwindow.h
    openglwidget.h
)

qt6_add_executable(stdgeo-gui ${SOURCES} ${HEADERS})

target_link_libraries(stdgeo-gui
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
)

if(WIN32)
    set_target_properties(stdgeo-gui PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()