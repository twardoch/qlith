#!/usr/bin/env bash

# Get the absolute path to the directory containing this script
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do
    dir="$(cd -P "$(dirname "$SOURCE")" && pwd)"
    SOURCE="$(readlink "$SOURCE")"
    [[ $SOURCE != /* ]] && SOURCE="$dir/$SOURCE"
done
dir="$(cd -P "$(dirname "$SOURCE")" && pwd)"

echo "Running qlith-pro"
echo "Current directory: \"$dir\""
echo "Application directory: \"$dir/build/qlith-pro.app/Contents/MacOS\""

# Check if an HTML file was provided as an argument
if [ "$#" -ge 1 ]; then
    html_file="$1"
else
    # Default file if none provided
    html_file="$dir/../test_files/basic_layout.html"
fi

# Execute the qlith-pro application with the specified HTML file
"$dir/build/qlith-pro.app/Contents/MacOS/qlith-pro" "$html_file"

# "./qlith-pro/build/qlith-pro.app/Contents/MacOS/qlith-pro" "./test_files/simple.html"
