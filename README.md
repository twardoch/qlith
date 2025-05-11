# Qlith - A Qt5-based Lightweight HTML Widget and Browser

Qlith is a Qt5-based widget and tiny browser that integrates the [litehtml](https://github.com/litehtml/litehtml) lightweight HTML rendering engine and [gumbo-parser](https://codeberg.org/gumbo-parser/gumbo-parser) for HTML parsing.

## 1. Features

- Lightweight HTML rendering through litehtml integration
- Fast HTML parsing with gumbo-parser
- Qt5 widget for easy integration into your applications
- Simple browser application for testing and previewing HTML content
- Support for basic CSS styling
- Image loading and rendering
- Link navigation

## 2. Requirements

- Qt 5.12 or newer
- CMake 3.10 or newer
- C++17 compatible compiler

## 3. Building

### 3.1. Building with system-provided litehtml and gumbo-parser

```bash
mkdir build && cd build
cmake .. -DUSE_SYSTEM_LITEHTML=ON -DUSE_SYSTEM_GUMBO=ON
make
```

### 3.2. Building with bundled litehtml and gumbo-parser

```bash
mkdir build && cd build
cmake .. -DLITEHTML_SOURCE_DIR=path/to/litehtml -DGUMBO_SOURCE_DIR=path/to/gumbo-parser
make
```

## 4. Usage

### 4.1. Using the QlithWidget in Your Application

```cpp
#include <QApplication>
#include <qlith/qlithwidget.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QlithWidget *widget = new QlithWidget();
    widget->resize(800, 600);
    widget->setHtml("<html><body><h1>Hello, Qlith!</h1></body></html>");
    widget->show();
    
    return app.exec();
}
```

### 4.2. Running the Browser

```bash
./qlith_browser [url]
```

## 5. License

Qlith is distributed under the same license as litehtml ([New BSD License](https://opensource.org/licenses/BSD-3-Clause)) and follows the licensing terms of its components:

- **litehtml**: [New BSD License](https://opensource.org/licenses/BSD-3-Clause)
- **gumbo-parser**: [Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0)

## 6. Acknowledgments

Qlith is a refactored and reorganized version of qlitehtml, designed to better incorporate litehtml and gumbo-parser.

- [litehtml](https://github.com/litehtml/litehtml)
- [gumbo-parser](https://codeberg.org/gumbo-parser/gumbo-parser)
- [qlitehtml](https://github.com/qlitehtml/qlitehtml) (original project) 

## 7. Qlith-Pro vs. Qlith-Mini: Detailed Analysis of Principal Differences

This analysis outlines the principal differences between `qlith-pro` and `qlith-mini`. The core distinction lies in their complexity, feature sets, and the depth of their respective rendering pipelines. `qlith-mini` presents as a lightweight HTML rendering widget, primarily acting as a straightforward wrapper around the LiteHTML parsing/layout engine and Qt's QPainter for drawing. In contrast, `qlith-pro` is a significantly more feature-rich and complex system, incorporating a more extensive custom graphics library and a deeper rendering stack on top of LiteHTML and Qt.

### 7.1. Core Rendering and Graphics Engine: A Deeper Dive

The most fundamental difference lies in how each project translates LiteHTML's rendering instructions into visible output.

#### 7.1.1. qlith-mini:

* Direct QPainter Integration: The ContainerQPainter class (`src/container_qpainter.cpp`) is the heart of its rendering. When LiteHTML needs to draw text, an image, or a background, it calls methods on ContainerQPainter. These methods then directly invoke corresponding QPainter functions. For example, `draw_text` in ContainerQPainter sets the font and color on the QPainter and calls QPainter::drawText.
* Basic Resource Handling: Font management involves creating QFont objects on-the-fly based on LiteHTML's font descriptions. Image loading is basic, typically involving loading files into QImage objects. There's no significant abstraction beyond what Qt's classes provide.
* Limited Abstraction: This direct approach is simple and efficient for basic rendering tasks but offers less flexibility for complex graphical operations or optimizations. The rendering capabilities are essentially a subset of what QPainter can do, as dictated by LiteHTML's drawing model.

#### 7.1.2. qlith-pro:

* Abstracted Graphics Pipeline: The `container_qt5` class (`src/gui/container_qt5.cpp`) also implements `litehtml::document_container`. However, instead of always calling QPainter directly, it often interacts with its own GraphicsContext class (`include/qlith/GraphicsContext.h`).
* GraphicsContext as an Intermediary: GraphicsContext acts as an abstraction layer. While it ultimately uses QPainter for the final pixel drawing, it provides a richer, more specialized API. This allows qlith-pro to:

* Implement more complex drawing logic before a QPainter call.
* Manage graphics state (like fill styles, stroke styles, shadows, transformations) in a more structured way.
* Potentially optimize drawing calls or batch operations.

* Specialized Graphics Classes:

* Transformations (AffineTransform.h, TransformationMatrix.h): These classes provide capabilities for complex 2D (and potentially 3D, though the focus seems 2D) transformations beyond simple QPainter::translate/scale/rotate. This is crucial for accurately rendering CSS transforms (e.g., transform: matrix(...), skew(), perspective()). TransformationMatrix suggests a full 4x4 matrix implementation, allowing for perspective and other advanced 3D-like effects on a 2D plane.
* Image Subsystem (Image.h, BitmapImage.h, ImageDecoder.h, etc.): qlith-pro has a dedicated image subsystem. Image is likely an abstract base class, with BitmapImage (and StillImageQt) as concrete implementations. The presence of ImageDecoder (and PNGImageDecoder) indicates a custom image decoding pipeline. This allows for:

* Optimized loading and caching of images.
* Handling of animated images (as suggested by FrameData in BitmapImage.h).
* Potentially supporting image formats or features not natively handled by QImage or handling them differently.
* Managing decoded image data more granularly (e.g., decodedSize() for memory tracking).

* Advanced Styling Primitives (Gradient.h, ContextShadow.h, PathQt.h):

* Gradient: Provides a more object-oriented way to define and use gradients (linear, radial, conic as seen in container_qt5.cpp) than constructing QGradient directly for every use. It can manage color stops and spread methods more explicitly.
* ContextShadow: Encapsulates shadow properties (color, blur, offset) and the logic to render them. This is more sophisticated than simple drop shadows and is essential for CSS box-shadow or text-shadow.
* PathQt: Wraps QPainterPath, likely to integrate path operations more smoothly within the GraphicsContext and other parts of the custom rendering engine. This is used for drawing complex vector shapes, clipping, etc.

* Resource Management (FontCache.h, PurgeableBuffer.h, SharedBuffer.h):

* FontCache: Provides a centralized way to load, store, and retrieve fonts, which can be more efficient than creating QFont objects repeatedly.
* PurgeableBuffer and SharedBuffer: These suggest more advanced memory management techniques for large data blobs, like image data. "Purgeable" implies that memory can be reclaimed under pressure and potentially reloaded, crucial for handling many large resources.

### 7.2. Feature Set and Capabilities: Elaboration

The differing graphics engines lead to a disparity in rendering capabilities and overall features.

#### 7.2.1. qlith-mini:

* Core HTML/CSS: It can render HTML structure and apply CSS as interpreted by LiteHTML. Styling for text, basic backgrounds, borders, and images is supported via ContainerQPainter's direct use of QPainter.
* Application Shell: The browser is functional for basic navigation (URL input, back/forward, reload, stop). It handles history and basic settings (window geometry).
* Graphics Limitations: Complex CSS features like advanced transforms, detailed shadows, non-trivial gradients, or sophisticated image manipulations are likely not supported or are rendered simplistically, as they would require more than basic QPainter calls mapped from LiteHTML.

#### 7.2.2. qlith-pro:

* Enhanced HTML/CSS Rendering: The custom graphics pipeline allows for more faithful and complete rendering of modern CSS.

* CSS Transforms: AffineTransform.h and TransformationMatrix.h are key to implementing CSS 2D and potentially 3D transforms (e.g., rotate3d, scale3d, perspective).
* CSS Shadows & Gradients: ContextShadow.h and Gradient.h enable richer box-shadow, text-shadow, and background-image: gradient(...) effects.
* Image Rendering: The custom image pipeline (Image.h, BitmapImage.h) would better handle various image formats, animated GIFs (implied by frame management), and potentially CSS image rendering properties like object-fit or advanced tiling/stretching if implemented.

* Resource Handling:

* FontCache.h suggests optimized font loading and management, crucial for pages with diverse typography.
* PurgeableBuffer.h and SharedBuffer.h point to more robust handling of binary data (like images), potentially improving performance and reducing memory footprint on pages with many or large images.

* Application Shell & UI:

* The use of mainwindow.ui (Qt Designer) suggests a more visually structured and potentially more feature-rich UI for the browser application itself, compared to qlith-mini's programmatically defined UI.
* The litehtmlWidget is likely a more refined and capable central component for HTML display.

* Modern Web Layouts: The inclusion of bootstrap.css and reset.css in resources/css/ strongly indicates that qlith-pro is designed to handle complex, modern web layouts that rely on such frameworks for styling and structure. This implies a higher degree_of CSS compatibility.

### 7.3. Project Structure and Complexity: Implications

#### 7.3.1. qlith-mini:

* Simplicity: The straightforward structure (core logic in QlithWidget and ContainerQPainter) makes it relatively easy to understand, integrate, and modify for basic HTML display needs.
* Maintainability for Scope: For its limited scope, maintainability is high due to fewer components.
* Limited Extensibility: Adding significantly advanced graphics features would require substantial rewrites or bolting on new systems, as the foundation is minimal.

#### 7.3.2. qlith-pro:

* Modularity: The separation of concerns (graphics engine in include/qlith/, UI in src/gui/, etc.) makes the larger codebase more manageable.
* Extensibility: The abstracted graphics pipeline (GraphicsContext, Image subsystem) allows for easier addition of new graphical features, image decoders, or rendering optimizations without overhauling the entire system.
* Steeper Learning Curve: The larger number of classes and interactions means a higher initial effort to understand the entire system.
* Maintainability for Complexity: While more complex, the modularity can aid maintainability for its advanced feature set, as changes can often be localized to specific subsystems.

### 7.4. Dependencies and Build System

* Both projects share core dependencies: Qt5 (Widgets, Network, Core, Gui, Svg) and LiteHTML (which includes gumbo-parser).

#### 7.4.1. qlith-mini:

* CMakeLists.txt explicitly links ZLIB.
* The build process is simpler, reflecting the project's nature.

#### 7.4.2. qlith-pro:

* CMakeLists.txt is more involved, handling UI files (mainwindow.ui), resource files (res.qrc), and a more complex source tree.
* It also builds litehtml as a subdirectory.
* The "dependency" on its own extensive internal graphics library is the most significant difference here.

### 7.5. UI and Application Components

#### 7.5.1. qlith-mini:

* The main rendering widget is QlithWidget.
* The browser application (browser/MainWindow.h/cpp) has its UI constructed programmatically. This is typical for simpler applications or when fine-grained control over UI creation is desired without visual designers.

#### 7.5.2. qlith-pro:

* The main rendering widget is litehtmlWidget.
* The browser application (src/gui/MainWindow.h/cpp) utilizes mainwindow.ui. This means Qt Designer was used to lay out the user interface, which can speed up UI development and allow for more complex visual arrangements. This often leads to a more polished end-user application.

### 7.6. Conceptual Rendering Flow Difference

Imagine rendering a `<div>` with a border, a gradient background, and a box-shadow:

#### 7.6.1. qlith-mini (`ContainerQPainter`):

1. LiteHTML calls `draw_solid_fill` (if simple color) or `draw_linear_gradient`. `ContainerQPainter` directly uses `QPainter::fillRect` with a `QColor` or a `QLinearGradient`.
2. LiteHTML calls `draw_borders`. `ContainerQPainter` sets QPen properties and uses `QPainter::drawLine` for each border side.
3. Shadows, if supported at all by LiteHTML's basic drawing model passed to `ContainerQPainter`, would be very rudimentary or require direct QPainter shadow effects, which might not map well to CSS box-shadow.

#### 7.6.2. qlith-pro (`container_qt5` and `GraphicsContext`):

1. LiteHTML calls `draw_linear_gradient`. `container_qt5` might pass this to `GraphicsContext`.
2. `GraphicsContext` uses its `Gradient` class to configure the gradient, then uses `QPainter` (possibly with transformations from `TransformationMatrix`) to draw it.
3. LiteHTML calls `draw_borders`. `container_qt5` passes this to `GraphicsContext`.
4. `GraphicsContext` configures its stroke style (color, thickness, style) and uses its `PathQt` or direct `QPainter` calls to draw the borders.
5. For box-shadow, `GraphicsContext` would use its `ContextShadow` class. This class would calculate the shadow's appearance (blur, offset, color) and render it, possibly by drawing a blurred, offset version of the element's shape or using platform-specific shadow APIs if available through `QPainter` or its own extensions. This process is significantly more involved and capable of producing CSS-compliant shadows.

### 7.7. Summary Table

|                     |                                                    |                                                                                                                                         |
|---------------------|----------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------|
| Feature Area        | qlith-mini                                         | qlith-pro                                                                                                                               |
| Primary Goal        | Lightweight HTML display                           | Feature-rich HTML rendering with advanced graphics                                                                                      |
| Rendering Core      | LiteHTML + QPainter (direct via ContainerQPainter) | LiteHTML + Custom Graphics Engine (using QPainter at low level via container_qt5 and GraphicsContext)                                   |
| Graphics Lib        | Minimal, direct QPainter usage                     | Extensive custom library (Transforms, Images, Decoders, Gradients, Shadows, Paths, FontCache, Buffers)                                  |
| Complexity          | Low                                                | High                                                                                                                                    |
| Directory Structure | Flat, fewer files                                  | Modular, many files (include/qlith/ is substantial)                                                                                     |
| Key Classes         | QlithWidget, ContainerQPainter                     | litehtmlWidget, container_qt5, GraphicsContext, Image, BitmapImage, AffineTransform, TransformationMatrix, Gradient, ContextShadow etc. |
| UI Design           | Programmatic (simple browser shell)                | Qt Designer (mainwindow.ui) for main application shell                                                                                  |
| CSS Resources       | Basic default CSS                                  | Includes bootstrap.css, reset.css (implies modern layout support)                                                                       |
| Advanced CSS        | Limited by direct QPainter mapping                 | Better support for CSS Transforms, Shadows, Gradients due to custom graphics classes                                                    |
| Memory/Perf.        | Basic Qt mechanisms                                | Potentially more optimized via FontCache, PurgeableBuffer, SharedBuffer                                                                 |

### 7.8. Conclusion

The principal difference between qlith-pro and qlith-mini is their architectural depth and resulting capabilities. qlith-mini is a lean, direct wrapper, suitable for embedding simple HTML content where advanced CSS features and high rendering fidelity are secondary. qlith-pro, conversely, invests in a comprehensive custom 2D graphics engine built on top of Qt. This engine allows qlith-pro to handle more complex HTML/CSS, including advanced styling and transformations, and manage resources more effectively, making it suitable for applications requiring richer web content display. The trade-off is a significantly more complex codebase.
