cmake_minimum_required(VERSION 3.16)

###############################################################################
# Project Settings



###############################################################################
# Build Tooling

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets Test)

###############################################################################
# Build Configuration

# Enable testing
enable_testing()

# Include parent directory for headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../..)

# Set up Qt6 testing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Define test sources (excluding the main.cpp from parent)
set(PARENT_SOURCES
    ../mainwindow.cpp
    ../geometryviewer.cpp
    ../terminalwidget.cpp
)

###############################################################################
# GeometryViewer Test

# Test for GeometryViewer
add_executable(test_geometryviewer 
    test_geometryviewer.cpp
    ../geometryviewer.cpp
)

target_link_libraries(test_geometryviewer PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Test
)

# Link to the Rust library
set(RUST_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/../../target/${CMAKE_BUILD_TYPE}")


target_link_libraries(test_geometryviewer PRIVATE "${RUST_LIB_DIR}/libstdgeo.so")
target_link_directories(test_geometryviewer PRIVATE ${RUST_LIB_DIR})


###############################################################################
# TerminalWidget Test

# Test for TerminalWidget
add_executable(test_terminalwidget
    test_terminalwidget.cpp
    ../terminalwidget.cpp
    ../stdgeoparser.cpp
)

# Link Qt Libraries
target_link_libraries(test_terminalwidget PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
)

# TODO Poss Consider a variable instead of libstdgeo.so
# Link Rust libraries
target_link_libraries(${TARGET_NAME} PRIVATE 
    "${RUST_LIB_DIR}/libstdgeo.so"
    "${RUST_LIB_DIR}/libstdgeo_parser.so"
)

target_link_directories(test_terminalwidget PRIVATE ${RUST_LIB_DIR})

###############################################################################
# Common Configurations

# Ensure Rust library is built first
# This adds cargo_build as a dependency to the first argument
add_dependencies(test_geometryviewer cargo_build)
add_dependencies(test_terminalwidget cargo_build)

# Register tests with CTest
add_test(NAME GeometryViewer COMMAND test_geometryviewer)
add_test(NAME TerminalWidget COMMAND test_terminalwidget)

# Set test properties
set_tests_properties(GeometryViewer PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

set_tests_properties(TerminalWidget PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

###############################################################################

# Create a target to run all Qt tests
add_custom_target(
    run_qt_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_geometryviewer test_terminalwidget
    COMMENT "Running Qt application tests"
)

