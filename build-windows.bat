@echo off
setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "BUILD_DIR=%SCRIPT_DIR%build"

:: --- Check dependencies ---
where cmake >nul 2>&1 || (
    echo ERROR: cmake not found. Install from https://cmake.org or via winget/scoop.
    exit /b 1
)

:: --- Parse flags ---
set "BUILD_TUI=OFF"
set "BUILD_GUI=ON"
set "BUILD_TYPE=Release"

:parse_args
if "%~1"=="" goto done_args
if /i "%~1"=="--tui" (
    set "BUILD_TUI=ON"
    set "BUILD_GUI=OFF"
)
if /i "%~1"=="debug" set "BUILD_TYPE=Debug"
if /i "%~1"=="release" set "BUILD_TYPE=Release"
shift
goto parse_args
:done_args

:: --- Configure ---
set CMAKE_ARGS=-DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DBUILD_GUI=%BUILD_GUI% -DBUILD_TUI=%BUILD_TUI% -DBUILD_TESTS=ON -DBUILD_JNI=OFF

:: Use vcpkg toolchain if VCPKG_ROOT is set
if defined VCPKG_ROOT (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
    echo Using vcpkg toolchain from %VCPKG_ROOT%
)

echo Configuring (%BUILD_TYPE%, GUI=%BUILD_GUI%, TUI=%BUILD_TUI%)...
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
if "%BUILD_GUI%"=="ON" (
    set "BINARY=%BUILD_DIR%\gui\%BUILD_TYPE%\swt-gui.exe"
    if not exist "!BINARY!" set "BINARY=%BUILD_DIR%\gui\swt-gui.exe"
) else (
    set "BINARY=%BUILD_DIR%\tui\%BUILD_TYPE%\simple-workout-tracker.exe"
    if not exist "!BINARY!" set "BINARY=%BUILD_DIR%\tui\simple-workout-tracker.exe"
)

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
