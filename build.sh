#!/bin/bash

set -e

# Check for command line arguments

# Target: clean
if [ "$1" = "clean" ] || [ "$1" = "-c" ] || [ "$1" = "--clean" ]; then
    echo "Cleaning StdGeo Qt GUI Application..."
    
    # Define the build directory
    BUILD_DIR="build"

    # If the build directory exists
    if [ -d "$BUILD_DIR" ]; then
        echo "Removing build directory: $BUILD_DIR"
        rm -rf "$BUILD_DIR"
        echo "Clean completed successfully!"
    else
        echo "Build directory does not exist, nothing to clean."
    fi

    exit 0

# Target: help
elif [ "$1" = "help" ] || [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    echo "StdGeo Qt GUI Application Build Script"
    echo ""
    echo "USAGE:"
    echo "  $0 [COMMAND]"
    echo ""
    echo "COMMANDS:"
    echo "  (no args)     Build the Qt OpenGL viewer application"
    echo "  clean, -c     Remove all build artifacts and the build directory"
    echo "  help, -h      Show this help message"
    echo ""
    echo "DESCRIPTION:"
    echo "  This script builds a Qt6-based OpenGL viewer with interactive controls."
    echo "  The application features a 3D cube that can be rotated, panned, and"
    echo "  zoomed using mouse controls, along with a text editor panel."
    echo ""
    echo "REQUIREMENTS:"
    echo "  - Qt 6.9.1 installed at ../../Qt/6.9.1/gcc_64 (relative to this script)"
    echo "  - CMake 3.16 or later"
    echo "  - C++ compiler with C++17 support"
    echo ""
    echo "OUTPUT:"
    echo "  The built executable will be located at: ./build/stdgeo-gui"
    exit 0
fi

echo "Building StdGeo Qt GUI Application..."

# Set Qt6 path using relative path
QT_DIR="../../Qt/6.9.1/gcc_64"

# Get the absolute path of the Qt directory,
# falling back to relative path if realpath fails
QT_ABS_PATH="$(realpath "$QT_DIR" 2>/dev/null || echo "$QT_DIR")"

# Check if the Qt directory exists
if [ -d "$QT_DIR" ]; then
    echo "Using Qt6 from: $QT_ABS_PATH"

    # Add Qt6 binaries (qmake, etc.) to PATH so they can be found
    export PATH="$QT_ABS_PATH/bin:$PATH"
    
    # Tell CMake where to find Qt6 installation
    export CMAKE_PREFIX_PATH="$QT_ABS_PATH"
    
    # Add Qt6 libraries to library search path for runtime linking
    export LD_LIBRARY_PATH="$QT_ABS_PATH/lib:$LD_LIBRARY_PATH"
else
    # Directory doesn't exist - show error and exit
    echo "Error: Qt6 not found at $QT_DIR"
    echo "Please ensure Qt 6.9.1 is installed in the parent directory."
    echo "Expected path: $QT_ABS_PATH"
    exit 1
fi

# Create build directory
BUILD_DIR="build"
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Configure with CMake
echo "Configuring with CMake..."
cmake ../stdgeo-gui

# Build
echo "Building..."
make -j$(nproc)

echo "Build completed successfully!"
echo "Executable location: $PWD/stdgeo-gui"
echo ""
echo "To run the application:"
echo "  ./build/stdgeo-gui"
