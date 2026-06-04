cmake_minimum_required(VERSION 3.16)
project(simple-workout-tracker VERSION 1.2.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_GUI "Build the Dear ImGui GUI application" ON)
option(BUILD_TUI "Build the ncurses TUI application" OFF)
option(BUILD_JNI "Build JNI bindings for Android" OFF)
option(BUILD_TESTS "Build tests" ON)

add_subdirectory(lib)

if(BUILD_GUI)
    add_subdirectory(gui)
endif()

if(BUILD_TUI)
    add_subdirectory(tui)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_JNI)
    add_library(swt-jni SHARED lib/jni/swt_jni.cpp)
    target_link_libraries(swt-jni PRIVATE swt)
    target_include_directories(swt-jni PRIVATE lib/include)
endif()
