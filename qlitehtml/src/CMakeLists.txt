set(QLITEHTML_LIB_SRCS
    qlitehtmlwidget.cpp
    container_qpainter.cpp
    qlitehtmlwidget.h
    qlitehtml_global.h
    container_qpainter.h
    container_qpainter_p.h
)

# Create qlitehtml library
add_library(qlitehtml SHARED ${QLITEHTML_LIB_SRCS})

target_compile_definitions(qlitehtml PRIVATE QLITEHTML_LIBRARY)

# Find the zlib package
find_package(ZLIB REQUIRED)

# Include directories
target_include_directories(qlitehtml PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${ZLIB_INCLUDE_DIRS}
)

# Link with Qt, litehtml and zlib
target_link_libraries(qlitehtml
    PUBLIC
    Qt5::Widgets
    Qt5::Network
    litehtml
    ${ZLIB_LIBRARIES}
)

# Installation
install(TARGETS qlitehtml
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES
    qlitehtmlwidget.h
    qlitehtml_global.h
    container_qpainter.h
    DESTINATION include
) 