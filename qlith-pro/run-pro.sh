#!/usr/bin/env bash

# Get the absolute path to the directory containing this script
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do
    dir="$(cd -P "$(dirname "$SOURCE")" && pwd)"
    SOURCE="$(readlink "$SOURCE")"
    [[ $SOURCE != /* ]] && SOURCE="$dir/$SOURCE"
done
dir="$(cd -P "$(dirname "$SOURCE")" && pwd)"

echo "qlith-pro"
echo "Current directory: \"$dir\""

# Locate the binary
app_path="$dir/build/qlith-pro.app/Contents/MacOS/qlith-pro"
if [ ! -f "$app_path" ]; then
    echo "App not found at $app_path"
    echo "Trying executable in root directory..."
    app_path="$dir/qlith-pro-run"

    if [ ! -f "$app_path" ]; then
        echo "Error: Could not find the qlith-pro executable."
        exit 1
    fi
fi

echo "Using executable: $app_path"

# Default HTML file
default_html="$dir/../examples/basic_layout.html"

# Check if we're doing export
if [ "$1" == "--png" ]; then
    # Export to PNG
    if [ "$#" -ge 3 ]; then
        # Use provided paths
        output_png="$2"
        input_html="$3"
    else
        # Use default paths
        output_png="$dir/../examples/output.png"
        input_html="$default_html"
    fi

    echo "Exporting to PNG: $output_png"
    echo "From HTML file: $input_html"
    "$app_path" --png "$output_png" "$input_html"

elif [ "$1" == "--svg" ]; then
    # Export to SVG
    if [ "$#" -ge 3 ]; then
        # Use provided paths
        output_svg="$2"
        input_html="$3"
    else
        # Use default paths
        output_svg="$dir/../examples/output.svg"
        input_html="$default_html"
    fi

    echo "Exporting to SVG: $output_svg"
    echo "From HTML file: $input_html"
    "$app_path" --svg "$output_svg" "$input_html"

else
    # Regular viewing mode
    if [ "$#" -ge 1 ]; then
        html_file="$1"
    else
        html_file="$default_html"
    fi

    echo "Viewing HTML file: $html_file"
    "$app_path" "$html_file"
fi

# "./qlith-pro/build/qlith-pro.app/Contents/MacOS/qlith-pro" "./examples/simple_test.html"
