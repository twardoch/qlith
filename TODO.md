# Qlith TODO List

## Completed Tasks ✓

- [X] **Project Setup & Cleanup:**
    - [X] Create `PLAN.md`, `TODO.md`, and `CHANGELOG.md`
    - [X] Remove `qlith-pro/src/gui/litehtmlwidget.h.bak`
    - [X] Record initial actions in `CHANGELOG.md`
    - [X] Update litehtml and gumbo-parser submodules
    - [X] Fix segfault issues in qlith-pro
    - [X] Add color_test program
    - [X] Initial consolidation of qlith-mini features

## Phase 1: Complete Consolidation (Priority: High)

### 1.1 Merge Remaining qlith-mini Features
- [ ] Compare ContainerQPainter vs container_qt5 implementations line-by-line
- [ ] Extract viewport handling logic from ContainerQPainter
- [ ] Port QlithWidget's cleaner signal/slot API to litehtmlWidget
- [ ] Migrate network resource loading from QlithWidget
- [ ] Update examples to use unified implementation
- [ ] Archive qlith-mini directory with README explaining deprecation

### 1.2 Simplify Graphics Abstraction Layer
- [ ] Replace Color class with QColor throughout codebase
- [ ] Replace IntPoint/FloatPoint with QPoint/QPointF
- [ ] Replace IntRect/FloatRect with QRect/QRectF
- [ ] Replace IntSize/FloatSize with QSize/QSizeF
- [ ] Evaluate AffineTransform usage and replace with QTransform where possible
- [ ] Simplify Gradient class to use QGradient directly
- [ ] Refactor GraphicsContext to minimize abstraction overhead
- [ ] Remove unused graphics primitives

### 1.3 Clean Up Build System
- [ ] Create root-level CMakeLists.txt that builds everything
- [ ] Add CMake option for USE_SYSTEM_LITEHTML
- [ ] Add CMake option for BUILD_EXAMPLES
- [ ] Add CMake option for BUILD_TESTS
- [ ] Create FindQlith.cmake for downstream projects
- [ ] Add install targets with proper versioning
- [ ] Generate pkg-config file

## Phase 2: Enhance Stability & Performance (Priority: High)

### 2.1 Memory Management
- [ ] Run valgrind on example programs
- [ ] Fix any memory leaks in font cache
- [ ] Fix any memory leaks in image cache
- [ ] Convert raw pointers to smart pointers where appropriate
- [ ] Add RAII wrappers for litehtml objects

### 2.2 Error Handling
- [ ] Add error return codes to all public APIs
- [ ] Implement proper error reporting mechanism
- [ ] Add null pointer checks in all interface methods
- [ ] Create fallback rendering for unsupported features
- [ ] Add configurable logging system (QtLogging-based)

### 2.3 Performance Optimization
- [ ] Profile with large HTML files (>1MB)
- [ ] Implement font metrics cache with LRU eviction
- [ ] Add lazy image loading with placeholder support
- [ ] Optimize paint event handling
- [ ] Investigate caching rendered content

### 2.4 Testing Infrastructure
- [ ] Create unit tests for container_qt5
- [ ] Create unit tests for litehtmlWidget
- [ ] Add rendering tests with known HTML/CSS inputs
- [ ] Set up GitHub Actions CI
- [ ] Add code coverage reporting

## Phase 3: Improve Deployability (Priority: Medium)

### 3.1 Platform Support
- [ ] Fix macOS bundle creation in CMake
- [ ] Add Windows MSVC project files
- [ ] Create Linux AppImage
- [ ] Test on Qt 5.12 (LTS) through 5.15
- [ ] Add static build configuration

### 3.2 Distribution
- [ ] Create release.sh script
- [ ] Set up GitHub releases with artifacts
- [ ] Submit to vcpkg
- [ ] Submit to Conan
- [ ] Create Homebrew formula
- [ ] Create AUR package

### 3.3 Documentation
- [ ] Generate Doxygen documentation
- [ ] Write getting started guide
- [ ] Create API reference
- [ ] Document CSS support level
- [ ] Add migration guide from qlith-mini

## Phase 4: Feature Enhancements (Priority: Low)

### 4.1 Core Rendering
- [ ] Audit CSS3 support gaps
- [ ] Implement @font-face support
- [ ] Add print media support
- [ ] Create PDF export via QPrinter
- [ ] Improve SVG rendering

### 4.2 Widget Features
- [ ] Implement text selection
- [ ] Add context menu with copy support
- [ ] Implement Ctrl+F find functionality
- [ ] Add zoom with Ctrl+/- shortcuts
- [ ] Improve keyboard navigation (tab order)

### 4.3 Developer Experience
- [ ] Create Qt Designer plugin
- [ ] Write QML wrapper component
- [ ] Add DOM inspector widget
- [ ] Create CSS property viewer
- [ ] Add performance profiler

### 4.4 Network Support
- [ ] Implement proper HTTP cache
- [ ] Add cookie jar support
- [ ] Handle 30x redirects
- [ ] Add proxy configuration
- [ ] Support data: URLs

## Phase 5: Long-term Improvements (Priority: Future)

### 5.1 Modernization
- [ ] Add Qt6 compatibility layer
- [ ] Migrate to C++17 features
- [ ] Investigate std::filesystem usage
- [ ] Research WebAssembly target
- [ ] Prototype iOS/Android support

### 5.2 Performance & Features
- [ ] Research OpenGL rendering backend
- [ ] Implement incremental rendering
- [ ] Add basic JavaScript stubs
- [ ] Improve table layout performance
- [ ] Add CSS animation stubs

### 5.3 Community Building
- [ ] Create CONTRIBUTING.md
- [ ] Set up issue templates
- [ ] Create pull request template
- [ ] Build example gallery website
- [ ] Set up Discord/Matrix channel

## Quick Wins (Can be done anytime)

- [ ] Fix all compiler warnings
- [ ] Run clang-format on entire codebase
- [ ] Update all copyright headers
- [ ] Remove commented-out code
- [ ] Fix TODO/FIXME comments in code
- [ ] Improve debug output formatting
- [ ] Add .clang-format file
- [ ] Create .editorconfig file

## Notes

- Items marked with high priority should be completed first
- Each phase builds on the previous one
- Testing should be ongoing throughout all phases
- Documentation updates should accompany code changes
- Performance benchmarks should be established early