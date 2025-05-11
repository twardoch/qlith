#!/bin/bash
# this_file: qlith-pro/build_macos.sh

dir=${0%/*}
if [ "$dir" = "$0" ]; then dir="."; fi
cd "$dir"

# Exit immediately if a command exits with a non-zero status.
set -e

# Set script to print each command as it is executed
set -x

# Default build type
BUILD_TYPE=${BUILD_TYPE:-Release}

# Define paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"
EXTERNAL_DIR="$PROJECT_ROOT/../external"

# Check for external dependencies
if [ ! -d "$EXTERNAL_DIR/litehtml" ]; then
    echo "Error: litehtml dependency not found at $EXTERNAL_DIR/litehtml"
    echo "Please ensure you have cloned the external repositories."
    exit 1
fi

if [ ! -d "$EXTERNAL_DIR/gumbo-parser" ]; then
    echo "Error: gumbo-parser dependency not found at $EXTERNAL_DIR/gumbo-parser"
    echo "Please ensure you have cloned the external repositories."
    exit 1
fi

# Check for Qt5
QT_PATH=$(brew --prefix qt@5 2>/dev/null || echo "")
if [ -z "$QT_PATH" ]; then
    echo "Error: Qt5 not found. Please install Qt5 using Homebrew (brew install qt@5)"
    exit 1
fi
echo "Using Qt5 from: $QT_PATH"

# Create the build directory if it doesn't exist
mkdir -p "$BUILD_DIR"

# Change to the build directory
cd "$BUILD_DIR"

# Configure the project using CMake
echo "Configuring project..."
cmake "$PROJECT_ROOT" \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_PREFIX_PATH="$QT_PATH" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build the project
echo "Building project..."
cmake --build . -- -j$(sysctl -n hw.ncpu)

echo "Build complete."
echo "Executable is located at: $BUILD_DIR/qlith-pro.app/Contents/MacOS/qlith-pro"

# Create an alias script for easy launching
if [ -f "$BUILD_DIR/qlith-pro.app/Contents/MacOS/qlith-pro" ]; then
    ln -sf "$BUILD_DIR/qlith-pro.app/Contents/MacOS/qlith-pro" "$PROJECT_ROOT/qlith-pro-run"
    chmod +x "$PROJECT_ROOT/qlith-pro-run"
    echo "Created launch script: $PROJECT_ROOT/qlith-pro-run"
fi
