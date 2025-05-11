#!/usr/bin/env bash

# Get the absolute path to the directory containing this script
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do
    dir="$(cd -P "$(dirname "$SOURCE")" && pwd)"
    SOURCE="$(readlink "$SOURCE")"
    [[ $SOURCE != /* ]] && SOURCE="$dir/$SOURCE"
done
dir="$(cd -P "$(dirname "$SOURCE")" && pwd)"

echo "Running in debug mode"
echo "Current directory: \"$dir\""
echo "Application directory: \"$dir/build/qlith-pro.app/Contents/MacOS\""

# Run with debug environment variable
# QLITH_DEBUG=1
"$dir/build/qlith-pro.app/Contents/MacOS/qlith-pro" "$dir/../test_files/fl8.html"

# "./qlith-pro/build/qlith-pro.app/Contents/MacOS/qlith-pro" "./test_files/simple.html"
