cmake_minimum_required(VERSION 3.16)
project(StdGeoViewer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets Test)

# Enable automatic MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Build Rust library and binaries
add_custom_target(rust_build ALL
    COMMAND cargo build --release
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Building Rust geometry library and binaries"
)

# Install Rust binaries
install(PROGRAMS ${CMAKE_SOURCE_DIR}/target/release/stdgeo
    DESTINATION bin
)

# Add the Rust library
set(RUST_LIB_DIR ${CMAKE_SOURCE_DIR}/target/release)
if(WIN32)
    set(RUST_LIB_NAME "stdgeo.dll")
elseif(APPLE)
    set(RUST_LIB_NAME "libstdgeo.dylib")
else()
    set(RUST_LIB_NAME "libstdgeo.so")
endif()

add_library(stdgeo_rust SHARED IMPORTED)
set_target_properties(stdgeo_rust PROPERTIES
    IMPORTED_LOCATION ${RUST_LIB_DIR}/${RUST_LIB_NAME}
)
add_dependencies(stdgeo_rust rust_build)

# Qt application sources
set(SOURCES
    src/qt/main.cpp
    src/qt/mainwindow.cpp
    src/qt/geometryviewer.cpp
    src/qt/commandline.cpp
    src/qt/headless.cpp
)

set(HEADERS
    src/qt/mainwindow.h
    src/qt/geometryviewer.h
    src/qt/commandline.h
    src/qt/headless.h
)

# Create the Qt application
add_executable(stdgeo_viewer ${SOURCES} ${HEADERS})

# Link Qt libraries
target_link_libraries(stdgeo_viewer
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    OpenGL::GL
    stdgeo_rust
)

# Set target dependencies
add_dependencies(stdgeo_viewer rust_build)

# Install targets
install(TARGETS stdgeo_viewer
    RUNTIME DESTINATION bin
)

# Enable testing
enable_testing()

# Find Google Test
find_package(GTest QUIET)
if(GTest_FOUND OR GTEST_FOUND)
    message(STATUS "Google Test found, enabling C++ tests")
    
    # Test sources (include Qt sources that tests depend on)
    set(TEST_SOURCES
        tests/test_ffi_integration.cpp
        tests/test_geometry_viewer.cpp
        tests/test_headless_operations.cpp
        src/qt/geometryviewer.cpp
        src/qt/commandline.cpp
        src/qt/mainwindow.cpp
        src/qt/headless.cpp
    )
    
    # Create test executable
    add_executable(stdgeo_tests ${TEST_SOURCES})
    
    # Link test libraries
    target_link_libraries(stdgeo_tests
        GTest::gtest
        GTest::gtest_main
        Qt6::Core
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::OpenGLWidgets
        Qt6::Test
        OpenGL::GL
        stdgeo_rust
    )
    
    # Set target dependencies
    add_dependencies(stdgeo_tests rust_build)
    
    # Add tests to CTest
    add_test(NAME FFIIntegrationTests COMMAND stdgeo_tests --gtest_filter="FFIIntegrationTest.*")
    add_test(NAME GeometryViewerTests COMMAND stdgeo_tests --gtest_filter="GeometryViewerTest.*")
    add_test(NAME CommandLineTests COMMAND stdgeo_tests --gtest_filter="CommandLineTest.*")
    add_test(NAME MainWindowTests COMMAND stdgeo_tests --gtest_filter="MainWindowTest.*")
    add_test(NAME IntegrationTests COMMAND stdgeo_tests --gtest_filter="IntegrationTest.*")
    add_test(NAME HeadlessOperationTests COMMAND stdgeo_tests --gtest_filter="HeadlessOperationTest.*")
    
else()
    message(WARNING "Google Test not found, C++ tests will not be built")
    message(STATUS "Install Google Test with: sudo apt install libgtest-dev cmake")
endif()

# Add Rust tests
add_test(NAME RustUnitTests 
    COMMAND cargo test
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)