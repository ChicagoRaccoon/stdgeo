# StdGeo GUI Application CMake Configuration
# This CMake file builds a Qt6-based OpenGL viewer with interactive controls

cmake_minimum_required(VERSION 3.16)

# Project configuration
project(stdgeo-gui VERSION 1.0.0 LANGUAGES CXX)

# Set C++17 standard (required for modern Qt6)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure Qt6 installation path using relative path
# This looks for Qt6 in ../../Qt/6.9.1/gcc_64 relative to this CMakeLists.txt
if(NOT DEFINED CMAKE_PREFIX_PATH)
    get_filename_component(QT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../Qt/6.9.1/gcc_64" ABSOLUTE)
    set(CMAKE_PREFIX_PATH "${QT_PATH}")
endif()

# Find required Qt6 components
# Core: Base Qt functionality
# Widgets: Traditional desktop UI widgets
# OpenGL: OpenGL integration
# OpenGLWidgets: OpenGL widget support
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)

# Apply Qt6 standard project configuration (enables automoc, autorcc, autouic)
qt6_standard_project_setup()

# Define source files for the application
set(SOURCES
    main.cpp
    mainwindow.cpp
    openglwidget.cpp
)

# Define header files (needed for Qt's meta-object compiler)
set(HEADERS
    mainwindow.h
    openglwidget.h
)

# Create the executable target with Qt6-specific setup
qt6_add_executable(stdgeo-gui ${SOURCES} ${HEADERS})

# Link required Qt6 libraries to the executable
# PRIVATE: These libraries are only used internally, not exposed to consumers
target_link_libraries(stdgeo-gui PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
)

# Windows-specific configuration
if(WIN32)
    set_target_properties(stdgeo-gui PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()