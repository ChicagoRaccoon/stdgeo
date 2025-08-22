//! Command execution interface for stdgeo-parser
//!
//! This module provides a bridge between stdgeo-parser and stdgeo library,
//! allowing parsed commands to be executed directly against a stdgeo CommandExecutor.

use crate::{FunctionCall as ParserFunctionCall, FunctionArgs as ParserFunctionArgs};
use stdgeo::{CommandExecutor, ExecutionResult, ExecutionError};

/// Convert parser function call types to stdgeo types
impl From<ParserFunctionCall> for stdgeo::FunctionCall {
    fn from(call: ParserFunctionCall) -> Self {
        stdgeo::FunctionCall {
            function: call.function,
            args: call.args.into(),
        }
    }
}

/// Convert parser function args to stdgeo types
impl From<ParserFunctionArgs> for stdgeo::FunctionArgs {
    fn from(args: ParserFunctionArgs) -> Self {
        match args {
            ParserFunctionArgs::Point { x, y } => stdgeo::FunctionArgs::Point { x, y },
            ParserFunctionArgs::Line { x1, y1, x2, y2 } => stdgeo::FunctionArgs::Line { x1, y1, x2, y2 },
            ParserFunctionArgs::Translate { dx, dy } => stdgeo::FunctionArgs::Translate { dx, dy },
            ParserFunctionArgs::Rotate { angle, center_x, center_y, use_degrees } => {
                stdgeo::FunctionArgs::Rotate { angle, center_x, center_y, use_degrees }
            },
            ParserFunctionArgs::Load { path, simple_format } => {
                stdgeo::FunctionArgs::Load { path, simple_format }
            },
            ParserFunctionArgs::Save { path, simple_format } => {
                stdgeo::FunctionArgs::Save { path, simple_format }
            },
            ParserFunctionArgs::NoArgs => stdgeo::FunctionArgs::NoArgs,
        }
    }
}

/// Command execution interface that bridges parser and stdgeo library
pub struct CommandExecutorInterface {
    executor: CommandExecutor,
}

impl CommandExecutorInterface {
    /// Create a new executor interface
    pub fn new() -> Self {
        Self {
            executor: CommandExecutor::new(),
        }
    }
    
    /// Create an executor interface from an existing stdgeo session
    pub fn from_session(session: stdgeo::GeometrySession) -> Self {
        Self {
            executor: CommandExecutor::from_session(session),
        }
    }
    
    /// Execute a parsed function call
    pub fn execute(&mut self, call: ParserFunctionCall) -> Result<String, String> {
        let stdgeo_call: stdgeo::FunctionCall = call.into();
        
        match self.executor.execute(stdgeo_call) {
            Ok(ExecutionResult::Success(Some(message))) => Ok(message),
            Ok(ExecutionResult::Success(None)) => Ok(String::new()),
            Ok(ExecutionResult::Data(data)) => Ok(data),
            Err(ExecutionError::UnknownFunction { function }) => {
                Err(format!("Unknown function: {}", function))
            },
            Err(ExecutionError::InvalidArguments { function, message }) => {
                Err(format!("Invalid arguments for {}: {}", function, message))
            },
            Err(ExecutionError::IoError(io_err)) => {
                Err(format!("IO error: {}", io_err))
            },
            Err(ExecutionError::SessionError { message }) => {
                Err(format!("Session error: {}", message))
            },
        }
    }
    
    /// Execute multiple function calls in sequence
    pub fn execute_batch(&mut self, calls: Vec<ParserFunctionCall>) -> Vec<Result<String, String>> {
        calls.into_iter().map(|call| self.execute(call)).collect()
    }
    
    /// Get access to the underlying executor for advanced operations
    pub fn executor(&self) -> &CommandExecutor {
        &self.executor
    }
    
    /// Get mutable access to the underlying executor
    pub fn executor_mut(&mut self) -> &mut CommandExecutor {
        &mut self.executor
    }
    
    /// Save current session to a file
    pub fn save_session_to_file<P: AsRef<std::path::Path>>(&self, path: P) -> Result<(), String> {
        self.executor.save_session_to_file(path)
            .map_err(|e| format!("Failed to save session: {}", e))
    }
    
    /// Load session from a file
    pub fn load_session_from_file<P: AsRef<std::path::Path>>(&mut self, path: P) -> Result<usize, String> {
        self.executor.load_session_from_file(path)
            .map_err(|e| format!("Failed to load session: {}", e))
    }
    
    /// Get session statistics
    pub fn session_info(&self) -> String {
        let session = self.executor.session();
        format!("Session contains {} geometry objects", session.count())
    }
}

impl Default for CommandExecutorInterface {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{Parser, Context, ParseResult};
    
    #[test]
    fn test_execute_point() {
        let mut interface = CommandExecutorInterface::new();
        
        let call = ParserFunctionCall {
            function: "point".to_string(),
            args: ParserFunctionArgs::Point { x: 1.0, y: 2.0 },
        };
        
        let result = interface.execute(call);
        assert!(result.is_ok());
        assert!(result.unwrap().contains("Created point"));
        assert_eq!(interface.executor().session().count(), 1);
    }
    
    #[test]
    fn test_execute_from_parser() {
        let mut interface = CommandExecutorInterface::new();
        let parser = Parser::new();
        let context = Context::new();
        
        // Parse a command and execute it
        match parser.parse_command(&context, "point 3.0 4.0") {
            Ok(ParseResult::FunctionCall(call)) => {
                let result = interface.execute(call);
                assert!(result.is_ok());
                assert_eq!(interface.executor().session().count(), 1);
            },
            _ => panic!("Failed to parse command"),
        }
    }
    
    #[test]
    fn test_batch_execution() {
        let mut interface = CommandExecutorInterface::new();
        
        let calls = vec![
            ParserFunctionCall {
                function: "point".to_string(),
                args: ParserFunctionArgs::Point { x: 1.0, y: 2.0 },
            },
            ParserFunctionCall {
                function: "point".to_string(),
                args: ParserFunctionArgs::Point { x: 3.0, y: 4.0 },
            },
            ParserFunctionCall {
                function: "count".to_string(),
                args: ParserFunctionArgs::NoArgs,
            },
        ];
        
        let results = interface.execute_batch(calls);
        assert_eq!(results.len(), 3);
        assert!(results.iter().all(|r| r.is_ok()));
        assert_eq!(interface.executor().session().count(), 2);
    }
}