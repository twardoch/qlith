# qlith-mini

A lightweight HTML rendering widget for Qt applications using the litehtml library.

## Overview

qlith-mini is a library that provides a Qt widget for rendering HTML content. It uses the [litehtml](https://github.com/litehtml/litehtml) library for HTML/CSS parsing and rendering, and Qt for the actual drawing and UI integration.

Features:
- Lightweight HTML rendering in Qt applications
- Support for most common HTML and CSS features
- Simple Qt widget integration
- Support for local and remote content
- Image and CSS resource loading

## Requirements

- Qt 5.15 or later
- CMake 3.10 or later
- A C++17 compatible compiler
- litehtml and gumbo-parser libraries (included as submodules)

## Building

### macOS

1. Clone the repository with submodules:
   ```bash
   git clone --recurse-submodules https://github.com/yourusername/qlith-mini.git
   cd qlith-mini
   ```

2. Install dependencies using Homebrew:
   ```bash
   brew install qt@5 cmake
   ```

3. Run the build script:
   ```bash
   ./build_macos.sh
   ```

4. Run the example browser:
   ```bash
   ./qlith-run
   ```

### Linux

1. Clone the repository with submodules:
   ```bash
   git clone --recurse-submodules https://github.com/yourusername/qlith-mini.git
   cd qlith-mini
   ```

2. Install dependencies:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install qt5-default qtbase5-dev cmake g++
   
   # Fedora
   sudo dnf install qt5-qtbase-devel cmake gcc-c++
   ```

3. Build the project:
   ```bash
   mkdir -p build && cd build
   cmake ..
   make -j$(nproc)
   ```

4. Run the example browser:
   ```bash
   ./browser/qlith
   ```

## Usage

### Basic Usage

```cpp
#include <QApplication>
#include <QMainWindow>
#include <qlith/qlithwidget.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QMainWindow window;
    QlithWidget *htmlWidget = new QlithWidget(&window);
    window.setCentralWidget(htmlWidget);
    
    // Load HTML content
    htmlWidget->setHtml("<html><body><h1>Hello World</h1></body></html>");
    
    // Or load from a URL
    // htmlWidget->load(QUrl("https://example.com"));
    
    window.resize(800, 600);
    window.show();
    
    return app.exec();
}
```

### Advanced Usage

```cpp
QlithWidget *htmlWidget = new QlithWidget(this);

// Set base URL for resolving relative paths
htmlWidget->setBaseUrl(QUrl("file:///path/to/resources/"));

// Set background color
htmlWidget->setBackgroundColor(Qt::white);

// Set default CSS
htmlWidget->setDefaultStyleSheet("body { font-family: Arial; }");

// Connect to signals
connect(htmlWidget, &QlithWidget::loadFinished, 
        this, &MainWindow::onLoadFinished);
connect(htmlWidget, &QlithWidget::linkClicked, 
        this, &MainWindow::onLinkClicked);
```

## License

qlith-mini is licensed under the MIT License. See the LICENSE file for details.

## Credits

This project uses the following open source libraries:
- [litehtml](https://github.com/litehtml/litehtml)
- [gumbo-parser](https://github.com/google/gumbo-parser)
- [Qt](https://www.qt.io/) 