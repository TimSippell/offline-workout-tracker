@echo off
setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "BUILD_DIR=%SCRIPT_DIR%build"

:: --- Check dependencies ---
where cmake >nul 2>&1 || (
    echo ERROR: cmake not found. Install from https://cmake.org or via winget/scoop.
    exit /b 1
)

:: --- Configure ---
set "BUILD_TYPE=%~1"
if "%BUILD_TYPE%"=="" set "BUILD_TYPE=Release"

set CMAKE_ARGS=-DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DBUILD_TUI=ON -DBUILD_TESTS=ON -DBUILD_JNI=OFF

:: Use vcpkg toolchain if VCPKG_ROOT is set
if defined VCPKG_ROOT (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
    echo Using vcpkg toolchain from %VCPKG_ROOT%
)

echo Configuring (%BUILD_TYPE%)...
cmake -S "%SCRIPT_DIR%" -B "%BUILD_DIR%" %CMAKE_ARGS%
if errorlevel 1 (
    echo CMake configuration failed.
    exit /b 1
)

:: --- Build ---
echo Building...
cmake --build "%BUILD_DIR%" --config %BUILD_TYPE% --parallel
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

:: --- Result ---
set "BINARY=%BUILD_DIR%\tui\%BUILD_TYPE%\simple-workout-tracker.exe"
if not exist "%BINARY%" set "BINARY=%BUILD_DIR%\tui\simple-workout-tracker.exe"

if exist "%BINARY%" (
    echo.
    echo BUILD SUCCESSFUL
    echo Binary: %BINARY%
    echo.
    echo Run with:
    echo   %BINARY%
) else (
    echo Build finished but binary not found at expected path.
    echo Check: dir /s /b "%BUILD_DIR%\*.exe"
)
