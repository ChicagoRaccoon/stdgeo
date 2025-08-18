use serde::{Deserialize, Serialize};
use std::f64::consts::PI;

#[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize)]
pub struct Point {
    pub x: f64,
    pub y: f64,
}

impl Point {
    pub fn new(x: f64, y: f64) -> Self {
        Self { x, y }
    }

    pub fn origin() -> Self {
        Self::new(0.0, 0.0)
    }

    pub fn translate(&self, dx: f64, dy: f64) -> Self {
        Self::new(self.x + dx, self.y + dy)
    }

    pub fn rotate(&self, angle_radians: f64, center: Point) -> Self {
        let cos_a = angle_radians.cos();
        let sin_a = angle_radians.sin();
        
        let translated_x = self.x - center.x;
        let translated_y = self.y - center.y;
        
        let rotated_x = translated_x * cos_a - translated_y * sin_a;
        let rotated_y = translated_x * sin_a + translated_y * cos_a;
        
        Self::new(rotated_x + center.x, rotated_y + center.y)
    }

    pub fn distance_to(&self, other: &Point) -> f64 {
        ((self.x - other.x).powi(2) + (self.y - other.y).powi(2)).sqrt()
    }
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct Line {
    pub start: Point,
    pub end: Point,
}

impl Line {
    pub fn new(start: Point, end: Point) -> Self {
        Self { start, end }
    }

    pub fn length(&self) -> f64 {
        self.start.distance_to(&self.end)
    }

    pub fn translate(&self, dx: f64, dy: f64) -> Self {
        Self::new(
            self.start.translate(dx, dy),
            self.end.translate(dx, dy),
        )
    }

    pub fn rotate(&self, angle_radians: f64, center: Point) -> Self {
        Self::new(
            self.start.rotate(angle_radians, center),
            self.end.rotate(angle_radians, center),
        )
    }

    pub fn midpoint(&self) -> Point {
        Point::new(
            (self.start.x + self.end.x) / 2.0,
            (self.start.y + self.end.y) / 2.0,
        )
    }
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub enum Geometry {
    Point(Point),
    Line(Line),
}

impl Geometry {
    pub fn translate(&self, dx: f64, dy: f64) -> Self {
        match self {
            Geometry::Point(p) => Geometry::Point(p.translate(dx, dy)),
            Geometry::Line(l) => Geometry::Line(l.translate(dx, dy)),
        }
    }

    pub fn rotate(&self, angle_radians: f64, center: Point) -> Self {
        match self {
            Geometry::Point(p) => Geometry::Point(p.rotate(angle_radians, center)),
            Geometry::Line(l) => Geometry::Line(l.rotate(angle_radians, center)),
        }
    }
}

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