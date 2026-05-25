find_package(GTest REQUIRED)

add_executable(swt_tests
    calculator_test.cpp
    converter_test.cpp
    repository_test.cpp
    data_io_test.cpp
    defaults_test.cpp
)

target_link_libraries(swt_tests PRIVATE swt GTest::gtest GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(swt_tests)
