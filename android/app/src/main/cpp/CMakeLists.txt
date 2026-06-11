cmake_minimum_required(VERSION 3.22.1)
project(owt-android LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OWT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../../..)

# SQLite (bundled for Android)
add_library(sqlite3 STATIC
    ${OWT_ROOT}/android/app/src/main/cpp/sqlite3.c
)
target_include_directories(sqlite3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# OWT library
add_library(owt STATIC
    ${OWT_ROOT}/lib/src/database.cpp
    ${OWT_ROOT}/lib/src/repository.cpp
    ${OWT_ROOT}/lib/src/calculator.cpp
    ${OWT_ROOT}/lib/src/converter.cpp
    ${OWT_ROOT}/lib/src/defaults.cpp
    ${OWT_ROOT}/lib/src/data_io.cpp
)
target_include_directories(owt PUBLIC ${OWT_ROOT}/lib/include)
target_link_libraries(owt PUBLIC sqlite3)

# JNI bridge
add_library(owt-jni SHARED
    ${OWT_ROOT}/lib/jni/owt_jni.cpp
)
target_link_libraries(owt-jni PRIVATE owt log)
target_include_directories(owt-jni PRIVATE ${OWT_ROOT}/lib/include)

# 16 KB page size support (required by Google Play)
target_link_options(owt-jni PRIVATE "-Wl,-z,max-page-size=16384")
