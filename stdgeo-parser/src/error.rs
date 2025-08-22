//! Error types for the stdgeo parser

use thiserror::Error;

/// Errors that can occur during command parsing and execution
#[derive(Error, Debug)]
pub enum ParserError {
    /// Command not found in any registered parsers
    #[error("Unknown command: {command}")]
    UnknownCommand { command: String },
    
    /// Invalid command syntax or arguments
    #[error("Invalid command syntax: {message}")]
    InvalidSyntax { message: String },
    
    /// Missing required arguments
    #[error("Missing required arguments: {message}")]
    MissingArguments { message: String },
    
    /// Invalid argument values
    #[error("Invalid argument: {message}")]
    InvalidArgument { message: String },
    
    /// Geometry operation errors
    #[error("Geometry error: {message}")]
    GeometryError { message: String },
    
    /// File I/O errors
    #[error("File error: {0}")]
    FileError(#[from] std::io::Error),
    
    /// JSON parsing errors  
    #[error("JSON error: {0}")]
    JsonError(#[from] serde_json::Error),
    
    /// Generic errors from underlying operations
    #[error("Operation failed: {0}")]
    OperationError(#[from] anyhow::Error),
}

/// Result type for parser operations
pub type ParserResult<T> = Result<T, ParserError>;