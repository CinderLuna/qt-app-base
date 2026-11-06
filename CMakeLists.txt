# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Cinderluna

cmake_minimum_required(VERSION 3.24)
project(QtAppBase VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

option(QT_APP_BASE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(BUILD_DEMO "Build the demonstration application" ON)
option(BUILD_TESTING "Build the test suite" ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Widgets Svg)

# --- Core Library Sources ---
set(QTAPPBASE_SOURCES
    src/WindowTitleLabel.cpp
    src/WindowHeader.cpp
    src/ResizeGripPainter.cpp
    src/BaseStatusBar.cpp
    src/Theme.cpp
    src/UiIcons.cpp
    src/BaseWindow.cpp
    src/BaseWindowChrome.cpp
    src/SearchLineEdit.cpp
)

set(QTAPPBASE_HEADERS
    include/qtappbase/WindowTitleLabel.h
    include/qtappbase/WindowHeader.h
    include/qtappbase/ResizeGripPainter.h
    include/qtappbase/BaseStatusBar.h
    include/qtappbase/Theme.h
    include/qtappbase/UiIcons.h
    include/qtappbase/BaseWindow.h
    include/qtappbase/SearchLineEdit.h
)

add_library(qtappbase STATIC ${QTAPPBASE_SOURCES} ${QTAPPBASE_HEADERS})
add_library(QtAppBase::Base ALIAS qtappbase)

target_include_directories(qtappbase
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(qtappbase
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Svg
)

if(WIN32)
    target_link_libraries(qtappbase PRIVATE dwmapi)
endif()

# Embedded Assets
file(GLOB FLUENT_TOOLBAR_ICONS CONFIGURE_DEPENDS "assets/icons/fluent/*.svg")
qt_add_resources(qtappbase toolbar_icons PREFIX "/icons" BASE "assets/icons" FILES ${FLUENT_TOOLBAR_ICONS})
qt_add_resources(qtappbase brand_icons PREFIX "/brand" BASE "assets/brand"
    FILES assets/brand/app-icon.svg assets/brand/app-mark.svg
)

# Compiler Warning Flags
if(MSVC)
    target_compile_options(qtappbase PRIVATE /W4 /utf-8 /GS /sdl)
    if(QT_APP_BASE_WARNINGS_AS_ERRORS)
        target_compile_options(qtappbase PRIVATE /WX)
    endif()
else()
    target_compile_options(qtappbase PRIVATE -Wall -Wextra -Wpedantic)
    if(QT_APP_BASE_WARNINGS_AS_ERRORS)
        target_compile_options(qtappbase PRIVATE -Werror)
    endif()
endif()

# --- Demo Executable ---
if(BUILD_DEMO)
    add_executable(qt_app_base_demo src/main.cpp)
    set_target_properties(qt_app_base_demo PROPERTIES WIN32_EXECUTABLE "$<NOT:$<CONFIG:Debug>>")
    target_link_libraries(qt_app_base_demo PRIVATE qtappbase)
endif()

# --- Testing ---
if(BUILD_TESTING)
    include(CTest)
    find_package(Qt6 COMPONENTS Test QUIET)
    if(Qt6Test_FOUND)
        add_executable(qt_app_base_tests tests/window_tests.cpp)
        target_link_libraries(qt_app_base_tests PRIVATE qtappbase Qt6::Test)
        add_test(NAME window_tests COMMAND qt_app_base_tests -platform offscreen)
    endif()
endif()
