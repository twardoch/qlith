set(QLITH_LIB_SRCS
    qlithwidget.cpp
    container_qpainter.cpp
    qlithwidget.h
    qlith_global.h
    container_qpainter.h
    container_qpainter_p.h
)

# Create qlith library
add_library(qlith SHARED ${QLITH_LIB_SRCS})

target_compile_definitions(qlith PRIVATE QLITH_LIBRARY)

# Include directories
target_include_directories(qlith 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${ZLIB_INCLUDE_DIRS}
)

# Link with Qt, litehtml and zlib
target_link_libraries(qlith
    PUBLIC
        Qt5::Widgets
        Qt5::Network
        litehtml
    PRIVATE
        ${ZLIB_LIBRARIES}
)

# Set output properties
set_target_properties(qlith PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME "qlith"
)

# Generate export header
include(GenerateExportHeader)
generate_export_header(qlith
    EXPORT_FILE_NAME qlith_export.h
)

# Installation
install(TARGETS qlith
    EXPORT qlithTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES
    qlithwidget.h
    qlith_global.h
    container_qpainter.h
    ${CMAKE_CURRENT_BINARY_DIR}/qlith_export.h
    DESTINATION include/qlith
)

# Export targets
install(EXPORT qlithTargets
    FILE qlithTargets.cmake
    NAMESPACE qlith::
    DESTINATION lib/cmake/qlith
)

# Create and install config file
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/qlithConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/qlithConfig.cmake
    @ONLY
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/qlithConfig.cmake
    ${CMAKE_BINARY_DIR}/qlithConfigVersion.cmake
    DESTINATION lib/cmake/qlith
) 