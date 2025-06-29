# Changelog

## [Unreleased]

### Added
- Created `PLAN.md` with the initial refactoring plan
- Created `TODO.md` with a checklist for the refactoring plan
- Created `CHANGELOG.md`
- Added color_test program to verify color parsing fixes
- Added runme.sh script for both qlith-mini and qlith-pro projects
- Enhanced error handling in container_qt5.cpp
- Improved drawing functions in container_qt5 implementation

### Changed
- Updated litehtml submodule to latest version
- Updated gumbo-parser submodule to latest commit 6b55c33
- Enhanced main.cpp to handle relative file paths for user input
- Adjusted build_macos.sh scripts for correct Qt5 directory and symbolic link creation
- Refactored font loading logic in main.cpp
- Improved initialization of GUI components in mainwindow.cpp
- Initial streamlining and qlith-mini consolidation (merged from feature/streamline-core-refactor-1)
- Marked qlith-mini features as merged into qlith-pro in TODO.md
- Updated main build script to focus on qlith-pro

### Removed
- Removed `qlith-pro/src/gui/litehtmlwidget.h.bak`
- Removed NEXT.md file containing detailed plan for fixing segfault in qlith-pro
- Removed qlith-mini from main build process (deprecated in favor of consolidated qlith-pro)

### Fixed
- Color resolution improvements in container_qt5
- Segfault issues in qlith-pro (via container_qt5 improvements)
- Proper handling of relative file paths in command-line usage
