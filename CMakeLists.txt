cmake_minimum_required(VERSION 3.16)
project(stdgeo LANGUAGES CXX)

###############################################################################
# Project Settings

# Note that most paths are relative to the build/ directory

# Project names
set(STDGEO_CLI_NAME "stdgeo-cli")
set(STDGEO_GUI_NAME "qt-viewer")
set(STDGEO_LIB_NAME "stdgeo")

# Project directories
set(STDGEO_CLI_DIR "${CMAKE_SOURCE_DIR}/${STDGEO_CLI_NAME}")
set(STDGEO_GUI_DIR "${CMAKE_SOURCE_DIR}/${STDGEO_GUI_NAME}")
set(STDGEO_LIB_DIR "${CMAKE_SOURCE_DIR}/${STDGEO_LIB_NAME}")

# Final output directories
set(BIN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(LIB_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib")

# Qt6 directory
set(LOCAL_QT6_DIR "${CMAKE_SOURCE_DIR}/../../Qt/6.9.1/gcc_64")

# Set Qt6 path if not already set
if(NOT DEFINED Qt6_DIR AND EXISTS "${LOCAL_QT6_DIR}")
    set(CMAKE_PREFIX_PATH "${LOCAL_QT6_DIR}" ${CMAKE_PREFIX_PATH})
    message("Qt6 found at: ${LOCAL_QT6_DIR}")
else()
    message(FATAL_ERROR "Qt6 directory does not exist: ${LOCAL_QT6_DIR}")
endif()

###############################################################################
# Find Build Resources

# Find Rust/Cargo
find_program(CARGO_EXECUTABLE cargo REQUIRED)
find_program(RUSTC_EXECUTABLE rustc REQUIRED)

# Find Qt6
# This command will set Qt6_DIR and Qt6_FOUND if stuff is found
find_package(Qt6 REQUIRED QUIET COMPONENTS Core Widgets OpenGL OpenGLWidgets)

###############################################################################
# Configure Build

# Configure the Rust (and Qt) builds
# Note that you do NOT need a --debug build flag with Cargo
if(CMAKE_BUILD_TYPE STREQUAL "debug")
    set(CARGO_BUILD_TYPE "debug")
    set(CARGO_BUILD_FLAG "")
    set(TARGET_DIR "debug")
else()
    set(CARGO_BUILD_TYPE "release")
    set(CARGO_BUILD_FLAG "--release")
    set(TARGET_DIR "release")
endif()

# Configure the Qt6 Build
# This line effectively calls the CMakeLists file in the specified subdirectory
add_subdirectory(${STDGEO_GUI_NAME})


# Get Rust & Cargo version information
execute_process(COMMAND ${RUSTC_EXECUTABLE} --version OUTPUT_VARIABLE RUSTC_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${CARGO_EXECUTABLE} --version OUTPUT_VARIABLE CARGO_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)

# Print Rust & Cargo version information
message(STATUS "Rust toolchain information:")
message(STATUS "  ${RUSTC_VERSION}")
message(STATUS "  ${CARGO_VERSION}")


###############################################################################
# Build Targets

# Custom target to build the qt-viewer
# Note that the stdgeo-qt-viewer target is defined in the qt-viewer directory
add_custom_target(qt_viewer DEPENDS stdgeo-qt-viewer)


# Rust build target
add_custom_target(
    cargo_build ALL
    COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR}/target
            ${CARGO_EXECUTABLE} build ${CARGO_BUILD_FLAG}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building Rust workspace with Cargo"
)

# Rust test target
add_custom_target(
    cargo_test
    COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR}/target
            ${CARGO_EXECUTABLE} test ${CARGO_BUILD_FLAG}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running Rust tests with Cargo"
    DEPENDS cargo_build
)

# Qt test target
add_custom_target(
    qt_test
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR} --target run_qt_tests
    COMMENT "Running Qt application tests"
    DEPENDS qt_viewer
)


# Rust Clippy target
add_custom_target(
    cargo_clippy
    COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR}/target
            ${CARGO_EXECUTABLE} clippy ${CARGO_BUILD_FLAG} -- -D warnings
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running Rust clippy linter"
    DEPENDS cargo_build
)

# Rust format-code target
add_custom_target(
    cargo_fmt
    COMMAND ${CARGO_EXECUTABLE} fmt
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Formatting Rust code"
)

# Copy binary to standard location
# Custom command that runs after the cargo_build target
add_custom_command(
    TARGET cargo_build POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${BIN_OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${LIB_OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_BINARY_DIR}/target/${TARGET_DIR}/stdgeo-cli
            ${BIN_OUTPUT_DIR}/stdgeo-cli
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_BINARY_DIR}/target/${TARGET_DIR}/libstdgeo.so
            ${LIB_OUTPUT_DIR}/libstdgeo.so
    COMMENT "Copying binary to output directory"
)

# TODO Poss We may want to change the destination
# Install target for stdgeo-cli
install(
    PROGRAMS ${BIN_OUTPUT_DIR}/stdgeo-cli
    DESTINATION bin
    COMPONENT Runtime
)

# TODO Poss We may want to change the destination
# TODO Poss We have a similar target in a sub CMakeLists file
# Install target for stdgeo-gui
install(
    TARGETS stdgeo-qt-viewer
    DESTINATION bin
    COMPONENT Runtime
)

# Clean target
add_custom_target(
    cargo_clean
    COMMAND ${CARGO_EXECUTABLE} clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Cleaning Rust build artifacts"
)

# Documentation target
add_custom_target(
    cargo_doc
    COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR}/target
            ${CARGO_EXECUTABLE} doc ${CARGO_BUILD_FLAG} --no-deps
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building Rust documentation"
)

# Benchmark target (if you add benchmarks later)
add_custom_target(
    cargo_bench
    COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR}/target
            ${CARGO_EXECUTABLE} bench ${CARGO_BUILD_FLAG}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running Rust benchmarks"
    DEPENDS cargo_build
)

# Add an alias for the main build target
add_custom_target(all_targets DEPENDS cargo_build cargo_test cargo_clippy qt_viewer)


