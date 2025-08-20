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
BUILD_TYPE="debug"
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
    -t, --type TYPE         Build type: debug or release (default: release)
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
    $0 -t debug -T          # debug build with tests
    $0 -c -i                # clean build and install
    $0 -t debug -v -T       # debug build with verbose output and tests

TARGETS:
    The script supports the following CMake targets:
    - cargo_build           Build the Rust workspace
    - cargo_test            Run Rust tests
    - cargo_clippy          Run clippy linter
    - cargo_fmt             Format code
    - cargo_clean           Clean build artifacts
    - cargo_doc             Generate documentation
    - qt_viewer             Build Qt6 geometry viewer
    - qt_test               Run Qt application tests
    - all_targets           Build, test, and lint (includes Qt if available)

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
if [[ "$BUILD_TYPE" != "debug" && "$BUILD_TYPE" != "release" ]]; then
    print_error "Invalid build type: $BUILD_TYPE. Must be 'debug' or 'release'."
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

# Configure Qt6 path if available
QT6_PATH="../../../Qt/6.9.1/gcc_64"
if [[ -d "$QT6_PATH" ]]; then
    export CMAKE_PREFIX_PATH="$QT6_PATH:$CMAKE_PREFIX_PATH"
    export PKG_CONFIG_PATH="$QT6_PATH/lib/pkgconfig:$PKG_CONFIG_PATH"
    export LD_LIBRARY_PATH="$QT6_PATH/lib:$LD_LIBRARY_PATH"
    print_status "Qt6 found at: $QT6_PATH"
else
    print_status "QT NOT FOUND at: $QT6_PATH"
fi

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
    print_status "Running Rust tests..."
    cmake --build . --target cargo_test
    
    if [[ $? -ne 0 ]]; then
        print_warning "Some Rust tests failed"
        # Continue to run Qt tests even if Rust tests fail
    else
        print_success "Rust tests passed"
    fi
    
    # Run Qt tests if Qt viewer was built
    if cmake --build . --target run_qt_tests &>/dev/null; then
        print_status "Running Qt tests..."
        cmake --build . --target run_qt_tests
        
        if [[ $? -ne 0 ]]; then
            print_warning "Qt tests failed or not available"
        else
            print_success "Qt tests passed"
        fi
    else
        print_warning "Qt tests not available (Qt6 not found or disabled)"
    fi
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

# Show binary locations
if [[ -f "bin/stdgeo" ]]; then
    BINARY_PATH=$(realpath "bin/stdgeo")
    print_status "Rust CLI binary available at: $BINARY_PATH"
    
    # Show basic usage
    echo
    print_status "To test the CLI:"
    echo "  $BINARY_PATH --help"
    echo "  $BINARY_PATH point -x 1.0 -y 2.0"
    echo "  $BINARY_PATH line --x1 0.0 --y1 0.0 --x2 3.0 --y2 4.0"
    echo "  $BINARY_PATH session"
fi

# Show Qt viewer if built
if [[ -f "qt-viewer/stdgeo-qt-viewer" ]]; then
    QT_BINARY_PATH=$(realpath "qt-viewer/stdgeo-qt-viewer")
    print_status "Qt GUI viewer available at: $QT_BINARY_PATH"
    echo
    print_status "To start the Qt viewer:"
    echo "  $QT_BINARY_PATH"
    echo "  # Features: Interactive 2D visualization, mouse controls, integrated terminal"
elif command -v qt6-config &> /dev/null || command -v qmake6 &> /dev/null; then
    print_warning "Qt6 found but Qt viewer not built. Check CMake configuration."
else
    print_warning "Qt viewer not available (Qt6 not found)"
fi