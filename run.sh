#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Cinderluna
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
DEMO_BIN="${BUILD_DIR}/qt_app_base_demo"

# Configure if build directory or CMake cache is missing
if [ ! -f "${BUILD_DIR}/CMakeCache.txt" ]; then
    echo "Configuring build directory..."
    cmake -S "${SCRIPT_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release
fi

# Build project to ensure latest changes are compiled
echo "Building Qt App Base Demo..."
cmake --build "${BUILD_DIR}" -j"$(nproc 2>/dev/null || echo 4)"

# Check display environment if running in interactive graphical mode
if [ -z "${DISPLAY:-}" ] && [ -z "${WAYLAND_DISPLAY:-}" ] && [ -z "${QT_QPA_PLATFORM:-}" ]; then
    echo "Warning: Neither DISPLAY nor WAYLAND_DISPLAY is set. You may need to set QT_QPA_PLATFORM=offscreen or run inside an X11/Wayland session."
fi

echo "Launching Qt App Base Demo..."
exec "${DEMO_BIN}" "$@"
