# Simple Workout Tracker

A minimal, offline-first workout tracker for Android. Built with a C++ core library and a Kotlin/Jetpack Compose UI.

<p align="center">
  <img src="screenshots/Workout.png" alt="Active workout" width="270">
  &nbsp;&nbsp;&nbsp;&nbsp;
  <img src="screenshots/Exercises.png" alt="Exercise library" width="270">
</p>

## Features

- Track sets, reps, and weight for each exercise
- Built-in exercise library with muscle group and equipment tags
- Create and reuse workout templates
- View workout history and progress over time
- Weight unit conversion (kg/lbs)
- All data stored locally in SQLite — no account required

## Architecture

The core logic lives in a shared C++ library (`lib/`) backed by SQLite. This library is used by:

- **Android app** — Kotlin + Jetpack Compose, calls the C++ lib via JNI
- **TUI** — terminal interface for desktop use

## Dependencies

### Core library (`lib/`)

- C++20 compiler (GCC 10+, Clang 12+)
- CMake 3.16+
- SQLite 3

### TUI (`tui/`)

- ncurses

### Android app (`android/`)

- Android SDK (API 35) + NDK
- Gradle 8.11+
- Kotlin 2.1 + Jetpack Compose
- Java 17

## Building

### Linux (TUI)

```sh
./build-linux.sh            # release build (default)
./build-linux.sh debug      # debug build
```

### Android APK

```sh
./build-android.sh          # release build (default)
./build-android.sh debug    # debug build
```

The script downloads the SQLite amalgamation automatically on first run.

## License

MIT
