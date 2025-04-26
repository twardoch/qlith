# Qlith - A Qt5-based Lightweight HTML Widget and Browser

Qlith is a Qt5-based widget and tiny browser that integrates the [litehtml](https://github.com/litehtml/litehtml) lightweight HTML rendering engine and [gumbo-parser](https://codeberg.org/gumbo-parser/gumbo-parser) for HTML parsing.

## Features

- Lightweight HTML rendering through litehtml integration
- Fast HTML parsing with gumbo-parser
- Qt5 widget for easy integration into your applications
- Simple browser application for testing and previewing HTML content
- Support for basic CSS styling
- Image loading and rendering
- Link navigation

## Requirements

- Qt 5.12 or newer
- CMake 3.10 or newer
- C++17 compatible compiler

## Building

### Building with system-provided litehtml and gumbo-parser

```bash
mkdir build && cd build
cmake .. -DUSE_SYSTEM_LITEHTML=ON -DUSE_SYSTEM_GUMBO=ON
make
```

### Building with bundled litehtml and gumbo-parser

```bash
mkdir build && cd build
cmake .. -DLITEHTML_SOURCE_DIR=path/to/litehtml -DGUMBO_SOURCE_DIR=path/to/gumbo-parser
make
```

## Usage

### Using the QlithWidget in Your Application

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

### Running the Browser

```bash
./qlith_browser [url]
```

## License

Qlith is distributed under the same license as litehtml ([New BSD License](https://opensource.org/licenses/BSD-3-Clause)) and follows the licensing terms of its components:

- **litehtml**: [New BSD License](https://opensource.org/licenses/BSD-3-Clause)
- **gumbo-parser**: [Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0)

## Acknowledgments

Qlith is a refactored and reorganized version of qlitehtml, designed to better incorporate litehtml and gumbo-parser.

- [litehtml](https://github.com/litehtml/litehtml)
- [gumbo-parser](https://codeberg.org/gumbo-parser/gumbo-parser)
- [qlitehtml](https://github.com/qlitehtml/qlitehtml) (original project) 