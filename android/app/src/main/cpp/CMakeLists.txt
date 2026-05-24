cmake_minimum_required(VERSION 3.22.1)
project(swt-android LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SWT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../../..)

# SQLite (bundled for Android)
add_library(sqlite3 STATIC
    ${SWT_ROOT}/android/app/src/main/cpp/sqlite3.c
)
target_include_directories(sqlite3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# SWT library
add_library(swt STATIC
    ${SWT_ROOT}/lib/src/database.cpp
    ${SWT_ROOT}/lib/src/repository.cpp
    ${SWT_ROOT}/lib/src/calculator.cpp
)
target_include_directories(swt PUBLIC ${SWT_ROOT}/lib/include)
target_link_libraries(swt PUBLIC sqlite3)

# JNI bridge
add_library(swt-jni SHARED
    ${SWT_ROOT}/lib/jni/swt_jni.cpp
)
target_link_libraries(swt-jni PRIVATE swt log)
target_include_directories(swt-jni PRIVATE ${SWT_ROOT}/lib/include)
