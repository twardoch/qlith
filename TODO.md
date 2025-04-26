# Refactoring Plan for qlith-pro

## 1. Goal

*   **Refactor Directory Structure:** Reorganize the `qlith-pro` directory for better clarity and maintainability. Move away from placing numerous source files in the root directory.
*   **Standardize Build System:** Migrate the build system entirely to CMake, replacing the `.pro` and `.pri` files.
*   **Ensure Qt5 Compatibility:** Verify and ensure the codebase builds and runs correctly with Qt5.
*   **Provide macOS Build Instructions:** Document the steps required to build `qlith-pro` using CMake and Qt5 on macOS.

## 2. Analysis of Current Structure

The current `qlith-pro` directory contains a mix of source files (`.cpp`, `.h`), resource files (`.qrc`, `.ui`, `css/`, `font/`, `img/`), CMake/QMake build files, and subdirectories (`src/gui`, `include`, `containers`, `resources`). This flat structure for source files makes it difficult to navigate and understand the codebase modules. The mix of CMake and QMake build files is confusing.

## 3. Proposed Directory Structure

```
qlith-pro/
├── src/                  # Core logic source files (.cpp)
│   └── gui/              # GUI-specific source files (.cpp)
├── include/              # Public header files (.h)
│   └── qlith/            # Namespace subdirectory for project headers
├── resources/            # Resource files (images, css, fonts, ui)
│   ├── css/
│   ├── font/
│   ├── img/
│   ├── mainwindow.ui
│   └── res.qrc
├── cmake/                # CMake helper modules (if any)
├── CMakeLists.txt        # Root CMake build script
└── build/                # Build output directory (ignored by git)
```

*   **`src/`**: Will contain all core `.cpp` files previously in the root.
*   **`src/gui/`**: Will contain GUI-related `.cpp` files (`container_qt5.cpp`, `mainwindow.cpp`, `main.cpp`, `fontcache.cpp`).
*   **`include/qlith/`**: Will contain all `.h` files previously in the root or `include/`. Using a subdirectory prevents potential name clashes.
*   **`resources/`**: Will consolidate all resource files.
*   **`cmake/`**: For any custom CMake modules (optional).
*   **`CMakeLists.txt`**: The main CMake script orchestrating the build.
*   **`build/`**: Standard directory for CMake out-of-source builds.

## 4. Refactoring Steps

1.  **Create New Directories:** Create the `src`, `include/qlith`, `resources` (if it doesn't fully match), and `cmake` directories within `qlith-pro`.
2.  **Move Source Files (`.cpp`)**:
    *   Move `affinetransform.cpp`, `bitmapimage.cpp`, `color.cpp`, `common.cpp`, `contextshadow.cpp`, `floatpoint.cpp`, `floatpoint3d.cpp`, `floatquad.cpp`, `floatrect.cpp`, `floatsize.cpp`, `gradient.cpp`, `graphicscontext.cpp`, `image.cpp`, `imagedecoder.cpp`, `imagedecoderqt.cpp`, `imageobserver.cpp`, `imagesource.cpp`, `intpoint.cpp`, `intrect.cpp`, `intsize.cpp`, `litehtml-qt.js_plugin_import.cpp`, `mimetyperegistry.cpp`, `mimetyperegistryqt.cpp`, `pathqt.cpp`, `pngimagedecoder.cpp`, `purgeablebuffer.cpp`, `rgba32bufferqt.cpp`, `shadowdata.cpp`, `sharedbuffer.cpp`, `stillimageqt.cpp`, `styleimage.cpp`, `transformationmatrix.cpp` into `qlith-pro/src/`.
    *   Move `qlith-pro/src/gui/container_qt5.cpp`, `fontcache.cpp`, `main.cpp`, `mainwindow.cpp` into `qlith-pro/src/gui/`.
3.  **Move Header Files (`.h`)**:
    *   Move `affinetransform.h`, `bitmapimage.h`, `color.h`, `common.h`, `contextshadow.h`, `floatpoint.h`, `floatpoint3d.h`, `floatquad.h`, `floatrect.h`, `floatsize.h`, `gradient.h`, `graphicscontext.h`, `image.h`, `imagedecoder.h`, `imagedecoderqt.h`, `imageobserver.h`, `imagesource.h`, `intpoint.h`, `intrect.h`, `intsize.h`, `mimetyperegistry.h`, `mimetyperegistryqt.h`, `pathqt.h`, `pngimagedecoder.h`, `purgeablebuffer.h`, `rgba32bufferqt.h`, `shadowdata.h`, `sharedbuffer.h`, `stillimageqt.h`, `styleimage.h`, `transformationmatrix.h`, `vectortraits.h`, and the contents of `qlith-pro/include/` into `qlith-pro/include/qlith/`.
    *   Move `qlith-pro/src/gui/container_qt5.h`, `fontcache.h`, `mainwindow.h` into `qlith-pro/include/qlith/`. (Alternatively, keep GUI headers separate in `include/qlith/gui/` if desired).
4.  **Move Resource Files**:
    *   Ensure all contents of `qlith-pro/resources/` (`css/`, `font/`, `img/`, `mainwindow.ui`, `master.css`) are present.
    *   Move `qlith-pro/res.qrc` into `qlith-pro/resources/`.
5.  **Update Include Paths**:
    *   Search and replace include statements in all `.cpp` and `.h` files. Examples:
        *   `#include "affinetransform.h"` becomes `#include "qlith/affinetransform.h"`
        *   `#include "gui/container_qt5.h"` becomes `#include "qlith/container_qt5.h"` (or `qlith/gui/container_qt5.h` if kept separate)
        *   `#include "../include/litehtml.h"` needs careful adjustment based on how `litehtml` is included/found by CMake. Might become `#include "litehtml.h"` if `external/litehtml/include` is added to include path.
    *   Add `// this_file: qlith-pro/path/to/file.ext` comment near the top of each source/header file.
6.  **Create Root `CMakeLists.txt`**:
    *   Create `qlith-pro/CMakeLists.txt`.
    *   Define project name, CMake version, C++ standard.
    *   `find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)`
    *   `set(CMAKE_AUTOMOC ON)`
    *   `set(CMAKE_AUTORCC ON)`
    *   `set(CMAKE_AUTOUIC ON)`
    *   `include_directories(include ${CMAKE_CURRENT_BINARY_DIR})`
    *   Add `external/litehtml/include` and potentially `external/gumbo-parser/src` to include directories (adjust based on how these dependencies are handled).
    *   List all source files in `src/` and `src/gui/` using `file(GLOB ...)` or manual listing.
    *   Define the resource file: `set(RESOURCE_FILE resources/res.qrc)`
    *   Define the UI file: `set(UI_FILES resources/mainwindow.ui)`
    *   `qt5_wrap_ui(UI_HEADERS ${UI_FILES})`
    *   `add_executable(qlith-pro main.cpp ${SOURCE_FILES} ${UI_HEADERS} ${RESOURCE_FILE})`
    *   Find `litehtml` library (assuming it's built and installed or added as a subdirectory). Example: `find_package(litehtml REQUIRED)` or link to its target if built via `add_subdirectory`.
    *   `target_link_libraries(qlith-pro Qt5::Core Qt5::Gui Qt5::Widgets litehtml)` (adjust `litehtml` target name).
7.  **Cleanup Old Build Files**:
    *   Delete `qlith-pro/litehtml-qt.pro`.
    *   Delete `qlith-pro/containers/container-qt5.pri`.
    *   Delete `qlith-pro/src/gui/CMakeLists.txt`.
    *   Delete `qlith-pro/CMakeLists.txt` (the old one, if it existed at the root).
    *   Clean the `qlith-pro/build/` directory.
8.  **Update `.gitignore`**: Add `qlith-pro/build/` to the root `.gitignore` file.

## 5. Qt5 Compatibility Check

*   Review source code (`.cpp`, `.h`) for any APIs specific to Qt6 or APIs deprecated/removed in Qt5 compared to the version previously used. Pay attention to `QPainter`, `QWidget`, event handling, and core classes.
*   Ensure the `find_package(Qt5 ...)` call in CMake correctly locates the Qt5 installation.
*   Build against Qt5 and resolve any compilation errors.

## 6. macOS Build Instructions (CMake + Qt5)

1.  **Prerequisites**:
    *   Install Xcode Command Line Tools: `xcode-select --install`
    *   Install CMake: `brew install cmake` (or download from cmake.org)
    *   Install Qt5: `brew install qt@5` (Recommended) or use the Qt Online Installer.
2.  **Configure**:
    *   Open Terminal and navigate to the `qlith` project root directory.
    *   Create a build directory inside `qlith-pro`:
        ```bash
        cd qlith-pro
        mkdir build && cd build
        ```
    *   Run CMake, specifying the path to your Qt5 installation. If installed via Homebrew:
        ```bash
        cmake .. -DCMAKE_PREFIX_PATH=$(brew --prefix qt@5)
        ```
        (If installed elsewhere, replace `$(brew --prefix qt@5)` with the path to the Qt5 installation, e.g., `/Users/your_user/Qt/5.15.2/clang_64`)
3.  **Build**:
    *   Run make from the build directory:
        ```bash
        make -j$(sysctl -n hw.ncpu) # Use all available CPU cores
        ```
4.  **Run**:
    *   The executable will be inside the `build` directory (e.g., `qlith-pro/build/qlith-pro`).
        ```bash
        ./qlith-pro
        ```
    *   For a proper application bundle, you might need to use `macdeployqt` (part of Qt tools) after building.

## 7. Testing

*   Perform a clean CMake configuration and build on macOS with Qt5.
*   Verify the application launches without errors.
*   Test core HTML/CSS rendering functionality.
*   Test GUI interactions (scrolling, clicking links, etc.).
*   Verify resource loading (images, CSS).
