#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# --- Parse flags ---
BUILD_TUI=OFF
BUILD_GUI=ON

for arg in "$@"; do
    case "$arg" in
        --tui) BUILD_TUI=ON; BUILD_GUI=OFF ;;
        debug|Debug) BUILD_TYPE_ARG="Debug" ;;
        release|Release) BUILD_TYPE_ARG="Release" ;;
    esac
done

# --- Check dependencies ---
MISSING=()
command -v cmake &>/dev/null || MISSING+=("cmake")
command -v make &>/dev/null && command -v ninja &>/dev/null || true

if [ "$BUILD_GUI" = "ON" ]; then
    pkg-config --exists glfw3 2>/dev/null || MISSING+=("libglfw3-dev")
fi
if [ "$BUILD_TUI" = "ON" ]; then
    pkg-config --exists ncurses 2>/dev/null || pkg-config --exists ncursesw 2>/dev/null || MISSING+=("libncurses-dev")
fi
pkg-config --exists sqlite3 2>/dev/null || MISSING+=("libsqlite3-dev")

if [ ${#MISSING[@]} -gt 0 ]; then
    echo "ERROR: Missing dependencies: ${MISSING[*]}"
    echo ""
    echo "Install with:"
    echo "  Ubuntu/Debian: sudo apt install ${MISSING[*]}"
    echo "  Arch:          sudo pacman -S cmake sqlite ncurses glfw"
    echo "  Fedora:        sudo dnf install cmake sqlite-devel ncurses-devel glfw-devel"
    exit 1
fi

# --- Configure ---
BUILD_TYPE="${BUILD_TYPE_ARG:-Release}"

CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DBUILD_GUI=$BUILD_GUI
    -DBUILD_TUI=$BUILD_TUI
    -DBUILD_TESTS=ON
    -DBUILD_JNI=OFF
)

# Prefer Ninja if available
if command -v ninja &>/dev/null; then
    CMAKE_ARGS+=(-G Ninja)
fi

echo "Configuring ($BUILD_TYPE, GUI=$BUILD_GUI, TUI=$BUILD_TUI)..."
cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" "${CMAKE_ARGS[@]}"

# --- Build ---
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"
echo "Building with $JOBS jobs..."
cmake --build "$BUILD_DIR" --parallel "$JOBS"

# --- Result ---
if [ "$BUILD_GUI" = "ON" ]; then
    BINARY="$BUILD_DIR/gui/swt-gui"
else
    BINARY="$BUILD_DIR/tui/simple-workout-tracker"
fi

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
