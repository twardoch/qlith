#!/usr/bin/env bash
# this_file: examples/run-examples.sh

# Get the absolute path to the directory containing this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "Running qlith examples renderer with all options..."
# python run-example.py all

python ./run-example.py run -e mini -i . -o mini --width 1024 --height 1024 -p -s

python ./run-example.py run -e pro -i . -o pro --width 1024 --height 1024 -p -s
