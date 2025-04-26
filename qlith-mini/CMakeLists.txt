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

# Find or build litehtml
add_subdirectory(${CMAKE_SOURCE_DIR}/external/litehtml ${CMAKE_BINARY_DIR}/litehtml)

# Find or build gumbo-parser
add_subdirectory(${CMAKE_SOURCE_DIR}/external/gumbo-parser ${CMAKE_BINARY_DIR}/gumbo-parser)

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