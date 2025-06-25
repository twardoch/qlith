- [X] **Project Setup & Cleanup:**
    - [X] Create `PLAN.md`, `TODO.md`, and `CHANGELOG.md`.
    - [X] Remove `qlith-pro/src/gui/litehtmlwidget.h.bak`.
    - [X] Record initial actions in `CHANGELOG.md`.

- [ ] **Consolidate `qlith-mini` into `qlith-pro` (or deprecate `qlith-mini`):**
    - [X] Analyze `container_qpainter` (mini) vs. `container_qt5` (pro). (Decision: `container_qt5` is preferred base)
    - [X] Analyze `QlithWidget` (mini) vs. `litehtmlWidget` (pro). (Decision: `litehtmlWidget` is preferred base)
    - [X] Analyze `qlith-mini/browser` vs. `qlith-pro/src/gui/` (MainWindow, main.cpp). (Decision: `qlith-pro` app structure preferred, with some logic merged from `qlith-mini`)
    - [X] Decide and act on `qlith-mini`'s fate (deprecate/remove or merge). (Action: Removed from main build script, features merged into qlith-pro)
    - [X] Update CMakeLists.txt for consolidation. (Action: Updated main build_macos.sh)

- [ ] **Streamline `qlith-pro` Graphics Primitives:**
    - [ ] Investigate custom graphics classes in `qlith-pro`.
    - [ ] Iteratively replace simpler custom classes (`Color`, `IntPoint`, etc.) with Qt equivalents if feasible.
    - [ ] Test thoroughly after each replacement.
    - [ ] Address complex custom classes (`AffineTransform`, `Image`, etc.) subsequently.

- [ ] **Refactor and Remove Redundancies in `qlith-pro`:**
    - [ ] Consolidate/simplify `FontCache`.
    - [ ] Review `import_css` in `container_qt5.cpp`.
    - [ ] Handle `litehtml-qt.js_plugin_import.cpp` (remove if not MVP).
    - [ ] Review `test_app/` (exclude from MVP build if dev-only).
    - [ ] Make debug logging (`qDebug()`, `QLITH_DEBUG_DIR`) conditional or remove for release.

- [ ] **Build System & Testing:**
    - [ ] Update all relevant `CMakeLists.txt` files.
    - [ ] Ensure `qlith-pro` (main target) builds cleanly.
    - [ ] Test rendering and export functionality.

- [ ] **Documentation & Finalization:**
    - [ ] Update `README.md`.
    - [ ] Ensure `PLAN.md` and `TODO.md` are up-to-date.
    - [ ] Final review of `CHANGELOG.md`.

- [ ] **Submit Changes:**
    - [ ] Commit all changes.
