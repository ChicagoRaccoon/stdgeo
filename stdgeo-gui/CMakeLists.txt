cmake_minimum_required(VERSION 3.16)
project(stdgeo-gui VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

###############################################################################
# Project configuration

# Link to the Rust library
set(RUST_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/../target/${CMAKE_BUILD_TYPE}")


###############################################################################
# Build tooling

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)


###############################################################################
# Build configuration

# Set rpath to prefer the Qt installation we're using
get_target_property(QT_CORE_LOCATION Qt6::Core LOCATION)
get_filename_component(QT_LIB_DIR "${QT_CORE_LOCATION}" DIRECTORY)

# Add Qt library directory to link directories
link_directories("${QT_LIB_DIR}")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

###############################################################################
# Target Definitions

set(TARGET_NAME stdgeo-gui)

# Define the executable
set(SOURCES
    main.cpp
    mainwindow.cpp
    geometryviewer.cpp
    terminalwidget.cpp
    stdgeoparser.cpp
)

set(HEADERS
    mainwindow.h
    geometryviewer.h
    terminalwidget.h
    stdgeoparser.h
)

# Enable Qt automatic MOC, UIC, and RCC
# MOC, UIC, and RCC are Qt's code generation tools.
# MOC: Meta-Object Compiler
# UIC: User Interface Compiler
# RCC: Resource Compiler
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


# This line defines the actual target
# Configuration of this target occurs on the subsequent lines
qt6_add_executable(${TARGET_NAME} ${SOURCES})

# Link Qt libraries
target_link_libraries(${TARGET_NAME} PRIVATE
    Qt6::Core 
    Qt6::Widgets 
    Qt6::OpenGL 
    Qt6::OpenGLWidgets
)

# TODO Poss Consider a variable instead of libstdgeo.so
# Link Rust libraries
target_link_libraries(${TARGET_NAME} PRIVATE 
    "${RUST_LIB_DIR}/libstdgeo.so"
    "${RUST_LIB_DIR}/libstdgeo_parser.so"
)
target_link_directories(${TARGET_NAME} PRIVATE ${RUST_LIB_DIR})

# Define relative paths to dynamically linked libraries.
# Note that we can statically link these libraries, too.
# Set RPATH to look in ../lib relative to executable.
# We also need to look in the Qt library directory
set_target_properties(${TARGET_NAME} PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib;${QT_LIB_DIR}"
    BUILD_WITH_INSTALL_RPATH TRUE
)


# Ensure Rust library is built first
# This adds the cargo_build dependency to this target
add_dependencies(${TARGET_NAME} cargo_build)

# Install
install(
    TARGETS ${TARGET_NAME}
    DESTINATION bin
)

# Post-build
# Copy the copiled binary to the bin directory
add_custom_command(
    TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE_DIR:${TARGET_NAME}>/${STDGEO_GUI_NAME}"
    "${CMAKE_CURRENT_BINARY_DIR}/../bin/${STDGEO_GUI_NAME}"
)

###############################################################################
# Tests

# Add tests subdirectory
# By placing this command here, we ensure that we build tests only after
# the main GUI has compiled
add_subdirectory(tests)

