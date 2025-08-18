use std::process::Command;
use std::fs;
use tempfile::TempDir;
use assert_cmd::prelude::*;

#[test]
fn test_cli_point_creation() {
    let mut cmd = Command::cargo_bin("stdgeo").unwrap();
    cmd.args(&["point", "-x", "3.0", "-y", "4.0"]);
    
    cmd.assert()
        .success()
        .stdout("Point: (3, 4)\n");
}

#[test]
fn test_cli_line_creation() {
    let mut cmd = Command::cargo_bin("stdgeo").unwrap();
    cmd.args(&["line", "--x1", "0.0", "--y1", "0.0", "--x2", "3.0", "--y2", "4.0"]);
    
    cmd.assert()
        .success()
        .stdout("Line: (0, 0) to (3, 4)\n");
}

#[test]
fn test_cli_point_save_and_read() {
    let temp_dir = TempDir::new().unwrap();
    let output_path = temp_dir.path().join("point.json");
    
    let mut cmd = Command::cargo_bin("stdgeo").unwrap();
    cmd.args(&["point", "-x", "5.0", "-y", "6.0", "-o", output_path.to_str().unwrap()]);
    
    cmd.assert().success();
    
    let mut cmd = Command::cargo_bin("stdgeo").unwrap();
    cmd.args(&["read", "-i", output_path.to_str().unwrap()]);
    
    cmd.assert()
        .success()
        .stdout(format!("Read 1 geometries from {}\n  1: Point (5, 6)\n", output_path.display()));
}

#[test]
fn test_cli_translate_operation() {
    let temp_dir = TempDir::new().unwrap();
    let input_path = temp_dir.path().join("input.json");
    let output_path = temp_dir.path().join("output.json");
    
    let mut cmd = Command::cargo_bin("stdgeo").unwrap();
    cmd.args(&["point", "-x", "1.0", "-y", "2.0", "-o", input_path.to_str().unwrap()]);
    cmd.assert().success();
    
    let mut cmd = Command::cargo_bin("stdgeo").unwrap();
    cmd.args(&[
        "translate",
        "-i", input_path.to_str().unwrap(),
        "-o", output_path.to_str().unwrap(),
        "--dx", "3.0",
        "--dy", "4.0"
    ]);
    
    cmd.assert().success();
    
    let mut cmd = Command::cargo_bin("stdgeo").unwrap();
    cmd.args(&["read", "-i", output_path.to_str().unwrap()]);
    
    cmd.assert()
        .success()
        .stdout(format!("Read 1 geometries from {}\n  1: Point (4, 6)\n", output_path.display()));
}

#[test]
fn test_cli_simple_format() {
    let temp_dir = TempDir::new().unwrap();
    let simple_path = temp_dir.path().join("simple.txt");
    
    fs::write(&simple_path, "point 1.0 2.0\nline 0.0 0.0 3.0 4.0\n").unwrap();
    
    let mut cmd = Command::cargo_bin("stdgeo").unwrap();
    cmd.args(&["read", "-i", simple_path.to_str().unwrap(), "--simple"]);
    
    cmd.assert()
        .success()
        .stdout(format!("Read 2 geometries from {}\n  1: Point (1, 2)\n  2: Line (0, 0) to (3, 4)\n", simple_path.display()));
}