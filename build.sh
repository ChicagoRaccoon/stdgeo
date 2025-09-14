#!/bin/bash

set -e

echo "Building StdGeo Qt GUI Application..."

# Set Qt6 path using relative path
QT_DIR="../../Qt/6.9.1/gcc_64"
QT_ABS_PATH="$(realpath "$QT_DIR" 2>/dev/null || echo "$QT_DIR")"
if [ -d "$QT_DIR" ]; then
    echo "Using Qt6 from: $QT_ABS_PATH"
    export PATH="$QT_ABS_PATH/bin:$PATH"
    export CMAKE_PREFIX_PATH="$QT_ABS_PATH"
    export LD_LIBRARY_PATH="$QT_ABS_PATH/lib:$LD_LIBRARY_PATH"
else
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
echo "  cd $BUILD_DIR && ./stdgeo-gui"
echo ""
echo "Or from the root directory:"
echo "  ./build/stdgeo-gui"