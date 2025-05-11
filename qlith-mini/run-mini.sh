#!/usr/bin/env bash
# this_file: qlith-mini/runme.sh

# Get the absolute path to the directory containing this script
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do
    dir="$(cd -P "$(dirname "$SOURCE")" && pwd)"
    SOURCE="$(readlink "$SOURCE")"
    [[ $SOURCE != /* ]] && SOURCE="$dir/$SOURCE"
done
dir="$(cd -P "$(dirname "$SOURCE")" && pwd)"

cd "$dir"

# Check if an HTML file was provided as an argument
if [ "$#" -ge 1 ]; then
    html_file="$1"
else
    # Default file if none provided
    html_file="$dir/../examples/basic_layout.html"
fi

# Execute the qlith application with the specified HTML file
"$dir/build/browser/qlith.app/Contents/MacOS/qlith" "$html_file"
