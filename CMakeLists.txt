cmake_minimum_required(VERSION 3.16)
project(aetherhud VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)  # Upgraded to C++17 for better feature support
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable automoc for Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set sources
set(SOURCES
    src/main.cpp
)

# Find Qt packages
find_package(Qt6 COMPONENTS
    Core
    Gui
    Widgets
    WebEngineWidgets
    WebChannel
    REQUIRED
)

# Fallback to Qt5 if Qt6 is not available
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS
        Core
        Gui
        Widgets
        WebEngineWidgets
        WebChannel
        REQUIRED
    )
endif()

# Add resource files
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.h"
)

# Add executable
add_executable(aetherhud
    ${SOURCES}
    resources/resources.qrc
)

# Include directories
target_include_directories(aetherhud PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link Qt libraries
target_link_libraries(aetherhud PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::WebEngineWidgets
    Qt::WebChannel
)

# Install configuration
install(TARGETS aetherhud
    RUNTIME DESTINATION bin
)

# Install additional resources
install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    DESTINATION share/aetherhud
)

# Enable warnings
if(MSVC)
    target_compile_options(aetherhud PRIVATE /W4)
else()
    target_compile_options(aetherhud PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    set_target_properties(aetherhud PROPERTIES
        WIN32_EXECUTABLE TRUE
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
elseif(APPLE)
    # macOS-specific settings
    set_target_properties(aetherhud PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.domain.aetherhud"
        MACOSX_BUNDLE_BUNDLE_NAME "AetherHUD"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
endif()