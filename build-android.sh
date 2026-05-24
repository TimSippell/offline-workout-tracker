#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ANDROID_DIR="$SCRIPT_DIR/android"

# --- Configuration ---
# Set ANDROID_SDK_ROOT if not already set
if [ -z "${ANDROID_SDK_ROOT:-}" ]; then
    if [ -d "$HOME/Android/Sdk" ]; then
        export ANDROID_SDK_ROOT="$HOME/Android/Sdk"
    elif [ -d "$HOME/Library/Android/sdk" ]; then
        export ANDROID_SDK_ROOT="$HOME/Library/Android/sdk"
    else
        echo "ERROR: ANDROID_SDK_ROOT not set and SDK not found in default locations."
        echo "Install Android SDK or set ANDROID_SDK_ROOT."
        exit 1
    fi
fi

echo "Using Android SDK: $ANDROID_SDK_ROOT"

# --- Download SQLite amalgamation if needed ---
SQLITE_DIR="$ANDROID_DIR/app/src/main/cpp"
if [ ! -f "$SQLITE_DIR/sqlite3.c" ]; then
    echo "Downloading SQLite amalgamation..."
    SQLITE_URL="https://www.sqlite.org/2024/sqlite-amalgamation-3470200.zip"
    TMPFILE=$(mktemp)
    curl -sL "$SQLITE_URL" -o "$TMPFILE"
    unzip -qo "$TMPFILE" -d /tmp/sqlite-extract
    cp /tmp/sqlite-extract/sqlite-amalgamation-*/sqlite3.c "$SQLITE_DIR/"
    cp /tmp/sqlite-extract/sqlite-amalgamation-*/sqlite3.h "$SQLITE_DIR/"
    rm -rf "$TMPFILE" /tmp/sqlite-extract
    echo "SQLite downloaded."
fi

# --- Ensure Gradle wrapper exists ---
if [ ! -f "$ANDROID_DIR/gradlew" ]; then
    echo "Generating Gradle wrapper..."
    cd "$ANDROID_DIR"
    if command -v gradle &>/dev/null; then
        gradle wrapper --gradle-version 8.11.1
    else
        # Download wrapper jar directly
        mkdir -p gradle/wrapper
        WRAPPER_URL="https://raw.githubusercontent.com/gradle/gradle/v8.11.1/gradle/wrapper/gradle-wrapper.jar"
        curl -sL "https://services.gradle.org/distributions/gradle-8.11.1-bin.zip" -o /dev/null
        cat > gradlew << 'GRADLEW'
#!/bin/sh
exec java -jar "$(dirname "$0")/gradle/wrapper/gradle-wrapper.jar" "$@"
GRADLEW
        chmod +x gradlew
        # Use sdkmanager's gradle or download wrapper jar
        echo "WARNING: gradle not found. Install Gradle or download wrapper jar manually."
        echo "Place gradle-wrapper.jar in android/gradle/wrapper/"
    fi
    cd "$SCRIPT_DIR"
fi

# --- Build ---
BUILD_TYPE="${1:-release}"

cd "$ANDROID_DIR"
rm -rf app/build/outputs/apk
echo "Building $BUILD_TYPE APK..."

if [ "$BUILD_TYPE" = "release" ]; then
    ./gradlew assembleRelease
    APK_DIR="app/build/outputs/apk/release"
else
    ./gradlew assembleDebug
    APK_DIR="app/build/outputs/apk/debug"
fi

APK_PATH=$(find "$APK_DIR" -name '*.apk' -print -quit 2>/dev/null)

if [ -n "$APK_PATH" ]; then
    echo ""
    echo "BUILD SUCCESSFUL"
    echo "APK: $ANDROID_DIR/$APK_PATH"
    echo ""
    echo "To install on device:"
    echo "  adb install $ANDROID_DIR/$APK_PATH"
else
    echo "Build finished but no APK found."
    echo "Check: find app/build/outputs -name '*.apk'"
fi
