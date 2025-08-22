//! FFI integration tests for stdgeo-parser
//!
//! These tests are currently disabled since the FFI interface has been disabled.

/*
use stdgeo_parser::ffi::*;
use std::ffi::CString;
use std::ptr;

/// Test basic FFI operations
#[test]
fn test_ffi_basic_operations() {
    // Create parser
    let handle = stdgeo_parser_create();
    assert!(!handle.is_null());
    
    // Test point creation
    let command = CString::new("point 1.0 2.0").unwrap();
    let result = stdgeo_parser_execute(handle, command.as_ptr());
    assert_eq!(result.status, 0);
    assert!(!result.message.is_null());
    
    // Verify the message content
    let message = unsafe { std::ffi::CStr::from_ptr(result.message) };
    let message_str = message.to_str().unwrap();
    assert!(message_str.contains("Created point"));
    
    // Free the message
    stdgeo_parser_free_string(result.message);
    
    // Check geometry count
    let count = stdgeo_parser_geometry_count(handle);
    assert_eq!(count, 1);
    
    // Test line creation
    let command = CString::new("line 0.0 0.0 3.0 4.0").unwrap();
    let result = stdgeo_parser_execute(handle, command.as_ptr());
    assert_eq!(result.status, 0);
    assert!(!result.message.is_null());
    stdgeo_parser_free_string(result.message);
    
    let count = stdgeo_parser_geometry_count(handle);
    assert_eq!(count, 2);
    
    // Clear context
    let clear_result = stdgeo_parser_clear_context(handle);
    assert_eq!(clear_result, 0);
    
    let count = stdgeo_parser_geometry_count(handle);
    assert_eq!(count, 0);
    
    // Destroy parser
    stdgeo_parser_destroy(handle);
}

/// Test FFI error handling
#[test]
fn test_ffi_error_handling() {
    let handle = stdgeo_parser_create();
    assert!(!handle.is_null());
    
    // Test unknown command
    let command = CString::new("unknown_command").unwrap();
    let result = stdgeo_parser_execute(handle, command.as_ptr());
    assert_eq!(result.status, 1); // Error status
    assert!(!result.message.is_null());
    
    let message = unsafe { std::ffi::CStr::from_ptr(result.message) };
    let message_str = message.to_str().unwrap();
    assert!(message_str.contains("Unknown command"));
    stdgeo_parser_free_string(result.message);
    
    // Test invalid syntax
    let command = CString::new("point 1.0").unwrap();
    let result = stdgeo_parser_execute(handle, command.as_ptr());
    assert_eq!(result.status, 1); // Error status
    assert!(!result.message.is_null());
    stdgeo_parser_free_string(result.message);
    
    stdgeo_parser_destroy(handle);
}

/// Test FFI with no output commands
#[test]
fn test_ffi_no_output() {
    let handle = stdgeo_parser_create();
    assert!(!handle.is_null());
    
    // Test empty command
    let command = CString::new("").unwrap();
    let result = stdgeo_parser_execute(handle, command.as_ptr());
    assert_eq!(result.status, 2); // No output status
    assert!(result.message.is_null());
    
    stdgeo_parser_destroy(handle);
}

/// Test FFI null pointer handling
#[test]
fn test_ffi_null_pointers() {
    // Test null handle
    let result = stdgeo_parser_execute(ptr::null_mut(), ptr::null());
    assert_eq!(result.status, 1);
    if !result.message.is_null() {
        stdgeo_parser_free_string(result.message);
    }
    
    // Test null command
    let handle = stdgeo_parser_create();
    let result = stdgeo_parser_execute(handle, ptr::null());
    assert_eq!(result.status, 1);
    if !result.message.is_null() {
        stdgeo_parser_free_string(result.message);
    }
    
    // Test geometry count with null handle
    let count = stdgeo_parser_geometry_count(ptr::null_mut());
    assert_eq!(count, -1);
    
    // Test clear with null handle
    let clear_result = stdgeo_parser_clear_context(ptr::null_mut());
    assert_eq!(clear_result, -1);
    
    stdgeo_parser_destroy(handle);
    
    // Test destroy with null (should be safe)
    stdgeo_parser_destroy(ptr::null_mut());
}

/// Test FFI with complex commands
#[test]
fn test_ffi_complex_commands() {
    let handle = stdgeo_parser_create();
    assert!(!handle.is_null());
    
    // Create some geometry
    let commands = [
        "point 1.0 2.0",
        "line 0.0 0.0 5.0 5.0",
        "point 3.0 4.0",
    ];
    
    for cmd in commands.iter() {
        let command = CString::new(*cmd).unwrap();
        let result = stdgeo_parser_execute(handle, command.as_ptr());
        assert_eq!(result.status, 0);
        stdgeo_parser_free_string(result.message);
    }
    
    let count = stdgeo_parser_geometry_count(handle);
    assert_eq!(count, 3);
    
    // Test transformation
    let command = CString::new("translate 2.0 3.0").unwrap();
    let result = stdgeo_parser_execute(handle, command.as_ptr());
    assert_eq!(result.status, 0);
    assert!(!result.message.is_null());
    
    let message = unsafe { std::ffi::CStr::from_ptr(result.message) };
    let message_str = message.to_str().unwrap();
    assert!(message_str.contains("Translated 3 objects"));
    stdgeo_parser_free_string(result.message);
    
    // Test list command
    let command = CString::new("list").unwrap();
    let result = stdgeo_parser_execute(handle, command.as_ptr());
    assert_eq!(result.status, 0);
    assert!(!result.message.is_null());
    
    let message = unsafe { std::ffi::CStr::from_ptr(result.message) };
    let message_str = message.to_str().unwrap();
    assert!(message_str.contains("3 geometry objects"));
    stdgeo_parser_free_string(result.message);
    
    stdgeo_parser_destroy(handle);
}

/// Test FFI with Unicode characters
#[test]
fn test_ffi_unicode() {
    let handle = stdgeo_parser_create();
    assert!(!handle.is_null());
    
    // Test command with spaces and special characters
    let command = CString::new("point 1.5 2.7").unwrap();
    let result = stdgeo_parser_execute(handle, command.as_ptr());
    assert_eq!(result.status, 0);
    stdgeo_parser_free_string(result.message);
    
    stdgeo_parser_destroy(handle);
}

/// Test FFI memory management
#[test]
fn test_ffi_memory_management() {
    // Create and destroy multiple parsers
    for _ in 0..10 {
        let handle = stdgeo_parser_create();
        assert!(!handle.is_null());
        
        // Execute a few commands
        let command = CString::new("point 1.0 2.0").unwrap();
        let result = stdgeo_parser_execute(handle, command.as_ptr());
        assert_eq!(result.status, 0);
        stdgeo_parser_free_string(result.message);
        
        stdgeo_parser_destroy(handle);
    }
}

/// Benchmark-style test for FFI performance
#[test]
fn test_ffi_performance() {
    let handle = stdgeo_parser_create();
    assert!(!handle.is_null());
    
    let start = std::time::Instant::now();
    
    // Execute many commands
    for i in 0..100 {
        let cmd = format!("point {} {}", i as f64, (i + 1) as f64);
        let command = CString::new(cmd).unwrap();
        let result = stdgeo_parser_execute(handle, command.as_ptr());
        assert_eq!(result.status, 0);
        stdgeo_parser_free_string(result.message);
    }
    
    let elapsed = start.elapsed();
    println!("FFI performance test: 100 commands in {:?}", elapsed);
    
    // Should have 100 points
    let count = stdgeo_parser_geometry_count(handle);
    assert_eq!(count, 100);
    
    stdgeo_parser_destroy(handle);
}
*/