#!/usr/bin/env bash
# this_file: ./build_macos.sh

# Get the absolute path to the directory containing this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "DIR $SCRIPT_DIR"

# qlith-mini is being deprecated, build qlith-pro only.
# cd "./qlith-mini"
# source ./build_macos.sh
# cd "$SCRIPT_DIR"

cd "./qlith-pro"
source ./build_macos.sh
cd "$SCRIPT_DIR"
