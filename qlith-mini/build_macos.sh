#!/bin/bash
# this_file: qlith-mini/build_macos.sh
dir=${0%/*}
if [ "$dir" = "$0" ]; then dir="."; fi
cd "$dir"

# Build script for qlith-mini on macOS

set -e

# Default build type
BUILD_TYPE=${BUILD_TYPE:-Release}

# Get the script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
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
QT_DIR="/usr/local/Cellar/qt@5/5.15.16_2"
if [ ! -d "$QT_DIR" ]; then
    echo "Error: Qt5 not found at $QT_DIR"
    echo "Please install Qt5 using Homebrew (brew install qt@5)"
    exit 1
fi
echo "Using Qt5 from: $QT_DIR"

# Clean build
if [ "$1" == "clean" ]; then
    echo "Cleaning build directory..."
    rm -rf "$PROJECT_ROOT/build"
fi

# Create build directory if it doesn't exist
mkdir -p "$PROJECT_ROOT/build"
cd "$PROJECT_ROOT/build"

# Run CMake with the appropriate options
echo "Configuring qlith-mini with CMake..."
cmake "$PROJECT_ROOT" \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_PREFIX_PATH="$QT_DIR"

# Build the project
echo "Building qlith-mini..."
cmake --build . -- -j$(sysctl -n hw.ncpu)

echo "Build complete."
echo "To run the browser: ./browser/qlith"

# Create an alias script for easy launching
if [ -f "$PROJECT_ROOT/build/browser/qlith.app/Contents/MacOS/qlith" ]; then
    ln -sf "$PROJECT_ROOT/build/browser/qlith.app/Contents/MacOS/qlith" "$PROJECT_ROOT/qlith-run"
    chmod +x "$PROJECT_ROOT/qlith-run"
    echo "Created launch script: $PROJECT_ROOT/qlith-run"
fi
