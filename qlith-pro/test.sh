#!/bin/bash
# Build and run tests for qlith-pro

# Go to script directory
cd "$(dirname "$0")"

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Run CMake
echo "Running CMake..."
cmake ..

# Build
echo "Building qlith-pro and tests..."
make -j4

# Run color test
echo "Running color test..."
./bin/color_test

# Test with the original simple.html
echo "Testing with simple.html..."
./qlith-pro.app/Contents/MacOS/qlith-pro "../test_files/simple.html"

# Return to original directory
cd ..

echo "Tests completed!"
