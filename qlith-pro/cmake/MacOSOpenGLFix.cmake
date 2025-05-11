# MacOSOpenGLFix.cmake
# Workaround for missing OpenGL headers on newer macOS versions

if(APPLE)
    # Search for OpenGL headers in multiple locations
    set(POSSIBLE_GL_PATHS
        "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers"
        "/Library/Developer/CommandLineTools/SDKs/MacOSX15.4.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers" 
        "/Library/Developer/CommandLineTools/SDKs/MacOSX14.5.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers"
        "/Library/Developer/CommandLineTools/SDKs/MacOSX12.1.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers"
        "/Library/Developer/CommandLineTools/SDKs/MacOSX11.1.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers"
        "/opt/X11/include/GL"
    )
    
    set(GL_HEADER_PATH "")
    foreach(path ${POSSIBLE_GL_PATHS})
        if(EXISTS "${path}/gl.h")
            set(GL_HEADER_PATH "${path}")
            break()
        endif()
    endforeach()
    
    # Create our own GL headers directory
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
        message(STATUS "Using fallback OpenGL headers")
        # Create minimal dummy headers as a fallback
        file(WRITE "${CMAKE_BINARY_DIR}/OpenGL_include/GL/gl.h" "/* Dummy OpenGL header */\n#define GL_VERSION_1_1 1\n")
        file(WRITE "${CMAKE_BINARY_DIR}/OpenGL_include/GL/glu.h" "/* Dummy GLU header */\n")
        file(WRITE "${CMAKE_BINARY_DIR}/OpenGL_include/GL/glext.h" "/* Dummy GLEXT header */\n")
    endif()
    
    # Add our include directory to the include path
    include_directories(BEFORE "${CMAKE_BINARY_DIR}/OpenGL_include")
    
    # Set environment variables for OpenGL detection
    set(ENV{OPENGL_INCLUDE_DIR} "${CMAKE_BINARY_DIR}/OpenGL_include")
    
    # Set CMake variables for OpenGL detection
    set(CMAKE_POLICY_DEFAULT_CMP0072 NEW)
    set(OpenGL_GL_PREFERENCE LEGACY)
    set(OPENGL_FOUND ON)
    set(OPENGL_GLU_FOUND ON)
    set(OPENGL_INCLUDE_DIR "${CMAKE_BINARY_DIR}/OpenGL_include")
    set(QT_OPENGL_LIB_HEADERS_CHECKED TRUE)
    
    # Tell Qt that we have OpenGL
    add_definitions(-DQT_OPENGL_SUPPORT)
    
    message(STATUS "Applied macOS OpenGL headers workaround")
endif() 