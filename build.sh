#!/bin/bash

# stdgeo build script - A wrapper around CMake for the Rust CLI geometry application

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
BUILD_TYPE="Release"
BUILD_DIR="build"
CLEAN=false
TEST=false
INSTALL=false
INSTALL_PREFIX="/usr/local"
VERBOSE=false
JOBS=$(nproc)

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Help function
show_help() {
    cat << EOF
stdgeo build script - Wrapper around CMake for Rust CLI geometry application

USAGE:
    $0 [OPTIONS]

OPTIONS:
    -t, --type TYPE         Build type: Debug or Release (default: Release)
    -d, --build-dir DIR     Build directory (default: build)
    -c, --clean             Clean build directory before building
    -T, --test              Run tests after building
    -i, --install           Install after building
    -p, --prefix PATH       Installation prefix (default: /usr/local)
    -j, --jobs N            Number of parallel jobs (default: $(nproc))
    -v, --verbose           Verbose output
    -h, --help              Show this help message

EXAMPLES:
    $0                      # Build in release mode
    $0 -t Debug -T          # Debug build with tests
    $0 -c -i                # Clean build and install
    $0 -t Debug -v -T       # Debug build with verbose output and tests

TARGETS:
    The script supports the following CMake targets:
    - cargo_build           Build the Rust workspace
    - cargo_test            Run all tests
    - cargo_clippy          Run clippy linter
    - cargo_fmt             Format code
    - cargo_clean           Clean build artifacts
    - cargo_doc             Generate documentation
    - all_targets           Build, test, and lint

EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -t|--type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -d|--build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -T|--test)
            TEST=true
            shift
            ;;
        -i|--install)
            INSTALL=true
            shift
            ;;
        -p|--prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use -h or --help for usage information."
            exit 1
            ;;
    esac
done

# Validate build type
if [[ "$BUILD_TYPE" != "Debug" && "$BUILD_TYPE" != "Release" ]]; then
    print_error "Invalid build type: $BUILD_TYPE. Must be 'Debug' or 'Release'."
    exit 1
fi

# Set verbose flag
if [[ "$VERBOSE" == true ]]; then
    CMAKE_VERBOSE="VERBOSE=1"
else
    CMAKE_VERBOSE=""
fi

print_status "Starting stdgeo build process..."
print_status "Build type: $BUILD_TYPE"
print_status "Build directory: $BUILD_DIR"
print_status "Jobs: $JOBS"

# Check if required tools are available
if ! command -v cmake &> /dev/null; then
    print_error "CMake is not installed or not in PATH"
    exit 1
fi

if ! command -v cargo &> /dev/null; then
    print_error "Cargo is not installed or not in PATH"
    exit 1
fi

# Clean build directory if requested
if [[ "$CLEAN" == true ]]; then
    print_status "Cleaning build directory..."
    if [[ -d "$BUILD_DIR" ]]; then
        rm -rf "$BUILD_DIR"
        print_success "Build directory cleaned"
    else
        print_warning "Build directory doesn't exist, nothing to clean"
    fi
fi

# Create and enter build directory
print_status "Setting up build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
print_status "Configuring with CMake..."
cmake_cmd="cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE"

if [[ "$INSTALL" == true ]]; then
    cmake_cmd="$cmake_cmd -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX"
fi

cmake_cmd="$cmake_cmd .."

print_status "Running: $cmake_cmd"
eval $cmake_cmd

if [[ $? -ne 0 ]]; then
    print_error "CMake configuration failed"
    exit 1
fi

print_success "CMake configuration completed"

# Build
print_status "Building project..."
cmake_build_cmd="cmake --build . --config $BUILD_TYPE -j $JOBS"

if [[ "$VERBOSE" == true ]]; then
    cmake_build_cmd="$cmake_build_cmd -- $CMAKE_VERBOSE"
fi

print_status "Running: $cmake_build_cmd"
eval $cmake_build_cmd

if [[ $? -ne 0 ]]; then
    print_error "Build failed"
    exit 1
fi

print_success "Build completed successfully"

# Run tests if requested
if [[ "$TEST" == true ]]; then
    print_status "Running tests..."
    cmake --build . --target cargo_test
    
    if [[ $? -ne 0 ]]; then
        print_error "Tests failed"
        exit 1
    fi
    
    print_success "All tests passed"
fi

# Install if requested
if [[ "$INSTALL" == true ]]; then
    print_status "Installing to $INSTALL_PREFIX..."
    cmake --build . --target install
    
    if [[ $? -ne 0 ]]; then
        print_error "Installation failed"
        exit 1
    fi
    
    print_success "Installation completed"
fi

print_success "stdgeo build process completed successfully!"

# Show binary location
if [[ -f "bin/stdgeo" ]]; then
    BINARY_PATH=$(realpath "bin/stdgeo")
    print_status "Binary available at: $BINARY_PATH"
    
    # Show basic usage
    echo
    print_status "To test the CLI:"
    echo "  $BINARY_PATH --help"
    echo "  $BINARY_PATH point -x 1.0 -y 2.0"
    echo "  $BINARY_PATH line --x1 0.0 --y1 0.0 --x2 3.0 --y2 4.0"
fi