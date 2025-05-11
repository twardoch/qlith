# Building qlith-pro on macOS

This document provides instructions for building qlith-pro on macOS systems.

## Prerequisites

1. **Xcode Command Line Tools**: Ensure you have Xcode Command Line Tools installed:
   ```bash
   xcode-select --install
   ```

2. **Homebrew**: Install [Homebrew](https://brew.sh/) if you don't have it:
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

3. **Qt5**: Install Qt5 using Homebrew:
   ```bash
   brew install qt@5
   ```

4. **CMake**: Install CMake if not already installed:
   ```bash
   brew install cmake
   ```

## Building

1. Clone the repository including submodules:
   ```bash
   git clone --recurse-submodules https://github.com/yourusername/qlith.git
   cd qlith/qlith-pro
   ```

2. Run the build script:
   ```bash
   ./build_macos.sh
   ```

   The script does the following:
   - Applies necessary fixes for macOS OpenGL compatibility
   - Configures the project with CMake
   - Builds the application
   - Creates a launcher script

3. Once built, you can run the application:
   ```bash
   ./qlith-pro-run
   ```

## Troubleshooting

### OpenGL Headers Issues

The build system includes a comprehensive workaround for missing OpenGL headers on modern macOS versions, which:

1. Modifies the Qt5GuiConfigExtras.cmake file to properly handle missing OpenGL headers
2. Searches for OpenGL headers in multiple locations:
   - Standard locations: `/System/Library/Frameworks/OpenGL.framework/Headers`
   - Xcode SDK locations: `/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers`
   - Command Line Tools locations: `/Library/Developer/CommandLineTools/SDKs/MacOSX*.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers`
3. Creates minimal dummy headers if real headers cannot be found
4. Sets up proper include paths for the build system

If you encounter OpenGL-related build failures, you can try to restore the original Qt configuration and run the build script again:

```bash
# Restore the original Qt5GuiConfigExtras.cmake file if needed
sudo cp /usr/local/opt/qt@5/lib/cmake/Qt5Gui/Qt5GuiConfigExtras.cmake.original /usr/local/opt/qt@5/lib/cmake/Qt5Gui/Qt5GuiConfigExtras.cmake

# Then run the build script again
./build_macos.sh
```

### Missing Libraries

If you encounter errors about missing libraries at runtime, ensure your Qt5 installation is properly linked:

```bash
brew link --force qt@5
```

## Known Issues

- The application may crash with a segmentation fault on some macOS configurations. This is a known issue being investigated.
- Some UI elements may not render properly on Retina displays due to scaling issues. 