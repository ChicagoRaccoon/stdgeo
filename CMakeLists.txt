cmake_minimum_required(VERSION 3.16)
project(stdgeo LANGUAGES CXX)

# Find Rust/Cargo
find_program(CARGO_EXECUTABLE cargo REQUIRED)
find_program(RUSTC_EXECUTABLE rustc REQUIRED)

# Set build type specific configurations
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

# Set output directory
set(BINARY_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin")

# Custom target to build the Rust project
add_custom_target(
    cargo_build ALL
    COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR}/target
            ${CARGO_EXECUTABLE} build ${CARGO_BUILD_FLAG}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building Rust workspace with Cargo"
)

# Custom target to run tests
add_custom_target(
    cargo_test
    COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR}/target
            ${CARGO_EXECUTABLE} test ${CARGO_BUILD_FLAG}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running Rust tests with Cargo"
    DEPENDS cargo_build
)

# Qt tests target
if(BUILD_QT_VIEWER AND Qt6_FOUND)
    add_custom_target(
        qt_test
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR} --target run_qt_tests
        COMMENT "Running Qt application tests"
        DEPENDS qt_viewer
    )
endif()

# Custom target to run clippy
add_custom_target(
    cargo_clippy
    COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR}/target
            ${CARGO_EXECUTABLE} clippy ${CARGO_BUILD_FLAG} -- -D warnings
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running Rust clippy linter"
    DEPENDS cargo_build
)

# Custom target to format code
add_custom_target(
    cargo_fmt
    COMMAND ${CARGO_EXECUTABLE} fmt
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Formatting Rust code"
)

# Copy binary to standard location
add_custom_command(
    TARGET cargo_build POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${BINARY_OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_BINARY_DIR}/target/${TARGET_DIR}/stdgeo
            ${BINARY_OUTPUT_DIR}/stdgeo
    COMMENT "Copying binary to output directory"
)

# Install target
install(
    PROGRAMS ${BINARY_OUTPUT_DIR}/stdgeo
    DESTINATION bin
    COMPONENT Runtime
)

# Install Qt viewer if built
if(BUILD_QT_VIEWER AND Qt6_FOUND)
    install(
        TARGETS stdgeo-qt-viewer
        DESTINATION bin
        COMPONENT Runtime
    )
endif()

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

# Qt6 application
option(BUILD_QT_VIEWER "Build Qt6 geometry viewer" ON)

if(BUILD_QT_VIEWER)
    # Set Qt6 path if not already set
    if(NOT DEFINED Qt6_DIR AND EXISTS "${CMAKE_SOURCE_DIR}/../Qt/6.9.1/gcc_64")
        set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/../Qt/6.9.1/gcc_64" ${CMAKE_PREFIX_PATH})
    endif()
    
    find_package(Qt6 QUIET COMPONENTS Core Widgets OpenGL OpenGLWidgets)
    if(Qt6_FOUND)
        add_subdirectory(qt-viewer)
        add_custom_target(qt_viewer DEPENDS stdgeo-qt-viewer)
        message(STATUS "Qt6 found - building Qt viewer")
    else()
        message(WARNING "Qt6 not found - skipping Qt viewer")
        set(BUILD_QT_VIEWER OFF)
    endif()
endif()

# Add an alias for the main build target
if(BUILD_QT_VIEWER AND Qt6_FOUND)
    add_custom_target(all_targets DEPENDS cargo_build cargo_test cargo_clippy qt_viewer)
else()
    add_custom_target(all_targets DEPENDS cargo_build cargo_test cargo_clippy)
endif()

# Print build information
message(STATUS "Rust toolchain information:")
execute_process(COMMAND ${RUSTC_EXECUTABLE} --version OUTPUT_VARIABLE RUSTC_VERSION)
execute_process(COMMAND ${CARGO_EXECUTABLE} --version OUTPUT_VARIABLE CARGO_VERSION)
message(STATUS "  ${RUSTC_VERSION}")
message(STATUS "  ${CARGO_VERSION}")
