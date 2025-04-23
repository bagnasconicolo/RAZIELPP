#------------------------------------------------------------------------------#
# CMakeLists.txt
#------------------------------------------------------------------------------#

cmake_minimum_required(VERSION 3.10)

# Project name and language


project(RazielNDVIpp VERSION 1.0 LANGUAGES CXX)


if(APPLE)
    set(CMAKE_MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/Info.plist")
endif()
# Enable AUTOMOC, AUTOUIC, and AUTORCC for Qt meta-object features
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# Homebrew Qt5 & OpenCV on macOS (adjust these prefixes if using Intel or different locations)
# -----------------------------------------------------------------------------
# Qt5Config.cmake lives here:
set(Qt5_DIR "/opt/homebrew/opt/qt@5/lib/cmake/Qt5")
# OpenCVConfig.cmake lives here:
set(OpenCV_DIR "/opt/homebrew/opt/opencv/lib/cmake/opencv4")

# -----------------------------------------------------------------------------
# Find Qt5 components
# -----------------------------------------------------------------------------
find_package(Qt5 COMPONENTS Widgets Core Gui REQUIRED)

# Include Qt5 headers
include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
)

# Add any Qt5 compile definitions (e.g. from pkg-config)
add_definitions(
    ${Qt5Core_DEFINITIONS}
    ${Qt5Widgets_DEFINITIONS}
    ${Qt5Gui_DEFINITIONS}
)

# -----------------------------------------------------------------------------
# Find OpenCV 4.9+
# -----------------------------------------------------------------------------
find_package(OpenCV 4.9 REQUIRED)

# -----------------------------------------------------------------------------
# Include directories for your headers and OpenCV
# -----------------------------------------------------------------------------
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------
set(SOURCES
    src/main.cpp
    src/CaptureThread.cpp
    src/NDVIApp.cpp
)

# Header files (for IDE integration)
set(HEADERS
    include/CaptureThread.h
    include/NDVIApp.h
)

# -----------------------------------------------------------------------------
# Define executable
# -----------------------------------------------------------------------------
add_executable(RazielNDVIpp MACOSX_BUNDLE
    ${SOURCES}
    ${HEADERS}
)

# -----------------------------------------------------------------------------
# Link Qt5 and OpenCV libraries
# -----------------------------------------------------------------------------
target_link_libraries(RazielNDVIpp
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    ${OpenCV_LIBS}
)