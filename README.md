# StdGeo 3D Geometry Viewer

A desktop application for viewing and manipulating 3D geometry with a Rust core and Qt frontend.

## Features

- **3D Geometry Library**: Core geometric types (Point3D, Vector3D, Triangle, Mesh) implemented in Rust
- **3D Viewer**: OpenGL-based 3D visualization with mouse navigation
- **Command Line Interface**: Interactive command execution within the application
- **Extensible Toolbars**: Customizable toolbars for geometry operations
- **Cross-Platform**: Built with Qt6 and Rust for multi-platform support

## Architecture

**Headless-First Design:**
- **Primary Command** (`stdgeo`): Pure Rust headless command-line tool (src/bin/stdgeo.rs)
- **GUI Application** (`stdgeo_viewer`): Qt-based GUI that provides visual abstraction over the headless core
- **Shared Core** (`src/geo/command.rs`): Unified command processor used by both CLI and GUI
- **Geometry Library** (`src/geo/geometry.rs`): Core 3D geometry types and operations
- **C FFI Layer** (`src/geo/ffi.rs`): Bridge between Rust core and Qt GUI

**Key Benefits:**
- Single source of truth for geometry operations
- Consistent behavior between CLI and GUI modes
- Easy automation and scripting
- Clean separation of concerns

## Dependencies

### System Requirements
- Qt6 (Core, Widgets, OpenGL, OpenGLWidgets)
- OpenGL
- Rust toolchain (cargo)
- CMake 3.16+
- C++17 compiler

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-opengl-dev libgl1-mesa-dev cmake build-essential
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

### Arch Linux
```bash
sudo pacman -S qt6-base qt6-opengl cmake rust
```

### macOS
```bash
brew install qt cmake rust
```

## Building

### Quick Build
```bash
./build.sh
```

### Manual Build
```bash
# Build Rust library
cargo build --release

# Build Qt application
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## Running

### Primary Command: stdgeo (Headless-First)
```bash
# Interactive command line
./target/release/stdgeo
./target/release/stdgeo --interactive  # Force interactive mode

# Scripted batch processing
cat script.txt | ./target/release/stdgeo
./target/release/stdgeo < script.txt

# Single command execution  
./target/release/stdgeo --command "cube 3.0"
echo "cube 3.0" | ./target/release/stdgeo
```

### GUI Mode (Built on Headless Core)
```bash
./build/stdgeo_viewer                   # GUI application
./build/stdgeo_viewer --headless        # GUI app in headless mode (legacy)
```

## Usage

### 3D Viewer
- **Mouse Navigation**: 
  - Left click + drag: Rotate camera
  - Scroll wheel: Zoom in/out
- **Menu Actions**: File operations, geometry creation
- **Toolbar**: Quick access to geometry tools

### Command Line Interface
Available in both GUI and headless modes:
- `cube [size]` - Create a cube (default size: 2.0)
- `clear` - Clear current geometry
- `stats` - Show geometry statistics
- `help` - Show available commands
- `exit`/`quit` - Exit application (headless mode only)

#### GUI Mode Examples:
```
> cube 3.0
Created cube with size 3.0

> clear
Cleared geometry
```

#### Headless Mode Examples:
```bash
# Interactive session
$ ./target/release/stdgeo
> cube 5.0
Created cube (size: 5) with 8 vertices and 12 triangles
> stats
Current geometry: 8 vertices, 12 triangles
Sample vertices:
  [0]: (-2.5, -2.5, -2.5)
  [1]: (2.5, -2.5, -2.5)
  [2]: (2.5, 2.5, -2.5)
> exit

# Script processing
$ ./target/release/stdgeo < examples/headless_demo.txt
$ cat examples/headless_demo.txt | ./target/release/stdgeo

# Single commands
$ ./target/release/stdgeo --command "cube 3.0"
```

#### Headless Script Format:
```bash
# Comments start with #
cube 2.0
stats
clear
exit
```

### Examples and Automation

The project includes comprehensive examples for various use cases:

```bash
# List all available examples
./list_examples.sh

# Run specific examples (primary method)
./target/release/stdgeo < examples/basic_operations.txt
./target/release/stdgeo < examples/batch_processing.txt
./target/release/stdgeo < examples/stress_test.txt

# Alternative convenience methods
./run_headless.sh -f examples/basic_operations.txt
./run_headless.sh -f examples/batch_processing.txt
./run_headless.sh -f examples/stress_test.txt

# Validate all examples work correctly
./test.sh --examples
```

**Available Examples:**
- **basic_operations.txt** - Fundamental cube creation and manipulation
- **batch_processing.txt** - Automated geometry processing workflows  
- **size_comparison.txt** - Compare cubes of different sizes
- **stress_test.txt** - System performance testing with rapid operations
- **error_handling.txt** - Edge cases and error condition testing
- **mathematical_sequences.txt** - Cubes following mathematical patterns
- **performance_benchmark.txt** - Geometry operation timing measurements
- **validation_workflow.txt** - Systematic geometry property testing
- **ci_cd_automation.txt** - Continuous integration testing scripts
- **data_validation.txt** - Mathematical correctness validation

## Project Structure

```
stdgeo/
├── src/
│   ├── lib.rs              # Rust library entry point
│   ├── main.rs             # Rust test binary
│   ├── bin/
│   │   └── stdgeo.rs       # Primary headless command binary
│   ├── geo/                # Geometry library module
│   │   ├── mod.rs          # Module declaration
│   │   ├── geometry.rs     # Core geometry types
│   │   ├── command.rs      # Headless command processor (core)
│   │   └── ffi.rs          # C FFI bindings
│   └── qt/                 # Qt GUI application (uses headless core)
│       ├── main.cpp        # Qt application entry
│       ├── mainwindow.*    # Main application window
│       ├── geometryviewer.*# OpenGL 3D viewer widget
│       ├── commandline.*   # Command line interface widget
│       └── headless.*      # Legacy headless mode (Qt-based)
├── tests/                  # C++ test suites
│   ├── test_ffi_integration.cpp
│   ├── test_geometry_viewer.cpp
│   └── test_headless_operations.cpp
├── examples/               # Example scripts for automation
├── include/
│   └── stdgeo.h           # C header for Rust library
├── CMakeLists.txt         # CMake build configuration
├── Cargo.toml             # Rust package configuration
├── build.sh               # Build script
└── test.sh                # Unified test runner
```

## Testing

The project includes comprehensive test suites for all components:

### Unified Test Runner
```bash
# Quick development testing (recommended for TDD)
./test.sh --quick

# Full comprehensive testing
./test.sh --full

# CI/CD friendly testing (uses CTest)
./test.sh --ci

# Individual test suites
./test.sh --rust     # Rust tests only
./test.sh --cpp      # C++ tests only
./test.sh --examples # Example validation only

# Complete testing including examples
./test.sh --all      # Everything: Rust, C++, examples, integration
```

The test suite includes:
- Rust unit tests (23+ tests)
- C FFI integration tests
- Qt component tests (with Google Test)
- Memory safety tests (with Valgrind)
- Build verification
- End-to-end integration tests

### Individual Test Suites

#### Rust Tests
```bash
# All Rust tests
cargo test

# Just geometry library tests
cargo test --lib

# Just FFI integration tests
cargo test ffi_tests
```

#### C++ Tests (requires Google Test)
```bash
cd build
./stdgeo_tests

# Run specific test suites
./stdgeo_tests --gtest_filter="FFIIntegrationTest.*"
./stdgeo_tests --gtest_filter="GeometryViewerTest.*"
./stdgeo_tests --gtest_filter="HeadlessOperationTest.*"
```

#### CMake CTest
```bash
cd build
ctest --output-on-failure
```

### Test Dependencies
- **Google Test**: For C++ unit testing
  ```bash
  sudo apt install libgtest-dev
  ```
- **Valgrind**: For memory safety testing
  ```bash
  sudo apt install valgrind
  ```

#### Example Validation
```bash
# Test all example scripts
./test.sh --examples

# Complete test suite including examples
./test.sh --all
```

### Test Coverage
- ✅ **Rust Geometry Library**: Complete unit test coverage (23+ tests)
- ✅ **C FFI Interface**: Integration and safety tests (10+ tests)  
- ✅ **Qt Components**: UI component and interaction tests (11+ tests)
- ✅ **Headless Operations**: Command-line interface testing (16+ tests)
- ✅ **Example Scripts**: Automated validation of all examples (11+ scripts)
- ✅ **Memory Safety**: Valgrind integration
- ✅ **Build System**: Cross-platform build verification
- ✅ **Integration**: End-to-end application testing

## Development

### Testing Rust Library
```bash
cargo run
```

### Adding New Geometry Types
1. Add the type to `src/geo/geometry.rs`
2. Add C FFI functions in `src/geo/ffi.rs`
3. Update `include/stdgeo.h`
4. Add Qt integration in the viewer
5. Add tests to both Rust and C++ test suites

### Extending Command Interface
Add new commands in `MainWindow::onCommandExecuted()` in `src/qt/mainwindow.cpp`

### Test-Driven Development
1. Write tests first in the appropriate test suite
2. Implement functionality to make tests pass
3. Run `./test_quick.sh` for rapid feedback
4. Run `./run_tests.sh` before committing changes

## License

This project is open source. See individual file headers for specific licensing information.
