//! C FFI interface for stdgeo-parser
//!
//! This module provides C FFI bindings for the stdgeo-parser library to be used
//! from C++ GUI applications.

use crate::{Parser, Context, ParseResult, CommandExecutorInterface};
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};
use std::ptr;

/// Opaque handle to a parser instance
#[repr(C)]
pub struct ParserHandle {
    parser: Parser,
    context: Context,
    executor: CommandExecutorInterface,
}

/// Result of a command execution
#[repr(C)]
pub struct CommandResultC {
    /// 0 = success, 1 = error, 2 = no output
    pub status: c_int,
    /// Message (owned by the result, must be freed with free_string)
    pub message: *mut c_char,
}

/// Create a new parser instance
#[no_mangle]
pub extern "C" fn stdgeo_parser_create() -> *mut ParserHandle {
    let handle = Box::new(ParserHandle {
        parser: Parser::new(),
        context: Context::new(),
        executor: CommandExecutorInterface::new(),
    });
    Box::into_raw(handle)
}

/// Destroy a parser instance
#[no_mangle]
pub extern "C" fn stdgeo_parser_destroy(handle: *mut ParserHandle) {
    if !handle.is_null() {
        unsafe {
            let _ = Box::from_raw(handle);
        }
    }
}

/// Execute a command string
#[no_mangle]
pub extern "C" fn stdgeo_parser_execute(
    handle: *mut ParserHandle,
    command: *const c_char,
) -> CommandResultC {
    if handle.is_null() || command.is_null() {
        return CommandResultC {
            status: 1,
            message: create_c_string("Invalid handle or command"),
        };
    }
    
    let handle = unsafe { &mut *handle };
    
    let command_str = match unsafe { CStr::from_ptr(command) }.to_str() {
        Ok(s) => s,
        Err(_) => {
            return CommandResultC {
                status: 1,
                message: create_c_string("Invalid UTF-8 in command"),
            };
        }
    };
    
    match handle.parser.parse_command(&handle.context, command_str) {
        Ok(ParseResult::FunctionCall(call)) => {
            match handle.executor.execute(call) {
                Ok(message) => CommandResultC {
                    status: 0,
                    message: if message.is_empty() { 
                        ptr::null_mut() 
                    } else { 
                        create_c_string(&message) 
                    },
                },
                Err(e) => CommandResultC {
                    status: 1,
                    message: create_c_string(&e),
                },
            }
        },
        Ok(ParseResult::Help(help_lines)) => {
            let help_text = help_lines.join("\n");
            CommandResultC {
                status: 0,
                message: create_c_string(&help_text),
            }
        },
        Err(e) => CommandResultC {
            status: 1,
            message: create_c_string(&e.to_string()),
        },
    }
}

/// Get the number of geometries in the context
#[no_mangle]
pub extern "C" fn stdgeo_parser_geometry_count(handle: *mut ParserHandle) -> c_int {
    if handle.is_null() {
        return -1;
    }
    
    let handle = unsafe { &*handle };
    handle.executor.executor().session().count() as c_int
}

/// Clear all geometries from the context
#[no_mangle]
pub extern "C" fn stdgeo_parser_clear_context(handle: *mut ParserHandle) -> c_int {
    if handle.is_null() {
        return -1;
    }
    
    let handle = unsafe { &mut *handle };
    handle.executor.executor_mut().session_mut().clear();
    0
}

/// Free a string allocated by this library
#[no_mangle]
pub extern "C" fn stdgeo_parser_free_string(s: *mut c_char) {
    if !s.is_null() {
        unsafe {
            let _ = CString::from_raw(s);
        }
    }
}

/// Helper function to create a C string
fn create_c_string(s: &str) -> *mut c_char {
    match CString::new(s) {
        Ok(cstr) => cstr.into_raw(),
        Err(_) => ptr::null_mut(),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::ffi::CString;
    
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
        
        // Free the message
        stdgeo_parser_free_string(result.message);
        
        // Check geometry count
        let count = stdgeo_parser_geometry_count(handle);
        assert_eq!(count, 1);
        
        // Clear context
        let clear_result = stdgeo_parser_clear_context(handle);
        assert_eq!(clear_result, 0);
        
        let count = stdgeo_parser_geometry_count(handle);
        assert_eq!(count, 0);
        
        // Destroy parser
        stdgeo_parser_destroy(handle);
    }
}
