cmake_minimum_required(VERSION 3.16)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets Test)

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
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(RUST_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/../../target/debug")
else()
    set(RUST_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/../../target/release")
endif()

target_link_libraries(test_geometryviewer PRIVATE "${RUST_LIB_DIR}/libstdgeo_lib.so")
target_link_directories(test_geometryviewer PRIVATE ${RUST_LIB_DIR})

# Test for TerminalWidget
add_executable(test_terminalwidget
    test_terminalwidget.cpp
    ../terminalwidget.cpp
)

target_link_libraries(test_terminalwidget PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Test
)

# Add qtermwidget if available
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(QTERMWIDGET qtermwidget6)
    if(QTERMWIDGET_FOUND)
        target_compile_definitions(test_terminalwidget PRIVATE HAVE_QTERMWIDGET)
        target_link_libraries(test_terminalwidget PRIVATE ${QTERMWIDGET_LIBRARIES})
        target_include_directories(test_terminalwidget PRIVATE ${QTERMWIDGET_INCLUDE_DIRS})
        
        target_compile_definitions(test_geometryviewer PRIVATE HAVE_QTERMWIDGET)
        target_link_libraries(test_geometryviewer PRIVATE ${QTERMWIDGET_LIBRARIES})
        target_include_directories(test_geometryviewer PRIVATE ${QTERMWIDGET_INCLUDE_DIRS})
    endif()
endif()

# Ensure Rust library is built first
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

# Create a target to run all Qt tests
add_custom_target(run_qt_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_geometryviewer test_terminalwidget
    COMMENT "Running Qt application tests"
)