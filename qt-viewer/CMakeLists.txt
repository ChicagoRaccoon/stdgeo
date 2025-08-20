cmake_minimum_required(VERSION 3.16)
project(stdgeo-qt-viewer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)

# Set rpath to prefer the Qt installation we're using
get_target_property(QT_CORE_LOCATION Qt6::Core LOCATION)
get_filename_component(QT_LIB_DIR "${QT_CORE_LOCATION}" DIRECTORY)
set(CMAKE_INSTALL_RPATH "${QT_LIB_DIR}")
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

# Add Qt library directory to link directories
link_directories("${QT_LIB_DIR}")


# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

# Define the executable
set(SOURCES
    main.cpp
    mainwindow.cpp
    geometryviewer.cpp
    terminalwidget.cpp
)

set(HEADERS
    mainwindow.h
    geometryviewer.h
    terminalwidget.h
)

# Enable Qt automatic MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt MOC handling
qt6_add_executable(stdgeo-qt-viewer ${SOURCES})

# Link libraries
target_link_libraries(stdgeo-qt-viewer PRIVATE
    Qt6::Core 
    Qt6::Widgets 
    Qt6::OpenGL 
    Qt6::OpenGLWidgets
)

# Link to the Rust library
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(RUST_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/../target/debug")
else()
    set(RUST_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/../target/release")
endif()

target_link_libraries(stdgeo-qt-viewer PRIVATE "${RUST_LIB_DIR}/libstdgeo_lib.so")
target_link_directories(stdgeo-qt-viewer PRIVATE ${RUST_LIB_DIR})


# Ensure Rust library is built first
add_dependencies(stdgeo-qt-viewer cargo_build)

# Install
install(TARGETS stdgeo-qt-viewer DESTINATION bin)

# Copy the Rust binary next to Qt binary for integrated CLI access
add_custom_command(TARGET stdgeo-qt-viewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_BINARY_DIR}/../bin/stdgeo"
    "$<TARGET_FILE_DIR:stdgeo-qt-viewer>/stdgeo-cli"
)

# Add tests subdirectory
add_subdirectory(tests)