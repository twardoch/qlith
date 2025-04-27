
## 1. Analysis of `qlith-mini` (Working App)

`qlith-mini` serves as a functional baseline example of integrating the `litehtml` library with Qt5.

**Architecture:**

*   **Library (`qlith`):** A shared library containing the core rendering logic.
    *   `QlithWidget`: The central `QWidget` subclass responsible for displaying HTML content. It uses the PIMPL pattern (`QlithWidgetPrivate`) to hide implementation details.
    *   `QlithWidgetPrivate`: Manages the `litehtml::document`, handles loading via `QNetworkAccessManager`, interacts with the `ContainerQPainter`, and manages internal state (like layout needs).
    *   `ContainerQPainter`: The implementation of `litehtml::document_container`. This is the crucial bridge between `litehtml` and Qt. It uses `QPainter` directly for all rendering tasks (text, backgrounds, images, borders). It manages fonts (`QFont`, `QFontMetrics`) and basic image loading (`QImage`).
*   **Browser (`qlith_browser`):** A simple executable demonstrating the `qlith` library.
    *   `MainWindow`: A standard `QMainWindow` that embeds `QlithWidget`. It provides basic browser chrome (URL bar, back/forward/reload/stop buttons, menus) and history management.

**Key Features & Implementation Details:**

*   **Build System (CMake):** Modern CMake setup. Uses `find_package(Qt5 ...)` correctly. Defines separate library and executable targets. Handles Qt's MOC, UIC, RCC automatically. Uses `generate_export_header` for the library. Provides installation rules and CMake package configuration files (`qlithConfig.cmake.in`, `qlithTargets.cmake`). Specifies C++17.
*   **Graphics:** Relies *directly* on `QPainter` for all rendering within `ContainerQPainter`. Uses standard Qt classes like `QFont`, `QFontMetrics`, `QColor`, `QImage`, `QRect`, `QPoint`.
*   **litehtml Integration:** `ContainerQPainter` implements the `litehtml::document_container` virtual methods. These methods translate `litehtml` drawing commands and resource requests into Qt API calls (e.g., `draw_text` calls `painter->drawText`, `load_image` tries to load a `QImage` from a local file).
*   **Networking:** Uses `QNetworkAccessManager` within `QlithWidgetPrivate` to fetch the main HTML document. `ContainerQPainter` has basic local file image loading but lacks network loading for resources (images, CSS) referenced within the HTML, which is a limitation.
*   **Event Handling:** `QlithWidget` reimplements standard Qt event handlers (`paintEvent`, `resizeEvent`, `mousePressEvent`, etc.) and translates relevant events (like mouse clicks) into calls to the `litehtml::document`.
*   **Modern C++:** Uses standard library features (`std::shared_ptr` for `litehtml::document`), smart pointers (`QScopedPointer` for PIMPL), and C++17.
*   **Simplicity:** The codebase is relatively small and focused on the core task of rendering HTML via `litehtml` and `QPainter`.

**Overall Assessment:** `qlith-mini` is a clean, functional, and relatively modern example of using `litehtml` with Qt5. Its direct use of `QPainter` makes the rendering implementation straightforward to understand within a Qt context.

## 2. Analysis of `qlith-pro` (Non-Compiling App)

`qlith-pro` appears to be an attempt at a more feature-rich or perhaps differently architected HTML renderer, but it shows signs of being incomplete, adapted from another framework (likely WebKit), and not fully integrated with Qt5 in its current state.

**Architecture:**

*   **Monolithic Structure (as presented):** The top-level CMakeLists.txt defines a single executable target `${PROJECT_NAME}` (which would be `qlith-pro`). It doesn't define a separate library target like `qlith-mini` does, although the source structure suggests a potential library component was intended or exists implicitly.
*   **GUI Component:** Contains `gui/` subdirectory with `container_qt5.cpp`, `fontcache.cpp`, `main.cpp`, `mainwindow.cpp`. Uses Qt Designer (`mainwindow.ui`).
*   **Core Rendering Logic:** Spread across numerous files in `src/` (e.g., `graphicscontext.cpp`, `image.cpp`, `bitmapimage.cpp`, etc.) and corresponding headers in `include/qlith/`.
*   **Key Components (Differences from qlith-mini):**
    *   **`container_qt5`:** The likely `litehtml::document_container` implementation. Unlike `qlith-mini`'s `ContainerQPainter`, this seems designed to work with a custom `GraphicsContext` abstraction layer.
    *   **`GraphicsContext` Class:** A significant addition. This class acts as an *abstraction layer* over the native drawing context (presumably `QPainter`). It manages graphics state (colors, strokes, fills, shadows, transforms) much like WebKit's `GraphicsContext`. This is a major architectural difference from `qlith-mini`.
    *   **Custom Graphics Primitives:** An extensive set of classes mimicking typical 2D graphics APIs (`AffineTransform`, `TransformationMatrix`, `FloatPoint`, `FloatRect`, `FloatQuad`, `IntPoint`, `IntRect`, `IntSize`, `Color`, `Gradient`, `PathQt`). These classes seem intended to be used *by* `GraphicsContext` and `container_qt5`, rather than using Qt's native `QPointF`, `QRectF`, `QColor`, `QPainterPath`, `QTransform` directly in all places.
    *   **Image Subsystem:** A more complex set of image-related classes (`Image`, `BitmapImage`, `ImageSource`, `ImageDecoder`, `PNGImageDecoder`, `RGBA32BufferQt`, `StillImageQt`). This suggests a more involved pipeline, potentially aiming for animation support (`BitmapImage`) and a more generic decoder system. `RGBA32BufferQt` seems intended to wrap Qt image data (`QImage`/`QPixmap`).
    *   **Font Handling:** Includes a dedicated `FontCache` class.
    *   **UI:** Uses Qt Designer (`.ui` file) for the main window layout, unlike `qlith-mini` which sets up the UI programmatically.
    *   **Resources:** Includes Bootstrap CSS, suggesting a goal of rendering more complex, modern web layouts.

**Unique Aspects & Potential Problems:**

1.  **Graphics Abstraction Layer (`GraphicsContext`, custom geometry):**
    *   **Uniqueness:** This is the most significant difference. Instead of directly using `QPainter` in the `document_container`, `qlith-pro` routes drawing through its own `GraphicsContext` class.
    *   **Problem:** This layer adds complexity. Is it fully implemented using Qt5/`QPainter` primitives? Is it correctly implemented? Does it offer advantages over direct `QPainter` usage significant enough to justify the maintenance overhead? It's a likely source of compilation errors and runtime bugs if not perfectly ported/implemented. The implementations (`src/*.cpp`) need thorough review.
2.  **WebKit/Other Framework Influence:**
    *   **Uniqueness:** Class names (`BitmapImage`, `ImageSource`, `GraphicsContext`), structure, and comments in headers (mentioning CG, Cairo, Skia, WX, `wtf` namespace hints) strongly indicate that significant portions of the code were adapted or directly copied from WebKit or a similar browser engine framework.
    *   **Problem:** Code adapted from other frameworks often doesn't translate perfectly. It might rely on utility functions, macros, or architectural assumptions not present in a pure Qt environment. This leads to compilation errors (missing headers/types), linking errors, and runtime issues. It might not follow Qt best practices.
3.  **Incompleteness/Outdatedness:**
    *   **Uniqueness:** The sheer number of graphics and image classes suggests a more ambitious scope than `qlith-mini`, but the fact it doesn't compile implies it's unfinished or based on older, now incompatible, versions of dependencies (like litehtml or Qt itself, though it targets Qt5).
    *   **Problem:** Many methods in the `.cpp` files might be stubs, contain placeholder comments (`// TODO`), or have implementations tied to the original framework that haven't been ported to Qt. The `document_container` implementation (`container_qt5`) might not match the exact API requirements of the *current* version of `litehtml`.
4.  **Complex Image Pipeline:**
    *   **Uniqueness:** The multi-class image system (`Image`, `BitmapImage`, `ImageSource`, `ImageDecoder`, etc.) is more complex than `qlith-mini`'s direct use of `QImage`.
    *   **Problem:** This pipeline needs to be fully functional. `ImageSource` likely needs network capabilities (missing in `qlith-mini` too, but the structure here implies it might have been intended). `RGBA32BufferQt`'s interaction with `QImage`/`QPixmap` needs to be correct. Animation support (`BitmapImage`) adds complexity.
5.  **Build System:**
    *   **Uniqueness:** Defines a single executable target directly.
    *   **Problem:** While functional, it might be less modular than `qlith-mini`'s library approach. More importantly, it needs to correctly compile and link *all* the custom source files and dependencies. Missing source files or incorrect include paths could cause compilation failures.

**Overall Assessment:** `qlith-pro` is a potentially more capable but currently non-functional project. Its architecture, heavily influenced by frameworks like WebKit, introduces significant complexity, especially the custom graphics abstraction layer. The main challenge is bridging the gap between this adapted codebase and a pure Qt5 environment, ensuring all components are correctly implemented and interact properly with Qt and the current `litehtml` API.

## 3. Necessary Modifications for `qlith-pro`

### 3.1. Part 1

./build_macos.sh

```
+ BUILD_TYPE=Release
+++ dirname ./build_macos.sh
++ cd .
++ pwd
+ SCRIPT_DIR=/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro
+ PROJECT_ROOT=/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro
+ BUILD_DIR=/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/build
+ EXTERNAL_DIR=/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/../external
+ '[' '!' -d /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/../external/litehtml ']'
+ '[' '!' -d /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/../external/gumbo-parser ']'
++ brew --prefix qt@5
+ QT_PATH=/usr/local/opt/qt@5
+ '[' -z /usr/local/opt/qt@5 ']'
+ echo 'Using Qt5 from: /usr/local/opt/qt@5'
Using Qt5 from: /usr/local/opt/qt@5
+ mkdir -p /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/build
+ cd /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/build
+ echo 'Configuring project...'
Configuring project...
+ cmake /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/usr/local/opt/qt@5 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
-- The CXX compiler identification is AppleClang 17.0.0.17000013
-- The C compiler identification is AppleClang 17.0.0.17000013
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- qlith-pro configuration complete
-- Configuring done (1.4s)
-- Generating done (0.2s)
-- Build files have been written to: /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/build
+ echo 'Building project...'
Building project...
++ sysctl -n hw.ncpu
+ cmake --build . -- -j16
[  0%] Built target gumbo_autogen_timestamp_deps
[  1%] Automatic MOC and UIC for target gumbo
[  1%] Built target gumbo_autogen
[  2%] Linking CXX static library libgumbo.a
/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib: file: libgumbo.a(mocs_compilation.cpp.o) has no symbols
/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib: file: libgumbo.a(mocs_compilation.cpp.o) has no symbols
[ 12%] Built target gumbo
[ 12%] Built target litehtml_autogen_timestamp_deps
[ 13%] Automatic MOC and UIC for target litehtml
[ 13%] Built target litehtml_autogen
[ 13%] Linking CXX static library liblitehtml.a
/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib: file: liblitehtml.a(mocs_compilation.cpp.o) has no symbols
/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib: file: liblitehtml.a(mocs_compilation.cpp.o) has no symbols
[ 64%] Built target litehtml
[ 64%] Built target qlith-pro_autogen_timestamp_deps
[ 65%] Automatic MOC and UIC for target qlith-pro
[ 65%] Built target qlith-pro_autogen
[ 65%] Building CXX object CMakeFiles/qlith-pro.dir/src/gui/container_qt5.cpp.o
[ 66%] Building CXX object CMakeFiles/qlith-pro.dir/src/gui/main.cpp.o
[ 67%] Building CXX object CMakeFiles/qlith-pro.dir/src/floatsize.cpp.o
[ 68%] Building CXX object CMakeFiles/qlith-pro.dir/src/gui/mainwindow.cpp.o
[ 69%] Building CXX object CMakeFiles/qlith-pro.dir/src/graphicscontext.cpp.o
[ 70%] Building CXX object CMakeFiles/qlith-pro.dir/src/imagedecoder.cpp.o
[ 71%] Building CXX object CMakeFiles/qlith-pro.dir/src/imagedecoderqt.cpp.o
[ 73%] Building CXX object CMakeFiles/qlith-pro.dir/src/imagesource.cpp.o
[ 73%] Building CXX object CMakeFiles/qlith-pro.dir/src/imageobserver.cpp.o
[ 74%] Building CXX object CMakeFiles/qlith-pro.dir/src/intpoint.cpp.o
[ 76%] Building CXX object CMakeFiles/qlith-pro.dir/src/pathqt.cpp.o
[ 77%] Building CXX object CMakeFiles/qlith-pro.dir/src/intrect.cpp.o
[ 77%] Building CXX object CMakeFiles/qlith-pro.dir/src/intsize.cpp.o
[ 78%] Building CXX object CMakeFiles/qlith-pro.dir/src/mimetyperegistryqt.cpp.o
[ 78%] Building CXX object CMakeFiles/qlith-pro.dir/src/mimetyperegistry.cpp.o
[ 78%] Building CXX object CMakeFiles/qlith-pro.dir/src/pngimagedecoder.cpp.o
[ 79%] Building CXX object CMakeFiles/qlith-pro.dir/src/purgeablebuffer.cpp.o
[ 80%] Building CXX object CMakeFiles/qlith-pro.dir/src/rgba32bufferqt.cpp.o
[ 81%] Building CXX object CMakeFiles/qlith-pro.dir/src/shadowdata.cpp.o
[ 82%] Building CXX object CMakeFiles/qlith-pro.dir/src/sharedbuffer.cpp.o
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/gui/container_qt5.cpp:1:
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:10:46: error: base class has incomplete type
   10 | class container_qt5 : public QObject, public litehtml::document_container
      |                                       ~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/../external/litehtml/include/litehtml/web_color.h:9:8: note: forward declaration of 'litehtml::document_container'
    9 |         class document_container;
      |               ^
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/gui/container_qt5.cpp:1:
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:22:32: error: implicit instantiation of undefined template 'QHash<QString, QByteArray>'
   22 |     QHash<QString, QByteArray> m_loaded_css;
      |                                ^
/usr/local/opt/qt@5/lib/QtCore.framework/Headers/qdatastream.h:60:37: note: template is declared here
   60 | template <class Key, class T> class QHash;
      |                                     ^
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/gui/mainwindow.cpp:3:
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/mainwindow.h:8:
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:10:46: error: base class has incomplete type
   10 | class container_qt5 : public QObject, public litehtml::document_container
      |                                       ~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/../external/litehtml/include/litehtml/web_color.h:9:8: note: forward declaration of 'litehtml::document_container'
    9 |         class document_container;
      |               ^
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/gui/mainwindow.cpp:3:
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/mainwindow.h:8:
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:22:32: error: implicit instantiation of undefined template 'QHash<QString, QByteArray>'
   22 |     QHash<QString, QByteArray> m_loaded_css;
      |                                ^
/usr/local/opt/qt@5/lib/QtCore.framework/Headers/qdatastream.h:60:37: note: template is declared here
   60 | template <class Key, class T> class QHash;
      |                                     ^
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/gui/container_qt5.cpp:1:
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:90:41: error: no type named 'tstring' in namespace 'litehtml'
   90 |     virtual void get_language(litehtml::tstring& language, litehtml::tstring& culture) const override;
      |                               ~~~~~~~~~~^
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/gui/mainwindow.cpp:3:
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/mainwindow.h:8:
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:90:41: error: no type named 'tstring' in namespace 'litehtml'
   90 |     virtual void get_language(litehtml::tstring& language, litehtml::tstring& culture) const override;
      |                               ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:90:70: error: no type named 'tstring' in namespace 'litehtml'
   90 |     virtual void get_language(litehtml::tstring& language, litehtml::tstring& culture) const override;
      |                                                            ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:98:18: error: 'get_media_features' marked 'override' but does not override any member functions
   98 |     virtual void get_media_features(litehtml::media_features& media) const override;
      |                  ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:108:81: error: no type named 'tchar_t' in namespace 'litehtml'
  108 |     virtual std::shared_ptr< litehtml::element > create_element(const litehtml::tchar_t* tag_name, const litehtml::string_map& attributes, const std::shared_ptr< litehtml::document >& doc) override;
      |                                                                       ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:116:18: error: 'get_client_rect' marked 'override' but does not override any member functions
  116 |     virtual void get_client_rect(litehtml::position& client) const override;
      |                  ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:123:18: error: 'del_clip' marked 'override' but does not override any member functions
  123 |     virtual void del_clip() override;
      |                  ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:134:18: error: 'set_clip' marked 'override' but does not override any member functions
  134 |     virtual void set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius, bool valid_x, bool valid_y) override;
      |                  ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:90:70: error: no type named 'tstring' in namespace 'litehtml'
   90 |     virtual void get_language(litehtml::tstring& language, litehtml::tstring& culture) const override;
      |                                                            ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:98:18: error: 'get_media_features' marked 'override' but does not override any member functions
   98 |     virtual void get_media_features(litehtml::media_features& media) const override;
      |                  ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:144:39: error: no type named 'tstring' in namespace 'litehtml'
  144 |     virtual void import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl) override;
      |                             ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:108:81: error: no type named 'tchar_t' in namespace 'litehtml'
  108 |     virtual std::shared_ptr< litehtml::element > create_element(const litehtml::tchar_t* tag_name, const litehtml::string_map& attributes, const std::shared_ptr< litehtml::document >& doc) override;
      |                                                                       ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:116:18: error: 'get_client_rect' marked 'override' but does not override any member functions
  116 |     virtual void get_client_rect(litehtml::position& client) const override;
      |                  ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:123:18: error: 'del_clip' marked 'override' but does not override any member functions
  123 |     virtual void del_clip() override;
      |                  ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:134:18: error: 'set_clip' marked 'override' but does not override any member functions
  134 |     virtual void set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius, bool valid_x, bool valid_y) override;
      |                  ^
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/gui/main.cpp:5:
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:10:46: error: base class has incomplete type
   10 | class container_qt5 : public QObject, public litehtml::document_container
      |                                       ~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/../external/litehtml/include/litehtml/web_color.h:9:8: note: forward declaration of 'litehtml::document_container'
    9 |         class document_container;
      |               ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:144:70: error: no type named 'tstring' in namespace 'litehtml'
  144 |     virtual void import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl) override;
      |                                                            ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:144:39: error: no type named 'tstring' in namespace 'litehtml'
  144 |     virtual void import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl) override;
      |                             ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:144:94: error: no type named 'tstring' in namespace 'litehtml'
  144 |     virtual void import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl) override;
      |                                                                                    ~~~~~~~~~~^
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/gui/main.cpp:5:
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:90:41: error: no type named 'tstring' in namespace 'litehtml'
   90 |     virtual void get_language(litehtml::tstring& language, litehtml::tstring& culture) const override;
      |                               ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:144:70: error: no type named 'tstring' in namespace 'litehtml'
  144 |     virtual void import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl) override;
      |                                                            ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:153:43: error: no type named 'tstring' in namespace 'litehtml'
  153 |     virtual void transform_text(litehtml::tstring& text, litehtml::text_transform tt) override;
      |                                 ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:90:70: error: no type named 'tstring' in namespace 'litehtml'
   90 |     virtual void get_language(litehtml::tstring& language, litehtml::tstring& culture) const override;
      |                                                            ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:98:18: error: 'get_media_features' marked 'override' but does not override any member functions
   98 |     virtual void get_media_features(litehtml::media_features& media) const override;
      |                  ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:144:94: error: no type named 'tstring' in namespace 'litehtml'
  144 |     virtual void import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl) override;
      |                                                                                    ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:161:45: error: no type named 'tchar_t' in namespace 'litehtml'
  161 |     virtual void set_cursor(const litehtml::tchar_t* cursor) override;
      |                                   ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:108:81: error: no type named 'tchar_t' in namespace 'litehtml'
  108 |     virtual std::shared_ptr< litehtml::element > create_element(const litehtml::tchar_t* tag_name, const litehtml::string_map& attributes, const std::shared_ptr< litehtml::document >& doc) override;
      |                                                                       ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:116:18: error: 'get_client_rect' marked 'override' but does not override any member functions
  116 |     virtual void get_client_rect(litehtml::position& client) const override;
      |                  ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:123:18: error: 'del_clip' marked 'override' but does not override any member functions
  123 |     virtual void del_clip() override;
      |                  ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:134:18: error: 'set_clip' marked 'override' but does not override any member functions
  134 |     virtual void set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius, bool valid_x, bool valid_y) override;
      |                  ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:170:50: error: no type named 'tchar_t' in namespace 'litehtml'
  170 |     virtual void on_anchor_click(const litehtml::tchar_t* url, const litehtml::element::ptr& el) override;
      |                                        ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:179:18: error: 'link' marked 'override' but does not override any member functions
  179 |     virtual void link(const std::shared_ptr< litehtml::document >& doc, const litehtml::element::ptr& el) override;
      |                  ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:153:43: error: no type named 'tstring' in namespace 'litehtml'
  153 |     virtual void transform_text(litehtml::tstring& text, litehtml::text_transform tt) override;
      |                                 ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:144:39: error: no type named 'tstring' in namespace 'litehtml'
  144 |     virtual void import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl) override;
      |                             ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:187:47: error: no type named 'tchar_t' in namespace 'litehtml'
  187 |     virtual void set_base_url(const litehtml::tchar_t* base_url) override;
      |                                     ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:161:45: error: no type named 'tchar_t' in namespace 'litehtml'
  161 |     virtual void set_cursor(const litehtml::tchar_t* cursor) override;
      |                                   ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:195:46: error: no type named 'tchar_t' in namespace 'litehtml'
  195 |     virtual void set_caption(const litehtml::tchar_t* caption) override;
      |                                    ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:206:18: error: 'draw_borders' marked 'override' but does not override any member functions
  206 |     virtual void draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) override;
      |                  ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:170:50: error: no type named 'tchar_t' in namespace 'litehtml'
  170 |     virtual void on_anchor_click(const litehtml::tchar_t* url, const litehtml::element::ptr& el) override;
      |                                        ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:179:18: error: 'link' marked 'override' but does not override any member functions
  179 |     virtual void link(const std::shared_ptr< litehtml::document >& doc, const litehtml::element::ptr& el) override;
      |                  ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:144:70: error: no type named 'tstring' in namespace 'litehtml'
  144 |     virtual void import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl) override;
      |                                                            ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:187:47: error: no type named 'tchar_t' in namespace 'litehtml'
  187 |     virtual void set_base_url(const litehtml::tchar_t* base_url) override;
      |                                     ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:195:46: error: no type named 'tchar_t' in namespace 'litehtml'
  195 |     virtual void set_caption(const litehtml::tchar_t* caption) override;
      |                                    ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:206:18: error: 'draw_borders' marked 'override' but does not override any member functions
  206 |     virtual void draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) override;
      |                  ^
fatal error: too many errors emitted, stopping now [-ferror-limit=]
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:144:94: error: no type named 'tstring' in namespace 'litehtml'
  144 |     virtual void import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl) override;
      |                                                                                    ~~~~~~~~~~^
fatal error: too many errors emitted, stopping now [-ferror-limit=]
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:153:43: error: no type named 'tstring' in namespace 'litehtml'
  153 |     virtual void transform_text(litehtml::tstring& text, litehtml::text_transform tt) override;
      |                                 ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:161:45: error: no type named 'tchar_t' in namespace 'litehtml'
  161 |     virtual void set_cursor(const litehtml::tchar_t* cursor) override;
      |                                   ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:170:50: error: no type named 'tchar_t' in namespace 'litehtml'
  170 |     virtual void on_anchor_click(const litehtml::tchar_t* url, const litehtml::element::ptr& el) override;
      |                                        ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:179:18: error: 'link' marked 'override' but does not override any member functions
  179 |     virtual void link(const std::shared_ptr< litehtml::document >& doc, const litehtml::element::ptr& el) override;
      |                  ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:187:47: error: no type named 'tchar_t' in namespace 'litehtml'
  187 |     virtual void set_base_url(const litehtml::tchar_t* base_url) override;
      |                                     ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:195:46: error: no type named 'tchar_t' in namespace 'litehtml'
  195 |     virtual void set_caption(const litehtml::tchar_t* caption) override;
      |                                    ~~~~~~~~~~^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:206:18: error: 'draw_borders' marked 'override' but does not override any member functions
  206 |     virtual void draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) override;
      |                  ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/container_qt5.h:215:74: error: no type named 'background_paint' in namespace 'litehtml'
  215 |     virtual void draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint& bg) override;
      |                                                                ~~~~~~~~~~^
fatal error: too many errors emitted, stopping now [-ferror-limit=]
20 errors generated.
20 errors generated.
gmake[2]: *** [CMakeFiles/qlith-pro.dir/build.make:162: CMakeFiles/qlith-pro.dir/src/gui/mainwindow.cpp.o] Error 1
gmake[2]: *** Waiting for unfinished jobs....
gmake[2]: *** [CMakeFiles/qlith-pro.dir/build.make:148: CMakeFiles/qlith-pro.dir/src/gui/main.cpp.o] Error 1
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/pngimagedecoder.cpp:2:
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/pngimagedecoder.h:3:
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/imagedecoder.h:293:51: warning: unused parameter 'clearBeforeFrame' [-Wunused-parameter]
  293 |         virtual void clearFrameBufferCache(size_t clearBeforeFrame) { }
      |                                                   ^
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/imagedecoderqt.cpp:2:
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/imagedecoderqt.h:5:
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/imagedecoder.h:293:51: warning: unused parameter 'clearBeforeFrame' [-Wunused-parameter]
  293 |         virtual void clearFrameBufferCache(size_t clearBeforeFrame) { }
      |                                                   ^
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/imagesource.cpp:8:
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/pngimagedecoder.cppIn file included from :/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/imagedecoderqt.h5::510:
: fatal error: /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/imagedecoder.h:293'png.h' file not found:51
: warning: unused parameter 'clearBeforeFrame' [-Wunused-parameter]
    5 | #include <png.h>
      |          ^~~~~~~
  293 |         virtual void clearFrameBufferCache(size_t clearBeforeFrame) { }
      |                                                   ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/mimetyperegistryqt.cpp:58:66: warning: unused parameter 'mimeType' [-Wunused-parameter]
   58 | bool MIMETypeRegistry::isApplicationPluginMIMEType(const String& mimeType)
      |                                                                  ^
1 warning and 1 error generated.
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/floatsize.cpp:2:
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/floatsize.h:4:
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/common.h:46:
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/floatpoint.h:77:52: error: unknown type name 'FloatSize'
   77 | inline FloatPoint& operator+=(FloatPoint& a, const FloatSize& b)
      |                                                    ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/imagedecoderqt.cpp:90:38: warning: comparison of integers of different signs: 'size_t' (aka 'unsigned long') and 'int' [-Wsign-compare]
   90 |                 for (size_t i = 0; i < m_frameBufferCache.size(); ++i)
      |                                    ~ ^ ~~~~~~~~~~~~~~~~~~~~~~~~~
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/imagedecoderqt.cpp:227:26: warning: comparison of integers of different signs: 'size_t' (aka 'unsigned long') and 'int' [-Wsign-compare]
  227 |     for (size_t i = 0; i < m_frameBufferCache.size(); ++i)
      |                        ~ ^ ~~~~~~~~~~~~~~~~~~~~~~~~~
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/floatpoint.h:89:52: error: unknown type name 'FloatSize'
   89 | inline FloatPoint& operator-=(FloatPoint& a, const FloatSize& b)
      |                                                    ^
gmake[2]: *** [CMakeFiles/qlith-pro.dir/build.make:498: CMakeFiles/qlith-pro.dir/src/pngimagedecoder.cpp.o] Error 1
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/floatpoint.h:95:56: error: unknown type name 'FloatSize'
   95 | inline FloatPoint operator+(const FloatPoint& a, const FloatSize& b)
      |                                                        ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/floatpoint.h:105:8: error: unknown type name 'FloatSize'
  105 | inline FloatSize operator-(const FloatPoint& a, const FloatPoint& b)
      |        ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/floatpoint.h:107:12: error: use of undeclared identifier 'FloatSize'
  107 |     return FloatSize(a.x() - b.x(), a.y() - b.y());
      |            ^
In file included from /Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/rgba32bufferqt.cpp:5:
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/imagedecoder.h:293:51: warning: unused parameter 'clearBeforeFrame' [-Wunused-parameter]
  293 |         virtual void clearFrameBufferCache(size_t clearBeforeFrame) { }
      |                                                   ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/include/qlith/floatpoint.h:110:56: error: unknown type name 'FloatSize'
  110 | inline FloatPoint operator-(const FloatPoint& a, const FloatSize& b)
      |                                                        ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/graphicscontext.cpp:955:90: warning: unused parameter 'antialiased' [-Wunused-parameter]
  955 | void GraphicsContext::clipConvexPolygon(size_t numPoints, const FloatPoint* points, bool antialiased)
      |                                                                                          ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/graphicscontext.cpp:1174:86: warning: unused parameter 'colorSpace' [-Wunused-parameter]
 1174 | void GraphicsContext::fillRect(const FloatRect& rect, const Color& color, ColorSpace colorSpace)
      |                                                                                      ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/graphicscontext.cpp:1202:195: warning: unused parameter 'colorSpace' [-Wunused-parameter]
 1202 | void GraphicsContext::fillRoundedRect(const IntRect& rect, const IntSize& topLeft, const IntSize& topRight, const IntSize& bottomLeft, const IntSize& bottomRight, const Color& color, ColorSpace colorSpace)
      |                                                                                                                                                                                                   ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/graphicscontext.cpp:1281:32: error: use of undeclared identifier 'color'
 1281 |     if (paintingDisabled() || !color.isValid())
      |                                ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/graphicscontext.cpp:1284:26: error: use of undeclared identifier 'rects'
 1284 |     unsigned rectCount = rects.size();
      |                          ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/graphicscontext.cpp:1286:10: error: use of undeclared identifier 'rects'
 1286 |     if (!rects.size())
      |          ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/graphicscontext.cpp:1300:19: error: use of undeclared identifier 'color'
 1300 |     nPen.setColor(color);
      |                   ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/graphicscontext.cpp:1314:28: error: use of undeclared identifier 'rects'; did you mean 'gets'?
 1314 |         p->drawRect(QRectF(rects[i]));
      |                            ^~~~~
      |                            gets
/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX15.4.sdk/usr/include/_stdio.h:262:18: note: 'gets' declared here
  262 | char *_LIBC_CSTR        gets(char *_LIBC_UNSAFE_INDEXABLE) _LIBC_PTRCHECK_REPLACED(fgets);
      |                         ^
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/graphicscontext.cpp:1314:28: error: subscript of pointer to function type 'char *(char *)'
 1314 |         p->drawRect(QRectF(rects[i]));
      |                            ^~~~~
1 warning generated.
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/pathqt.cpp:97:25: warning: unused function 'scratchContext' [-Wunused-function]
   97 | static GraphicsContext* scratchContext()
      |                         ^~~~~~~~~~~~~~
6 errors generated.
gmake[2]: *** [CMakeFiles/qlith-pro.dir/build.make:302: CMakeFiles/qlith-pro.dir/src/floatsize.cpp.o] Error 1
1 warning generated.
1 warning generated.
3 warnings and 6 errors generated.
1 warning generated.
gmake[2]: *** [CMakeFiles/qlith-pro.dir/build.make:330: CMakeFiles/qlith-pro.dir/src/graphicscontext.cpp.o] Error 1
20 errors generated.
gmake[2]: *** [CMakeFiles/qlith-pro.dir/build.make:120: CMakeFiles/qlith-pro.dir/src/gui/container_qt5.cpp.o] Error 1
3 warnings generated.
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/sharedbuffer.cpp:141:28: warning: comparison of integers of different signs: 'unsigned int' and 'int' [-Wsign-compare]
  141 |     for (unsigned i = 0; i < m_segments.size(); ++i)
      |                          ~ ^ ~~~~~~~~~~~~~~~~~
/Users/adam/Developer/vcs/github.twardoch/pub/qlith/qlith-pro/src/sharedbuffer.cpp:187:32: warning: comparison of integers of different signs: 'unsigned int' and 'int' [-Wsign-compare]
  187 |         for (unsigned i = 0; i < m_segments.size(); ++i) {
      |                              ~ ^ ~~~~~~~~~~~~~~~~~
2 warnings generated.
gmake[1]: *** [CMakeFiles/Makefile2:140: CMakeFiles/qlith-pro.dir/all] Error 2
gmake: *** [Makefile:136: all] Error 2
```


### 3.2. Part 2

To make `qlith-pro` work, focusing on using Qt5 and modern `litehtml`, the following modifications are essential:

1.  **Simplify/Fix the Graphics Layer:**
    *   **Recommended:** *Replace* the custom `GraphicsContext` abstraction. Refactor `container_qt5` to use `QPainter` *directly* for drawing operations, similar to `qlith-mini`'s `ContainerQPainter`. This leverages Qt's mature 2D API and removes a major source of complexity and potential bugs.
    *   **Alternative (Harder):** *Complete and debug* the existing `GraphicsContext` implementation, ensuring all its methods correctly map to `QPainter` calls. This preserves the original architecture but is much more work and harder to maintain.
2.  **Use Qt Primitives:** Wherever the custom graphics classes (`FloatRect`, `IntPoint`, `AffineTransform`, etc.) are used, especially within `container_qt5` and the UI, replace them with their idiomatic Qt counterparts (`QRectF`, `QPoint`, `QTransform`, `QColor`, `QPainterPath`) unless the custom class provides essential functionality not available in Qt that `litehtml` *absolutely requires*.
3.  **Update `container_qt5`:**
    *   Ensure its implementation of `litehtml::document_container` methods matches the signatures and requirements of the version of `litehtml` being used.
    *   Modify drawing methods (`draw_text`, `draw_background`, `draw_borders`, etc.) to use `QPainter` (either directly or via a *working* `GraphicsContext`). Adapt logic from `qlith-mini`'s `ContainerQPainter` where applicable.
    *   Implement or fix resource loading (`load_image`, `import_css`) using `QNetworkAccessManager` (like `qlith-mini`) or ensure the existing `ImageSource`/`ImageDecoder` pipeline works correctly for network resources.
4.  **Fix the Image Pipeline:**
    *   Ensure `ImageSource`, `ImageDecoderQt`, `BitmapImage`, etc., compile and function correctly.
    *   Verify `RGBA32BufferQt` correctly handles `QImage`/`QPixmap` data, especially regarding color formats (premultiplied alpha).
    *   Consider if this complex pipeline is necessary initially. Perhaps start with simpler image handling like `qlith-mini` and add complexity later if needed (e.g., for animations).
5.  **Build System Cleanup:**
    *   Ensure `CMakeLists.txt` correctly finds Qt5 (check paths, maybe use `find_package(Qt5 ... REQUIRED)` components more explicitly).
    *   Ensure all necessary source files are included in the target.
    *   Ensure correct include directories are set for `qlith` headers, `litehtml` headers, and generated UI headers.
    *   Link against all necessary Qt modules (Core, Gui, Widgets, Network are present; Svg might be needed if SVG images or paths are used extensively).
6.  **Code Modernization & Cleanup:**
    *   Remove dead code, especially commented-out sections or `#ifdef` blocks related to other platforms (CG, Cairo, etc.).
    *   Replace raw pointers with smart pointers (`std::unique_ptr`, `std::shared_ptr`, `QScopedPointer`) where appropriate.
    *   Ensure consistency with C++17 standard.
    *   Address any Qt5 deprecation warnings.
7.  **UI Integration:** Ensure `MainWindow` correctly creates and manages the `litehtmlWidget` and connects UI elements (if any planned beyond display) to appropriate slots.

## 4. Detailed Plan for Junior Developer

This plan focuses on the **recommended strategy** of replacing the custom graphics abstraction with direct Qt usage, leveraging `qlith-mini` as a guide.

**Goal:** Make `qlith-pro` compile, run, and render a basic HTML page using Qt5 and `litehtml`.

**Prerequisites:**

*   Basic understanding of C++ (including classes, pointers, basic templates).
*   Basic understanding of Qt5 (Widgets, Signals/Slots, `QPainter`, `QImage`, `QNetworkAccessManager`).
*   Familiarity with CMake build system.
*   Git for version control (commit often!).
*   A working C++ compiler and Qt5 development environment.
*   The `qlith-pro` and `qlith-mini` source code, plus the `litehtml` dependency.

**High-Level Strategy:**

1.  Set up the build environment and ensure basic compilation (even with errors).
2.  Refactor `container_qt5` to use `QPainter` directly, removing the `GraphicsContext` layer.
3.  Replace custom geometry/graphics types with Qt types where possible.
4.  Simplify or fix the image loading mechanism.
5.  Integrate the `litehtmlWidget` with the `MainWindow`.
6.  Clean up and test incrementally.

**Detailed Steps:**

**(Phase 1: Build System & Basic Structure)**

1.  **Goal:** Get the project to start compiling, even if it fails with many errors initially.
    *   **Files:** `CMakeLists.txt`, `build_macos.sh` (or equivalent build commands).
    *   **Actions:**
        *   Carefully review `CMakeLists.txt`. Ensure `find_package(Qt5 ...)` points to the correct Qt5 installation. Add `Gui` explicitly if missing: `find_package(Qt5 5.15 COMPONENTS Core Gui Widgets Network REQUIRED)`.
        *   Ensure `target_include_directories` includes the `include`, `include/qlith`, `src`, `src/gui`, and `${CMAKE_CURRENT_BINARY_DIR}` paths correctly.
        *   Ensure `target_link_libraries` includes `Qt5::Core`, `Qt5::Gui`, `Qt5::Widgets`, `Qt5::Network`, `litehtml`, and `gumbo`.
        *   Ensure all `.cpp` files listed in `set(SOURCES ...)` actually exist and are needed.
        *   Run the build script (`./build_macos.sh` or `cmake .. && make`). Expect errors, but aim for CMake configuration to succeed and compilation to start.
    *   **Tip:** Focus on fixing "file not found" or "undefined reference" errors related to CMake setup first.

2.  **Goal:** Stub out or comment out problematic code to achieve an initial link, even if the app crashes immediately.
    *   **Files:** Primarily `src/*.cpp`, `src/gui/*.cpp`.
    *   **Actions:**
        *   Identify major compilation error sources. Likely candidates are the custom graphics classes and `GraphicsContext`.
        *   Temporarily comment out the *bodies* of functions causing hard errors, leaving just the function signature and a `return;` or `return {};` statement. *Do not* comment out entire classes or headers yet unless absolutely necessary.
        *   Try compiling again. Repeat until it links or the errors become more manageable (e.g., focused within specific classes).
    *   **Tip:** Commit this "stubbed out" state so you can revert if needed.

**(Phase 2: Refactoring the Container & Graphics)**

3.  **Goal:** Start refactoring `container_qt5` to use `QPainter` directly, removing the `GraphicsContext` dependency.
    *   **Files:** `src/gui/container_qt5.cpp`, `include/qlith/container_qt5.h`, `src/graphicscontext.cpp`, `include/qlith/graphicscontext.h`. Reference: `qlith-mini/src/container_qpainter.cpp`, `qlith-mini/include/container_qpainter.h`.
    *   **Actions:**
        *   Modify `container_qt5::draw_text`. Instead of taking `litehtml::uint_ptr hdc`, change it (and the corresponding `litehtml::document_container` override) to potentially take a `QPainter*` or manage a member `QPainter*` pointer set via `beginPaint`/`endPaint` methods (similar to `qlith-mini`).
        *   Implement `draw_text` using `painter->setFont()` and `painter->drawText()`. You'll need to manage fonts (see Step 4). Use `QColor` directly. Replace uses of custom `Color` class.
        *   Modify `container_qt5::create_font`. Adapt the logic from `qlith-mini`'s `ContainerQPainter::create_font`. Use `QFont` and `QFontMetrics`. Return a `uintptr_t` representing the `QFont*` or an index into a font cache you create within `container_qt5`. Use the existing `FontCache` class or simplify like `qlith-mini`.
        *   Implement `container_qt5::delete_font` and `container_qt5::text_width` using `QFont` and `QFontMetrics`.
        *   Modify `container_qt5::draw_background`. Implement it using `painter->fillRect()` for solid colors and `painter->drawImage()` or `painter->drawTiledPixmap()` for images (see Step 5). Replace uses of custom `Color`.
        *   Modify `container_qt5::draw_borders`. Implement it using `painter->setPen()`, `painter->drawLine()`, potentially `painter->drawRect()`. Adapt logic for different border styles (`Qt::PenStyle`). Replace uses of custom `Color`.
        *   Gradually remove calls to the custom `GraphicsContext` class from within `container_qt5`.
        *   Replace uses of custom geometry types (`FloatRect`, `IntPoint`, etc.) within `container_qt5` methods with `QRectF`, `QPointF`, `QPoint`, `QSizeF`, etc.
    *   **Tip:** Focus on one drawing method at a time (e.g., `draw_text`). Get it working with `QPainter` before moving to the next. Compile *very* frequently.

4.  **Goal:** Manage fonts using Qt primitives.
    *   **Files:** `src/gui/container_qt5.cpp`, `include/qlith/container_qt5.h`, `src/gui/fontcache.cpp`, `include/qlith/fontcache.h`. Reference: `qlith-mini/src/container_qpainter.cpp`.
    *   **Actions:**
        *   Decide whether to keep `FontCache` or use a simpler `QMap<uintptr_t, QFont>` inside `container_qt5` like `qlith-mini`. For simplicity now, the `QMap` approach is easier.
        *   Ensure `create_font` correctly creates `QFont` objects based on `litehtml` parameters and stores them (or a pointer/ID) associated with the returned `uintptr_t`.
        *   Ensure `delete_font` correctly removes/releases the font resources.
        *   Ensure `text_width` and `draw_text` correctly retrieve and use the `QFont` based on the passed `uintptr_t`.
        *   Ensure `get_default_font_name` and `get_default_font_size` return appropriate values.

5.  **Goal:** Handle image loading and drawing using Qt primitives.
    *   **Files:** `src/gui/container_qt5.cpp`, `include/qlith/container_qt5.h`, `src/image*.cpp`, `include/qlith/image*.h`, `src/rgba32bufferqt.*`. Reference: `qlith-mini/src/container_qpainter.cpp`, `qlith-mini/src/qlithwidget.cpp` (for `QNetworkAccessManager`).
    *   **Actions:**
        *   Simplify the image loading first. In `container_qt5::load_image`, implement basic loading using `QImage::load()` for local files, similar to `qlith-mini`. Store loaded images in a `QMap<QString, QImage>`.
        *   Implement `container_qt5::get_image_size` to return dimensions from the cached `QImage`.
        *   Implement `draw_background` (for image backgrounds) and `draw_list_marker` (for image markers) using `painter->drawImage()` or `painter->drawTiledPixmap()` with the cached `QImage`.
        *   Later, add network loading: Introduce `QNetworkAccessManager` (perhaps in `container_qt5` or a dedicated helper class) to fetch images. Connect its `finished` signal to a slot that processes the reply, creates a `QImage`, caches it, and triggers a widget update. Adapt the `pendingResources` map concept from `qlith-mini` if needed to track ongoing requests.
        *   Defer fixing/using the complex `BitmapImage`/`ImageSource` pipeline unless absolutely necessary for basic functionality (it's likely overkill initially).
    *   **Tip:** Get local images drawing first, then tackle network loading.

**(Phase 3: Widget Integration & UI)**

6.  **Goal:** Ensure `litehtmlWidget` correctly uses `container_qt5` and handles events.
    *   **Files:** `src/gui/container_qt5.cpp`, `include/qlith/container_qt5.h`, `src/gui/mainwindow.cpp`, `include/qlith/mainwindow.h`, `resources/mainwindow.ui`. Reference: `qlith-mini/src/qlithwidget.cpp`.
    *   **Actions:**
        *   Ensure `litehtmlWidget::paintEvent` correctly calls `container->repaint(painter)`.
        *   Ensure `litehtmlWidget` mouse events (`mouseMoveEvent`, `mousePressEvent`, `mouseReleaseEvent`) correctly translate coordinates (accounting for scrolling, like in `container_qt5::repaint`) and call the appropriate `container->getDocument()->on_...` methods. Trigger `update()` or `repaint()` if `litehtml` indicates redraws are needed.
        *   Ensure `litehtmlWidget::resizeEvent` calls `container->getDocument()->media_changed()` to trigger potential relayouts.
        *   In `MainWindow`, ensure the `litehtmlWidget` is created and placed correctly in the layout defined by `mainwindow.ui`.
        *   Load a simple test HTML string or local file into the document in `MainWindow`'s constructor or an initialization function. Use `litehtml::document::createFromString(...)`.
        *   Connect scrollbars (if using the UI version) or implement wheel events in `litehtmlWidget` to update the scroll position in `container_qt5` and trigger repaints.

**(Phase 4: Cleanup & Testing)**

7.  **Goal:** Remove unused code and perform basic tests.
    *   **Files:** All `.cpp` and `.h` files.
    *   **Actions:**
        *   Delete the `GraphicsContext` class files and other unused custom graphics primitive files if the refactor was successful.
        *   Remove commented-out code blocks that are no longer needed.
        *   Remove `#ifdef` blocks related to other platforms.
        *   Compile with high warning levels (`-Wall -Wextra`) and fix warnings.
        *   Test rendering of simple HTML (headings, paragraphs, links, basic CSS).
        *   Test rendering of local images.
        *   Test basic interactions (link clicking, scrolling).
        *   Test rendering with the included `bootstrap.css` (expect visual glitches, but it should parse and apply some styles).
    *   **Tip:** Use a simple `test.html` file locally for consistent testing.

**General Tips for the Junior Developer:**

*   **Compile Frequently:** After almost every small change, try to compile. It's easier to fix one error than fifty.
*   **Version Control:** Use Git. Commit small, logical changes with clear messages. If something breaks badly, you can revert.
*   **Use the Debugger:** Learn to use your IDE's debugger (or gdb/lldb) to step through code, inspect variables, and understand crashes.
*   **Read the Errors:** Compiler errors often tell you exactly what's wrong (missing semicolon, unknown type, wrong function signature).
*   **Consult `qlith-mini`:** It's your working example. Compare how it solves similar problems (drawing text, handling events, integrating with litehtml).
*   **Consult Qt Documentation:** Use Qt Assistant or the online docs extensively for `QPainter`, `QFont`, `QImage`, `QNetworkAccessManager`, etc.
*   **Consult `litehtml` Documentation/Examples:** Understand the `document_container` interface and how `litehtml` expects it to behave.
*   **Start Simple:** Don't try to fix everything at once. Get basic text rendering working, then backgrounds, then borders, then images.
*   **Ask Questions:** If stuck, ask a senior developer, explaining what you've tried and what the specific problem is.

