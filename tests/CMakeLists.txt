find_package(GTest REQUIRED)

add_executable(owt_tests
    calculator_test.cpp
    converter_test.cpp
    repository_test.cpp
    data_io_test.cpp
    defaults_test.cpp
)

target_link_libraries(owt_tests PRIVATE owt GTest::gtest GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(owt_tests)
