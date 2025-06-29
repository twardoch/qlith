# Qlith Development Plan

## Executive Summary

Qlith is a Qt5-based HTML rendering library built on litehtml. The project currently has two implementations: qlith-mini (lightweight) and qlith-pro (feature-rich). This plan outlines the path to create a unified, stable, and maintainable HTML rendering solution.

## Current State Analysis

### Architecture Overview
- **qlith-mini**: Simple QPainter-based implementation (~1,500 LOC)
- **qlith-pro**: Complex custom graphics layer (~10,000+ LOC)
- **Core dependencies**: litehtml (HTML/CSS engine), gumbo-parser (HTML5 parser), Qt5

### Key Findings
1. Significant code duplication between mini and pro versions
2. qlith-pro's custom graphics layer largely reimplements Qt functionality
3. Recent consolidation efforts have begun but are incomplete
4. Build system needs modernization for easier deployment

## Phase 1: Complete Consolidation (1-2 weeks)

### 1.1 Merge Remaining qlith-mini Features
- [ ] Extract any unique features from ContainerQPainter that aren't in container_qt5
- [ ] Port QlithWidget's cleaner API design to litehtmlWidget
- [ ] Migrate example/test cases to use unified implementation
- [ ] Archive qlith-mini directory (keep for reference, remove from builds)

### 1.2 Simplify Graphics Abstraction Layer
- [ ] **Immediate replacements** (low risk):
  - `Color` → `QColor`
  - `IntPoint/FloatPoint` → `QPoint/QPointF`
  - `IntRect/FloatRect` → `QRect/QRectF`
  - `IntSize/FloatSize` → `QSize/QSizeF`
- [ ] **Evaluate and replace** (medium risk):
  - `AffineTransform/TransformationMatrix` → `QTransform`
  - `Gradient` → `QGradient` family
  - `Path` → `QPainterPath`
- [ ] **Refactor GraphicsContext** to be a thin wrapper around QPainter
- [ ] Keep custom implementations only where Qt lacks functionality

### 1.3 Clean Up Build System
- [ ] Create unified CMakeLists.txt at project root
- [ ] Support both bundled and system litehtml/gumbo
- [ ] Add CMake options for debug/release builds
- [ ] Create proper installation targets
- [ ] Add pkg-config support

## Phase 2: Enhance Stability & Performance (2-3 weeks)

### 2.1 Memory Management
- [ ] Audit and fix memory leaks (especially in image/font caching)
- [ ] Implement proper RAII patterns throughout
- [ ] Add smart pointer usage where appropriate
- [ ] Profile memory usage with complex HTML documents

### 2.2 Error Handling
- [ ] Replace silent failures with proper error reporting
- [ ] Add comprehensive nullptr checks
- [ ] Implement graceful degradation for unsupported features
- [ ] Add logging system with configurable levels

### 2.3 Performance Optimization
- [ ] Profile rendering pipeline with large documents
- [ ] Optimize font metrics caching
- [ ] Implement lazy image loading
- [ ] Add viewport-based rendering optimization
- [ ] Consider multi-threaded layout calculation

### 2.4 Testing Infrastructure
- [ ] Create unit tests for core components
- [ ] Add rendering regression tests
- [ ] Implement automated visual diff testing
- [ ] Set up CI/CD pipeline

## Phase 3: Improve Deployability (1-2 weeks)

### 3.1 Platform Support
- [ ] **macOS**: Create proper .app bundle with embedded frameworks
- [ ] **Windows**: Add MSVC support, create installer
- [ ] **Linux**: Create packages for major distributions (deb, rpm)
- [ ] Add static linking option for self-contained builds

### 3.2 Distribution
- [ ] Create release scripts for all platforms
- [ ] Set up GitHub Actions for automated releases
- [ ] Publish to package managers (Homebrew, vcpkg, Conan)
- [ ] Create pre-built binaries for common configurations

### 3.3 Documentation
- [ ] Write comprehensive API documentation
- [ ] Create usage examples and tutorials
- [ ] Document build instructions for all platforms
- [ ] Add architecture documentation for contributors

## Phase 4: Feature Enhancements (3-4 weeks)

### 4.1 Core Rendering
- [ ] Improve CSS3 support (flexbox, grid)
- [ ] Add WebFont loading support
- [ ] Implement print rendering
- [ ] Add PDF export capability

### 4.2 Widget Features
- [ ] Add text selection and copying
- [ ] Implement find-in-page functionality
- [ ] Add zoom controls
- [ ] Improve keyboard navigation

### 4.3 Developer Experience
- [ ] Create Qt Designer plugin
- [ ] Add QML component wrapper
- [ ] Provide CSS debugging tools
- [ ] Create visual HTML inspector

### 4.4 Network Support
- [ ] Improve HTTP/HTTPS resource loading
- [ ] Add caching system for network resources
- [ ] Implement proper redirect handling
- [ ] Add proxy support

## Phase 5: Long-term Improvements (Ongoing)

### 5.1 Modernization
- [ ] Consider Qt6 support (dual Qt5/Qt6 compatibility)
- [ ] Evaluate C++17/20 features for code improvement
- [ ] Investigate WebAssembly compilation
- [ ] Consider mobile platform support (iOS, Android)

### 5.2 Performance & Features
- [ ] GPU-accelerated rendering path
- [ ] Incremental layout/rendering
- [ ] Better JavaScript stub support
- [ ] SVG rendering improvements

### 5.3 Community Building
- [ ] Set up proper issue templates
- [ ] Create contribution guidelines
- [ ] Establish code review process
- [ ] Build example gallery

## Success Metrics

### Technical
- Clean build on all major platforms
- No memory leaks in typical usage
- Rendering performance comparable to QtWebEngine for static content
- 90%+ test coverage for core components

### Usability
- Single header include for basic usage
- 5-minute quick start guide
- Comprehensive examples covering common use cases
- Active community support

### Adoption
- Available in major package managers
- Used in at least 10 open-source projects
- Regular release cycle (quarterly)
- Responsive to bug reports and feature requests

## Risk Mitigation

### Technical Risks
- **litehtml limitations**: Maintain capability to switch rendering engines
- **Qt version compatibility**: Abstract Qt-specific code where possible
- **Performance regression**: Continuous benchmarking in CI

### Project Risks
- **Maintainer availability**: Document everything, automate releases
- **Scope creep**: Maintain focus on core HTML/CSS rendering
- **Breaking changes**: Semantic versioning, deprecation policy

## Next Steps

1. Review and refine this plan with stakeholders
2. Create detailed task breakdowns for Phase 1
3. Set up project management tools (issues, milestones)
4. Begin implementation starting with highest-impact items
5. Establish regular progress reviews

This plan provides a roadmap from the current state to a production-ready, maintainable HTML rendering library that serves the Qt ecosystem well.