//! # StdGeo CLI Application
//!
//! A command-line interface for geometric operations and file format conversions.
//! Supports both single-command mode and interactive session mode.
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
//! - `session` - Start interactive session mode
//!
//! ### Interactive Session Mode
//! When started in session mode, the CLI maintains state between commands:
//! - Geometry objects persist across commands
//! - Commands can reference previously created objects
//! - Session state can be saved to files
//! - Commands are parsed interactively with readline support
//!
//! ### File Format Support
//! The CLI supports both JSON and simple text formats:
//! - JSON format: Machine-readable, preserves precision
//! - Simple format: Human-editable, compact representation
//!
//! ### Transformation Pipeline
//! Operations follow a consistent pattern:
//! 1. Read geometry from input file or session state
//! 2. Apply transformation (if applicable)
//! 3. Write result to output file or update session state
//! 4. Provide user feedback
//!
//! ### Error Handling
//! Uses anyhow for ergonomic error handling with context preservation.
//! All file operations and transformations are wrapped in Result types.
//!
//! ## Usage Examples
//!
//! ### Single Command Mode
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
//!
//! ### Interactive Session Mode
//! ```bash
//! # Start interactive session
//! stdgeo session
//! 
//! # Then issue commands interactively:
//! > point 3.0 4.0
//! > line 0.0 0.0 5.0 5.0
//! > translate 1.0 2.0
//! > list
//! > save output.json
//! > quit
//! ```

use clap::{Parser, Subcommand};
use stdgeo::{
    geometry::{Geometry, Point, Line, degrees_to_radians},
    io::{read_geometry_file, write_geometry_file, read_simple_format, write_simple_format}
};
use anyhow::{Result, Context};
use std::path::PathBuf;
use rustyline::{DefaultEditor, Result as RustyResult};

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
/// - Interactive: Session
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
    /// Start an interactive session for geometry operations.
    ///
    /// In session mode, you can issue commands interactively and maintain
    /// state between operations. Geometry objects persist throughout the session.
    Session,
}

/// Session state for interactive mode
struct Session {
    geometries: Vec<Geometry>,
    editor: DefaultEditor,
}

impl Session {
    fn new() -> RustyResult<Self> {
        let editor = DefaultEditor::new()?;
        Ok(Session {
            geometries: Vec::new(),
            editor,
        })
    }

    fn run(&mut self) -> Result<()> {
        println!("StdGeo Interactive Session");
        println!("Type 'help' for available commands, 'quit' to exit");
        
        loop {
            let readline = self.editor.readline("stdgeo> ");
            match readline {
                Ok(line) => {
                    if line.trim().is_empty() {
                        continue;
                    }
                    
                    self.editor.add_history_entry(&line).ok();
                    
                    if let Err(e) = self.handle_command(&line) {
                        println!("Error: {}", e);
                    }
                }
                Err(rustyline::error::ReadlineError::Interrupted) => {
                    println!("Interrupted");
                    break;
                }
                Err(rustyline::error::ReadlineError::Eof) => {
                    println!("EOF");
                    break;
                }
                Err(err) => {
                    println!("Error: {:?}", err);
                    break;
                }
            }
        }
        
        Ok(())
    }

    fn handle_command(&mut self, line: &str) -> Result<()> {
        let parts: Vec<&str> = line.split_whitespace().collect();
        if parts.is_empty() {
            return Ok(());
        }

        match parts[0] {
            "help" => self.show_help(),
            "quit" | "exit" => std::process::exit(0),
            "point" => self.create_point(&parts[1..])?,
            "line" => self.create_line(&parts[1..])?,
            "list" => self.list_geometries(),
            "clear" => self.clear_geometries(),
            "translate" => self.translate_all(&parts[1..])?,
            "rotate" => self.rotate_all(&parts[1..])?,
            "load" => self.load_from_file(&parts[1..])?,
            "save" => self.save_to_file(&parts[1..])?,
            "count" => self.show_count(),
            _ => println!("Unknown command: {}. Type 'help' for available commands.", parts[0]),
        }
        
        Ok(())
    }

    fn show_help(&self) {
        println!("Available commands:");
        println!("  point <x> <y>                    - Create a point");
        println!("  line <x1> <y1> <x2> <y2>        - Create a line segment");
        println!("  list                             - List all geometry objects");
        println!("  count                            - Show number of objects");
        println!("  clear                            - Clear all objects");
        println!("  translate <dx> <dy>              - Translate all objects");
        println!("  rotate <angle> [<center_x> <center_y>] [--degrees] - Rotate all objects");
        println!("  load <filename> [--simple]      - Load objects from file");
        println!("  save <filename> [--simple]      - Save objects to file");
        println!("  help                             - Show this help");
        println!("  quit, exit                       - Exit session");
    }

    fn create_point(&mut self, args: &[&str]) -> Result<()> {
        if args.len() != 2 {
            return Err(anyhow::anyhow!("point command requires 2 arguments: <x> <y>"));
        }
        
        let x: f64 = args[0].parse().context("Invalid x coordinate")?;
        let y: f64 = args[1].parse().context("Invalid y coordinate")?;
        
        let point = Geometry::Point(Point::new(x, y));
        self.geometries.push(point);
        
        println!("Created point ({}, {})", x, y);
        Ok(())
    }

    fn create_line(&mut self, args: &[&str]) -> Result<()> {
        if args.len() != 4 {
            return Err(anyhow::anyhow!("line command requires 4 arguments: <x1> <y1> <x2> <y2>"));
        }
        
        let x1: f64 = args[0].parse().context("Invalid x1 coordinate")?;
        let y1: f64 = args[1].parse().context("Invalid y1 coordinate")?;
        let x2: f64 = args[2].parse().context("Invalid x2 coordinate")?;
        let y2: f64 = args[3].parse().context("Invalid y2 coordinate")?;
        
        let line = Geometry::Line(Line::new(Point::new(x1, y1), Point::new(x2, y2)));
        self.geometries.push(line);
        
        println!("Created line from ({}, {}) to ({}, {})", x1, y1, x2, y2);
        Ok(())
    }

    fn list_geometries(&self) {
        if self.geometries.is_empty() {
            println!("No geometry objects in session");
            return;
        }
        
        println!("Session contains {} geometry objects:", self.geometries.len());
        for (i, geometry) in self.geometries.iter().enumerate() {
            match geometry {
                Geometry::Point(p) => println!("  {}: Point ({}, {})", i + 1, p.x, p.y),
                Geometry::Line(l) => println!("  {}: Line ({}, {}) to ({}, {})", 
                    i + 1, l.start.x, l.start.y, l.end.x, l.end.y),
            }
        }
    }

    fn clear_geometries(&mut self) {
        let count = self.geometries.len();
        self.geometries.clear();
        println!("Cleared {} geometry objects", count);
    }

    fn show_count(&self) {
        println!("Session contains {} geometry objects", self.geometries.len());
    }

    fn translate_all(&mut self, args: &[&str]) -> Result<()> {
        if args.len() != 2 {
            return Err(anyhow::anyhow!("translate command requires 2 arguments: <dx> <dy>"));
        }
        
        let dx: f64 = args[0].parse().context("Invalid dx displacement")?;
        let dy: f64 = args[1].parse().context("Invalid dy displacement")?;
        
        let count = self.geometries.len();
        if count == 0 {
            println!("No objects to translate");
            return Ok(());
        }
        
        for geometry in &mut self.geometries {
            *geometry = geometry.translate(dx, dy);
        }
        
        println!("Translated {} objects by ({}, {})", count, dx, dy);
        Ok(())
    }

    fn rotate_all(&mut self, args: &[&str]) -> Result<()> {
        if args.len() < 1 {
            return Err(anyhow::anyhow!("rotate command requires at least 1 argument: <angle> [<center_x> <center_y>] [--degrees]"));
        }
        
        let mut angle: f64 = args[0].parse().context("Invalid angle")?;
        let mut center = Point::origin();
        let mut degrees = false;
        
        let mut i = 1;
        while i < args.len() {
            match args[i] {
                "--degrees" => degrees = true,
                arg if i + 1 < args.len() => {
                    center.x = arg.parse().context("Invalid center x coordinate")?;
                    center.y = args[i + 1].parse().context("Invalid center y coordinate")?;
                    i += 1;
                }
                _ => return Err(anyhow::anyhow!("Invalid rotate command syntax")),
            }
            i += 1;
        }
        
        if degrees {
            angle = degrees_to_radians(angle);
        }
        
        let count = self.geometries.len();
        if count == 0 {
            println!("No objects to rotate");
            return Ok(());
        }
        
        for geometry in &mut self.geometries {
            *geometry = geometry.rotate(angle, center);
        }
        
        let angle_unit = if degrees { "degrees" } else { "radians" };
        println!("Rotated {} objects by {} {} around ({}, {})", 
            count, args[0], angle_unit, center.x, center.y);
        Ok(())
    }

    fn load_from_file(&mut self, args: &[&str]) -> Result<()> {
        if args.is_empty() {
            return Err(anyhow::anyhow!("load command requires a filename"));
        }
        
        let filename = args[0];
        let simple = args.contains(&"--simple");
        
        let path = PathBuf::from(filename);
        let geometries = if simple {
            read_simple_format(&path)?
        } else {
            read_geometry_file(&path)?
        };
        
        let count = geometries.len();
        self.geometries.extend(geometries);
        
        println!("Loaded {} geometry objects from {}", count, filename);
        Ok(())
    }

    fn save_to_file(&mut self, args: &[&str]) -> Result<()> {
        if args.is_empty() {
            return Err(anyhow::anyhow!("save command requires a filename"));
        }
        
        let filename = args[0];
        let simple = args.contains(&"--simple");
        
        if self.geometries.is_empty() {
            println!("No objects to save");
            return Ok(());
        }
        
        let path = PathBuf::from(filename);
        if simple {
            write_simple_format(&path, &self.geometries)?;
        } else {
            write_geometry_file(&path, &self.geometries)?;
        }
        
        println!("Saved {} geometry objects to {}", self.geometries.len(), filename);
        Ok(())
    }

    /// Execute a single command (used for non-interactive mode)
    fn execute_single_command(&mut self, command: &Commands) -> Result<()> {
        match command {
            Commands::Point { x, y, output } => {
                self.create_point(&[&x.to_string(), &y.to_string()])?;
                if let Some(output_path) = output {
                    write_geometry_file(output_path, &self.geometries)?;
                    println!("Point saved to {}", output_path.display());
                } else {
                    println!("Point: ({}, {})", x, y);
                }
            }
            
            Commands::Line { x1, y1, x2, y2, output } => {
                self.create_line(&[&x1.to_string(), &y1.to_string(), 
                                  &x2.to_string(), &y2.to_string()])?;
                if let Some(output_path) = output {
                    write_geometry_file(output_path, &self.geometries)?;
                    println!("Line saved to {}", output_path.display());
                } else {
                    println!("Line: ({}, {}) to ({}, {})", x1, y1, x2, y2);
                }
            }
            
            Commands::Read { input, simple } => {
                let filename = input.to_string_lossy();
                if *simple {
                    self.load_from_file(&[&filename, "--simple"])?;
                } else {
                    self.load_from_file(&[&filename])?;
                }
                self.list_geometries();
            }
            
            Commands::Write { input, output, simple } => {
                let geometries = read_geometry_file(input)?;
                if *simple {
                    write_simple_format(output, &geometries)?;
                } else {
                    write_geometry_file(output, &geometries)?;
                }
                println!("Wrote {} geometries to {}", geometries.len(), output.display());
            }
            
            Commands::Translate { input, output, dx, dy, simple } => {
                // Load geometries into session
                let filename = input.to_string_lossy();
                if *simple {
                    self.load_from_file(&[&filename, "--simple"])?;
                } else {
                    self.load_from_file(&[&filename])?;
                }
                
                // Apply translation
                let dx_str = dx.to_string();
                let dy_str = dy.to_string();
                self.translate_all(&[&dx_str, &dy_str])?;
                
                // Save results
                if *simple {
                    write_simple_format(output, &self.geometries)?;
                } else {
                    write_geometry_file(output, &self.geometries)?;
                }
                println!("Translated {} geometries by ({}, {}) and saved to {}", 
                    self.geometries.len(), dx, dy, output.display());
            }
            
            Commands::Rotate { input, output, angle, center_x, center_y, degrees, simple } => {
                // Load geometries into session
                let filename = input.to_string_lossy();
                if *simple {
                    self.load_from_file(&[&filename, "--simple"])?;
                } else {
                    self.load_from_file(&[&filename])?;
                }
                
                // Prepare rotation arguments
                let angle_str = angle.to_string();
                let center_x_str = center_x.to_string();
                let center_y_str = center_y.to_string();
                
                let mut args: Vec<&str> = vec![&angle_str];
                if *center_x != 0.0 || *center_y != 0.0 {
                    args.push(&center_x_str);
                    args.push(&center_y_str);
                }
                if *degrees {
                    args.push("--degrees");
                }
                
                // Apply rotation
                self.rotate_all(&args)?;
                
                // Save results
                if *simple {
                    write_simple_format(output, &self.geometries)?;
                } else {
                    write_geometry_file(output, &self.geometries)?;
                }
                
                let angle_unit = if *degrees { "degrees" } else { "radians" };
                println!("Rotated {} geometries by {} {} around ({}, {}) and saved to {}", 
                    self.geometries.len(), angle, angle_unit, center_x, center_y, output.display());
            }
            
            Commands::Session => {
                // This should never be reached due to the match in main
                unreachable!("Session command should be handled in main")
            }
        }
        
        Ok(())
    }
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
    
    // Handle commands using unified session-based approach
    match cli.command {
        // Handle interactive session command
        Commands::Session => {
            let mut session = Session::new()
                .context("Failed to initialize interactive session")?;
            session.run()?;
        }
        
        // All other commands use session internally for consistency
        _ => {
            let mut session = Session::new()
                .context("Failed to initialize session for single command")?;
            session.execute_single_command(&cli.command)?;
        }
    }
    
    Ok(())
}