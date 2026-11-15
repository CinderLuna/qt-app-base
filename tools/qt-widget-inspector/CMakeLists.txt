# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Cinderluna

cmake_minimum_required(VERSION 3.24)
project(QtWidgetInspector VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)

add_library(qt_widget_inspector
    "${CMAKE_CURRENT_SOURCE_DIR}/Inspector.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Inspector.h"
)
add_library(QtWidgetInspector::Inspector ALIAS qt_widget_inspector)

target_compile_features(qt_widget_inspector PUBLIC cxx_std_17)
target_compile_definitions(qt_widget_inspector PUBLIC QTAPPBASE_DEBUG)
target_include_directories(qt_widget_inspector PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(qt_widget_inspector PUBLIC Qt6::Widgets)

add_executable(qt_widget_inspector_demo demo.cpp)
target_link_libraries(qt_widget_inspector_demo PRIVATE QtWidgetInspector::Inspector)

if(PROJECT_IS_TOP_LEVEL)
    include(CTest)
endif()

if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_executable(qt_widget_inspector_tests tests.cpp)
    target_link_libraries(qt_widget_inspector_tests PRIVATE QtWidgetInspector::Inspector Qt6::Test)
    add_test(NAME widget_inspector COMMAND qt_widget_inspector_tests -platform offscreen)
endif()
