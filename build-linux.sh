#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# --- Check dependencies ---
MISSING=()
command -v cmake &>/dev/null || MISSING+=("cmake")
command -v make &>/dev/null && command -v ninja &>/dev/null || true
pkg-config --exists sqlite3 2>/dev/null || MISSING+=("libsqlite3-dev")
pkg-config --exists ncurses 2>/dev/null || pkg-config --exists ncursesw 2>/dev/null || MISSING+=("libncurses-dev")

if [ ${#MISSING[@]} -gt 0 ]; then
    echo "ERROR: Missing dependencies: ${MISSING[*]}"
    echo ""
    echo "Install with:"
    echo "  Ubuntu/Debian: sudo apt install ${MISSING[*]}"
    echo "  Arch:          sudo pacman -S cmake sqlite ncurses"
    echo "  Fedora:        sudo dnf install cmake sqlite-devel ncurses-devel"
    exit 1
fi

# --- Configure ---
BUILD_TYPE="${1:-Release}"
BUILD_TYPE="$(echo "$BUILD_TYPE" | sed 's/.*/\L&/;s/^./\U&/')"  # normalize: release -> Release

CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DBUILD_TUI=ON
    -DBUILD_TESTS=ON
    -DBUILD_JNI=OFF
)

# Prefer Ninja if available
if command -v ninja &>/dev/null; then
    CMAKE_ARGS+=(-G Ninja)
fi

echo "Configuring ($BUILD_TYPE)..."
cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" "${CMAKE_ARGS[@]}"

# --- Build ---
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"
echo "Building with $JOBS jobs..."
cmake --build "$BUILD_DIR" --parallel "$JOBS"

# --- Result ---
BINARY="$BUILD_DIR/tui/simple-workout-tracker"
if [ -f "$BINARY" ]; then
    echo ""
    echo "BUILD SUCCESSFUL"
    echo "Binary: $BINARY"
    echo ""
    echo "Run with:"
    echo "  $BINARY"
else
    echo "Build finished but binary not found at expected path."
    echo "Check: find $BUILD_DIR -type f -executable"
fi
