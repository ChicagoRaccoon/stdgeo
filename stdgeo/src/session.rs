//! # Session Management Module
//!
//! This module provides geometry session management for applications that need
//! to maintain collections of geometry objects with operations like add, remove,
//! transform, and persist to files.
//!
//! ## Usage
//!
//! ```rust
//! use stdgeo::{GeometrySession, Point, Line, Geometry};
//!
//! let mut session = GeometrySession::new();
//! 
//! // Add geometries
//! session.add_geometry(Geometry::Point(Point::new(1.0, 2.0)));
//! session.add_geometry(Geometry::Line(Line::new(Point::new(0.0, 0.0), Point::new(3.0, 4.0))));
//!
//! // Query session
//! println!("Session has {} objects", session.count());
//!
//! // Transform all objects
//! session.translate_all(1.0, 1.0);
//!
//! // Save and load
//! session.save_to_file("my_session.json")?;
//! session.load_from_file("my_session.json")?;
//! ```

use crate::geometry::{Geometry, Point};
use crate::io::{read_geometry_file, write_geometry_file, read_simple_format, write_simple_format, IoError};
use std::path::Path;
use std::collections::HashMap;

/// A geometry session that manages a collection of geometry objects
///
/// The session provides methods for adding, removing, transforming, and persisting
/// geometry objects. It acts as a container and coordinator for geometry operations.
#[derive(Debug, Clone)]
pub struct GeometrySession {
    /// Collection of geometry objects in the session
    geometries: Vec<Geometry>,
    
    /// Named variables for future use (e.g., "origin", "bounds", etc.)
    variables: HashMap<String, Geometry>,
}

impl GeometrySession {
    /// Create a new empty geometry session
    pub fn new() -> Self {
        Self {
            geometries: Vec::new(),
            variables: HashMap::new(),
        }
    }

    /// Create a session from an existing collection of geometries
    pub fn from_geometries(geometries: Vec<Geometry>) -> Self {
        Self {
            geometries,
            variables: HashMap::new(),
        }
    }

    /// Add a geometry object to the session
    pub fn add_geometry(&mut self, geometry: Geometry) {
        self.geometries.push(geometry);
    }

    /// Get the number of geometry objects in the session
    pub fn count(&self) -> usize {
        self.geometries.len()
    }

    /// Check if the session is empty
    pub fn is_empty(&self) -> bool {
        self.geometries.is_empty()
    }

    /// Clear all geometry objects from the session
    pub fn clear(&mut self) {
        self.geometries.clear();
    }

    /// Get a reference to all geometries in the session
    pub fn geometries(&self) -> &[Geometry] {
        &self.geometries
    }

    /// Get a mutable reference to all geometries in the session
    pub fn geometries_mut(&mut self) -> &mut Vec<Geometry> {
        &mut self.geometries
    }

    /// Remove a geometry at the specified index
    pub fn remove_geometry(&mut self, index: usize) -> Option<Geometry> {
        if index < self.geometries.len() {
            Some(self.geometries.remove(index))
        } else {
            None
        }
    }

    /// Translate all geometries in the session by the given displacement
    pub fn translate_all(&mut self, dx: f64, dy: f64) {
        for geometry in &mut self.geometries {
            *geometry = geometry.translate(dx, dy);
        }
    }

    /// Rotate all geometries in the session around a center point
    pub fn rotate_all(&mut self, angle: f64, center: Point) {
        for geometry in &mut self.geometries {
            *geometry = geometry.rotate(angle, center);
        }
    }

    /// Apply a custom transformation to all geometries
    pub fn transform_all<F>(&mut self, transform: F)
    where
        F: Fn(Geometry) -> Geometry,
    {
        for geometry in &mut self.geometries {
            *geometry = transform(geometry.clone());
        }
    }

    /// Load geometries from a file and replace current session content
    pub fn load_from_file<P: AsRef<Path>>(&mut self, path: P) -> Result<usize, IoError> {
        let geometries = read_geometry_file(path)?;
        let count = geometries.len();
        self.geometries = geometries;
        Ok(count)
    }

    /// Load geometries from a simple text format file
    pub fn load_from_simple_file<P: AsRef<Path>>(&mut self, path: P) -> Result<usize, IoError> {
        let geometries = read_simple_format(path)?;
        let count = geometries.len();
        self.geometries = geometries;
        Ok(count)
    }

    /// Append geometries from a file to the current session
    pub fn append_from_file<P: AsRef<Path>>(&mut self, path: P) -> Result<usize, IoError> {
        let geometries = read_geometry_file(path)?;
        let count = geometries.len();
        self.geometries.extend(geometries);
        Ok(count)
    }

    /// Save the session geometries to a file
    pub fn save_to_file<P: AsRef<Path>>(&self, path: P) -> Result<(), IoError> {
        write_geometry_file(path, &self.geometries)
    }

    /// Save the session geometries to a simple text format file
    pub fn save_to_simple_file<P: AsRef<Path>>(&self, path: P) -> Result<(), IoError> {
        write_simple_format(path, &self.geometries)
    }

    /// Get a formatted string listing all geometries in the session
    pub fn list_geometries(&self) -> String {
        if self.geometries.is_empty() {
            "No geometry objects in session".to_string()
        } else {
            let mut result = format!("Session contains {} geometry objects:\n", self.geometries.len());
            for (i, geometry) in self.geometries.iter().enumerate() {
                match geometry {
                    Geometry::Point(p) => {
                        result.push_str(&format!("  {}: Point ({}, {})\n", i + 1, p.x, p.y));
                    }
                    Geometry::Line(l) => {
                        result.push_str(&format!("  {}: Line ({}, {}) to ({}, {})\n", 
                            i + 1, l.start.x, l.start.y, l.end.x, l.end.y));
                    }
                }
            }
            result
        }
    }
    
    /// Store a named variable (for future use)
    pub fn set_variable(&mut self, name: String, geometry: Geometry) {
        self.variables.insert(name, geometry);
    }
    
    /// Get a named variable (for future use)
    pub fn get_variable(&self, name: &str) -> Option<&Geometry> {
        self.variables.get(name)
    }
}

impl Default for GeometrySession {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::geometry::{Point, Line};
    use tempfile::TempDir;

    #[test]
    fn test_session_basic_operations() {
        let mut session = GeometrySession::new();
        
        // Test empty session
        assert_eq!(session.count(), 0);
        assert!(session.is_empty());
        
        // Add geometry
        session.add_geometry(Geometry::Point(Point::new(1.0, 2.0)));
        assert_eq!(session.count(), 1);
        assert!(!session.is_empty());
        
        session.add_geometry(Geometry::Line(Line::new(Point::new(0.0, 0.0), Point::new(3.0, 4.0))));
        assert_eq!(session.count(), 2);
        
        // Test clear
        session.clear();
        assert_eq!(session.count(), 0);
        assert!(session.is_empty());
    }

    #[test]
    fn test_session_transformations() {
        let mut session = GeometrySession::new();
        session.add_geometry(Geometry::Point(Point::new(1.0, 2.0)));
        session.add_geometry(Geometry::Point(Point::new(3.0, 4.0)));
        
        // Test translation
        session.translate_all(1.0, 1.0);
        
        match &session.geometries()[0] {
            Geometry::Point(p) => {
                assert_eq!(p.x, 2.0);
                assert_eq!(p.y, 3.0);
            }
            _ => panic!("Expected point"),
        }
    }

    #[test]
    fn test_session_file_operations() {
        let temp_dir = TempDir::new().unwrap();
        let file_path = temp_dir.path().join("test_session.json");
        
        let mut session = GeometrySession::new();
        session.add_geometry(Geometry::Point(Point::new(1.0, 2.0)));
        session.add_geometry(Geometry::Line(Line::new(Point::new(0.0, 0.0), Point::new(3.0, 4.0))));
        
        // Save session
        session.save_to_file(&file_path).unwrap();
        
        // Load into new session
        let mut new_session = GeometrySession::new();
        let count = new_session.load_from_file(&file_path).unwrap();
        
        assert_eq!(count, 2);
        assert_eq!(new_session.count(), 2);
    }

    #[test]
    fn test_session_listing() {
        let mut session = GeometrySession::new();
        
        // Test empty listing
        let listing = session.list_geometries();
        assert!(listing.contains("No geometry objects"));
        
        // Add objects and test listing
        session.add_geometry(Geometry::Point(Point::new(1.0, 2.0)));
        session.add_geometry(Geometry::Line(Line::new(Point::new(0.0, 0.0), Point::new(3.0, 4.0))));
        
        let listing = session.list_geometries();
        assert!(listing.contains("2 geometry objects"));
        assert!(listing.contains("Point (1, 2)"));
        assert!(listing.contains("Line (0, 0) to (3, 4)"));
    }
}