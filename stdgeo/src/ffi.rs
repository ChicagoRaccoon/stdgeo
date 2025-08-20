//! # C FFI Interface for Qt Integration
//!
//! This module provides a C-compatible interface to the stdgeo library
//! for use with Qt applications. It enables shared memory access to
//! geometry data and real-time updates for visualization.

use crate::geometry::{Geometry, Point, Line};
use std::boxed::Box;
use std::ffi::CStr;
use std::os::raw::{c_char, c_double, c_int};

/// C-compatible Point structure
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct CPoint {
    pub x: c_double,
    pub y: c_double,
}

impl From<Point> for CPoint {
    fn from(point: Point) -> Self {
        CPoint { x: point.x, y: point.y }
    }
}

impl From<CPoint> for Point {
    fn from(cpoint: CPoint) -> Self {
        Point::new(cpoint.x, cpoint.y)
    }
}

/// C-compatible Line structure
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct CLine {
    pub start: CPoint,
    pub end: CPoint,
}

impl From<Line> for CLine {
    fn from(line: Line) -> Self {
        CLine {
            start: line.start.into(),
            end: line.end.into(),
        }
    }
}

impl From<CLine> for Line {
    fn from(cline: CLine) -> Self {
        Line::new(cline.start.into(), cline.end.into())
    }
}

/// C-compatible geometry type enumeration
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub enum CGeometryType {
    Point = 0,
    Line = 1,
}

/// C-compatible geometry structure
#[repr(C)]
#[derive(Clone, Copy)]
pub union CGeometryData {
    pub point: CPoint,
    pub line: CLine,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct CGeometry {
    pub geometry_type: CGeometryType,
    pub data: CGeometryData,
}

impl From<Geometry> for CGeometry {
    fn from(geometry: Geometry) -> Self {
        match geometry {
            Geometry::Point(point) => CGeometry {
                geometry_type: CGeometryType::Point,
                data: CGeometryData { point: point.into() },
            },
            Geometry::Line(line) => CGeometry {
                geometry_type: CGeometryType::Line,
                data: CGeometryData { line: line.into() },
            },
        }
    }
}

impl From<CGeometry> for Geometry {
    fn from(cgeometry: CGeometry) -> Self {
        unsafe {
            match cgeometry.geometry_type {
                CGeometryType::Point => Geometry::Point(cgeometry.data.point.into()),
                CGeometryType::Line => Geometry::Line(cgeometry.data.line.into()),
            }
        }
    }
}

/// Opaque handle to a geometry collection
pub struct GeometryCollection {
    geometries: Vec<Geometry>,
}

/// Create a new empty geometry collection
#[no_mangle]
pub extern "C" fn geometry_collection_new() -> *mut GeometryCollection {
    let collection = Box::new(GeometryCollection {
        geometries: Vec::new(),
    });
    Box::into_raw(collection)
}

/// Free a geometry collection
#[no_mangle]
pub extern "C" fn geometry_collection_free(collection: *mut GeometryCollection) {
    if !collection.is_null() {
        unsafe {
            let _ = Box::from_raw(collection);
        }
    }
}

/// Get the number of geometries in the collection
#[no_mangle]
pub extern "C" fn geometry_collection_size(collection: *const GeometryCollection) -> c_int {
    if collection.is_null() {
        return 0;
    }
    unsafe {
        (*collection).geometries.len() as c_int
    }
}

/// Add a point to the collection
#[no_mangle]
pub extern "C" fn geometry_collection_add_point(
    collection: *mut GeometryCollection,
    x: c_double,
    y: c_double,
) -> c_int {
    if collection.is_null() {
        return -1;
    }
    unsafe {
        let point = Geometry::Point(Point::new(x, y));
        (*collection).geometries.push(point);
        ((*collection).geometries.len() - 1) as c_int
    }
}

/// Add a line to the collection
#[no_mangle]
pub extern "C" fn geometry_collection_add_line(
    collection: *mut GeometryCollection,
    x1: c_double,
    y1: c_double,
    x2: c_double,
    y2: c_double,
) -> c_int {
    if collection.is_null() {
        return -1;
    }
    unsafe {
        let line = Geometry::Line(Line::new(Point::new(x1, y1), Point::new(x2, y2)));
        (*collection).geometries.push(line);
        ((*collection).geometries.len() - 1) as c_int
    }
}

/// Get a geometry from the collection by index
#[no_mangle]
pub extern "C" fn geometry_collection_get(
    collection: *const GeometryCollection,
    index: c_int,
    out_geometry: *mut CGeometry,
) -> c_int {
    if collection.is_null() || out_geometry.is_null() || index < 0 {
        return -1;
    }
    
    unsafe {
        let geometries = &(*collection).geometries;
        if (index as usize) >= geometries.len() {
            return -1;
        }
        
        let geometry = &geometries[index as usize];
        *out_geometry = geometry.clone().into();
        0
    }
}

/// Get all geometries as a C array (caller must not free the returned pointer)
#[no_mangle]
pub extern "C" fn geometry_collection_get_all(
    collection: *const GeometryCollection,
    out_geometries: *mut *const CGeometry,
    out_count: *mut c_int,
) -> c_int {
    if collection.is_null() || out_geometries.is_null() || out_count.is_null() {
        return -1;
    }
    
    unsafe {
        let geometries = &(*collection).geometries;
        let count = geometries.len();
        
        if count == 0 {
            *out_geometries = std::ptr::null();
            *out_count = 0;
            return 0;
        }
        
        // Convert to C geometries (this leaks memory, but Qt will read immediately)
        let c_geometries: Vec<CGeometry> = geometries.iter().map(|g| g.clone().into()).collect();
        let ptr = c_geometries.as_ptr();
        std::mem::forget(c_geometries); // Prevent deallocation
        
        *out_geometries = ptr;
        *out_count = count as c_int;
        0
    }
}

/// Clear all geometries from the collection
#[no_mangle]
pub extern "C" fn geometry_collection_clear(collection: *mut GeometryCollection) -> c_int {
    if collection.is_null() {
        return -1;
    }
    unsafe {
        (*collection).geometries.clear();
        0
    }
}

/// Translate all geometries in the collection
#[no_mangle]
pub extern "C" fn geometry_collection_translate(
    collection: *mut GeometryCollection,
    dx: c_double,
    dy: c_double,
) -> c_int {
    if collection.is_null() {
        return -1;
    }
    unsafe {
        for geometry in &mut (*collection).geometries {
            *geometry = geometry.translate(dx, dy);
        }
        0
    }
}

/// Rotate all geometries in the collection
#[no_mangle]
pub extern "C" fn geometry_collection_rotate(
    collection: *mut GeometryCollection,
    angle: c_double,
    center_x: c_double,
    center_y: c_double,
) -> c_int {
    if collection.is_null() {
        return -1;
    }
    unsafe {
        let center = Point::new(center_x, center_y);
        for geometry in &mut (*collection).geometries {
            *geometry = geometry.rotate(angle, center);
        }
        0
    }
}

/// Load geometries from a JSON file
#[no_mangle]
pub extern "C" fn geometry_collection_load_json(
    collection: *mut GeometryCollection,
    filename: *const c_char,
) -> c_int {
    if collection.is_null() || filename.is_null() {
        return -1;
    }
    
    unsafe {
        let c_str = CStr::from_ptr(filename);
        let filename_str = match c_str.to_str() {
            Ok(s) => s,
            Err(_) => return -1,
        };
        
        let path = std::path::PathBuf::from(filename_str);
        match crate::io::read_geometry_file(&path) {
            Ok(geometries) => {
                (*collection).geometries.extend(geometries);
                0
            }
            Err(_) => -1,
        }
    }
}

/// Save geometries to a JSON file
#[no_mangle]
pub extern "C" fn geometry_collection_save_json(
    collection: *const GeometryCollection,
    filename: *const c_char,
) -> c_int {
    if collection.is_null() || filename.is_null() {
        return -1;
    }
    
    unsafe {
        let c_str = CStr::from_ptr(filename);
        let filename_str = match c_str.to_str() {
            Ok(s) => s,
            Err(_) => return -1,
        };
        
        let path = std::path::PathBuf::from(filename_str);
        match crate::io::write_geometry_file(&path, &(*collection).geometries) {
            Ok(_) => 0,
            Err(_) => -1,
        }
    }
}

/// Get bounding box of all geometries
#[no_mangle]
pub extern "C" fn geometry_collection_bounding_box(
    collection: *const GeometryCollection,
    min_x: *mut c_double,
    min_y: *mut c_double,
    max_x: *mut c_double,
    max_y: *mut c_double,
) -> c_int {
    if collection.is_null() || min_x.is_null() || min_y.is_null() 
        || max_x.is_null() || max_y.is_null() {
        return -1;
    }
    
    unsafe {
        let geometries = &(*collection).geometries;
        if geometries.is_empty() {
            return -1;
        }
        
        let mut min_x_val = f64::INFINITY;
        let mut min_y_val = f64::INFINITY;
        let mut max_x_val = f64::NEG_INFINITY;
        let mut max_y_val = f64::NEG_INFINITY;
        
        for geometry in geometries {
            match geometry {
                Geometry::Point(point) => {
                    min_x_val = min_x_val.min(point.x);
                    min_y_val = min_y_val.min(point.y);
                    max_x_val = max_x_val.max(point.x);
                    max_y_val = max_y_val.max(point.y);
                }
                Geometry::Line(line) => {
                    min_x_val = min_x_val.min(line.start.x).min(line.end.x);
                    min_y_val = min_y_val.min(line.start.y).min(line.end.y);
                    max_x_val = max_x_val.max(line.start.x).max(line.end.x);
                    max_y_val = max_y_val.max(line.start.y).max(line.end.y);
                }
            }
        }
        
        *min_x = min_x_val;
        *min_y = min_y_val;
        *max_x = max_x_val;
        *max_y = max_y_val;
        0
    }
}