//! # StdGeo Library
//!
//! This library provides core geometric primitives and operations for 2D geometry.
//! 
//! ## Theory of Operation
//! 
//! The library is organized into two main modules:
//! - `geometry`: Contains geometric primitives (Point, Line) and transformation operations
//! - `io`: Handles reading and writing geometric data in various formats (JSON, simple text)
//! 
//! The design follows a functional programming approach where geometric objects are immutable
//! and transformations return new objects rather than modifying existing ones. This ensures
//! thread safety and prevents accidental mutation bugs.
//! 
//! ## Architecture
//! 
//! ```text
//! stdgeo-lib
//! ├── geometry    - Core geometric types and operations
//! │   ├── Point   - 2D point with x,y coordinates
//! │   ├── Line    - Line segment defined by two points
//! │   └── Geometry - Enum wrapper for polymorphic operations
//! └── io          - Serialization and file I/O
//!     ├── JSON format    - Standard serde_json serialization
//!     └── Simple format  - Human-readable text format
//! ```

pub mod geometry;
pub mod io;
pub mod ffi;

pub use geometry::*;
pub use io::*;