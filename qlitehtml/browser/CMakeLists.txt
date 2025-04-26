set(BROWSER_SRCS
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

# Define browser executable
add_executable(litehtml_browser
    MACOSX_BUNDLE
    ${BROWSER_SRCS}
)

# Include directories
target_include_directories(litehtml_browser PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Link with required libraries
target_link_libraries(litehtml_browser
    PRIVATE
    Qt5::Widgets
    Qt5::Network
    Qt5::Svg
    qlitehtml
)

# Installation
install(TARGETS litehtml_browser
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
