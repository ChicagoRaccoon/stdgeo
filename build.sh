#!/bin/bash

set -e

echo "Building StdGeo Qt GUI Application..."

# Check if Qt6 is available
if ! command -v qmake6 >/dev/null 2>&1 && ! command -v qmake >/dev/null 2>&1; then
    echo "Error: Qt6 not found. Please install Qt6 development packages."
    echo "On Ubuntu/Debian: sudo apt install qt6-base-dev qt6-base-dev-tools libqt6opengl6-dev"
    echo "On Fedora: sudo dnf install qt6-qtbase-devel qt6-qttools-devel qt6-qtopengl-devel"
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