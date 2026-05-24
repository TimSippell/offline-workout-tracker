find_package(SQLite3 REQUIRED)

add_library(swt STATIC
    src/database.cpp
    src/repository.cpp
    src/calculator.cpp
)

target_include_directories(swt PUBLIC include)
target_link_libraries(swt PUBLIC SQLite3::SQLite3)
