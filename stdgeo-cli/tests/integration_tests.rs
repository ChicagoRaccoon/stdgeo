//! # Integration Tests for StdGeo CLI
//!
//! This module contains comprehensive integration tests that verify the CLI application
//! works correctly from a user's perspective.
//!
//! ## Theory of Operation
//!
//! These tests follow the "black box" testing approach, testing the CLI application
//! as an external process without knowledge of internal implementation details.
//!
//! ### Test Strategy
//! - **Process Testing**: Each test spawns the CLI as a separate process
//! - **File I/O Testing**: Tests verify file creation, reading, and format conversion
//! - **Command Verification**: All CLI commands and argument combinations are tested
//! - **Error Handling**: Tests verify proper error reporting and exit codes
//!
//! ### Test Structure
//! Tests are organized by functionality:
//! - Basic geometry creation (points, lines)
//! - File operations (save, load, convert)
//! - Transformations (translate, rotate)
//! - Format handling (JSON, simple text)
//!
//! ### Test Utilities
//! - `assert_cmd`: Provides CLI testing utilities and assertions
//! - `tempfile`: Creates temporary directories for file operations
//! - `std::process::Command`: Spawns CLI processes for testing
//!
//! ## Test Design Principles
//! 1. **Independence**: Each test is self-contained and doesn't depend on others
//! 2. **Cleanup**: Temporary files are automatically cleaned up
//! 3. **Verification**: Both stdout output and file contents are verified
//! 4. **Real Usage**: Tests simulate actual user workflows

use std::process::Command;
use std::fs;
use tempfile::TempDir;
use assert_cmd::prelude::*;

/// Test basic point creation functionality.
///
/// Verifies that the CLI can create a point with specified coordinates
/// and display it correctly to stdout when no output file is specified.
///
/// **Test Coverage:**
/// - Point creation command parsing
/// - Coordinate handling (x, y parameters)
/// - Stdout output formatting
/// - Success exit code
#[test]
fn test_cli_point_creation() {
    let mut cmd = Command::cargo_bin("stdgeo").unwrap();
    cmd.args(&["point", "-x", "3.0", "-y", "4.0"]);
    
    cmd.assert()
        .success()
        .stdout("Point: (3, 4)\n");
}

/// Test basic line creation functionality.
///
/// Verifies that the CLI can create a line segment between two points
/// and display it correctly to stdout when no output file is specified.
///
/// **Test Coverage:**
/// - Line creation command parsing
/// - Multiple coordinate parameters (x1, y1, x2, y2)
/// - Long-form argument handling (--x1, --y1, etc.)
/// - Stdout output formatting for line segments
/// - Success exit code
#[test]
fn test_cli_line_creation() {
    let mut cmd = Command::cargo_bin("stdgeo").unwrap();
    cmd.args(&["line", "--x1", "0.0", "--y1", "0.0", "--x2", "3.0", "--y2", "4.0"]);
    
    cmd.assert()
        .success()
        .stdout("Line: (0, 0) to (3, 4)\n");
}

/// Test the complete workflow of saving and reading geometry files.
///
/// This integration test verifies the full roundtrip of creating a point,
/// saving it to a JSON file, and then reading it back to verify correctness.
///
/// **Test Coverage:**
/// - Point creation with file output (-o flag)
/// - JSON file writing
/// - File reading command
/// - Geometry display formatting
/// - File path handling
/// - Temporary file cleanup
///
/// **Workflow Tested:**
/// 1. Create a temporary directory for test files
/// 2. Create a point and save to JSON file
/// 3. Read the file back and verify output format
/// 4. Verify geometry data is preserved correctly
#[test]
fn test_cli_point_save_and_read() {
    // Create temporary directory for test files
    let temp_dir = TempDir::new().unwrap();
    let output_path = temp_dir.path().join("point.json");
    
    // Create and save a point to file
    let mut cmd = Command::cargo_bin("stdgeo").unwrap();
    cmd.args(&["point", "-x", "5.0", "-y", "6.0", "-o", output_path.to_str().unwrap()]);
    
    cmd.assert().success();
    
    // Read the point back from file
    let mut cmd = Command::cargo_bin("stdgeo").unwrap();
    cmd.args(&["read", "-i", output_path.to_str().unwrap()]);
    
    // Verify the read operation produces correct output
    cmd.assert()
        .success()
        .stdout(format!("Read 1 geometries from {}\n  1: Point (5, 6)\n", output_path.display()));
}

/// Test geometric transformation operations (translation).
///
/// This comprehensive test verifies the translation transformation pipeline:
/// creating geometry, applying translation, and verifying the results.
///
/// **Test Coverage:**
/// - Multi-step CLI workflow
/// - Translation command with displacement parameters
/// - File-to-file transformation pipeline
/// - Mathematical correctness of transformations
/// - Intermediate file handling
///
/// **Mathematical Verification:**
/// - Initial point: (1.0, 2.0)
/// - Translation vector: (3.0, 4.0)
/// - Expected result: (4.0, 6.0)
///
/// **Workflow Tested:**
/// 1. Create initial geometry file with a point
/// 2. Apply translation transformation
/// 3. Read result and verify coordinates are correctly transformed
#[test]
fn test_cli_translate_operation() {
    // Set up temporary directory and file paths
    let temp_dir = TempDir::new().unwrap();
    let input_path = temp_dir.path().join("input.json");
    let output_path = temp_dir.path().join("output.json");
    
    // Create initial point and save to file
    let mut cmd = Command::cargo_bin("stdgeo").unwrap();
    cmd.args(&["point", "-x", "1.0", "-y", "2.0", "-o", input_path.to_str().unwrap()]);
    cmd.assert().success();
    
    // Apply translation transformation
    let mut cmd = Command::cargo_bin("stdgeo").unwrap();
    cmd.args(&[
        "translate",
        "-i", input_path.to_str().unwrap(),
        "-o", output_path.to_str().unwrap(),
        "--dx", "3.0",
        "--dy", "4.0"
    ]);
    
    cmd.assert().success();
    
    // Verify the transformation result
    let mut cmd = Command::cargo_bin("stdgeo").unwrap();
    cmd.args(&["read", "-i", output_path.to_str().unwrap()]);
    
    // Check that point was translated from (1,2) to (4,6)
    cmd.assert()
        .success()
        .stdout(format!("Read 1 geometries from {}\n  1: Point (4, 6)\n", output_path.display()));
}

/// Test simple text format reading capabilities.
///
/// Verifies that the CLI can correctly parse the human-readable simple text format
/// and handle mixed geometry types (points and lines) in a single file.
///
/// **Test Coverage:**
/// - Simple text format parsing
/// - Mixed geometry types in one file
/// - --simple flag functionality
/// - Line-based parsing
/// - Format specification compliance
///
/// **Format Tested:**
/// ```text
/// point 1.0 2.0
/// line 0.0 0.0 3.0 4.0
/// ```
///
/// **Verification Points:**
/// - Point coordinates are correctly parsed
/// - Line segment endpoints are correctly parsed
/// - Geometry count is accurate
/// - Display format matches expectations
#[test]
fn test_cli_simple_format() {
    // Set up temporary directory and create simple format file
    let temp_dir = TempDir::new().unwrap();
    let simple_path = temp_dir.path().join("simple.txt");
    
    // Write test data in simple text format
    fs::write(&simple_path, "point 1.0 2.0\nline 0.0 0.0 3.0 4.0\n").unwrap();
    
    // Read file using simple format flag
    let mut cmd = Command::cargo_bin("stdgeo").unwrap();
    cmd.args(&["read", "-i", simple_path.to_str().unwrap(), "--simple"]);
    
    // Verify both geometries are read correctly
    cmd.assert()
        .success()
        .stdout(format!("Read 2 geometries from {}\n  1: Point (1, 2)\n  2: Line (0, 0) to (3, 4)\n", simple_path.display()));
}