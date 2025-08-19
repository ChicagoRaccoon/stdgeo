//! # StdGeo CLI Application
//!
//! A command-line interface for geometric operations and file format conversions.
//!
//! ## Theory of Operation
//!
//! This CLI application provides a user-friendly interface to the stdgeo-lib library.
//! It implements the following architectural patterns:
//!
//! ### Command Structure
//! The application uses the clap library for command-line parsing, organizing
//! functionality into subcommands:
//! - `point` - Create individual points
//! - `line` - Create individual line segments
//! - `read` - Load geometry from files
//! - `write` - Convert between file formats
//! - `translate` - Apply translation transformations
//! - `rotate` - Apply rotation transformations
//!
//! ### File Format Support
//! The CLI supports both JSON and simple text formats:
//! - JSON format: Machine-readable, preserves precision
//! - Simple format: Human-editable, compact representation
//!
//! ### Transformation Pipeline
//! Operations follow a consistent pattern:
//! 1. Read geometry from input file
//! 2. Apply transformation (if applicable)
//! 3. Write result to output file
//! 4. Provide user feedback
//!
//! ### Error Handling
//! Uses anyhow for ergonomic error handling with context preservation.
//! All file operations and transformations are wrapped in Result types.
//!
//! ## Usage Examples
//!
//! ```bash
//! # Create a point and save to file
//! stdgeo point -x 3.0 -y 4.0 -o point.json
//!
//! # Create a line segment
//! stdgeo line --x1 0.0 --y1 0.0 --x2 3.0 --y2 4.0 -o line.json
//!
//! # Read and display geometries
//! stdgeo read -i shapes.json
//!
//! # Convert between formats
//! stdgeo write -i shapes.json -o shapes.txt --simple
//!
//! # Apply transformations
//! stdgeo translate -i input.json -o output.json --dx 5.0 --dy 10.0
//! stdgeo rotate -i input.json -o output.json --angle 90.0 --degrees
//! ```

use clap::{Parser, Subcommand};
use stdgeo_lib::{
    geometry::{Geometry, Point, Line, degrees_to_radians},
    io::{read_geometry_file, write_geometry_file, read_simple_format, write_simple_format}
};
use anyhow::Result;
use std::path::PathBuf;

/// Main CLI structure using clap's derive API.
///
/// This struct defines the top-level command-line interface and metadata
/// for the stdgeo application.
#[derive(Parser)]
#[command(name = "stdgeo")]
#[command(about = "A CLI tool for geometric operations")]
#[command(version = "0.1.0")]
struct Cli {
    /// The subcommand to execute
    #[command(subcommand)]
    command: Commands,
}

/// Enumeration of all available CLI subcommands.
///
/// Each variant represents a different operation that can be performed
/// by the CLI application. The commands are organized by functionality:
/// - Creation: Point, Line
/// - I/O: Read, Write
/// - Transformation: Translate, Rotate
#[derive(Subcommand)]
enum Commands {
    /// Create a point with specified coordinates.
    ///
    /// If no output file is specified, the point is printed to stdout.
    /// Otherwise, it's saved to the specified file in JSON format.
    Point {
        /// X-coordinate of the point
        #[arg(short, long)]
        x: f64,
        /// Y-coordinate of the point
        #[arg(short, long)]
        y: f64,
        /// Optional output file path (JSON format)
        #[arg(short, long)]
        output: Option<PathBuf>,
    },
    /// Create a line segment between two points.
    ///
    /// If no output file is specified, the line is printed to stdout.
    /// Otherwise, it's saved to the specified file in JSON format.
    Line {
        /// X-coordinate of the starting point
        #[arg(long)]
        x1: f64,
        /// Y-coordinate of the starting point
        #[arg(long)]
        y1: f64,
        /// X-coordinate of the ending point
        #[arg(long)]
        x2: f64,
        /// Y-coordinate of the ending point
        #[arg(long)]
        y2: f64,
        /// Optional output file path (JSON format)
        #[arg(short, long)]
        output: Option<PathBuf>,
    },
    /// Read and display geometries from a file.
    ///
    /// Supports both JSON and simple text formats. The format is
    /// automatically detected unless --simple is specified.
    Read {
        /// Input file path to read from
        #[arg(short, long)]
        input: PathBuf,
        /// Force interpretation as simple text format
        #[arg(long, default_value = "false")]
        simple: bool,
    },
    /// Convert geometry files between formats.
    ///
    /// Input is always treated as JSON format unless otherwise specified.
    /// Output format is controlled by the --simple flag.
    Write {
        /// Input file path (JSON format)
        #[arg(short, long)]
        input: PathBuf,
        /// Output file path
        #[arg(short, long)]
        output: PathBuf,
        /// Write output in simple text format instead of JSON
        #[arg(long, default_value = "false")]
        simple: bool,
    },
    /// Translate (move) geometries by a displacement vector.
    ///
    /// All geometries in the input file are moved by the same
    /// displacement (dx, dy). The operation preserves shapes and orientations.
    Translate {
        /// Input file path
        #[arg(short, long)]
        input: PathBuf,
        /// Output file path
        #[arg(short, long)]
        output: PathBuf,
        /// Displacement along the X-axis
        #[arg(long)]
        dx: f64,
        /// Displacement along the Y-axis
        #[arg(long)]
        dy: f64,
        /// Use simple text format for both input and output
        #[arg(long, default_value = "false")]
        simple: bool,
    },
    /// Rotate geometries around a center point.
    ///
    /// All geometries are rotated by the same angle around the specified
    /// center point. Positive angles result in counter-clockwise rotation.
    Rotate {
        /// Input file path
        #[arg(short, long)]
        input: PathBuf,
        /// Output file path
        #[arg(short, long)]
        output: PathBuf,
        /// Rotation angle (radians by default, degrees if --degrees is specified)
        #[arg(short, long)]
        angle: f64,
        /// X-coordinate of the rotation center
        #[arg(long, default_value = "0.0")]
        center_x: f64,
        /// Y-coordinate of the rotation center
        #[arg(long, default_value = "0.0")]
        center_y: f64,
        /// Interpret angle as degrees instead of radians
        #[arg(long, default_value = "false")]
        degrees: bool,
        /// Use simple text format for both input and output
        #[arg(long, default_value = "false")]
        simple: bool,
    },
}

/// Main entry point for the CLI application.
///
/// This function orchestrates the entire application flow:
/// 1. Parse command-line arguments using clap
/// 2. Dispatch to the appropriate handler based on the subcommand
/// 3. Handle errors and provide user feedback
///
/// # Error Handling
/// Uses anyhow::Result for automatic error propagation and context.
/// All errors are displayed with helpful context to the user.
fn main() -> Result<()> {
    let cli = Cli::parse();
    
    // Dispatch to appropriate command handler
    match cli.command {
        // Handle point creation command
        Commands::Point { x, y, output } => {
            let point = Geometry::Point(Point::new(x, y));
            if let Some(output_path) = output {
                // Save point to file
                write_geometry_file(&output_path, &[point])?;
                println!("Point saved to {}", output_path.display());
            } else {
                // Display point to stdout
                println!("Point: ({}, {})", x, y);
            }
        }
        
        // Handle line creation command
        Commands::Line { x1, y1, x2, y2, output } => {
            let line = Geometry::Line(Line::new(Point::new(x1, y1), Point::new(x2, y2)));
            if let Some(output_path) = output {
                // Save line to file
                write_geometry_file(&output_path, &[line])?;
                println!("Line saved to {}", output_path.display());
            } else {
                // Display line to stdout
                println!("Line: ({}, {}) to ({}, {})", x1, y1, x2, y2);
            }
        }
        
        // Handle file reading and display command
        Commands::Read { input, simple } => {
            // Choose appropriate reader based on format flag
            let geometries = if simple {
                read_simple_format(&input)?
            } else {
                read_geometry_file(&input)?
            };
            
            // Display summary and detailed listing
            println!("Read {} geometries from {}", geometries.len(), input.display());
            for (i, geometry) in geometries.iter().enumerate() {
                match geometry {
                    Geometry::Point(p) => println!("  {}: Point ({}, {})", i + 1, p.x, p.y),
                    Geometry::Line(l) => println!("  {}: Line ({}, {}) to ({}, {})", 
                        i + 1, l.start.x, l.start.y, l.end.x, l.end.y),
                }
            }
        }
        
        // Handle file format conversion command
        Commands::Write { input, output, simple } => {
            // Always read input as JSON format
            let geometries = read_geometry_file(&input)?;
            
            // Write in requested format
            if simple {
                write_simple_format(&output, &geometries)?;
            } else {
                write_geometry_file(&output, &geometries)?;
            }
            
            println!("Wrote {} geometries to {}", geometries.len(), output.display());
        }
        
        // Handle translation transformation command
        Commands::Translate { input, output, dx, dy, simple } => {
            // Read geometries using appropriate format
            let geometries = if simple {
                read_simple_format(&input)?
            } else {
                read_geometry_file(&input)?
            };
            
            // Apply translation transformation to all geometries
            let translated: Vec<Geometry> = geometries
                .iter()
                .map(|g| g.translate(dx, dy))
                .collect();
            
            // Write results using appropriate format
            if simple {
                write_simple_format(&output, &translated)?;
            } else {
                write_geometry_file(&output, &translated)?;
            }
            
            println!("Translated {} geometries by ({}, {}) and saved to {}", 
                translated.len(), dx, dy, output.display());
        }
        
        // Handle rotation transformation command
        Commands::Rotate { input, output, angle, center_x, center_y, degrees, simple } => {
            // Read geometries using appropriate format
            let geometries = if simple {
                read_simple_format(&input)?
            } else {
                read_geometry_file(&input)?
            };
            
            // Convert angle to radians if necessary
            let angle_rad = if degrees {
                degrees_to_radians(angle)
            } else {
                angle
            };
            
            // Apply rotation transformation around specified center
            let center = Point::new(center_x, center_y);
            let rotated: Vec<Geometry> = geometries
                .iter()
                .map(|g| g.rotate(angle_rad, center))
                .collect();
            
            // Write results using appropriate format
            if simple {
                write_simple_format(&output, &rotated)?;
            } else {
                write_geometry_file(&output, &rotated)?;
            }
            
            // Provide feedback with original angle units
            let angle_unit = if degrees { "degrees" } else { "radians" };
            println!("Rotated {} geometries by {} {} around ({}, {}) and saved to {}", 
                rotated.len(), angle, angle_unit, center_x, center_y, output.display());
        }
    }
    
    Ok(())
}