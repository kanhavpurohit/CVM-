#!/usr/bin/env bash
# build.sh — configure + build CVM++ via CMake, then copy cvm.exe to project root.
# Usage:   ./build.sh           (Release)
#          ./build.sh Debug     (Debug)
#          ./build.sh --clean   (wipe build/ first)

set -euo pipefail

cd "$(dirname "$0")"

CONFIG="Release"
CLEAN=0
for arg in "$@"; do
    case "$arg" in
        --clean|-c) CLEAN=1 ;;
        Debug|Release|RelWithDebInfo|MinSizeRel) CONFIG="$arg" ;;
        *) echo "Unknown arg: $arg" >&2; exit 1 ;;
    esac
done

if ! command -v cmake >/dev/null 2>&1; then
    echo "CMake not found. Install it or add it to PATH." >&2
    exit 1
fi

if [[ "$CLEAN" -eq 1 && -d build ]]; then
    echo "Cleaning build/..."
    rm -rf build
fi

echo "Configuring ($CONFIG)..."
cmake -B build -S .

echo "Building..."
cmake --build build --config "$CONFIG"

# MSVC drops the exe under build/<Config>/; single-config generators put it in build/.
for candidate in "build/$CONFIG/cvm.exe" "build/cvm.exe" "build/$CONFIG/cvm" "build/cvm"; do
    if [[ -f "$candidate" ]]; then
        cp "$candidate" "./cvm.exe"
        echo
        echo "Built: ./cvm.exe"
        echo "Try:   ./cvm.exe examples/loop.cvm"
        exit 0
    fi
done

echo "Build succeeded but cvm.exe not found." >&2
exit 1
