cmake_minimum_required(VERSION 3.16)
project(stdgeo LANGUAGES NONE)

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
add_custom_target(all_targets DEPENDS cargo_build cargo_test cargo_clippy)

# Print build information
message(STATUS "Rust toolchain information:")
execute_process(COMMAND ${RUSTC_EXECUTABLE} --version OUTPUT_VARIABLE RUSTC_VERSION)
execute_process(COMMAND ${CARGO_EXECUTABLE} --version OUTPUT_VARIABLE CARGO_VERSION)
message(STATUS "  ${RUSTC_VERSION}")
message(STATUS "  ${CARGO_VERSION}")
