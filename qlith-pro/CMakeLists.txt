cmake_minimum_required(VERSION 3.16)
project(qlith-pro VERSION 0.1.0 LANGUAGES CXX C)

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# Enable Qt features
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set UI search paths
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/resources)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# External dependencies paths
set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external")
set(LITEHTML_DIR "${EXTERNAL_DIR}/litehtml")

# Find Qt5 packages
find_package(Qt5 5.15 COMPONENTS Core Gui Widgets Network REQUIRED)

# Build external dependencies - litehtml (which includes gumbo)
add_subdirectory(${LITEHTML_DIR} ${CMAKE_BINARY_DIR}/litehtml)

# Define source files
set(SOURCES
    # Gui sources
    src/gui/container_qt5.cpp
    src/gui/fontcache.cpp
    src/gui/litehtmlwidget.cpp
    src/gui/main.cpp
    src/gui/mainwindow.cpp
    
    # Core sources - Comment out problematic files
    # src/affinetransform.cpp
    # src/bitmapimage.cpp
    # src/color.cpp
    # src/common.cpp
    # src/contextshadow.cpp
    # src/floatpoint.cpp
    # src/floatpoint3d.cpp
    # src/floatquad.cpp
    # src/floatrect.cpp
    # src/floatsize.cpp
    # src/gradient.cpp
    # src/graphicscontext.cpp
    # src/image.cpp
    # src/imagedecoder.cpp
    # src/imagedecoderqt.cpp
    # src/imageobserver.cpp
    # src/imagesource.cpp
    # src/intpoint.cpp
    # src/intrect.cpp
    # src/intsize.cpp
    # src/mimetyperegistry.cpp
    # src/mimetyperegistryqt.cpp
    # src/pathqt.cpp
    # src/pngimagedecoder.cpp
    # src/purgeablebuffer.cpp
    # src/rgba32bufferqt.cpp
    # src/shadowdata.cpp
    # src/sharedbuffer.cpp
    # src/stillimageqt.cpp
    # src/styleimage.cpp
    # src/transformationmatrix.cpp
    # src/litehtml-qt.js_plugin_import.cpp
)

# Define header files with Q_OBJECT macro for Qt MOC
set(MOC_HEADERS
    include/qlith/container_qt5.h
    include/qlith/litehtmlwidget.h
    include/qlith/mainwindow.h
)

# Process Qt UI files
set(UI_FILES
    resources/mainwindow.ui
)

# Instead of using qt5_wrap_ui directly, we'll rely on AUTOUIC
# Make sure the UI files are included in the sources list
set(ALL_SOURCES
    ${SOURCES}
    ${MOC_HEADERS}
)

# Process Qt resources
set(RESOURCE_FILES
    resources/res.qrc
)
qt5_add_resources(RESOURCES ${RESOURCE_FILES})

# Create executable
add_executable(${PROJECT_NAME}
    ${ALL_SOURCES}
    ${UI_FILES}
    ${RESOURCES}
)

# Include directories
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}  # For auto-generated files including UI headers
        ${LITEHTML_DIR}/include
        ${LITEHTML_DIR}/include/litehtml
        ${LITEHTML_DIR}/src/gumbo/include
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    litehtml
    gumbo
)

# Add preprocessor definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x050F00
)

# Install rules
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add compiler-specific flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
    
    # Add macOS-specific flags if needed
    if(APPLE)
        target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated-declarations)
        set(CMAKE_MACOSX_RPATH 1)
    endif()
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()

# Set application properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.github.twardoch.qlith-pro"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

message(STATUS "qlith-pro configuration complete")