#!/usr/bin/env bash
# this_file: ./build_macos.sh

# Get the absolute path to the directory containing this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

cd mini
source ./build.sh
cd ..

cd pro
source ./build.sh
cd ..
