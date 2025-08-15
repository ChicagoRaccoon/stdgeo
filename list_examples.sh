#!/bin/bash

# StdGeo Example Catalog
# Lists all available examples with descriptions and usage

echo "📚 StdGeo Headless Mode Examples"
echo "================================="
echo ""

if [[ ! -d "examples" ]]; then
    echo "❌ Examples directory not found"
    exit 1
fi

echo "Available examples:"
echo ""

# List all examples with descriptions
for example_file in examples/*.txt; do
    if [[ -f "$example_file" ]]; then
        name=$(basename "$example_file" .txt)
        description=$(head -2 "$example_file" | tail -1 | sed 's/^# //')
        commands=$(grep -c "^[a-z]" "$example_file" || echo "0")
        
        echo "📄 ${name}"
        echo "   Description: ${description}"
        echo "   Commands: ${commands}"
        echo "   Usage: ./target/release/stdgeo < examples/${name}.txt"
        echo "   Or:    ./run_headless.sh -f examples/${name}.txt"
        echo ""
    fi
done

echo "Quick commands:"
echo "  ./target/release/stdgeo -i              # Interactive mode"
echo "  ./target/release/stdgeo -c \"cube 3.0\"   # Single command"
echo "  ./run_headless.sh -i                    # Interactive mode (alternative)"
echo "  ./run_headless.sh -d                    # Run demo"
echo ""
echo "Testing:"
echo "  ./test.sh --examples                    # Validate all examples"
echo "  ./test.sh --full                        # Complete test suite"