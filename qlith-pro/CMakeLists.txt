cmake_minimum_required(VERSION 3.16)
project(qlith-pro VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include Qt packages
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Core Gui Widgets REQUIRED)
endif()

# External dependencies
set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external")

# Litehtml
add_subdirectory("${EXTERNAL_DIR}/litehtml" "${CMAKE_BINARY_DIR}/litehtml" EXCLUDE_FROM_ALL)
include_directories("${EXTERNAL_DIR}/litehtml/include")

# Gumbo Parser
include_directories("${EXTERNAL_DIR}/gumbo-parser/src")

# Set include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Define source files
set(SOURCES
    # Gui sources
    src/gui/container_qt5.cpp
    src/gui/fontcache.cpp
    src/gui/main.cpp
    src/gui/mainwindow.cpp
    
    # Core sources
    src/affinetransform.cpp
    src/bitmapimage.cpp
    src/color.cpp
    src/common.cpp
    src/contextshadow.cpp
    src/floatpoint.cpp
    src/floatpoint3d.cpp
    src/floatquad.cpp
    src/floatrect.cpp
    src/floatsize.cpp
    src/gradient.cpp
    src/graphicscontext.cpp
    src/image.cpp
    src/imagedecoder.cpp
    src/imagedecoderqt.cpp
    src/imageobserver.cpp
    src/imagesource.cpp
    src/intpoint.cpp
    src/intrect.cpp
    src/intsize.cpp
    src/mimetyperegistry.cpp
    src/mimetyperegistryqt.cpp
    src/pathqt.cpp
    src/pngimagedecoder.cpp
    src/purgeablebuffer.cpp
    src/rgba32bufferqt.cpp
    src/shadowdata.cpp
    src/sharedbuffer.cpp
    src/stillimageqt.cpp
    src/styleimage.cpp
    src/transformationmatrix.cpp
)

# Process Qt UI files
set(UI_FILES
    resources/mainwindow.ui
)
qt_wrap_ui(UI_HEADERS ${UI_FILES})

# Process Qt resources
set(RESOURCE_FILES
    resources/res.qrc
)
qt_add_resources(RESOURCES ${RESOURCE_FILES})

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${UI_HEADERS} ${RESOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    litehtml
)

# Install rules
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Optional: Enable IDE folders for Visual Studio and Xcode
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add compiler-specific flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()

message(STATUS "qlith-pro configuration complete")