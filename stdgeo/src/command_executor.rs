//! Command execution module for stdgeo
//!
//! This module provides command execution capabilities that accept parsed function calls
//! from stdgeo-parser and execute them against a managed geometry session.
//! This centralizes all geometry operations and state management in the stdgeo library.

use crate::{GeometrySession, Geometry, Point, Line, degrees_to_radians};
use std::path::PathBuf;
use thiserror::Error;

/// Function call representation (matches stdgeo-parser types)
#[derive(Debug, Clone)]
pub struct FunctionCall {
    pub function: String,
    pub args: FunctionArgs,
}

/// Function arguments (matches stdgeo-parser types)
#[derive(Debug, Clone)]
pub enum FunctionArgs {
    Point { x: f64, y: f64 },
    Line { x1: f64, y1: f64, x2: f64, y2: f64 },
    Translate { dx: f64, dy: f64 },
    Rotate { angle: f64, center_x: f64, center_y: f64, use_degrees: bool },
    Load { path: PathBuf, simple_format: bool },
    Save { path: PathBuf, simple_format: bool },
    NoArgs,
}

/// Result of command execution
#[derive(Debug, Clone)]
pub enum ExecutionResult {
    /// Command executed successfully with optional message
    Success(Option<String>),
    /// Command executed successfully and returned data
    Data(String),
}

/// Command execution errors
#[derive(Error, Debug)]
pub enum ExecutionError {
    #[error("Unknown function: {function}")]
    UnknownFunction { function: String },
    
    #[error("Invalid arguments for function {function}: {message}")]
    InvalidArguments { function: String, message: String },
    
    #[error("IO error: {0}")]
    IoError(#[from] crate::io::IoError),
    
    #[error("Session error: {message}")]
    SessionError { message: String },
}

/// Command executor that manages geometry session and executes function calls
#[derive(Debug)]
pub struct CommandExecutor {
    session: GeometrySession,
}

impl CommandExecutor {
    /// Create a new command executor with an empty session
    pub fn new() -> Self {
        Self {
            session: GeometrySession::new(),
        }
    }
    
    /// Create a command executor from an existing session
    pub fn from_session(session: GeometrySession) -> Self {
        Self { session }
    }
    
    /// Get a reference to the current session
    pub fn session(&self) -> &GeometrySession {
        &self.session
    }
    
    /// Get a mutable reference to the current session
    pub fn session_mut(&mut self) -> &mut GeometrySession {
        &mut self.session
    }
    
    /// Execute a function call and return the result
    pub fn execute(&mut self, call: FunctionCall) -> Result<ExecutionResult, ExecutionError> {
        match call.function.as_str() {
            "point" => self.execute_point(call.args),
            "line" => self.execute_line(call.args),
            "list" => self.execute_list(call.args),
            "clear" => self.execute_clear(call.args),
            "count" => self.execute_count(call.args),
            "translate" => self.execute_translate(call.args),
            "rotate" => self.execute_rotate(call.args),
            "load" => self.execute_load(call.args),
            "save" => self.execute_save(call.args),
            _ => Err(ExecutionError::UnknownFunction {
                function: call.function,
            }),
        }
    }
    
    /// Execute multiple function calls in sequence
    pub fn execute_batch(&mut self, calls: Vec<FunctionCall>) -> Vec<Result<ExecutionResult, ExecutionError>> {
        calls.into_iter().map(|call| self.execute(call)).collect()
    }
    
    /// Save current session to a file
    pub fn save_session_to_file<P: AsRef<std::path::Path>>(&self, path: P) -> Result<(), ExecutionError> {
        self.session.save_to_file(path).map_err(ExecutionError::from)
    }
    
    /// Load session from a file
    pub fn load_session_from_file<P: AsRef<std::path::Path>>(&mut self, path: P) -> Result<usize, ExecutionError> {
        self.session.load_from_file(path).map_err(ExecutionError::from)
    }
    
    fn execute_point(&mut self, args: FunctionArgs) -> Result<ExecutionResult, ExecutionError> {
        if let FunctionArgs::Point { x, y } = args {
            self.session.add_geometry(Geometry::Point(Point::new(x, y)));
            Ok(ExecutionResult::Success(Some(format!("Created point ({}, {})", x, y))))
        } else {
            Err(ExecutionError::InvalidArguments {
                function: "point".to_string(),
                message: "Expected Point arguments".to_string(),
            })
        }
    }
    
    fn execute_line(&mut self, args: FunctionArgs) -> Result<ExecutionResult, ExecutionError> {
        if let FunctionArgs::Line { x1, y1, x2, y2 } = args {
            self.session.add_geometry(Geometry::Line(Line::new(
                Point::new(x1, y1),
                Point::new(x2, y2),
            )));
            Ok(ExecutionResult::Success(Some(format!(
                "Created line from ({}, {}) to ({}, {})",
                x1, y1, x2, y2
            ))))
        } else {
            Err(ExecutionError::InvalidArguments {
                function: "line".to_string(),
                message: "Expected Line arguments".to_string(),
            })
        }
    }
    
    fn execute_list(&self, _args: FunctionArgs) -> Result<ExecutionResult, ExecutionError> {
        Ok(ExecutionResult::Data(self.session.list_geometries()))
    }
    
    fn execute_clear(&mut self, _args: FunctionArgs) -> Result<ExecutionResult, ExecutionError> {
        let count = self.session.count();
        self.session.clear();
        Ok(ExecutionResult::Success(Some(format!("Cleared {} geometry objects", count))))
    }
    
    fn execute_count(&self, _args: FunctionArgs) -> Result<ExecutionResult, ExecutionError> {
        Ok(ExecutionResult::Data(format!("Session contains {} geometry objects", self.session.count())))
    }
    
    fn execute_translate(&mut self, args: FunctionArgs) -> Result<ExecutionResult, ExecutionError> {
        if let FunctionArgs::Translate { dx, dy } = args {
            let count = self.session.count();
            if count == 0 {
                Ok(ExecutionResult::Success(Some("No objects to translate".to_string())))
            } else {
                self.session.translate_all(dx, dy);
                Ok(ExecutionResult::Success(Some(format!(
                    "Translated {} objects by ({}, {})",
                    count, dx, dy
                ))))
            }
        } else {
            Err(ExecutionError::InvalidArguments {
                function: "translate".to_string(),
                message: "Expected Translate arguments".to_string(),
            })
        }
    }
    
    fn execute_rotate(&mut self, args: FunctionArgs) -> Result<ExecutionResult, ExecutionError> {
        if let FunctionArgs::Rotate { angle, center_x, center_y, use_degrees } = args {
            let mut final_angle = angle;
            if use_degrees {
                final_angle = degrees_to_radians(angle);
            }
            
            let count = self.session.count();
            if count == 0 {
                Ok(ExecutionResult::Success(Some("No objects to rotate".to_string())))
            } else {
                let center = Point::new(center_x, center_y);
                self.session.rotate_all(final_angle, center);
                
                let angle_unit = if use_degrees { "degrees" } else { "radians" };
                Ok(ExecutionResult::Success(Some(format!(
                    "Rotated {} objects by {} {} around ({}, {})",
                    count, angle, angle_unit, center_x, center_y
                ))))
            }
        } else {
            Err(ExecutionError::InvalidArguments {
                function: "rotate".to_string(),
                message: "Expected Rotate arguments".to_string(),
            })
        }
    }
    
    fn execute_load(&mut self, args: FunctionArgs) -> Result<ExecutionResult, ExecutionError> {
        if let FunctionArgs::Load { path, simple_format } = args {
            let count = if simple_format {
                self.session.load_from_simple_file(&path)?
            } else {
                self.session.load_from_file(&path)?
            };
            
            Ok(ExecutionResult::Success(Some(format!(
                "Loaded {} geometry objects from {}",
                count,
                path.display()
            ))))
        } else {
            Err(ExecutionError::InvalidArguments {
                function: "load".to_string(),
                message: "Expected Load arguments".to_string(),
            })
        }
    }
    
    fn execute_save(&mut self, args: FunctionArgs) -> Result<ExecutionResult, ExecutionError> {
        if let FunctionArgs::Save { path, simple_format } = args {
            if self.session.is_empty() {
                Ok(ExecutionResult::Success(Some("No objects to save".to_string())))
            } else {
                if simple_format {
                    self.session.save_to_simple_file(&path)?;
                } else {
                    self.session.save_to_file(&path)?;
                }
                
                Ok(ExecutionResult::Success(Some(format!(
                    "Saved {} geometry objects to {}",
                    self.session.count(),
                    path.display()
                ))))
            }
        } else {
            Err(ExecutionError::InvalidArguments {
                function: "save".to_string(),
                message: "Expected Save arguments".to_string(),
            })
        }
    }
}

impl Default for CommandExecutor {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
        
    #[test]
    fn test_point_execution() {
        let mut executor = CommandExecutor::new();
        
        let call = FunctionCall {
            function: "point".to_string(),
            args: FunctionArgs::Point { x: 1.0, y: 2.0 },
        };
        
        let result = executor.execute(call).unwrap();
        assert!(matches!(result, ExecutionResult::Success(_)));
        assert_eq!(executor.session().count(), 1);
    }
    
    #[test]
    fn test_line_execution() {
        let mut executor = CommandExecutor::new();
        
        let call = FunctionCall {
            function: "line".to_string(),
            args: FunctionArgs::Line { x1: 0.0, y1: 0.0, x2: 3.0, y2: 4.0 },
        };
        
        let result = executor.execute(call).unwrap();
        assert!(matches!(result, ExecutionResult::Success(_)));
        assert_eq!(executor.session().count(), 1);
    }
    
    #[test]
    fn test_count_execution() {
        let mut executor = CommandExecutor::new();
        
        // Add some geometry
        executor.session_mut().add_geometry(Geometry::Point(Point::new(1.0, 2.0)));
        
        let call = FunctionCall {
            function: "count".to_string(),
            args: FunctionArgs::NoArgs,
        };
        
        let result = executor.execute(call).unwrap();
        if let ExecutionResult::Data(data) = result {
            assert!(data.contains("1 geometry objects"));
        } else {
            panic!("Expected Data result");
        }
    }
    
    #[test]
    fn test_unknown_function() {
        let mut executor = CommandExecutor::new();
        
        let call = FunctionCall {
            function: "unknown".to_string(),
            args: FunctionArgs::NoArgs,
        };
        
        let result = executor.execute(call);
        assert!(result.is_err());
        assert!(matches!(result.unwrap_err(), ExecutionError::UnknownFunction { .. }));
    }
    
    #[test]
    fn test_batch_execution() {
        let mut executor = CommandExecutor::new();
        
        let calls = vec![
            FunctionCall {
                function: "point".to_string(),
                args: FunctionArgs::Point { x: 1.0, y: 2.0 },
            },
            FunctionCall {
                function: "point".to_string(),
                args: FunctionArgs::Point { x: 3.0, y: 4.0 },
            },
            FunctionCall {
                function: "count".to_string(),
                args: FunctionArgs::NoArgs,
            },
        ];
        
        let results = executor.execute_batch(calls);
        assert_eq!(results.len(), 3);
        assert!(results.iter().all(|r| r.is_ok()));
        assert_eq!(executor.session().count(), 2);
    }
}