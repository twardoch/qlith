set(BROWSER_SRCS
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

# Define browser executable
add_executable(qlith_browser
    MACOSX_BUNDLE
    ${BROWSER_SRCS}
)

# Include directories
target_include_directories(qlith_browser PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Link with required libraries
target_link_libraries(qlith_browser
    PRIVATE
    Qt5::Widgets
    Qt5::Network
    Qt5::Svg
    qlith
)

# Set application properties
set_target_properties(qlith_browser PROPERTIES
    OUTPUT_NAME "qlith"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/cmake/MacOSXBundleInfo.plist.in"
)

# Installation
install(TARGETS qlith_browser
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
) 