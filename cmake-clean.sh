#!/usr/bin/env bash
# cmake-clean.sh

set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"

rm -rf "$BUILD_DIR"
find . -name CMakeCache.txt -delete
find . -name CMakeFiles -type d -prune -exec rm -rf {} +
find . -name cmake_install.cmake -delete
find . -name CTestTestfile.cmake -delete
find . -name Makefile -delete
