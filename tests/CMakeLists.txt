enable_testing()

add_executable(bloch_tests
    test_main.cpp
    test_lexer.cpp
    test_ast.cpp
    test_parser.cpp
    test_semantics.cpp
    test_runtime.cpp
    test_integration.cpp
)

target_link_libraries(bloch_tests
    bloch_lib
)

add_test(NAME bloch_tests COMMAND bloch_tests)
