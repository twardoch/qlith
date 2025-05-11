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

# Set up CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include macOS OpenGL fix
if(APPLE)
  # Find the actual OpenGL headers from Xcode or CommandLineTools
  set(POSSIBLE_GL_PATHS
    "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers"
    "/Library/Developer/CommandLineTools/SDKs/MacOSX15.4.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers" 
    "/Library/Developer/CommandLineTools/SDKs/MacOSX14.5.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers"
    "/Library/Developer/CommandLineTools/SDKs/MacOSX12.1.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers"
    "/Library/Developer/CommandLineTools/SDKs/MacOSX11.1.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers"
  )
  
  set(GL_HEADER_PATH "")
  foreach(path ${POSSIBLE_GL_PATHS})
    if(EXISTS "${path}/gl.h")
      set(GL_HEADER_PATH "${path}")
      break()
    endif()
  endforeach()
  
  # Create OpenGL headers directory
  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/OpenGL_include")
  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/OpenGL_include/GL")
  
  if(GL_HEADER_PATH)
    message(STATUS "Found OpenGL headers at: ${GL_HEADER_PATH}")
    
    # Copy the real OpenGL headers to our include directory
    file(GLOB GL_HEADERS "${GL_HEADER_PATH}/*.h")
    foreach(header ${GL_HEADERS})
      get_filename_component(header_name ${header} NAME)
      file(COPY ${header} DESTINATION "${CMAKE_BINARY_DIR}/OpenGL_include/GL")
    endforeach()
  else()
    message(STATUS "Using empty OpenGL headers as fallback")
    # Create dummy files as fallback if real headers not found
    file(WRITE "${CMAKE_BINARY_DIR}/OpenGL_include/GL/gl.h" "/* Dummy OpenGL header */\n#define GL_VERSION_1_1 1\n")
    file(WRITE "${CMAKE_BINARY_DIR}/OpenGL_include/GL/glu.h" "/* Dummy GLU header */\n")
    file(WRITE "${CMAKE_BINARY_DIR}/OpenGL_include/GL/glext.h" "/* Dummy GLEXT header */\n")
  endif()
  
  # Set environment variables to tell Qt where to find the OpenGL headers
  set(ENV{OPENGL_INCLUDE_DIR} "${CMAKE_BINARY_DIR}/OpenGL_include")
  set(ENV{GLU_INCLUDE_DIR} "${CMAKE_BINARY_DIR}/OpenGL_include")
  
  # Add the include directories for OpenGL headers to the INCLUDE path
  include_directories(BEFORE "${CMAKE_BINARY_DIR}/OpenGL_include")
  
  # Set OpenGL flags
  set(CMAKE_POLICY_DEFAULT_CMP0072 NEW)
  set(OpenGL_GL_PREFERENCE LEGACY)
  set(OPENGL_FOUND ON)
  set(OPENGL_GLU_FOUND ON)
  set(OPENGL_INCLUDE_DIR "${CMAKE_BINARY_DIR}/OpenGL_include")
  
  # Override Qt OpenGL headers check
  set(QT_OPENGL_LIB_HEADERS_CHECKED TRUE)
  
  message(STATUS "Applied macOS OpenGL headers workaround")
endif()

# Find Qt5 packages
find_package(Qt5 5.15 COMPONENTS Core Gui Widgets Network Svg REQUIRED)

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

# Process Qt resources
set(RESOURCE_FILES
    resources/res.qrc
)
qt5_add_resources(RESOURCES ${RESOURCE_FILES})

# Create executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${MOC_HEADERS}
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
    Qt5::Svg
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

# Add color test program
add_executable(color_test
    test_app/color_test.cpp
)

target_include_directories(color_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}
        ${LITEHTML_DIR}/include
        ${LITEHTML_DIR}/include/litehtml
        ${LITEHTML_DIR}/src/gumbo/include
)

target_link_libraries(color_test PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    litehtml
)

# Set application properties for color_test
set_target_properties(color_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)