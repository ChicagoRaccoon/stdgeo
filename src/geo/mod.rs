//! StdGeo Geometry Library
//! 
//! This module contains the core 3D geometry types and operations for the StdGeo library.
//! It provides Point3D, Vector3D, Triangle, and Mesh types with comprehensive mathematical
//! operations and C FFI bindings for integration with Qt.

pub mod geometry;
pub mod ffi;
pub mod command;

// Re-export the main geometry types for easier access
pub use geometry::*;
pub use command::*;