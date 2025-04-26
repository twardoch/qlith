#!/bin/bash
# this_file: qlith-pro/build_macos.sh

# Exit immediately if a command exits with a non-zero status.
set -e

# Define the project root and build directory relative to the script location
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"

# Create the build directory if it doesn't exist
mkdir -p "$BUILD_DIR"

# Change to the build directory
cd "$BUILD_DIR"

# Configure the project using CMake
# Pass the project root directory to CMake
# Add -G Xcode potentially if you want an Xcode project, or Ninja for Ninja builds.
# Specify Qt path if necessary, e.g., -DCMAKE_PREFIX_PATH=/path/to/qt
echo "Configuring project..."
cmake "$PROJECT_ROOT" -G Ninja -DCMAKE_PREFIX_PATH=$(brew --prefix qt@5)

# Build the project
echo "Building project..."
cmake --build .

echo "Build complete. Executable might be in $BUILD_DIR or a subdirectory."
