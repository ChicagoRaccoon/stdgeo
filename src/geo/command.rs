use crate::geo::geometry::*;
use std::io::{self, Write};

/// Core command processor for geometry operations
/// This is the headless-first core that both CLI and GUI use
pub struct CommandProcessor {
    current_mesh: Option<Mesh>,
    verbose: bool,
}

/// Result of command execution
#[derive(Debug, Clone)]
pub struct CommandResult {
    pub success: bool,
    pub message: String,
    pub geometry_changed: bool,
}

impl CommandResult {
    pub fn success(message: impl Into<String>) -> Self {
        Self {
            success: true,
            message: message.into(),
            geometry_changed: false,
        }
    }
    
    pub fn success_with_geometry_change(message: impl Into<String>) -> Self {
        Self {
            success: true,
            message: message.into(),
            geometry_changed: true,
        }
    }
    
    pub fn error(message: impl Into<String>) -> Self {
        Self {
            success: false,
            message: message.into(),
            geometry_changed: false,
        }
    }
}

impl CommandProcessor {
    /// Create a new command processor
    pub fn new(verbose: bool) -> Self {
        Self {
            current_mesh: None,
            verbose,
        }
    }
    
    /// Process a single command and return the result
    pub fn process_command(&mut self, command: &str) -> CommandResult {
        let command = command.trim();
        
        // Skip empty lines and comments
        if command.is_empty() || command.starts_with('#') {
            return CommandResult::success("");
        }
        
        let parts: Vec<&str> = command.split_whitespace().collect();
        if parts.is_empty() {
            return CommandResult::success("");
        }
        
        let cmd = parts[0].to_lowercase();
        
        match cmd.as_str() {
            "help" => self.help_command(),
            "cube" => self.cube_command(&parts),
            "clear" => self.clear_command(),
            "stats" => self.stats_command(),
            "quit" | "exit" => CommandResult::success("exit"),
            _ => CommandResult::error(format!("Unknown command: {}\nType 'help' for available commands", parts[0])),
        }
    }
    
    /// Get current mesh (for GUI integration)
    pub fn current_mesh(&self) -> Option<&Mesh> {
        self.current_mesh.as_ref()
    }
    
    /// Set verbose mode
    pub fn set_verbose(&mut self, verbose: bool) {
        self.verbose = verbose;
    }
    
    /// Run interactive session (for CLI use)
    pub fn run_interactive(&mut self) -> io::Result<()> {
        println!("StdGeo Headless Mode");
        println!("Type 'help' for commands, 'exit' to quit");
        
        self.print_help();
        
        loop {
            print!("> ");
            io::stdout().flush()?;
            
            let mut input = String::new();
            match io::stdin().read_line(&mut input) {
                Ok(0) => break, // EOF
                Ok(_) => {
                    let result = self.process_command(&input);
                    
                    if !result.message.is_empty() {
                        if result.message == "exit" {
                            break;
                        }
                        println!("{}", result.message);
                    }
                    
                    if !result.success && self.verbose {
                        eprintln!("Command failed");
                    }
                }
                Err(error) => {
                    eprintln!("Error reading input: {}", error);
                    break;
                }
            }
        }
        
        Ok(())
    }
    
    /// Run batch processing from stdin (for scripted use)
    pub fn run_batch(&mut self) -> io::Result<()> {
        if self.verbose {
            println!("StdGeo Batch Mode");
        }
        
        let stdin = io::stdin();
        loop {
            let mut input = String::new();
            match stdin.read_line(&mut input) {
                Ok(0) => break, // EOF
                Ok(_) => {
                    let result = self.process_command(&input);
                    
                    if !result.message.is_empty() && result.message != "exit" {
                        println!("{}", result.message);
                    }
                    
                    if result.message == "exit" {
                        break;
                    }
                    
                    if !result.success && self.verbose {
                        eprintln!("Command failed: {}", result.message);
                    }
                }
                Err(error) => {
                    eprintln!("Error reading input: {}", error);
                    return Err(error);
                }
            }
        }
        
        Ok(())
    }
    
    fn help_command(&self) -> CommandResult {
        CommandResult::success(self.get_help_text())
    }
    
    fn get_help_text(&self) -> String {
        "Available Commands:
  cube [size]  - Create a cube (default size: 2.0)
  clear        - Clear current geometry
  stats        - Show current geometry statistics
  help         - Show this help message
  exit/quit    - Exit the application".to_string()
    }
    
    fn print_help(&self) {
        println!("{}", self.get_help_text());
    }
    
    fn cube_command(&mut self, parts: &[&str]) -> CommandResult {
        let size = if parts.len() > 1 {
            match parts[1].parse::<f64>() {
                Ok(s) if s > 0.0 => s,
                Ok(_) => return CommandResult::error("Error: Cube size must be positive"),
                Err(_) => return CommandResult::error("Error: Invalid cube size"),
            }
        } else {
            2.0
        };
        
        self.current_mesh = Some(Mesh::create_cube(size));
        
        let mesh = self.current_mesh.as_ref().unwrap();
        CommandResult::success_with_geometry_change(format!(
            "Created cube (size: {}) with {} vertices and {} triangles",
            size,
            mesh.vertices.len(),
            mesh.triangles.len()
        ))
    }
    
    fn clear_command(&mut self) -> CommandResult {
        self.current_mesh = None;
        CommandResult::success_with_geometry_change("Cleared geometry".to_string())
    }
    
    fn stats_command(&self) -> CommandResult {
        match &self.current_mesh {
            Some(mesh) => {
                let mut result = format!(
                    "Current geometry: {} vertices, {} triangles",
                    mesh.vertices.len(),
                    mesh.triangles.len()
                );
                
                if !mesh.vertices.is_empty() {
                    result.push_str("\nSample vertices:");
                    for (i, vertex) in mesh.vertices.iter().enumerate().take(3) {
                        result.push_str(&format!("\n  [{}]: ({}, {}, {})", i, vertex.x, vertex.y, vertex.z));
                    }
                }
                
                CommandResult::success(result)
            }
            None => CommandResult::success("No geometry loaded".to_string()),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_command_processor_creation() {
        let processor = CommandProcessor::new(false);
        assert!(processor.current_mesh.is_none());
        assert!(!processor.verbose);
    }
    
    #[test]
    fn test_help_command() {
        let mut processor = CommandProcessor::new(false);
        let result = processor.process_command("help");
        assert!(result.success);
        assert!(result.message.contains("Available Commands"));
        assert!(result.message.contains("cube"));
    }
    
    #[test]
    fn test_cube_command() {
        let mut processor = CommandProcessor::new(false);
        let result = processor.process_command("cube 3.0");
        assert!(result.success);
        assert!(result.geometry_changed);
        assert!(result.message.contains("Created cube (size: 3)"));
        assert!(processor.current_mesh.is_some());
    }
    
    #[test]
    fn test_cube_command_default_size() {
        let mut processor = CommandProcessor::new(false);
        let result = processor.process_command("cube");
        assert!(result.success);
        assert!(result.message.contains("Created cube (size: 2)"));
    }
    
    #[test]
    fn test_cube_command_invalid_size() {
        let mut processor = CommandProcessor::new(false);
        let result = processor.process_command("cube invalid");
        assert!(!result.success);
        assert!(result.message.contains("Invalid cube size"));
    }
    
    #[test]
    fn test_cube_command_negative_size() {
        let mut processor = CommandProcessor::new(false);
        let result = processor.process_command("cube -1.0");
        assert!(!result.success);
        assert!(result.message.contains("must be positive"));
    }
    
    #[test]
    fn test_clear_command() {
        let mut processor = CommandProcessor::new(false);
        processor.process_command("cube 1.0");
        assert!(processor.current_mesh.is_some());
        
        let result = processor.process_command("clear");
        assert!(result.success);
        assert!(result.geometry_changed);
        assert!(result.message.contains("Cleared geometry"));
        assert!(processor.current_mesh.is_none());
    }
    
    #[test]
    fn test_stats_command_with_geometry() {
        let mut processor = CommandProcessor::new(false);
        processor.process_command("cube 2.0");
        
        let result = processor.process_command("stats");
        assert!(result.success);
        assert!(result.message.contains("8 vertices"));
        assert!(result.message.contains("12 triangles"));
        assert!(result.message.contains("Sample vertices"));
    }
    
    #[test]
    fn test_stats_command_no_geometry() {
        let mut processor = CommandProcessor::new(false);
        let result = processor.process_command("stats");
        assert!(result.success);
        assert!(result.message.contains("No geometry loaded"));
    }
    
    #[test]
    fn test_unknown_command() {
        let mut processor = CommandProcessor::new(false);
        let result = processor.process_command("unknown");
        assert!(!result.success);
        assert!(result.message.contains("Unknown command"));
        assert!(result.message.contains("Type 'help'"));
    }
    
    #[test]
    fn test_empty_command() {
        let mut processor = CommandProcessor::new(false);
        let result = processor.process_command("");
        assert!(result.success);
        assert!(result.message.is_empty());
    }
    
    #[test]
    fn test_comment_command() {
        let mut processor = CommandProcessor::new(false);
        let result = processor.process_command("# This is a comment");
        assert!(result.success);
        assert!(result.message.is_empty());
    }
    
    #[test]
    fn test_exit_command() {
        let mut processor = CommandProcessor::new(false);
        let result = processor.process_command("exit");
        assert!(result.success);
        assert_eq!(result.message, "exit");
    }
    
    #[test]
    fn test_quit_command() {
        let mut processor = CommandProcessor::new(false);
        let result = processor.process_command("quit");
        assert!(result.success);
        assert_eq!(result.message, "exit");
    }
}