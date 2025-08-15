#!/bin/bash

# Example Generator for StdGeo Headless Mode
# Automatically creates example scripts for various scenarios

echo "Generating StdGeo example scripts..."

# Create examples directory if it doesn't exist
mkdir -p examples

# Generate a performance benchmark script
cat > examples/performance_benchmark.txt << 'EOF'
# Performance Benchmark Example
# Measures geometry operation timing

# Small cubes (should be fast)
cube 0.1
cube 0.5
cube 1.0
cube 1.5
cube 2.0

# Medium cubes  
cube 5.0
cube 10.0
cube 15.0
cube 20.0

# Large cubes (may take more time)
cube 50.0
cube 100.0
cube 200.0

exit
EOF

# Generate a mathematical sequence script
cat > examples/mathematical_sequences.txt << 'EOF'
# Mathematical Sequences Example
# Creates cubes following mathematical patterns

# Powers of 2
cube 1.0
cube 2.0
cube 4.0
cube 8.0
cube 16.0
cube 32.0

clear

# Square roots
cube 1.0
cube 1.414
cube 1.732
cube 2.0
cube 2.236
cube 2.449

clear

# Prime numbers (scaled down)
cube 0.2
cube 0.3
cube 0.5
cube 0.7
cube 1.1
cube 1.3

exit
EOF

# Generate a data validation script
cat > examples/data_validation.txt << 'EOF'
# Data Validation Example
# Validates that cube properties are mathematically correct

# Create unit cube and verify 8 vertices
cube 1.0
stats

# Create 2x2x2 cube - should still have 8 vertices  
cube 2.0
stats

# Create large cube - verify scaling
cube 10.0
stats

# Verify clearing works
clear
stats

# Verify recreation
cube 5.0
stats

exit
EOF

# Generate CI/CD automation script
cat > examples/ci_cd_automation.txt << 'EOF'
# CI/CD Automation Example
# Script designed for continuous integration testing

# Test basic functionality
cube 2.0
stats

# Verify stats show expected vertex count (should be 8)
# Verify stats show expected triangle count (should be 12)

# Test edge case
cube 0.1
stats

# Test large scale
cube 100.0
stats  

# Cleanup test
clear
stats

# Final validation
cube 1.0
stats

exit
EOF

echo "✅ Generated example scripts:"
echo "  - examples/performance_benchmark.txt"
echo "  - examples/mathematical_sequences.txt" 
echo "  - examples/data_validation.txt"
echo "  - examples/ci_cd_automation.txt"
echo ""
echo "Run examples with:"
echo "  ./run_headless.sh -f examples/[script_name].txt"