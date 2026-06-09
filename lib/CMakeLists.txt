find_package(SQLite3 REQUIRED)

add_library(owt STATIC
    src/database.cpp
    src/repository.cpp
    src/calculator.cpp
    src/converter.cpp
    src/defaults.cpp
    src/data_io.cpp
)

target_include_directories(owt PUBLIC include)
target_link_libraries(owt PUBLIC SQLite3::SQLite3)
