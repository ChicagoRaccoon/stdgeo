#!/bin/bash

echo "Testing StdGeo CLI with session mode"
echo "===================================="

echo ""
echo "1. Testing single command mode (backward compatibility):"
echo "   Creating a point:"
./target/debug/stdgeo point -x 3.0 -y 4.0

echo ""
echo "   Creating a line:"
./target/debug/stdgeo line --x1 0.0 --y1 0.0 --x2 5.0 --y2 5.0

echo ""
echo "   Saving point to file:"
./target/debug/stdgeo point -x 10.0 -y 20.0 -o test_point.json
echo "   Content of test_point.json:"
cat test_point.json

echo ""
echo "2. Reading the saved file:"
./target/debug/stdgeo read -i test_point.json

echo ""
echo "3. Session mode is available - run './target/debug/stdgeo session' to start interactive mode"
echo "   In session mode you can use commands like:"
echo "   > point 3.0 4.0"
echo "   > line 0.0 0.0 5.0 5.0"
echo "   > list"
echo "   > translate 2.0 2.0"
echo "   > save output.json"
echo "   > quit"

echo ""
echo "Demo completed successfully!"