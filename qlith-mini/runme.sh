#!/usr/bin/env bash

# Get the absolute path to the directory containing this script
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do
    dir="$(cd -P "$(dirname "$SOURCE")" && pwd)"
    SOURCE="$(readlink "$SOURCE")"
    [[ $SOURCE != /* ]] && SOURCE="$dir/$SOURCE"
done
dir="$(cd -P "$(dirname "$SOURCE")" && pwd)"

cd "$dir"

"$dir/build/browser/qlith.app/Contents/MacOS/qlith" "$dir/../test_files/fl8.html"
