#!/bin/bash

# StdGeo GUI Build Script
# Usage: ./build.sh [debug|release] [clean]

set -e  # Exit on error

# Default build type
BUILD_TYPE="Release"

# Parse arguments
for arg in "$@"; do
    case $arg in
        debug|Debug|DEBUG)
            BUILD_TYPE="Debug"
            ;;
        release|Release|RELEASE)
            BUILD_TYPE="Release"
            ;;
        clean|Clean|CLEAN)
            echo "Cleaning build directories..."
            rm -rf stdgeo-gui/build
            echo "Clean complete."
            exit 0
            ;;
        *)
            echo "Usage: $0 [debug|release] [clean]"
            echo "  debug/release: Set build type (default: release)"
            echo "  clean: Remove build directories"
            exit 1
            ;;
    esac
done

echo "======================================"
echo "Building StdGeo GUI"
echo "Build Type: $BUILD_TYPE"
echo "======================================"

# Create build directory
BUILD_DIR="stdgeo-gui/build"
mkdir -p "$BUILD_DIR"

# Configure with CMake
echo ""
echo "Configuring CMake..."
cd "$BUILD_DIR"
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..

# Build
echo ""
echo "Building..."
cmake --build . -j$(nproc)

echo ""
echo "======================================"
echo "Build complete!"
echo "======================================"
echo "Executable location: $BUILD_DIR/stdgeo-gui"
echo ""
echo "To run the application:"
echo "  cd $BUILD_DIR && ./stdgeo-gui"
echo ""
echo "Or from the project root:"
echo "  ./stdgeo-gui/build/stdgeo-gui"