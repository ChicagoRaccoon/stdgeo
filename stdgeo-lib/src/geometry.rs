//! # Geometry Module
//!
//! This module provides fundamental 2D geometric primitives and operations.
//!
//! ## Theory of Operation
//!
//! The geometry system is built around immutable data structures that represent
//! mathematical geometric concepts:
//!
//! - **Point**: A 2D coordinate (x, y) in Cartesian space
//! - **Line**: A line segment defined by two points (start and end)
//! - **Geometry**: A polymorphic wrapper that can hold any geometric primitive
//!
//! All transformations (translate, rotate) are implemented as pure functions that
//! return new objects rather than modifying existing ones. This approach:
//! - Ensures thread safety
//! - Prevents accidental mutations
//! - Makes the code more predictable and easier to reason about
//! - Follows functional programming principles
//!
//! ## Coordinate System
//!
//! The library uses a standard Cartesian coordinate system:
//! - Origin (0,0) is at the bottom-left
//! - X-axis increases to the right
//! - Y-axis increases upward
//! - Rotation is counter-clockwise (positive angles)
//!
//! ## Mathematical Operations
//!
//! ### Translation
//! Translation moves geometric objects by a displacement vector (dx, dy):
//! ```text
//! new_point = (x + dx, y + dy)
//! ```
//!
//! ### Rotation
//! Rotation uses standard 2D rotation matrices around a center point:
//! ```text
//! x' = (x - cx) * cos(θ) - (y - cy) * sin(θ) + cx
//! y' = (x - cx) * sin(θ) + (y - cy) * cos(θ) + cy
//! ```
//! Where (cx, cy) is the center of rotation and θ is the angle in radians.

use serde::{Deserialize, Serialize};
use std::f64::consts::PI;

/// A 2D point in Cartesian coordinates.
///
/// Represents a location in 2D space with x and y coordinates.
/// Points are immutable and all operations return new Point instances.
///
/// # Examples
///
/// ```
/// use stdgeo_lib::Point;
///
/// let p1 = Point::new(3.0, 4.0);
/// let p2 = p1.translate(1.0, 1.0);  // Returns Point(4.0, 5.0)
/// assert_eq!(p1.distance_to(&Point::origin()), 5.0);
/// ```
#[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize)]
pub struct Point {
    /// X-coordinate in the Cartesian plane
    pub x: f64,
    /// Y-coordinate in the Cartesian plane
    pub y: f64,
}

impl Point {
    /// Creates a new point with the given coordinates.
    ///
    /// # Arguments
    /// * `x` - The x-coordinate
    /// * `y` - The y-coordinate
    ///
    /// # Examples
    /// ```
    /// use stdgeo_lib::Point;
    /// let point = Point::new(3.0, 4.0);
    /// assert_eq!(point.x, 3.0);
    /// assert_eq!(point.y, 4.0);
    /// ```
    pub fn new(x: f64, y: f64) -> Self {
        Self { x, y }
    }

    /// Creates a point at the origin (0, 0).
    ///
    /// # Examples
    /// ```
    /// use stdgeo_lib::Point;
    /// let origin = Point::origin();
    /// assert_eq!(origin.x, 0.0);
    /// assert_eq!(origin.y, 0.0);
    /// ```
    pub fn origin() -> Self {
        Self::new(0.0, 0.0)
    }

    /// Translates this point by the given displacement vector.
    ///
    /// Returns a new point that is displaced by (dx, dy) from this point.
    /// The original point is not modified.
    ///
    /// # Arguments
    /// * `dx` - Displacement along the x-axis
    /// * `dy` - Displacement along the y-axis
    ///
    /// # Examples
    /// ```
    /// use stdgeo_lib::Point;
    /// let p1 = Point::new(1.0, 2.0);
    /// let p2 = p1.translate(3.0, 4.0);
    /// assert_eq!(p2.x, 4.0);
    /// assert_eq!(p2.y, 6.0);
    /// ```
    pub fn translate(&self, dx: f64, dy: f64) -> Self {
        Self::new(self.x + dx, self.y + dy)
    }

    /// Rotates this point around a center point by the given angle.
    ///
    /// Uses the standard 2D rotation transformation matrix to rotate this point
    /// counter-clockwise around the specified center point. Returns a new point
    /// representing the rotated position.
    ///
    /// # Arguments
    /// * `angle_radians` - Rotation angle in radians (positive = counter-clockwise)
    /// * `center` - The center point to rotate around
    ///
    /// # Mathematical Background
    /// The rotation uses the transformation:
    /// ```text
    /// x' = (x - cx) * cos(θ) - (y - cy) * sin(θ) + cx
    /// y' = (x - cx) * sin(θ) + (y - cy) * cos(θ) + cy
    /// ```
    ///
    /// # Examples
    /// ```
    /// use stdgeo_lib::Point;
    /// use std::f64::consts::PI;
    /// 
    /// let point = Point::new(1.0, 0.0);
    /// let rotated = point.rotate(PI / 2.0, Point::origin());
    /// // Should be approximately (0.0, 1.0) after 90° rotation
    /// ```
    pub fn rotate(&self, angle_radians: f64, center: Point) -> Self {
        let cos_a = angle_radians.cos();
        let sin_a = angle_radians.sin();
        
        // Translate to origin relative to center
        let translated_x = self.x - center.x;
        let translated_y = self.y - center.y;
        
        // Apply rotation matrix
        let rotated_x = translated_x * cos_a - translated_y * sin_a;
        let rotated_y = translated_x * sin_a + translated_y * cos_a;
        
        // Translate back
        Self::new(rotated_x + center.x, rotated_y + center.y)
    }

    /// Calculates the Euclidean distance between this point and another point.
    ///
    /// Uses the standard distance formula: √[(x₂-x₁)² + (y₂-y₁)²]
    ///
    /// # Arguments
    /// * `other` - The other point to measure distance to
    ///
    /// # Returns
    /// The distance as a positive floating-point number
    ///
    /// # Examples
    /// ```
    /// use stdgeo_lib::Point;
    /// let p1 = Point::new(0.0, 0.0);
    /// let p2 = Point::new(3.0, 4.0);
    /// assert_eq!(p1.distance_to(&p2), 5.0);  // 3-4-5 triangle
    /// ```
    pub fn distance_to(&self, other: &Point) -> f64 {
        ((self.x - other.x).powi(2) + (self.y - other.y).powi(2)).sqrt()
    }
}

/// A line segment defined by two points.
///
/// Represents a straight line segment from a start point to an end point.
/// Lines are immutable and all operations return new Line instances.
///
/// # Examples
///
/// ```
/// use stdgeo_lib::{Point, Line};
///
/// let line = Line::new(Point::new(0.0, 0.0), Point::new(3.0, 4.0));
/// assert_eq!(line.length(), 5.0);
/// let midpoint = line.midpoint();
/// assert_eq!(midpoint.x, 1.5);
/// assert_eq!(midpoint.y, 2.0);
/// ```
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct Line {
    /// The starting point of the line segment
    pub start: Point,
    /// The ending point of the line segment
    pub end: Point,
}

impl Line {
    /// Creates a new line segment between two points.
    ///
    /// # Arguments
    /// * `start` - The starting point of the line segment
    /// * `end` - The ending point of the line segment
    ///
    /// # Examples
    /// ```
    /// use stdgeo_lib::{Point, Line};
    /// let line = Line::new(Point::new(0.0, 0.0), Point::new(1.0, 1.0));
    /// ```
    pub fn new(start: Point, end: Point) -> Self {
        Self { start, end }
    }

    /// Calculates the length of this line segment.
    ///
    /// Returns the Euclidean distance between the start and end points.
    ///
    /// # Examples
    /// ```
    /// use stdgeo_lib::{Point, Line};
    /// let line = Line::new(Point::new(0.0, 0.0), Point::new(3.0, 4.0));
    /// assert_eq!(line.length(), 5.0);
    /// ```
    pub fn length(&self) -> f64 {
        self.start.distance_to(&self.end)
    }

    /// Translates this line by the given displacement vector.
    ///
    /// Both the start and end points are translated by the same displacement,
    /// preserving the line's direction and length.
    ///
    /// # Arguments
    /// * `dx` - Displacement along the x-axis
    /// * `dy` - Displacement along the y-axis
    ///
    /// # Examples
    /// ```
    /// use stdgeo_lib::{Point, Line};
    /// let line = Line::new(Point::new(0.0, 0.0), Point::new(1.0, 1.0));
    /// let translated = line.translate(5.0, 10.0);
    /// assert_eq!(translated.start.x, 5.0);
    /// assert_eq!(translated.start.y, 10.0);
    /// ```
    pub fn translate(&self, dx: f64, dy: f64) -> Self {
        Self::new(
            self.start.translate(dx, dy),
            self.end.translate(dx, dy),
        )
    }

    /// Rotates this line around a center point by the given angle.
    ///
    /// Both the start and end points are rotated around the specified center,
    /// preserving the line's length but changing its orientation.
    ///
    /// # Arguments
    /// * `angle_radians` - Rotation angle in radians (positive = counter-clockwise)
    /// * `center` - The center point to rotate around
    ///
    /// # Examples
    /// ```
    /// use stdgeo_lib::{Point, Line};
    /// use std::f64::consts::PI;
    /// 
    /// let line = Line::new(Point::new(0.0, 0.0), Point::new(1.0, 0.0));
    /// let rotated = line.rotate(PI / 2.0, Point::origin());
    /// // Line should now be vertical instead of horizontal
    /// ```
    pub fn rotate(&self, angle_radians: f64, center: Point) -> Self {
        Self::new(
            self.start.rotate(angle_radians, center),
            self.end.rotate(angle_radians, center),
        )
    }

    /// Calculates the midpoint of this line segment.
    ///
    /// Returns the point that is exactly halfway between the start and end points.
    ///
    /// # Examples
    /// ```
    /// use stdgeo_lib::{Point, Line};
    /// let line = Line::new(Point::new(0.0, 0.0), Point::new(4.0, 6.0));
    /// let midpoint = line.midpoint();
    /// assert_eq!(midpoint.x, 2.0);
    /// assert_eq!(midpoint.y, 3.0);
    /// ```
    pub fn midpoint(&self) -> Point {
        Point::new(
            (self.start.x + self.end.x) / 2.0,
            (self.start.y + self.end.y) / 2.0,
        )
    }
}

/// A polymorphic wrapper for different geometric primitives.
///
/// This enum allows uniform handling of different geometric types,
/// enabling collections and operations that work with mixed geometry types.
///
/// # Design Pattern
/// This follows the "sum type" or "tagged union" pattern, providing
/// type safety while allowing polymorphic behavior through pattern matching.
///
/// # Examples
///
/// ```
/// use stdgeo_lib::{Point, Line, Geometry};
///
/// let shapes = vec![
///     Geometry::Point(Point::new(1.0, 2.0)),
///     Geometry::Line(Line::new(Point::new(0.0, 0.0), Point::new(3.0, 4.0))),
/// ];
///
/// for shape in shapes {
///     let translated = shape.translate(5.0, 5.0);
///     // Works uniformly for all geometry types
/// }
/// ```
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub enum Geometry {
    /// A point geometry
    Point(Point),
    /// A line segment geometry
    Line(Line),
}

impl Geometry {
    /// Translates this geometry by the given displacement vector.
    ///
    /// This method provides a uniform interface for translating any geometric type.
    /// It delegates to the appropriate translation method for the underlying type.
    ///
    /// # Arguments
    /// * `dx` - Displacement along the x-axis
    /// * `dy` - Displacement along the y-axis
    ///
    /// # Examples
    /// ```
    /// use stdgeo_lib::{Point, Geometry};
    /// let geom = Geometry::Point(Point::new(1.0, 2.0));
    /// let translated = geom.translate(3.0, 4.0);
    /// ```
    pub fn translate(&self, dx: f64, dy: f64) -> Self {
        match self {
            Geometry::Point(p) => Geometry::Point(p.translate(dx, dy)),
            Geometry::Line(l) => Geometry::Line(l.translate(dx, dy)),
        }
    }

    /// Rotates this geometry around a center point by the given angle.
    ///
    /// This method provides a uniform interface for rotating any geometric type.
    /// It delegates to the appropriate rotation method for the underlying type.
    ///
    /// # Arguments
    /// * `angle_radians` - Rotation angle in radians (positive = counter-clockwise)
    /// * `center` - The center point to rotate around
    ///
    /// # Examples
    /// ```
    /// use stdgeo_lib::{Point, Geometry};
    /// use std::f64::consts::PI;
    /// 
    /// let geom = Geometry::Point(Point::new(1.0, 0.0));
    /// let rotated = geom.rotate(PI / 2.0, Point::origin());
    /// ```
    pub fn rotate(&self, angle_radians: f64, center: Point) -> Self {
        match self {
            Geometry::Point(p) => Geometry::Point(p.rotate(angle_radians, center)),
            Geometry::Line(l) => Geometry::Line(l.rotate(angle_radians, center)),
        }
    }
}

/// Converts degrees to radians.
///
/// This utility function converts angle measurements from degrees to radians,
/// which are required by the trigonometric functions used in rotations.
///
/// # Mathematical Background
/// The conversion formula is: radians = degrees × π / 180
///
/// # Arguments
/// * `degrees` - Angle in degrees
///
/// # Returns
/// The equivalent angle in radians
///
/// # Examples
/// ```
/// use stdgeo_lib::degrees_to_radians;
/// use std::f64::consts::PI;
/// 
/// assert!((degrees_to_radians(180.0) - PI).abs() < f64::EPSILON);
/// assert!((degrees_to_radians(90.0) - PI/2.0).abs() < f64::EPSILON);
/// assert!((degrees_to_radians(0.0)).abs() < f64::EPSILON);
/// ```
pub fn degrees_to_radians(degrees: f64) -> f64 {
    degrees * PI / 180.0
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::f64::EPSILON;

    fn approx_eq(a: f64, b: f64) -> bool {
        (a - b).abs() < EPSILON * 10.0
    }

    #[test]
    fn test_point_creation() {
        let p = Point::new(3.0, 4.0);
        assert_eq!(p.x, 3.0);
        assert_eq!(p.y, 4.0);
    }

    #[test]
    fn test_point_origin() {
        let origin = Point::origin();
        assert_eq!(origin.x, 0.0);
        assert_eq!(origin.y, 0.0);
    }

    #[test]
    fn test_point_translate() {
        let p = Point::new(1.0, 2.0);
        let translated = p.translate(3.0, 4.0);
        assert_eq!(translated.x, 4.0);
        assert_eq!(translated.y, 6.0);
    }

    #[test]
    fn test_point_distance() {
        let p1 = Point::new(0.0, 0.0);
        let p2 = Point::new(3.0, 4.0);
        assert_eq!(p1.distance_to(&p2), 5.0);
    }

    #[test]
    fn test_point_rotate() {
        let p = Point::new(1.0, 0.0);
        let center = Point::origin();
        let rotated = p.rotate(PI / 2.0, center);
        assert!(approx_eq(rotated.x, 0.0));
        assert!(approx_eq(rotated.y, 1.0));
    }

    #[test]
    fn test_line_creation() {
        let start = Point::new(0.0, 0.0);
        let end = Point::new(3.0, 4.0);
        let line = Line::new(start, end);
        assert_eq!(line.start, start);
        assert_eq!(line.end, end);
    }

    #[test]
    fn test_line_length() {
        let line = Line::new(Point::new(0.0, 0.0), Point::new(3.0, 4.0));
        assert_eq!(line.length(), 5.0);
    }

    #[test]
    fn test_line_midpoint() {
        let line = Line::new(Point::new(0.0, 0.0), Point::new(4.0, 6.0));
        let midpoint = line.midpoint();
        assert_eq!(midpoint.x, 2.0);
        assert_eq!(midpoint.y, 3.0);
    }

    #[test]
    fn test_line_translate() {
        let line = Line::new(Point::new(0.0, 0.0), Point::new(1.0, 1.0));
        let translated = line.translate(5.0, 10.0);
        assert_eq!(translated.start.x, 5.0);
        assert_eq!(translated.start.y, 10.0);
        assert_eq!(translated.end.x, 6.0);
        assert_eq!(translated.end.y, 11.0);
    }

    #[test]
    fn test_line_rotate() {
        let line = Line::new(Point::new(0.0, 0.0), Point::new(1.0, 0.0));
        let center = Point::origin();
        let rotated = line.rotate(PI / 2.0, center);
        assert!(approx_eq(rotated.start.x, 0.0));
        assert!(approx_eq(rotated.start.y, 0.0));
        assert!(approx_eq(rotated.end.x, 0.0));
        assert!(approx_eq(rotated.end.y, 1.0));
    }

    #[test]
    fn test_geometry_translate() {
        let point_geom = Geometry::Point(Point::new(1.0, 2.0));
        let line_geom = Geometry::Line(Line::new(Point::new(0.0, 0.0), Point::new(1.0, 1.0)));

        let translated_point = point_geom.translate(5.0, 10.0);
        let translated_line = line_geom.translate(5.0, 10.0);

        match translated_point {
            Geometry::Point(p) => {
                assert_eq!(p.x, 6.0);
                assert_eq!(p.y, 12.0);
            }
            _ => panic!("Expected point"),
        }

        match translated_line {
            Geometry::Line(l) => {
                assert_eq!(l.start.x, 5.0);
                assert_eq!(l.start.y, 10.0);
                assert_eq!(l.end.x, 6.0);
                assert_eq!(l.end.y, 11.0);
            }
            _ => panic!("Expected line"),
        }
    }

    #[test]
    fn test_degrees_to_radians() {
        assert!(approx_eq(degrees_to_radians(180.0), PI));
        assert!(approx_eq(degrees_to_radians(90.0), PI / 2.0));
        assert!(approx_eq(degrees_to_radians(0.0), 0.0));
    }
}