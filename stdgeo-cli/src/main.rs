//! # StdGeo CLI Application
//!
//! A minimal command-line interface that acts as a wrapper around stdgeo-parser.
//! The CLI is stateless and delegates all parsing and execution to stdgeo-parser.
//!
//! This CLI is truly minimal:
//! - Accepts command line arguments as a single string
//! - Uses stdgeo-parser to parse and execute commands
//! - Does not manage any geometry state itself
//! - Supports interactive session mode through stdgeo-parser

use clap::Parser;
use stdgeo_parser::{Parser as StdgeoParser, Context as ParserContext, ParseResult, CommandExecutorInterface};
use anyhow::{Result, Context as AnyhowContext};
use rustyline::{DefaultEditor, Result as RustyResult};

/// Main CLI structure using clap's derive API.
#[derive(Parser)]
#[command(name = "stdgeo")]
#[command(about = "A CLI tool for geometric operations")]
#[command(version = "0.1.0")]
struct Cli {
    /// The command to execute (if not specified, starts interactive session)
    #[arg(trailing_var_arg = true, allow_hyphen_values = true)]
    command: Vec<String>,
    
    /// Start an interactive session
    #[arg(short, long, default_value = "false")]
    interactive: bool,
}

/// Interactive session for geometry operations
/// 
/// Maintains state by delegating to stdgeo-parser's CommandExecutorInterface.
/// The CLI itself remains stateless.
struct Session {
    editor: DefaultEditor,
    parser: StdgeoParser,
    parser_context: ParserContext,
    executor: CommandExecutorInterface,
}

impl Session {
    fn new() -> RustyResult<Self> {
        let editor = DefaultEditor::new()?;
        let parser = StdgeoParser::new();
        let parser_context = ParserContext::new();
        let executor = CommandExecutorInterface::new();
        
        Ok(Session {
            editor,
            parser,
            parser_context,
            executor,
        })
    }
    
    fn run(&mut self) -> Result<()> {
        println!("StdGeo Interactive Session");
        println!("Type 'help' for available commands, 'quit' to exit");
        
        loop {
            let readline = self.editor.readline("stdgeo> ");
            match readline {
                Ok(line) => {
                    if line.trim().is_empty() {
                        continue;
                    }
                    
                    self.editor.add_history_entry(&line).ok();
                    
                    // Handle built-in session commands
                    if line.trim() == "quit" || line.trim() == "exit" {
                        break;
                    }
                    
                    // Parse and execute command
                    match self.parser.parse_command(&self.parser_context, &line) {
                        Ok(ParseResult::FunctionCall(call)) => {
                            match self.executor.execute(call) {
                                Ok(message) if !message.is_empty() => println!("{}", message),
                                Ok(_) => {},
                                Err(e) => println!("Error: {}", e),
                            }
                        },
                        Ok(ParseResult::Help(help)) => {
                            for line in help {
                                println!("{}", line);
                            }
                        },
                        Err(e) => println!("Error: {}", e),
                    }
                }
                Err(rustyline::error::ReadlineError::Interrupted) => {
                    println!("Interrupted");
                    break;
                }
                Err(rustyline::error::ReadlineError::Eof) => {
                    println!("EOF");
                    break;
                }
                Err(err) => {
                    println!("Error: {:?}", err);
                    break;
                }
            }
        }
        
        // No cleanup needed - executor manages its own state
        
        Ok(())
    }
}


/// Execute a command string directly (stateless, for single commands)
fn execute_command_string(command_str: &str) -> Result<()> {
    let parser = StdgeoParser::new();
    let parser_context = ParserContext::new();
    let mut executor = CommandExecutorInterface::new();
    
    match parser.parse_command(&parser_context, command_str) {
        Ok(ParseResult::FunctionCall(call)) => {
            // For stateless execution, we need to handle different command types differently
            match call.function.as_str() {
                "point" | "line" => {
                    // These commands create geometry but don't persist in stateless mode
                    match executor.execute(call) {
                        Ok(message) if !message.is_empty() => println!("{}", message),
                        Ok(_) => {},
                        Err(e) => return Err(anyhow::anyhow!("Execution error: {}", e)),
                    }
                },
                "list" | "count" | "clear" => {
                    // These require state, so they don't make sense in stateless mode
                    println!("Command '{}' requires interactive mode (use --interactive)", call.function);
                },
                "translate" | "rotate" => {
                    // These require existing geometry state
                    println!("Command '{}' requires interactive mode or file input/output (use --interactive)", call.function);
                },
                "load" | "save" => {
                    // These work with files directly
                    match executor.execute(call) {
                        Ok(message) if !message.is_empty() => println!("{}", message),
                        Ok(_) => {},
                        Err(e) => return Err(anyhow::anyhow!("Execution error: {}", e)),
                    }
                },
                _ => {
                    return Err(anyhow::anyhow!("Unknown command: {}", call.function));
                }
            }
        },
        Ok(ParseResult::Help(help)) => {
            for line in help {
                println!("{}", line);
            }
        },
        Err(e) => return Err(e.into()),
    }
    
    Ok(())
}


/// Main entry point for the CLI application.
fn main() -> Result<()> {
    let cli = Cli::parse();
    
    if cli.interactive || cli.command.is_empty() {
        // Start interactive session
        let mut session = Session::new()
            .context("Failed to initialize interactive session")?;
        session.run()?;
    } else {
        // Execute single command
        let command_str = cli.command.join(" ");
        execute_command_string(&command_str)?;
    }
    
    Ok(())
}