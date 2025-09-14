cmake_minimum_required(VERSION 3.16)

project(stdgeo-gui VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Qt6 path if not already set
if(NOT DEFINED CMAKE_PREFIX_PATH)
    get_filename_component(QT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../Qt/6.9.1/gcc_64" ABSOLUTE)
    set(CMAKE_PREFIX_PATH "${QT_PATH}")
endif()

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

target_link_libraries(stdgeo-gui PRIVATE
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