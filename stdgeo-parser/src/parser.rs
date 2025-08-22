//! Main parser implementation

use crate::{CommandParser, ParseResult, Context, CoreCommandParser, ParserError, ParserResult};

/// Main parser that coordinates command parsing
/// 
/// The parser maintains a list of command parsers and tries each one
/// in order until a command is recognized and parsed into a function call.
pub struct Parser {
    parsers: Vec<Box<dyn CommandParser>>,
}

impl Parser {
    /// Create a new parser with core commands only
    pub fn new() -> Self {
        Self {
            parsers: vec![Box::new(CoreCommandParser::new())],
        }
    }
    
    /// Create a new parser with custom parsers
    pub fn with_parsers(parsers: Vec<Box<dyn CommandParser>>) -> Self {
        Self { parsers }
    }
    
    /// Add a command parser to the parser
    pub fn add_parser(&mut self, parser: Box<dyn CommandParser>) {
        self.parsers.push(parser);
    }
    
    /// Parse a command string into a function call
    pub fn parse_command(&self, context: &Context, input: &str) -> ParserResult<ParseResult> {
        let input = input.trim();
        if input.is_empty() {
            return Err(ParserError::InvalidSyntax {
                message: "Empty command".to_string()
            });
        }
        
        let parts: Vec<&str> = input.split_whitespace().collect();
        if parts.is_empty() {
            return Err(ParserError::InvalidSyntax {
                message: "Empty command".to_string()
            });
        }
        
        let command = parts[0];
        let args = &parts[1..];
        
        // Try each parser in order
        for parser in &self.parsers {
            if let Some(result) = parser.try_parse(context, command, args) {
                return result;
            }
        }
        
        // No parser recognized the command
        Err(ParserError::UnknownCommand {
            command: command.to_string()
        })
    }
    
    /// Get help text from all registered parsers
    pub fn get_all_help(&self) -> Vec<String> {
        let mut help = Vec::new();
        for parser in &self.parsers {
            help.extend(parser.get_help());
        }
        help
    }
    
    /// Validate a command (check if it can be parsed)
    pub fn validate_command(&self, context: &Context, input: &str) -> ParserResult<()> {
        match self.parse_command(context, input) {
            Ok(_) => Ok(()),
            Err(e) => Err(e),
        }
    }
}

impl Default for Parser {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{FunctionCall, FunctionArgs};
    
    #[test]
    fn test_basic_command_parsing() {
        let parser = Parser::new();
        let context = Context::new();
        
        // Test point parsing
        let result = parser.parse_command(&context, "point 1.0 2.0");
        assert!(result.is_ok());
        if let Ok(ParseResult::FunctionCall(call)) = result {
            assert_eq!(call.function, "point");
            if let FunctionArgs::Point { x, y } = call.args {
                assert_eq!(x, 1.0);
                assert_eq!(y, 2.0);
            } else {
                panic!("Expected Point args");
            }
        }
        
        // Test line parsing
        let result = parser.parse_command(&context, "line 0.0 0.0 3.0 4.0");
        assert!(result.is_ok());
        if let Ok(ParseResult::FunctionCall(call)) = result {
            assert_eq!(call.function, "line");
            if let FunctionArgs::Line { x1, y1, x2, y2 } = call.args {
                assert_eq!(x1, 0.0);
                assert_eq!(y1, 0.0);
                assert_eq!(x2, 3.0);
                assert_eq!(y2, 4.0);
            } else {
                panic!("Expected Line args");
            }
        }
    }
    
    #[test]
    fn test_unknown_command() {
        let parser = Parser::new();
        let context = Context::new();
        
        let result = parser.parse_command(&context, "unknown_command");
        assert!(result.is_err());
        
        if let Err(ParserError::UnknownCommand { command }) = result {
            assert_eq!(command, "unknown_command");
        } else {
            panic!("Expected UnknownCommand error");
        }
    }
    
    #[test]
    fn test_invalid_syntax() {
        let parser = Parser::new();
        let context = Context::new();
        
        // Point with wrong number of arguments
        let result = parser.parse_command(&context, "point 1.0");
        assert!(result.is_err());
        
        // Invalid number format
        let result = parser.parse_command(&context, "point abc def");
        assert!(result.is_err());
    }
    
    #[test]
    fn test_empty_input() {
        let parser = Parser::new();
        let context = Context::new();
        
        let result = parser.parse_command(&context, "");
        assert!(result.is_err());
    }
}