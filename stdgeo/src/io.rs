//! # I/O Module
//!
//! This module handles serialization and deserialization of geometric data
//! in multiple formats.
//!
//! ## Theory of Operation
//!
//! The I/O system supports two primary formats:
//!
//! ### JSON Format
//! - Uses serde_json for serialization
//! - Preserves full type information and precision
//! - Suitable for programmatic interchange
//! - Human-readable but verbose
//!
//! ### Simple Text Format
//! - Custom line-based format for human editing
//! - Each geometry object on a separate line
//! - Format: "point x y" or "line x1 y1 x2 y2"
//! - Supports comments (lines starting with '#')
//! - More compact and easier to edit manually
//!
//! ## Error Handling
//!
//! The module defines a comprehensive error type that wraps:
//! - File system I/O errors
//! - JSON parsing errors
//! - Format validation errors
//!
//! All functions return `Result<T, IoError>` for proper error propagation.
//!
//! ## Format Examples
//!
//! ### JSON Format
//! ```json
//! [
//!   { "Point": { "x": 1.0, "y": 2.0 } },
//!   { "Line": { "start": { "x": 0.0, "y": 0.0 }, "end": { "x": 3.0, "y": 4.0 } } }
//! ]
//! ```
//!
//! ### Simple Format
//! ```text
//! # Geometry file
//! point 1.0 2.0
//! line 0.0 0.0 3.0 4.0
//! ```

use crate::geometry::{Geometry, Point, Line};
use serde_json;
use std::fs;
use std::io::{self, Write};
use std::path::Path;

/// Comprehensive error type for I/O operations.
///
/// This enum captures all possible error conditions that can occur during
/// file I/O and parsing operations, providing context for error handling.
///
/// # Error Categories
/// - `FileError`: Underlying file system operations (read, write, permissions)
/// - `ParseError`: JSON parsing and serialization errors
/// - `InvalidFormat`: Custom format validation errors with descriptive messages
#[derive(Debug)]
pub enum IoError {
    /// Wraps standard I/O errors from file operations
    FileError(io::Error),
    /// Wraps JSON serialization/deserialization errors
    ParseError(serde_json::Error),
    /// Custom format validation errors with descriptive context
    InvalidFormat(String),
}

/// Automatic conversion from standard I/O errors.
///
/// This allows using the `?` operator with functions that return `io::Error`,
/// automatically wrapping them in `IoError::FileError`.
impl From<io::Error> for IoError {
    fn from(error: io::Error) -> Self {
        IoError::FileError(error)
    }
}

/// Automatic conversion from JSON parsing errors.
///
/// This allows using the `?` operator with serde_json operations,
/// automatically wrapping them in `IoError::ParseError`.
impl From<serde_json::Error> for IoError {
    fn from(error: serde_json::Error) -> Self {
        IoError::ParseError(error)
    }
}

/// Display implementation for user-friendly error messages.
///
/// Provides clear, contextual error messages that help users understand
/// what went wrong during I/O operations.
impl std::fmt::Display for IoError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            IoError::FileError(e) => write!(f, "File error: {}", e),
            IoError::ParseError(e) => write!(f, "Parse error: {}", e),
            IoError::InvalidFormat(msg) => write!(f, "Invalid format: {}", msg),
        }
    }
}

/// Standard Error trait implementation for integration with error handling libraries.
impl std::error::Error for IoError {}

/// Convenient type alias for operations that may fail with IoError.
///
/// This shorthand type reduces boilerplate in function signatures and
/// provides consistent error handling throughout the module.
pub type Result<T> = std::result::Result<T, IoError>;

/// Reads geometric data from a JSON file.
///
/// This function deserializes a JSON file containing an array of Geometry objects.
/// It handles empty files gracefully by returning an empty vector.
///
/// # Arguments
/// * `path` - Path to the JSON file to read
///
/// # Returns
/// A vector of Geometry objects parsed from the file
///
/// # Errors
/// - `IoError::FileError` if the file cannot be read
/// - `IoError::ParseError` if the JSON is malformed or doesn't match the expected schema
///
/// # Examples
/// ```no_run
/// use stdgeo_lib::io::read_geometry_file;
/// 
/// let geometries = read_geometry_file("shapes.json")?;
/// println!("Loaded {} geometries", geometries.len());
/// # Ok::<(), Box<dyn std::error::Error>>(())
/// ```
pub fn read_geometry_file<P: AsRef<Path>>(path: P) -> Result<Vec<Geometry>> {
    let content = fs::read_to_string(path)?;
    
    // Handle empty files gracefully
    if content.trim().is_empty() {
        return Ok(Vec::new());
    }
    
    // Parse JSON array of geometries
    let geometries: Vec<Geometry> = serde_json::from_str(&content)?;
    Ok(geometries)
}

/// Writes geometric data to a JSON file.
///
/// This function serializes a collection of Geometry objects to a pretty-printed
/// JSON file. The output is formatted for human readability.
///
/// # Arguments
/// * `path` - Path where the JSON file should be written
/// * `geometries` - Slice of Geometry objects to serialize
///
/// # Errors
/// - `IoError::FileError` if the file cannot be written (permissions, disk space, etc.)
/// - `IoError::ParseError` if serialization fails (should be rare with valid data)
///
/// # Examples
/// ```no_run
/// use stdgeo_lib::{Point, Geometry};
/// use stdgeo_lib::io::write_geometry_file;
/// 
/// let geometries = vec![Geometry::Point(Point::new(1.0, 2.0))];
/// write_geometry_file("output.json", &geometries)?;
/// # Ok::<(), Box<dyn std::error::Error>>(())
/// ```
pub fn write_geometry_file<P: AsRef<Path>>(path: P, geometries: &[Geometry]) -> Result<()> {
    // Serialize to pretty-printed JSON
    let json = serde_json::to_string_pretty(geometries)?;
    fs::write(path, json)?;
    Ok(())
}

/// Reads geometric data from a simple text format file.
///
/// This function parses a human-readable text format where each line represents
/// one geometric object. The format supports comments and is designed for easy
/// manual editing.
///
/// # Format Specification
/// - `point x y` - Creates a point at coordinates (x, y)
/// - `line x1 y1 x2 y2` - Creates a line from (x1, y1) to (x2, y2)
/// - Lines starting with '#' are treated as comments and ignored
/// - Empty lines are ignored
/// - Whitespace is used to separate components
///
/// # Arguments
/// * `path` - Path to the simple format file to read
///
/// # Returns
/// A vector of Geometry objects parsed from the file
///
/// # Errors
/// - `IoError::FileError` if the file cannot be read
/// - `IoError::InvalidFormat` if a line doesn't match the expected format or contains invalid numbers
///
/// # Examples
/// ```no_run
/// use stdgeo_lib::io::read_simple_format;
/// 
/// // File contents:
/// // # Simple geometry file
/// // point 1.0 2.0
/// // line 0.0 0.0 3.0 4.0
/// 
/// let geometries = read_simple_format("shapes.txt")?;
/// # Ok::<(), Box<dyn std::error::Error>>(())
/// ```
pub fn read_simple_format<P: AsRef<Path>>(path: P) -> Result<Vec<Geometry>> {
    let content = fs::read_to_string(path)?;
    let mut geometries = Vec::new();
    
    // Process each line in the file
    for (line_num, line) in content.lines().enumerate() {
        let line = line.trim();
        
        // Skip empty lines and comments
        if line.is_empty() || line.starts_with('#') {
            continue;
        }
        
        // Parse line into whitespace-separated components
        let parts: Vec<&str> = line.split_whitespace().collect();
        match parts.as_slice() {
            ["point", x, y] => {
                // Parse point coordinates with error context
                let x: f64 = x.parse().map_err(|_| {
                    IoError::InvalidFormat(format!("Invalid x coordinate on line {}", line_num + 1))
                })?;
                let y: f64 = y.parse().map_err(|_| {
                    IoError::InvalidFormat(format!("Invalid y coordinate on line {}", line_num + 1))
                })?;
                geometries.push(Geometry::Point(Point::new(x, y)));
            }
            ["line", x1, y1, x2, y2] => {
                // Parse line coordinates with error context
                let x1: f64 = x1.parse().map_err(|_| {
                    IoError::InvalidFormat(format!("Invalid x1 coordinate on line {}", line_num + 1))
                })?;
                let y1: f64 = y1.parse().map_err(|_| {
                    IoError::InvalidFormat(format!("Invalid y1 coordinate on line {}", line_num + 1))
                })?;
                let x2: f64 = x2.parse().map_err(|_| {
                    IoError::InvalidFormat(format!("Invalid x2 coordinate on line {}", line_num + 1))
                })?;
                let y2: f64 = y2.parse().map_err(|_| {
                    IoError::InvalidFormat(format!("Invalid y2 coordinate on line {}", line_num + 1))
                })?;
                geometries.push(Geometry::Line(Line::new(
                    Point::new(x1, y1),
                    Point::new(x2, y2),
                )));
            }
            _ => {
                // Invalid format - provide clear error message
                return Err(IoError::InvalidFormat(format!(
                    "Invalid format on line {}: {}",
                    line_num + 1,
                    line
                )));
            }
        }
    }
    
    Ok(geometries)
}

/// Writes geometric data to a simple text format file.
///
/// This function serializes a collection of Geometry objects to a human-readable
/// text format. The output includes a header with format documentation and
/// each geometry object on its own line.
///
/// # Output Format
/// The generated file includes:
/// - Header comments describing the format
/// - One geometry object per line
/// - Format: "point x y" for points, "line x1 y1 x2 y2" for lines
///
/// # Arguments
/// * `path` - Path where the simple format file should be written
/// * `geometries` - Slice of Geometry objects to serialize
///
/// # Errors
/// - `IoError::FileError` if the file cannot be written (permissions, disk space, etc.)
///
/// # Examples
/// ```no_run
/// use stdgeo_lib::{Point, Line, Geometry};
/// use stdgeo_lib::io::write_simple_format;
/// 
/// let geometries = vec![
///     Geometry::Point(Point::new(1.0, 2.0)),
///     Geometry::Line(Line::new(Point::new(0.0, 0.0), Point::new(3.0, 4.0))),
/// ];
/// write_simple_format("output.txt", &geometries)?;
/// # Ok::<(), Box<dyn std::error::Error>>(())
/// ```
pub fn write_simple_format<P: AsRef<Path>>(path: P, geometries: &[Geometry]) -> Result<()> {
    let mut file = fs::File::create(path)?;
    
    // Write format documentation header
    writeln!(file, "# Simple geometry format")?;
    writeln!(file, "# point x y")?;
    writeln!(file, "# line x1 y1 x2 y2")?;
    writeln!(file)?;
    
    // Write each geometry object
    for geometry in geometries {
        match geometry {
            Geometry::Point(p) => {
                writeln!(file, "point {} {}", p.x, p.y)?;
            }
            Geometry::Line(l) => {
                writeln!(file, "line {} {} {} {}", l.start.x, l.start.y, l.end.x, l.end.y)?;
            }
        }
    }
    
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fs;
    use tempfile::NamedTempFile;

    #[test]
    fn test_read_write_geometry_file() {
        let geometries = vec![
            Geometry::Point(Point::new(1.0, 2.0)),
            Geometry::Line(Line::new(Point::new(0.0, 0.0), Point::new(3.0, 4.0))),
        ];

        let temp_file = NamedTempFile::new().unwrap();
        let path = temp_file.path();

        write_geometry_file(&path, &geometries).unwrap();
        let read_geometries = read_geometry_file(&path).unwrap();

        assert_eq!(geometries.len(), read_geometries.len());
        for (original, read) in geometries.iter().zip(read_geometries.iter()) {
            assert_eq!(original, read);
        }
    }

    #[test]
    fn test_read_empty_file() {
        let temp_file = NamedTempFile::new().unwrap();
        let path = temp_file.path();
        fs::write(path, "").unwrap();

        let geometries = read_geometry_file(path).unwrap();
        assert_eq!(geometries.len(), 0);
    }

    #[test]
    fn test_simple_format_read_write() {
        let geometries = vec![
            Geometry::Point(Point::new(1.5, 2.5)),
            Geometry::Line(Line::new(Point::new(0.0, 1.0), Point::new(3.0, 4.0))),
        ];

        let temp_file = NamedTempFile::new().unwrap();
        let path = temp_file.path();

        write_simple_format(&path, &geometries).unwrap();
        let read_geometries = read_simple_format(&path).unwrap();

        assert_eq!(geometries.len(), read_geometries.len());
        for (original, read) in geometries.iter().zip(read_geometries.iter()) {
            assert_eq!(original, read);
        }
    }

    #[test]
    fn test_simple_format_with_comments() {
        let temp_file = NamedTempFile::new().unwrap();
        let path = temp_file.path();

        let content = "# This is a comment\npoint 1.0 2.0\n# Another comment\nline 0.0 0.0 3.0 4.0\n";
        fs::write(path, content).unwrap();

        let geometries = read_simple_format(path).unwrap();
        assert_eq!(geometries.len(), 2);

        match &geometries[0] {
            Geometry::Point(p) => {
                assert_eq!(p.x, 1.0);
                assert_eq!(p.y, 2.0);
            }
            _ => panic!("Expected point"),
        }

        match &geometries[1] {
            Geometry::Line(l) => {
                assert_eq!(l.start.x, 0.0);
                assert_eq!(l.start.y, 0.0);
                assert_eq!(l.end.x, 3.0);
                assert_eq!(l.end.y, 4.0);
            }
            _ => panic!("Expected line"),
        }
    }

    #[test]
    fn test_simple_format_invalid_format() {
        let temp_file = NamedTempFile::new().unwrap();
        let path = temp_file.path();

        let content = "invalid format line\n";
        fs::write(path, content).unwrap();

        let result = read_simple_format(path);
        assert!(result.is_err());
        match result.unwrap_err() {
            IoError::InvalidFormat(_) => {}
            _ => panic!("Expected InvalidFormat error"),
        }
    }
}