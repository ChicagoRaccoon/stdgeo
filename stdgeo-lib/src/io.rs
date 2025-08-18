use crate::geometry::{Geometry, Point, Line};
use serde_json;
use std::fs;
use std::io::{self, Write};
use std::path::Path;

#[derive(Debug)]
pub enum IoError {
    FileError(io::Error),
    ParseError(serde_json::Error),
    InvalidFormat(String),
}

impl From<io::Error> for IoError {
    fn from(error: io::Error) -> Self {
        IoError::FileError(error)
    }
}

impl From<serde_json::Error> for IoError {
    fn from(error: serde_json::Error) -> Self {
        IoError::ParseError(error)
    }
}

impl std::fmt::Display for IoError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            IoError::FileError(e) => write!(f, "File error: {}", e),
            IoError::ParseError(e) => write!(f, "Parse error: {}", e),
            IoError::InvalidFormat(msg) => write!(f, "Invalid format: {}", msg),
        }
    }
}

impl std::error::Error for IoError {}

pub type Result<T> = std::result::Result<T, IoError>;

pub fn read_geometry_file<P: AsRef<Path>>(path: P) -> Result<Vec<Geometry>> {
    let content = fs::read_to_string(path)?;
    
    if content.trim().is_empty() {
        return Ok(Vec::new());
    }
    
    let geometries: Vec<Geometry> = serde_json::from_str(&content)?;
    Ok(geometries)
}

pub fn write_geometry_file<P: AsRef<Path>>(path: P, geometries: &[Geometry]) -> Result<()> {
    let json = serde_json::to_string_pretty(geometries)?;
    fs::write(path, json)?;
    Ok(())
}

pub fn read_simple_format<P: AsRef<Path>>(path: P) -> Result<Vec<Geometry>> {
    let content = fs::read_to_string(path)?;
    let mut geometries = Vec::new();
    
    for (line_num, line) in content.lines().enumerate() {
        let line = line.trim();
        if line.is_empty() || line.starts_with('#') {
            continue;
        }
        
        let parts: Vec<&str> = line.split_whitespace().collect();
        match parts.as_slice() {
            ["point", x, y] => {
                let x: f64 = x.parse().map_err(|_| {
                    IoError::InvalidFormat(format!("Invalid x coordinate on line {}", line_num + 1))
                })?;
                let y: f64 = y.parse().map_err(|_| {
                    IoError::InvalidFormat(format!("Invalid y coordinate on line {}", line_num + 1))
                })?;
                geometries.push(Geometry::Point(Point::new(x, y)));
            }
            ["line", x1, y1, x2, y2] => {
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

pub fn write_simple_format<P: AsRef<Path>>(path: P, geometries: &[Geometry]) -> Result<()> {
    let mut file = fs::File::create(path)?;
    
    writeln!(file, "# Simple geometry format")?;
    writeln!(file, "# point x y")?;
    writeln!(file, "# line x1 y1 x2 y2")?;
    writeln!(file)?;
    
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