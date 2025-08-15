#!/bin/bash

# StdGeo Headless Runner
# Convenience script for running headless mode

if [[ $# -eq 0 ]]; then
    echo "StdGeo Headless Runner"
    echo ""
    echo "Usage:"
    echo "  $0 -i                    # Interactive mode"
    echo "  $0 -f script.txt         # Run script file"
    echo "  $0 -c \"command\"          # Execute single command"
    echo "  $0 -d                    # Run demo script"
    echo ""
    echo "Examples:"
    echo "  $0 -i                    # Start interactive session"
    echo "  $0 -c \"cube 3.0\"         # Create a 3x3x3 cube"
    echo "  $0 -f my_script.txt      # Run custom script"
    echo "  $0 -d                    # Run built-in demo"
    exit 0
fi

case "$1" in
    -i|--interactive)
        echo "Starting interactive headless mode..."
        ./target/release/stdgeo --interactive
        ;;
    -f|--file)
        if [[ -z "$2" ]]; then
            echo "Error: No script file specified"
            exit 1
        fi
        if [[ ! -f "$2" ]]; then
            echo "Error: Script file '$2' not found"
            exit 1
        fi
        echo "Running script: $2"
        ./target/release/stdgeo < "$2"
        ;;
    -c|--command)
        if [[ -z "$2" ]]; then
            echo "Error: No command specified"
            exit 1
        fi
        echo "Executing: $2"
        ./target/release/stdgeo --command "$2"
        ;;
    -d|--demo)
        echo "Running demo script..."
        ./target/release/stdgeo < examples/headless_demo.txt
        ;;
    *)
        echo "Unknown option: $1"
        echo "Use '$0' without arguments for help"
        exit 1
        ;;
esac