#!/bin/bash
# this_file: qlith/build_macos.sh
# Build script for qlith on macOS

set -e

# Default build type
BUILD_TYPE=${BUILD_TYPE:-Release}

# Paths to dependencies
LITEHTML_PATH=${LITEHTML_PATH:-"../litehtml"}
GUMBO_PATH=${GUMBO_PATH:-"../gumbo-parser"}

# Clean build
if [ "$1" == "clean" ]; then
    echo "Cleaning build directory..."
    rm -rf build
fi

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Run CMake with the appropriate options
echo "Configuring qlith with CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DLITEHTML_SOURCE_DIR=${LITEHTML_PATH} \
    -DGUMBO_SOURCE_DIR=${GUMBO_PATH}

# Build the project
echo "Building qlith..."
make -j$(sysctl -n hw.ncpu)

echo "Build complete. You can find the binaries in the build directory."
echo "To run the browser: ./browser/qlith"
