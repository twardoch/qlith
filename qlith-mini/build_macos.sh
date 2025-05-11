#!/bin/bash
# this_file: qlith-mini/build_macos.sh

dir=${0%/*}
if [ "$dir" = "$0" ]; then dir="."; fi
cd "$dir"

# Exit immediately if a command exits with a non-zero status.
set -e

# Set script to print each command as it is executed
set -x

# Default build type
BUILD_TYPE=${BUILD_TYPE:-Release}

# Define paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"
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
QT_PATH=$(brew --prefix qt@5 2>/dev/null || echo "")
if [ -z "$QT_PATH" ]; then
    echo "Error: Qt5 not found. Please install Qt5 using Homebrew (brew install qt@5)"
    exit 1
fi
echo "Using Qt5 from: $QT_PATH"

# Apply OpenGL headers workaround for macOS
QT5_CONFIG_FILE="$QT_PATH/lib/cmake/Qt5Gui/Qt5GuiConfigExtras.cmake"
if [ -f "$QT5_CONFIG_FILE" ]; then
    # Create a backup if it doesn't exist
    if [ ! -f "${QT5_CONFIG_FILE}.original" ]; then
        sudo cp "$QT5_CONFIG_FILE" "${QT5_CONFIG_FILE}.original"
        echo "Created backup of original Qt5GuiConfigExtras.cmake"
    fi

    # Check if we need to apply the fix
    if ! grep -q "QtGui_OpenGL_Headers" "$QT5_CONFIG_FILE"; then
        echo "Applying comprehensive OpenGL headers fix to Qt5GuiConfigExtras.cmake..."

        # Copy the fixed version to a temporary location
        mkdir -p /tmp/qt5fix
        cat >/tmp/qt5fix/opengl_fix.cmake <<EOF
set(_GL_INCDIRS "/System/Library/Frameworks/OpenGL.framework/Headers" "/System/Library/Frameworks/AGL.framework/Headers")

# Try to find OpenGL headers in standard locations
find_path(_qt5gui_OPENGL_INCLUDE_DIR gl.h
    PATHS \${_GL_INCDIRS}
)

# Check Xcode SDK locations as fallback
if (NOT _qt5gui_OPENGL_INCLUDE_DIR)
    message(STATUS "Checking Xcode SDK locations for OpenGL headers...")
    find_path(_qt5gui_OPENGL_INCLUDE_DIR gl.h
        PATHS
        "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers"
        "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers"
    )
endif()

# If still not found, create a dummy directory with empty headers
if (NOT _qt5gui_OPENGL_INCLUDE_DIR)
    message(STATUS "Failed to find \"gl.h\" in \"\${_GL_INCDIRS}\". Using dummy headers.")
    
    # Create a dummy GL directory with empty headers
    set(_qt5gui_OPENGL_INCLUDE_DIR "\${CMAKE_BINARY_DIR}/QtGui_OpenGL_Headers")
    file(MAKE_DIRECTORY "\${_qt5gui_OPENGL_INCLUDE_DIR}/GL")
    
    # Create minimal dummy headers
    if (NOT EXISTS "\${_qt5gui_OPENGL_INCLUDE_DIR}/GL/gl.h")
        file(WRITE "\${_qt5gui_OPENGL_INCLUDE_DIR}/GL/gl.h" "/* Dummy OpenGL header */\\n#define GL_VERSION_1_1 1\\n")
    endif()
    if (NOT EXISTS "\${_qt5gui_OPENGL_INCLUDE_DIR}/GL/glu.h")
        file(WRITE "\${_qt5gui_OPENGL_INCLUDE_DIR}/GL/glu.h" "/* Dummy GLU header */\\n")
    endif()
    if (NOT EXISTS "\${_qt5gui_OPENGL_INCLUDE_DIR}/GL/glext.h")
        file(WRITE "\${_qt5gui_OPENGL_INCLUDE_DIR}/GL/glext.h" "/* Dummy GLEXT header */\\n")
    endif()
endif()

unset(_GL_INCDIRS)

# Always append the OpenGL include directory, even if it's a dummy one
list(APPEND Qt5Gui_INCLUDE_DIRS \${_qt5gui_OPENGL_INCLUDE_DIR})
set_property(TARGET Qt5::Gui APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES \${_qt5gui_OPENGL_INCLUDE_DIR})

# Keep the rest of the original file below this line
EOF

        # Get the rest of the original file (after the problematic part)
        sed -n '/macro(_qt5gui_find_extra_libs/,$p' "${QT5_CONFIG_FILE}.original" >>/tmp/qt5fix/opengl_fix.cmake

        # Apply the fix
        sudo cp /tmp/qt5fix/opengl_fix.cmake "$QT5_CONFIG_FILE"
        echo "Applied OpenGL headers fix."
    else
        echo "OpenGL headers fix already applied."
    fi
else
    echo "Warning: Could not find Qt5GuiConfigExtras.cmake to patch."
fi

# Create the build directory if it doesn't exist
mkdir -p "$BUILD_DIR"

# Change to the build directory
cd "$BUILD_DIR"

# Configure the project using CMake
echo "Configuring qlith-mini..."
cmake "$PROJECT_ROOT" \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_PREFIX_PATH="$QT_PATH" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build the project
echo "Building qlith-mini..."
cmake --build . -- -j$(sysctl -n hw.ncpu)

echo "Build complete."
echo "Executable is located at: $BUILD_DIR/browser/qlith.app/Contents/MacOS/qlith"

# Create an alias script for easy launching
if [ -f "$BUILD_DIR/browser/qlith.app/Contents/MacOS/qlith" ]; then
    ln -sf "$BUILD_DIR/browser/qlith.app/Contents/MacOS/qlith" "$PROJECT_ROOT/qlith-run"
    chmod +x "$PROJECT_ROOT/qlith-run"
    echo "Created launch script: $PROJECT_ROOT/qlith-run"
fi
