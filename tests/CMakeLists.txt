include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(bloch_tests
    test_lexer.cpp
    test_ast.cpp
    test_parser.cpp
    test_semantics.cpp
)

target_link_libraries(bloch_tests
    gtest_main
    bloch_lib
)

include(GoogleTest)
gtest_discover_tests(bloch_tests)
