#!/usr/bin/env bash
# cmake-install.sh

set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"

cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -j"$(nproc)"
sudo cmake --install "$BUILD_DIR"
