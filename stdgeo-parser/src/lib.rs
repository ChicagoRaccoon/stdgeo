//! # StdGeo Parser Library
//!
//! A command line argument parser library for stdgeo geometry operations.
//! This library provides a unified command parsing interface that parses
//! commands into function calls that can be executed by applications.
//!
//! ## Architecture
//!
//! The parser is designed to be pure argument parsing:
//! - Parses command strings into validated function calls
//! - Does not manage geometry objects or execute functions
//! - Applications handle function execution and memory management
//! - The parser is extensible with custom command parsers
//!
//! ## Usage
//!
//! ```rust
//! use stdgeo_parser::{Parser, Context, ParseResult, FunctionCall};
//!
//! let context = Context::new();
//! let parser = Parser::new();
//! 
//! match parser.parse_command(&context, "point 1.0 2.0") {
//!     Ok(ParseResult::FunctionCall(call)) => {
//!         // Application executes the function call
//!         println!("Function: {}", call.function);
//!     },
//!     Ok(ParseResult::Help(help)) => {
//!         for line in help {
//!             println!("{}", line);
//!         }
//!     },
//!     Err(e) => eprintln!("Parse error: {}", e),
//! }
//! ```

pub mod commands;
pub mod context;
pub mod error;
pub mod parser;
pub mod ffi;
pub mod executor;

pub use commands::*;
pub use context::*;
pub use error::*;
pub use parser::*;
pub use executor::*;