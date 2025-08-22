//! Execution context for command parsing operations

use std::path::PathBuf;

/// Context for command parsing and execution coordination
/// 
/// This context only handles parsing state and does not manage geometry objects.
/// Geometry memory management should be handled by the calling application.
#[derive(Debug)]
pub struct Context {
    /// Current working directory for file operations
    pub working_directory: PathBuf,
    
    /// Settings and configuration
    pub settings: ContextSettings,
}

/// Configuration settings for the context
#[derive(Debug, Clone)]
pub struct ContextSettings {
    /// Default output format for geometry files
    pub default_format: OutputFormat,
    
    /// Precision for floating point output
    pub precision: usize,
    
    /// Whether to use verbose output
    pub verbose: bool,
}

/// Supported output formats
#[derive(Debug, Clone, PartialEq)]
pub enum OutputFormat {
    /// JSON format (machine readable)
    Json,
    /// Simple text format (human readable)
    Simple,
}

impl Default for ContextSettings {
    fn default() -> Self {
        Self {
            default_format: OutputFormat::Json,
            precision: 6,
            verbose: false,
        }
    }
}

impl Context {
    /// Create a new context with default settings
    pub fn new() -> Self {
        Self {
            working_directory: std::env::current_dir().unwrap_or_else(|_| PathBuf::from(".")),
            settings: ContextSettings::default(),
        }
    }
    
    /// Create a new context with custom settings
    pub fn with_settings(settings: ContextSettings) -> Self {
        Self {
            working_directory: std::env::current_dir().unwrap_or_else(|_| PathBuf::from(".")),
            settings,
        }
    }
    
    /// Set the working directory
    pub fn set_working_directory(&mut self, path: PathBuf) {
        self.working_directory = path;
    }
    
    /// Resolve a relative path against the working directory
    pub fn resolve_path(&self, path: &PathBuf) -> PathBuf {
        if path.is_absolute() {
            path.clone()
        } else {
            self.working_directory.join(path)
        }
    }
}

impl Default for Context {
    fn default() -> Self {
        Self::new()
    }
}