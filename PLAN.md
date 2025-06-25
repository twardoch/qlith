1.  **Project Setup & Cleanup:**
    *   Create `PLAN.md`, `TODO.md`, and `CHANGELOG.md`.
    *   Remove `qlith-pro/src/gui/litehtmlwidget.h.bak`.
    *   Record initial actions in `CHANGELOG.md`.

2.  **Consolidate `qlith-mini` into `qlith-pro` (or deprecate `qlith-mini`):**
    *   **Analyze `container_qpainter` (mini) vs. `container_qt5` (pro):**
        *   Identify unique, valuable features in `container_qpainter`.
        *   Merge these features into `container_qt5` if applicable.
        *   Prioritize `container_qt5` as the primary `litehtml::document_container` implementation.
    *   **Analyze `QlithWidget` (mini) vs. `litehtmlWidget` (pro):**
        *   Identify unique, valuable features in `QlithWidget`.
        *   Merge these into `litehtmlWidget`.
        *   Prioritize `litehtmlWidget` as the primary rendering widget.
    *   **Analyze `qlith-mini/browser` vs. `qlith-pro/src/gui/` (MainWindow, main.cpp):**
        *   The `qlith-pro`'s `MainWindow` and `main.cpp` seem more complete with CLI parsing for export.
        *   Ensure all necessary functionality from `qlith-mini/browser` (if any unique) is present in the `qlith-pro` equivalent.
    *   **Decision & Action:**
        *   If `qlith-mini`'s functionality is fully subsumed and improved by `qlith-pro` after potential merges, mark `qlith-mini` directory for removal.
        *   Update CMakeLists.txt files to reflect this consolidation (remove `qlith-mini` targets, ensure `qlith-pro` builds stand-alone).

3.  **Streamline `qlith-pro` Graphics Primitives:**
    *   **Investigate custom graphics classes** in `qlith-pro/include/qlith/` and `qlith-pro/src/` (e.g., `Color`, `FloatPoint`, `FloatRect`, `IntPoint`, `IntRect`, `FloatSize`, `IntSize`, `AffineTransform`, `TransformationMatrix`, `Image`, `BitmapImage`, `Gradient`, `Path`).
    *   For each custom class, compare its functionality with the Qt equivalent (e.g., `QColor`, `QPointF`, `QRectF`, `QTransform`, `QImage`, `QGradient`, `QPainterPath`).
    *   **Decision criteria for replacement:**
        *   Does the Qt class provide all necessary features used by `litehtml` or the surrounding `qlith-pro` code?
        *   Are there specific performance characteristics or memory layouts of the custom classes that are critical and not met by Qt versions?
        *   Does `litehtml`'s API (e.g., `litehtml::document_container` methods taking `litehtml::web_color`, `litehtml::position`) impose constraints that make direct Qt class usage difficult? (Note: `container_qt5` already bridges this gap).
    *   **Action Plan (Iterative):**
        *   Start with simpler classes like `Color`, `IntPoint`, `IntRect`, `IntSize`, `FloatPoint`, `FloatRect`, `FloatSize`.
        *   Attempt to replace their usage within `qlith-pro` with Qt equivalents.
        *   Refactor `container_qt5` and other parts of `qlith-pro` to use Qt types directly where possible. This involves changing method signatures and internal logic.
        *   Test thoroughly after each replacement.
        *   Address more complex classes like `AffineTransform`, `TransformationMatrix`, `Image`, `Gradient`, `Path` subsequently. These are more likely to have specific reasons for custom implementation. `QImage` and `QPixmap` are already used in places, so `Image` and `BitmapImage` might be wrappers or extensions.
    *   **Goal:** Reduce custom graphics code significantly if Qt equivalents are suitable, simplifying maintenance and leveraging Qt's optimizations.

4.  **Refactor and Remove Redundancies in `qlith-pro`:**
    *   **Consolidate `FontCache`:** Evaluate if `qlith-pro/src/gui/fontcache.cpp` can be replaced by direct usage of `QFontDatabase` or simplified.
    *   **Review `import_css` in `container_qt5.cpp`:** Confirm that the current NO-OP behavior is sufficient for MVP or if basic local CSS file loading (from a known resources path) is needed. For MVP, a NO-OP (relying on litehtml's master CSS) is likely fine.
    *   **Handle `litehtml-qt.js_plugin_import.cpp`:** Determine if WebAssembly/JS plugin support is MVP. If not, remove this file and related CMake entries.
    *   **Review `test_app/`:** Decide if this app needs to be part of the MVP build or if it's purely for dev testing. If the latter, exclude from default release builds.
    *   **Address `qDebug()` and `QLITH_DEBUG_DIR`:** Make extensive debug logging conditional (e.g., via macros controlled by CMake build type) or remove for release builds to improve performance and reduce log noise.

5.  **Build System & Testing:**
    *   Update all relevant `CMakeLists.txt` files to reflect the changes from consolidation and streamlining.
    *   Ensure the project (likely `qlith-pro` as the main target) builds cleanly.
    *   Run existing examples (e.g., `examples/run-examples.sh` if it can be adapted) or create minimal test cases to verify rendering and export functionality after major changes.

6.  **Documentation & Finalization:**
    *   Update `README.md` to reflect the streamlined structure and usage.
    *   Ensure `PLAN.md` and `TODO.md` are fully updated to reflect completed work.
    *   Final review of `CHANGELOG.md`.

7.  **Submit Changes:**
    *   Commit all changes with a comprehensive commit message.
