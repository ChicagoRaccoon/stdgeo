use stdgeo::geo::CommandProcessor;
use std::env;
use std::process;

fn main() {
    let args: Vec<String> = env::args().collect();
    
    // Parse command line arguments
    let mut interactive = false;
    let mut verbose = false;
    let mut single_command = None;
    
    let mut i = 1;
    while i < args.len() {
        match args[i].as_str() {
            "-i" | "--interactive" => interactive = true,
            "-v" | "--verbose" => verbose = true,
            "-c" | "--command" => {
                if i + 1 < args.len() {
                    single_command = Some(args[i + 1].clone());
                    i += 1;
                } else {
                    eprintln!("Error: --command requires an argument");
                    print_usage();
                    process::exit(1);
                }
            }
            "-h" | "--help" => {
                print_usage();
                process::exit(0);
            }
            "--version" => {
                println!("stdgeo {}", env!("CARGO_PKG_VERSION"));
                process::exit(0);
            }
            _ => {
                eprintln!("Error: Unknown argument '{}'", args[i]);
                print_usage();
                process::exit(1);
            }
        }
        i += 1;
    }
    
    let mut processor = CommandProcessor::new(verbose);
    
    // Execute single command if provided
    if let Some(cmd) = single_command {
        let result = processor.process_command(&cmd);
        if !result.message.is_empty() && result.message != "exit" {
            println!("{}", result.message);
        }
        process::exit(if result.success { 0 } else { 1 });
    }
    
    // Run interactive or batch mode
    let result = if interactive || atty::is(atty::Stream::Stdin) {
        processor.run_interactive()
    } else {
        processor.run_batch()
    };
    
    if let Err(error) = result {
        eprintln!("Error: {}", error);
        process::exit(1);
    }
}

fn print_usage() {
    println!("stdgeo - 3D Geometry Command Line Tool");
    println!();
    println!("USAGE:");
    println!("    stdgeo [OPTIONS]");
    println!();
    println!("OPTIONS:");
    println!("    -i, --interactive    Force interactive mode (show prompt)");
    println!("    -v, --verbose        Enable verbose output");
    println!("    -c, --command <CMD>  Execute a single command and exit");
    println!("    -h, --help          Show this help message");
    println!("        --version       Show version information");
    println!();
    println!("EXAMPLES:");
    println!("    stdgeo                           # Interactive mode (if TTY)");
    println!("    stdgeo -i                        # Force interactive mode");
    println!("    echo \"cube 3.0\" | stdgeo          # Batch mode");
    println!("    stdgeo -c \"cube 2.0\"              # Single command");
    println!("    cat script.txt | stdgeo          # Process script file");
    println!();
    println!("COMMANDS:");
    println!("    cube [size]  - Create a cube (default size: 2.0)");
    println!("    clear        - Clear current geometry");
    println!("    stats        - Show current geometry statistics");
    println!("    help         - Show available commands");
    println!("    exit/quit    - Exit the application");
}