//! Command parsing and function call definitions

use crate::{Context, ParserError, ParserResult};
use std::path::PathBuf;

/// Result of command parsing
#[derive(Debug, Clone)]
pub enum ParseResult {
    /// Successfully parsed a function call
    FunctionCall(FunctionCall),
    /// Help request
    Help(Vec<String>),
}

/// Represents a parsed function call with arguments
#[derive(Debug, Clone)]
pub struct FunctionCall {
    /// The function name
    pub function: String,
    /// Parsed and validated arguments
    pub args: FunctionArgs,
}

/// Function arguments for different command types
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

/// Trait for command parsers
/// 
/// This allows extending the parser with additional command types
pub trait CommandParser {
    /// Try to parse a command into a function call
    /// 
    /// Returns None if the command is not recognized by this parser
    fn try_parse(&self, context: &Context, command: &str, args: &[&str]) -> Option<ParserResult<ParseResult>>;
    
    /// Get help text for commands handled by this parser
    fn get_help(&self) -> Vec<String>;
}

/// Core command parser for geometry operations
#[derive(Debug)]
pub struct CoreCommandParser;

impl CoreCommandParser {
    pub fn new() -> Self {
        Self
    }
}

impl CommandParser for CoreCommandParser {
    fn try_parse(&self, context: &Context, command: &str, args: &[&str]) -> Option<ParserResult<ParseResult>> {
        match command {
            "point" => Some(self.parse_point(context, args)),
            "line" => Some(self.parse_line(context, args)),
            "list" => Some(self.parse_list(context, args)),
            "clear" => Some(self.parse_clear(context, args)),
            "count" => Some(self.parse_count(context, args)),
            "translate" => Some(self.parse_translate(context, args)),
            "rotate" => Some(self.parse_rotate(context, args)),
            "load" => Some(self.parse_load(context, args)),
            "save" => Some(self.parse_save(context, args)),
            "help" => Some(self.parse_help(context, args)),
            _ => None,
        }
    }
    
    fn get_help(&self) -> Vec<String> {
        vec![
            "Core geometry commands:".to_string(),
            "  point <x> <y>                    - Create a point".to_string(),
            "  line <x1> <y1> <x2> <y2>        - Create a line segment".to_string(),
            "  list                             - List all geometry objects".to_string(),
            "  count                            - Show number of objects".to_string(),
            "  clear                            - Clear all objects".to_string(),
            "  translate <dx> <dy>              - Translate all objects".to_string(),
            "  rotate <angle> [<center_x> <center_y>] [--degrees] - Rotate all objects".to_string(),
            "  load <filename> [--simple]      - Load objects from file".to_string(),
            "  save <filename> [--simple]      - Save objects to file".to_string(),
            "  help                             - Show available commands".to_string(),
        ]
    }
}

impl CoreCommandParser {
    fn parse_point(&self, _context: &Context, args: &[&str]) -> ParserResult<ParseResult> {
        if args.len() != 2 {
            return Err(ParserError::InvalidSyntax {
                message: "point command requires 2 arguments: <x> <y>".to_string()
            });
        }
        
        let x: f64 = args[0].parse().map_err(|_| ParserError::InvalidArgument {
            message: format!("Invalid x coordinate: {}", args[0])
        })?;
        
        let y: f64 = args[1].parse().map_err(|_| ParserError::InvalidArgument {
            message: format!("Invalid y coordinate: {}", args[1])
        })?;
        
        Ok(ParseResult::FunctionCall(FunctionCall {
            function: "point".to_string(),
            args: FunctionArgs::Point { x, y },
        }))
    }
    
    fn parse_line(&self, _context: &Context, args: &[&str]) -> ParserResult<ParseResult> {
        if args.len() != 4 {
            return Err(ParserError::InvalidSyntax {
                message: "line command requires 4 arguments: <x1> <y1> <x2> <y2>".to_string()
            });
        }
        
        let x1: f64 = args[0].parse().map_err(|_| ParserError::InvalidArgument {
            message: format!("Invalid x1 coordinate: {}", args[0])
        })?;
        
        let y1: f64 = args[1].parse().map_err(|_| ParserError::InvalidArgument {
            message: format!("Invalid y1 coordinate: {}", args[1])
        })?;
        
        let x2: f64 = args[2].parse().map_err(|_| ParserError::InvalidArgument {
            message: format!("Invalid x2 coordinate: {}", args[2])
        })?;
        
        let y2: f64 = args[3].parse().map_err(|_| ParserError::InvalidArgument {
            message: format!("Invalid y2 coordinate: {}", args[3])
        })?;
        
        Ok(ParseResult::FunctionCall(FunctionCall {
            function: "line".to_string(),
            args: FunctionArgs::Line { x1, y1, x2, y2 },
        }))
    }
    
    fn parse_list(&self, _context: &Context, _args: &[&str]) -> ParserResult<ParseResult> {
        Ok(ParseResult::FunctionCall(FunctionCall {
            function: "list".to_string(),
            args: FunctionArgs::NoArgs,
        }))
    }
    
    fn parse_clear(&self, _context: &Context, _args: &[&str]) -> ParserResult<ParseResult> {
        Ok(ParseResult::FunctionCall(FunctionCall {
            function: "clear".to_string(),
            args: FunctionArgs::NoArgs,
        }))
    }
    
    fn parse_count(&self, _context: &Context, _args: &[&str]) -> ParserResult<ParseResult> {
        Ok(ParseResult::FunctionCall(FunctionCall {
            function: "count".to_string(),
            args: FunctionArgs::NoArgs,
        }))
    }
    
    fn parse_translate(&self, _context: &Context, args: &[&str]) -> ParserResult<ParseResult> {
        if args.len() != 2 {
            return Err(ParserError::InvalidSyntax {
                message: "translate command requires 2 arguments: <dx> <dy>".to_string()
            });
        }
        
        let dx: f64 = args[0].parse().map_err(|_| ParserError::InvalidArgument {
            message: format!("Invalid dx displacement: {}", args[0])
        })?;
        
        let dy: f64 = args[1].parse().map_err(|_| ParserError::InvalidArgument {
            message: format!("Invalid dy displacement: {}", args[1])
        })?;
        
        Ok(ParseResult::FunctionCall(FunctionCall {
            function: "translate".to_string(),
            args: FunctionArgs::Translate { dx, dy },
        }))
    }
    
    fn parse_rotate(&self, _context: &Context, args: &[&str]) -> ParserResult<ParseResult> {
        if args.is_empty() {
            return Err(ParserError::InvalidSyntax {
                message: "rotate command requires at least 1 argument: <angle> [<center_x> <center_y>] [--degrees]".to_string()
            });
        }
        
        let angle: f64 = args[0].parse().map_err(|_| ParserError::InvalidArgument {
            message: format!("Invalid angle: {}", args[0])
        })?;
        
        let mut center_x = 0.0;
        let mut center_y = 0.0;
        let mut use_degrees = false;
        
        let mut i = 1;
        while i < args.len() {
            match args[i] {
                "--degrees" => use_degrees = true,
                arg if i + 1 < args.len() => {
                    center_x = arg.parse().map_err(|_| ParserError::InvalidArgument {
                        message: format!("Invalid center x coordinate: {}", arg)
                    })?;
                    center_y = args[i + 1].parse().map_err(|_| ParserError::InvalidArgument {
                        message: format!("Invalid center y coordinate: {}", args[i + 1])
                    })?;
                    i += 1;
                }
                _ => return Err(ParserError::InvalidSyntax {
                    message: "Invalid rotate command syntax".to_string()
                }),
            }
            i += 1;
        }
        
        Ok(ParseResult::FunctionCall(FunctionCall {
            function: "rotate".to_string(),
            args: FunctionArgs::Rotate { angle, center_x, center_y, use_degrees },
        }))
    }
    
    fn parse_load(&self, context: &Context, args: &[&str]) -> ParserResult<ParseResult> {
        if args.is_empty() {
            return Err(ParserError::MissingArguments {
                message: "load command requires a filename".to_string()
            });
        }
        
        let filename = args[0];
        let simple_format = args.contains(&"--simple");
        
        let path = context.resolve_path(&PathBuf::from(filename));
        
        Ok(ParseResult::FunctionCall(FunctionCall {
            function: "load".to_string(),
            args: FunctionArgs::Load { path, simple_format },
        }))
    }
    
    fn parse_save(&self, context: &Context, args: &[&str]) -> ParserResult<ParseResult> {
        if args.is_empty() {
            return Err(ParserError::MissingArguments {
                message: "save command requires a filename".to_string()
            });
        }
        
        let filename = args[0];
        let simple_format = args.contains(&"--simple");
        
        let path = context.resolve_path(&PathBuf::from(filename));
        
        Ok(ParseResult::FunctionCall(FunctionCall {
            function: "save".to_string(),
            args: FunctionArgs::Save { path, simple_format },
        }))
    }
    
    fn parse_help(&self, _context: &Context, _args: &[&str]) -> ParserResult<ParseResult> {
        let help_text = self.get_help();
        Ok(ParseResult::Help(help_text))
    }
}