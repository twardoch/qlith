cmake_minimum_required(VERSION 3.7)
project(qlitehtml VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Debug options
set(QLDW_DEBUG_DIR "${CMAKE_BINARY_DIR}/debug" CACHE PATH "Directory to store debug files")
add_definitions(-DQLDW_DEBUG_DIR="${QLDW_DEBUG_DIR}")

# Find Qt5 packages
find_package(Qt5 COMPONENTS Widgets Network Svg REQUIRED)

# Find litehtml package
find_package(litehtml REQUIRED)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(browser) 