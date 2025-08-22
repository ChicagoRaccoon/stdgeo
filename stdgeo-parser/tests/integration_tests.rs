//! Integration tests for stdgeo-parser
//!
//! These tests are currently disabled since the parser API has been changed
//! to be a pure argument parser that doesn't execute functions.

/*
use stdgeo_parser::{Parser, Context, CommandResult};
use tempfile::TempDir;

/// Test basic command parsing and execution
#[test]
fn test_basic_commands() {
    let parser = Parser::new();
    let mut context = Context::new();
    
    // Test point creation
    let result = parser.parse_and_execute(&mut context, "point 1.0 2.0");
    assert!(result.is_ok());
    match result.unwrap() {
        CommandResult::Success(msg) => {
            assert!(msg.contains("Created point"));
            assert!(msg.contains("1") && msg.contains("2"));
        }
        _ => panic!("Expected success result"),
    }
    assert_eq!(context.geometry_count(), 1);
    
    // Test line creation
    let result = parser.parse_and_execute(&mut context, "line 0.0 0.0 3.0 4.0");
    assert!(result.is_ok());
    match result.unwrap() {
        CommandResult::Success(msg) => {
            assert!(msg.contains("Created line"));
        }
        _ => panic!("Expected success result"),
    }
    assert_eq!(context.geometry_count(), 2);
    
    // Test count command
    let result = parser.parse_and_execute(&mut context, "count");
    assert!(result.is_ok());
    match result.unwrap() {
        CommandResult::Success(msg) => {
            assert!(msg.contains("2"));
        }
        _ => panic!("Expected success result"),
    }
}

/// Test transformations
#[test]
fn test_transformations() {
    let parser = Parser::new();
    let mut context = Context::new();
    
    // Create some geometry
    parser.parse_and_execute(&mut context, "point 1.0 1.0").unwrap();
    parser.parse_and_execute(&mut context, "line 0.0 0.0 1.0 1.0").unwrap();
    assert_eq!(context.geometry_count(), 2);
    
    // Test translation
    let result = parser.parse_and_execute(&mut context, "translate 2.0 3.0");
    assert!(result.is_ok());
    match result.unwrap() {
        CommandResult::Success(msg) => {
            assert!(msg.contains("Translated 2 objects"));
            assert!(msg.contains("2") && msg.contains("3"));
        }
        _ => panic!("Expected success result"),
    }
    
    // Test rotation
    let result = parser.parse_and_execute(&mut context, "rotate 1.57 0.0 0.0 --degrees");
    assert!(result.is_ok());
    match result.unwrap() {
        CommandResult::Success(msg) => {
            assert!(msg.contains("Rotated 2 objects"));
            assert!(msg.contains("degrees"));
        }
        _ => panic!("Expected success result"),
    }
}

/// Test file operations
#[test]
fn test_file_operations() {
    let parser = Parser::new();
    let mut context = Context::new();
    let temp_dir = TempDir::new().unwrap();
    
    // Create geometry
    parser.parse_and_execute(&mut context, "point 5.0 6.0").unwrap();
    parser.parse_and_execute(&mut context, "line 1.0 2.0 3.0 4.0").unwrap();
    
    // Test save to JSON
    let json_file = temp_dir.path().join("test.json");
    let save_cmd = format!("save {}", json_file.display());
    let result = parser.parse_and_execute(&mut context, &save_cmd);
    assert!(result.is_ok());
    assert!(json_file.exists());
    
    // Test save to simple format
    let simple_file = temp_dir.path().join("test.txt");
    let save_cmd = format!("save {} --simple", simple_file.display());
    let result = parser.parse_and_execute(&mut context, &save_cmd);
    assert!(result.is_ok());
    assert!(simple_file.exists());
    
    // Clear context and test load
    parser.parse_and_execute(&mut context, "clear").unwrap();
    assert_eq!(context.geometry_count(), 0);
    
    let load_cmd = format!("load {}", json_file.display());
    let result = parser.parse_and_execute(&mut context, &load_cmd);
    assert!(result.is_ok());
    assert_eq!(context.geometry_count(), 2);
    
    // Test load simple format
    parser.parse_and_execute(&mut context, "clear").unwrap();
    let load_cmd = format!("load {} --simple", simple_file.display());
    let result = parser.parse_and_execute(&mut context, &load_cmd);
    assert!(result.is_ok());
    assert_eq!(context.geometry_count(), 2);
}

/// Test error cases
#[test]
fn test_error_cases() {
    let parser = Parser::new();
    let mut context = Context::new();
    
    // Test unknown command
    let result = parser.parse_and_execute(&mut context, "unknown_command");
    assert!(result.is_err());
    
    // Test invalid syntax
    let result = parser.parse_and_execute(&mut context, "point 1.0");
    assert!(result.is_err());
    
    let result = parser.parse_and_execute(&mut context, "line 1.0 2.0");
    assert!(result.is_err());
    
    // Test invalid number format
    let result = parser.parse_and_execute(&mut context, "point abc def");
    assert!(result.is_err());
    
    // Test operations on empty context
    let result = parser.parse_and_execute(&mut context, "translate 1.0 2.0");
    assert!(result.is_ok()); // Should succeed but report no objects
    match result.unwrap() {
        CommandResult::Success(msg) => {
            assert!(msg.contains("No objects"));
        }
        _ => panic!("Expected success result"),
    }
}

/// Test command validation
#[test]
fn test_command_validation() {
    let parser = Parser::new();
    
    // Test valid commands (validation only checks if command exists, not syntax)
    assert!(parser.validate_command("point 1.0 2.0").is_ok());
    assert!(parser.validate_command("line 0.0 0.0 1.0 1.0").is_ok());
    assert!(parser.validate_command("help").is_ok());
    assert!(parser.validate_command("").is_ok()); // Empty is valid
    
    // Test invalid commands (command doesn't exist)
    assert!(parser.validate_command("unknown_command").is_err());
    
    // Note: validate_command only checks if the command exists, not syntax
    // So "point 1.0" would pass validation but fail execution
    assert!(parser.validate_command("point 1.0").is_ok()); // Command exists
}

/// Test help functionality
#[test]
fn test_help() {
    let parser = Parser::new();
    let mut context = Context::new();
    
    let result = parser.parse_and_execute(&mut context, "help");
    assert!(result.is_ok());
    
    match result.unwrap() {
        CommandResult::Success(msg) => {
            assert!(msg.contains("point"));
            assert!(msg.contains("line"));
            assert!(msg.contains("translate"));
            assert!(msg.contains("rotate"));
            assert!(msg.contains("load"));
            assert!(msg.contains("save"));
        }
        _ => panic!("Expected success result"),
    }
    
    // Test get_all_help method
    let help_lines = parser.get_all_help();
    assert!(!help_lines.is_empty());
    assert!(help_lines.iter().any(|line| line.contains("point")));
}

/// Test empty input handling
#[test]
fn test_empty_input() {
    let parser = Parser::new();
    let mut context = Context::new();
    
    // Test various empty inputs
    let inputs = ["", "   ", "\t", "\n"];
    
    for input in inputs.iter() {
        let result = parser.parse_and_execute(&mut context, input);
        assert!(result.is_ok());
        match result.unwrap() {
            CommandResult::NoOutput => {}, // Expected
            _ => panic!("Expected NoOutput result for input: '{}'", input),
        }
    }
}

/// Test geometry list functionality
#[test]
fn test_geometry_list() {
    let parser = Parser::new();
    let mut context = Context::new();
    
    // Test list with no geometries
    let result = parser.parse_and_execute(&mut context, "list");
    assert!(result.is_ok());
    match result.unwrap() {
        CommandResult::Success(msg) => {
            assert!(msg.contains("No geometry objects"));
        }
        _ => panic!("Expected success result"),
    }
    
    // Add geometries and test list
    parser.parse_and_execute(&mut context, "point 1.0 2.0").unwrap();
    parser.parse_and_execute(&mut context, "line 0.0 0.0 3.0 4.0").unwrap();
    
    let result = parser.parse_and_execute(&mut context, "list");
    assert!(result.is_ok());
    match result.unwrap() {
        CommandResult::Success(msg) => {
            assert!(msg.contains("2 geometry objects"));
            assert!(msg.contains("Point"));
            assert!(msg.contains("Line"));
        }
        _ => panic!("Expected success result"),
    }
}

/// Test context working directory functionality
#[test]
fn test_working_directory() {
    let temp_dir = TempDir::new().unwrap();
    let mut context = Context::new();
    
    // Set working directory
    context.set_working_directory(temp_dir.path().to_path_buf());
    
    // Test that relative paths are resolved correctly
    let relative_path = std::path::PathBuf::from("test.json");
    let resolved = context.resolve_path(&relative_path);
    assert_eq!(resolved, temp_dir.path().join("test.json"));
    
    // Test that absolute paths are unchanged
    let absolute_path = std::path::PathBuf::from("/tmp/test.json");
    let resolved = context.resolve_path(&absolute_path);
    assert_eq!(resolved, absolute_path);
}
*/