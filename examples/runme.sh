#!/usr/bin/env bash
# this_file: examples/runme.sh

# Get the absolute path to the directory containing this script
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do
    dir="$(cd -P "$(dirname "$SOURCE")" && pwd)"
    SOURCE="$(readlink "$SOURCE")"
    [[ $SOURCE != /* ]] && SOURCE="$dir/$SOURCE"
done
dir="$(cd -P "$(dirname "$SOURCE")" && pwd)"

# Move to the root directory
cd "$dir/.."
root_dir="$(pwd)"

# Define paths
mini_app="$root_dir/qlith-mini/build/browser/qlith.app/Contents/MacOS/qlith"
pro_app="$root_dir/qlith-pro/build/qlith-pro.app/Contents/MacOS/qlith-pro"
examples_dir="$root_dir/examples"
mini_output_dir="$examples_dir/mini"
pro_output_dir="$examples_dir/pro"

# Create output directories if they don't exist
mkdir -p "$mini_output_dir" "$pro_output_dir"

# Check if the applications exist
if [ ! -x "$mini_app" ]; then
    echo "Error: qlith-mini application not found at $mini_app"
    echo "Please build the qlith-mini application first"
    exit 1
fi

if [ ! -x "$pro_app" ]; then
    echo "Error: qlith-pro application not found at $pro_app"
    echo "Please build the qlith-pro application first"
    exit 1
fi

# Function to process a file with both applications
process_file() {
    local html_file="$1"
    local filename=$(basename "$html_file")
    local name_without_ext="${filename%.html}"

    echo "Processing $filename..."

    # Process with qlith-mini
    echo "  Rendering with qlith-mini..."
    "$mini_app" --svg "$mini_output_dir/${name_without_ext}.svg" --png "$mini_output_dir/${name_without_ext}.png" "$html_file"

    # Process with qlith-pro
    echo "  Rendering with qlith-pro..."
    "$pro_app" --svg "$pro_output_dir/${name_without_ext}.svg" --png "$pro_output_dir/${name_without_ext}.png" "$html_file"

    echo "  Done with $filename"
    echo
}

# Process all HTML files in the examples directory
echo "Starting to process all HTML files in $examples_dir"
echo "=========================================================="

# Count the HTML files
html_files=($(find "$examples_dir" -maxdepth 1 -name "*.html"))
file_count=${#html_files[@]}

if [ $file_count -eq 0 ]; then
    echo "No HTML files found in $examples_dir"
    exit 1
fi

echo "Found $file_count HTML files to process"
echo

# Process each HTML file
for html_file in "${html_files[@]}"; do
    process_file "$html_file"
done

echo "=========================================================="
echo "Processing complete!"
echo "Output files for qlith-mini are in $mini_output_dir"
echo "Output files for qlith-pro are in $pro_output_dir"
