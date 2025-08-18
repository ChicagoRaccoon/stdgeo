use clap::{Parser, Subcommand};
use stdgeo_lib::{
    geometry::{Geometry, Point, Line, degrees_to_radians},
    io::{read_geometry_file, write_geometry_file, read_simple_format, write_simple_format}
};
use anyhow::Result;
use std::path::PathBuf;

#[derive(Parser)]
#[command(name = "stdgeo")]
#[command(about = "A CLI tool for geometric operations")]
#[command(version = "0.1.0")]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand)]
enum Commands {
    Point {
        #[arg(short, long)]
        x: f64,
        #[arg(short, long)]
        y: f64,
        #[arg(short, long)]
        output: Option<PathBuf>,
    },
    Line {
        #[arg(long)]
        x1: f64,
        #[arg(long)]
        y1: f64,
        #[arg(long)]
        x2: f64,
        #[arg(long)]
        y2: f64,
        #[arg(short, long)]
        output: Option<PathBuf>,
    },
    Read {
        #[arg(short, long)]
        input: PathBuf,
        #[arg(long, default_value = "false")]
        simple: bool,
    },
    Write {
        #[arg(short, long)]
        input: PathBuf,
        #[arg(short, long)]
        output: PathBuf,
        #[arg(long, default_value = "false")]
        simple: bool,
    },
    Translate {
        #[arg(short, long)]
        input: PathBuf,
        #[arg(short, long)]
        output: PathBuf,
        #[arg(long)]
        dx: f64,
        #[arg(long)]
        dy: f64,
        #[arg(long, default_value = "false")]
        simple: bool,
    },
    Rotate {
        #[arg(short, long)]
        input: PathBuf,
        #[arg(short, long)]
        output: PathBuf,
        #[arg(short, long)]
        angle: f64,
        #[arg(long, default_value = "0.0")]
        center_x: f64,
        #[arg(long, default_value = "0.0")]
        center_y: f64,
        #[arg(long, default_value = "false")]
        degrees: bool,
        #[arg(long, default_value = "false")]
        simple: bool,
    },
}

fn main() -> Result<()> {
    let cli = Cli::parse();
    
    match cli.command {
        Commands::Point { x, y, output } => {
            let point = Geometry::Point(Point::new(x, y));
            if let Some(output_path) = output {
                write_geometry_file(&output_path, &[point])?;
                println!("Point saved to {}", output_path.display());
            } else {
                println!("Point: ({}, {})", x, y);
            }
        }
        
        Commands::Line { x1, y1, x2, y2, output } => {
            let line = Geometry::Line(Line::new(Point::new(x1, y1), Point::new(x2, y2)));
            if let Some(output_path) = output {
                write_geometry_file(&output_path, &[line])?;
                println!("Line saved to {}", output_path.display());
            } else {
                println!("Line: ({}, {}) to ({}, {})", x1, y1, x2, y2);
            }
        }
        
        Commands::Read { input, simple } => {
            let geometries = if simple {
                read_simple_format(&input)?
            } else {
                read_geometry_file(&input)?
            };
            
            println!("Read {} geometries from {}", geometries.len(), input.display());
            for (i, geometry) in geometries.iter().enumerate() {
                match geometry {
                    Geometry::Point(p) => println!("  {}: Point ({}, {})", i + 1, p.x, p.y),
                    Geometry::Line(l) => println!("  {}: Line ({}, {}) to ({}, {})", 
                        i + 1, l.start.x, l.start.y, l.end.x, l.end.y),
                }
            }
        }
        
        Commands::Write { input, output, simple } => {
            let geometries = read_geometry_file(&input)?;
            
            if simple {
                write_simple_format(&output, &geometries)?;
            } else {
                write_geometry_file(&output, &geometries)?;
            }
            
            println!("Wrote {} geometries to {}", geometries.len(), output.display());
        }
        
        Commands::Translate { input, output, dx, dy, simple } => {
            let geometries = if simple {
                read_simple_format(&input)?
            } else {
                read_geometry_file(&input)?
            };
            
            let translated: Vec<Geometry> = geometries
                .iter()
                .map(|g| g.translate(dx, dy))
                .collect();
            
            if simple {
                write_simple_format(&output, &translated)?;
            } else {
                write_geometry_file(&output, &translated)?;
            }
            
            println!("Translated {} geometries by ({}, {}) and saved to {}", 
                translated.len(), dx, dy, output.display());
        }
        
        Commands::Rotate { input, output, angle, center_x, center_y, degrees, simple } => {
            let geometries = if simple {
                read_simple_format(&input)?
            } else {
                read_geometry_file(&input)?
            };
            
            let angle_rad = if degrees {
                degrees_to_radians(angle)
            } else {
                angle
            };
            
            let center = Point::new(center_x, center_y);
            let rotated: Vec<Geometry> = geometries
                .iter()
                .map(|g| g.rotate(angle_rad, center))
                .collect();
            
            if simple {
                write_simple_format(&output, &rotated)?;
            } else {
                write_geometry_file(&output, &rotated)?;
            }
            
            let angle_unit = if degrees { "degrees" } else { "radians" };
            println!("Rotated {} geometries by {} {} around ({}, {}) and saved to {}", 
                rotated.len(), angle, angle_unit, center_x, center_y, output.display());
        }
    }
    
    Ok(())
}