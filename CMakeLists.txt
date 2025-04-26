cmake_minimum_required(VERSION 3.10)
project(qlith VERSION 1.0 LANGUAGES CXX)

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# Enable Qt features
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Debug options
set(QLITH_DEBUG_DIR "${CMAKE_BINARY_DIR}/debug" CACHE PATH "Directory to store debug files")
add_definitions(-DQLITH_DEBUG_DIR="${QLITH_DEBUG_DIR}")

# Find Qt5 packages
find_package(Qt5 COMPONENTS Widgets Network Svg REQUIRED)

# Option to build with system-provided litehtml
option(USE_SYSTEM_LITEHTML "Use system-provided litehtml" OFF)
option(USE_SYSTEM_GUMBO "Use system-provided gumbo-parser" OFF)

# Find or build litehtml
if(USE_SYSTEM_LITEHTML)
  find_package(litehtml REQUIRED)
else()
  # Use litehtml from the parent directory
  set(LITEHTML_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../litehtml" CACHE PATH "Path to litehtml source")
  add_subdirectory(${LITEHTML_SOURCE_DIR} ${CMAKE_BINARY_DIR}/litehtml)
endif()

# Find or build gumbo-parser
if(USE_SYSTEM_GUMBO)
  find_package(gumbo REQUIRED)
else()
  # Use gumbo-parser from the parent directory
  set(GUMBO_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../gumbo-parser" CACHE PATH "Path to gumbo-parser source")
  add_subdirectory(${GUMBO_SOURCE_DIR} ${CMAKE_BINARY_DIR}/gumbo-parser)
endif()

# Find the zlib package
find_package(ZLIB REQUIRED)

# Include module path for custom Find*.cmake files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Add subdirectories
add_subdirectory(src)
add_subdirectory(browser)

# Install CMake config files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/qlithConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

# Packaging
set(CPACK_PACKAGE_NAME "qlith")
set(CPACK_PACKAGE_VENDOR "qlith")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Qt-based lightweight HTML renderer")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "qlith")
include(CPack) 