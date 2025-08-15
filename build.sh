#!/bin/bash

# Build script for StdGeo Viewer
set -e

echo "Building StdGeo 3D Geometry Viewer..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
echo "Building..."
make -j$(nproc)

echo "Build complete!"
echo ""
echo "Available commands:"
echo "  ./target/release/stdgeo        # Headless geometry tool (primary)"
echo "  ./build/stdgeo_viewer          # GUI application"
echo "  ./build/stdgeo_viewer --headless  # GUI app in headless mode"