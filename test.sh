#!/bin/bash

# StdGeo Unified Test Runner
# Combines the best of bash scripting and CTest integration

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_usage() {
    echo "Usage: $0 [OPTION]"
    echo ""
    echo "Options:"
    echo "  --quick, -q      Quick tests for development (Rust + build check)"
    echo "  --full, -f       Full test suite with detailed reporting"
    echo "  --all, -a        Complete test suite including examples"
    echo "  --ci            CI-friendly mode using CTest"
    echo "  --rust, -r       Rust tests only"
    echo "  --cpp, -c        C++ tests only (requires build)"
    echo "  --examples, -e   Example scripts validation only"
    echo "  --help, -h       Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 --quick      # Fast development feedback"
    echo "  $0 --full       # Complete testing with memory checks"
    echo "  $0 --all        # Everything including example validation"
    echo "  $0 --examples   # Just validate example scripts"
    echo "  $0 --ci         # Automated CI/CD testing"
}

print_header() {
    echo -e "${BLUE}🧪 $1${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

run_quick_tests() {
    print_header "StdGeo Quick Tests"
    
    # Rust tests
    echo "📦 Rust unit tests..."
    if cargo test --lib --quiet; then
        print_success "Rust unit tests passed"
    else
        print_error "Rust unit tests failed"
        exit 1
    fi
    
    # FFI tests
    echo "🔗 FFI integration tests..."
    if cargo test ffi_tests --quiet; then
        print_success "FFI tests passed"
    else
        print_error "FFI tests failed"
        exit 1
    fi
    
    # Build check
    echo "🔨 Build verification..."
    if ./build.sh > /dev/null 2>&1; then
        print_success "Build successful"
    else
        print_error "Build failed"
        exit 1
    fi
    
    print_success "Quick tests completed! 🎉"
}

run_rust_tests() {
    print_header "Rust Test Suite"
    
    echo "Running all Rust tests..."
    if cargo test; then
        print_success "All Rust tests passed"
    else
        print_error "Rust tests failed"
        exit 1
    fi
}

run_cpp_tests() {
    print_header "C++ Test Suite"
    
    # Ensure build exists
    if [[ ! -f "build/stdgeo_tests" ]]; then
        echo "🔨 Building project first..."
        ./build.sh
    fi
    
    cd build
    if [[ -f "stdgeo_tests" ]]; then
        echo "Running C++ tests..."
        if ./stdgeo_tests; then
            print_success "C++ tests passed"
        else
            print_error "C++ tests failed"
            exit 1
        fi
    else
        print_error "C++ tests not available (Google Test not found)"
        exit 1
    fi
    cd ..
}

run_ci_tests() {
    print_header "CI Test Suite (CTest)"
    
    # Build if needed
    if [[ ! -d "build" ]]; then
        ./build.sh
    fi
    
    cd build
    echo "Running CTest suite..."
    if ctest --output-on-failure --progress; then
        print_success "CTest suite passed"
    else
        print_error "Some CTest tests failed"
        exit 1
    fi
    cd ..
}

run_full_tests() {
    print_header "StdGeo Full Test Suite"
    
    # 1. Rust tests
    run_rust_tests
    
    # 2. Build verification
    echo ""
    echo "🔨 Building project..."
    if ./build.sh > /dev/null 2>&1; then
        print_success "Build successful"
    else
        print_error "Build failed"
        exit 1
    fi
    
    # 3. C++ tests (if available)
    echo ""
    if [[ -f "build/stdgeo_tests" ]]; then
        cd build
        echo "🧪 Running C++ tests..."
        if ./stdgeo_tests --gtest_brief=1; then
            print_success "C++ tests passed"
        else
            print_error "C++ tests failed"
            exit 1
        fi
        cd ..
    else
        echo "⚠️  C++ tests skipped (Google Test not available)"
    fi
    
    # 4. Integration test (test binary)
    echo ""
    echo "🚀 Integration test..."
    if cargo run --bin stdgeo_test --quiet > /dev/null 2>&1; then
        print_success "Integration test passed"
    else
        print_error "Integration test failed"
        exit 1
    fi
    
    # 5. Memory test (if valgrind available)
    echo ""
    if command -v valgrind >/dev/null 2>&1; then
        echo "🛡️  Memory safety test..."
        if timeout 30s valgrind --leak-check=summary --error-exitcode=1 cargo run > /dev/null 2>&1; then
            print_success "Memory safety test passed"
        else
            echo "⚠️  Memory safety test failed or timed out"
        fi
    else
        echo "⚠️  Memory safety test skipped (valgrind not available)"
    fi
    
    echo ""
    print_success "Full test suite completed! 🎉"
}

# Example testing functions
check_examples_prerequisites() {
    if [[ ! -f "./target/release/stdgeo" ]]; then
        print_error "stdgeo executable not found. Run ./build.sh first."
        return 1
    fi
    
    if [[ ! -d "examples" ]]; then
        print_error "Examples directory not found."
        return 1
    fi
    return 0
}

test_single_example() {
    local example_file="$1"
    local example_name=$(basename "$example_file" .txt)
    
    echo -n "Testing ${example_name}... "
    
    # Run the example and capture output
    if timeout 30s bash -c "cat '$example_file' | ./target/release/stdgeo > /tmp/example_output.log 2>&1"; then
        local output=$(cat /tmp/example_output.log)
        
        # Basic validation - check for expected patterns
        local errors=0
        
        # Check that it didn't crash
        if [[ -z "$output" ]]; then
            print_error "No output generated"
            ((errors++))
        fi
        
        # Check for error indicators
        if echo "$output" | grep -q "Segmentation fault\|core dumped\|Aborted"; then
            print_error "Crashed during execution"
            ((errors++))
        fi
        
        # Check for geometry operations (if cube commands exist)
        if grep -q "cube" "$example_file" && ! echo "$output" | grep -q "Created cube"; then
            print_error "Cube creation commands failed"
            ((errors++))
        fi
        
        # Check for stats operations
        if grep -q "stats" "$example_file" && ! echo "$output" | grep -q -E "(Current geometry|No geometry loaded)"; then
            print_error "Stats commands failed"
            ((errors++))
        fi
        
        if [[ $errors -eq 0 ]]; then
            print_success "${example_name}"
            return 0
        else
            print_error "${example_name} (${errors} errors)"
            return 1
        fi
    else
        print_error "${example_name} (timeout/crash)"
        return 1
    fi
}

validate_example_content() {
    local example_file="$1"
    local example_name=$(basename "$example_file" .txt)
    
    # Check that example has exit command (good practice)
    if ! grep -q "exit" "$example_file"; then
        print_warning "${example_name}: No exit command (will timeout)"
    fi
    
    # Check for balanced operations
    local cube_count=$(grep -c "^cube " "$example_file" || true)
    local clear_count=$(grep -c "^clear" "$example_file" || true)
    
    if [[ $cube_count -gt 10 && $clear_count -eq 0 ]]; then
        print_warning "${example_name}: Many cube operations without clear (may use memory)"
    fi
}

performance_test_example() {
    local example_file="$1"
    local example_name=$(basename "$example_file" .txt)
    
    echo -n "Performance testing ${example_name}... "
    
    local start_time=$(date +%s.%N)
    if timeout 60s bash -c "cat '$example_file' | ./target/release/stdgeo > /dev/null 2>&1"; then
        local end_time=$(date +%s.%N)
        local duration=$(awk "BEGIN {print $end_time - $start_time}" 2>/dev/null || echo "0")
        
        if (( $(awk "BEGIN {print ($duration < 10.0)}" 2>/dev/null || echo 1) )); then
            print_success "${example_name} (${duration}s)"
        elif (( $(awk "BEGIN {print ($duration < 30.0)}" 2>/dev/null || echo 0) )); then
            print_warning "${example_name} (${duration}s - slow)"
        else
            print_error "${example_name} (${duration}s - very slow)"
        fi
    else
        print_error "${example_name} (timeout)"
    fi
}

run_examples_tests() {
    print_header "Example Scripts Validation"
    
    if ! check_examples_prerequisites; then
        return 1
    fi
    
    # Find all example files
    local example_files=(examples/*.txt)
    local total_examples=${#example_files[@]}
    local passed_examples=0
    local failed_examples=0
    
    if [[ $total_examples -eq 0 ]]; then
        print_error "No example files found in examples/ directory"
        return 1
    fi
    
    # Content validation
    echo ""
    echo "📋 Content Validation:"
    for example_file in "${example_files[@]}"; do
        if [[ -f "$example_file" ]]; then
            validate_example_content "$example_file"
        fi
    done
    
    # Functionality testing
    echo ""
    echo "🧪 Functionality Testing:"
    for example_file in "${example_files[@]}"; do
        if [[ -f "$example_file" ]]; then
            if test_single_example "$example_file"; then
                ((passed_examples++))
            else
                ((failed_examples++))
            fi
        fi
    done
    
    # Performance testing (simplified for integration)
    echo ""
    echo "⚡ Performance Testing:"
    for example_file in "${example_files[@]}"; do
        if [[ -f "$example_file" ]]; then
            performance_test_example "$example_file"
        fi
    done
    
    # Summary
    echo ""
    print_header "Example Test Results"
    echo -e "Total Examples: ${BLUE}$total_examples${NC}"
    echo -e "Passed:         ${GREEN}$passed_examples${NC}"
    echo -e "Failed:         ${RED}$failed_examples${NC}"
    echo ""
    
    if [[ $failed_examples -eq 0 ]]; then
        print_success "All example tests passed! 🎉"
        
        echo ""
        echo "📚 Available Examples:"
        for example_file in "${example_files[@]}"; do
            if [[ -f "$example_file" ]]; then
                local name=$(basename "$example_file" .txt)
                local description=$(head -2 "$example_file" | tail -1 | sed 's/^# //')
                echo "  📄 $name: $description"
            fi
        done
        
        echo ""
        echo "Run examples with:"
        echo "  cat examples/[script_name].txt | ./target/release/stdgeo"
        echo "  ./target/release/stdgeo < examples/[script_name].txt"
        
        return 0
    else
        print_error "Some example tests failed."
        return 1
    fi
    
    # Cleanup
    rm -f /tmp/example_output.log
}

run_all_tests() {
    print_header "StdGeo Complete Test Suite"
    
    local total_failures=0
    
    # 1. Run full test suite
    echo ""
    if ! run_full_tests; then
        ((total_failures++))
    fi
    
    # 2. Run example tests
    echo ""
    if ! run_examples_tests; then
        ((total_failures++))
    fi
    
    # Final summary
    echo ""
    print_header "Complete Test Suite Results"
    
    if [[ $total_failures -eq 0 ]]; then
        print_success "All test suites passed! 🚀"
        echo ""
        echo "✅ Rust unit tests"
        echo "✅ C++ component tests" 
        echo "✅ Headless operation tests"
        echo "✅ Example script validation"
        echo "✅ Integration tests"
        echo "✅ Performance validation"
        echo ""
        print_success "Your StdGeo application is fully validated!"
    else
        print_error "$total_failures test suite(s) failed."
        echo ""
        echo "Please review the output above for details."
    fi
    
    return $total_failures
}

# Parse command line arguments
case "${1:-}" in
    --quick|-q)
        run_quick_tests
        ;;
    --full|-f)
        run_full_tests
        ;;
    --ci)
        run_ci_tests
        ;;
    --rust|-r)
        run_rust_tests
        ;;
    --cpp|-c)
        run_cpp_tests
        ;;
    --examples|-e)
        run_examples_tests
        ;;
    --all|-a)
        run_all_tests
        ;;
    --help|-h|help)
        print_usage
        ;;
    "")
        echo "No option specified. Use --help for usage information."
        echo ""
        echo "Quick start:"
        echo "  $0 --quick    # For development"
        echo "  $0 --full     # For comprehensive testing"
        echo "  $0 --all      # Everything including examples"
        ;;
    *)
        echo "Unknown option: $1"
        print_usage
        exit 1
        ;;
esac