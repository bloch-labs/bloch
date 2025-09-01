enable_testing()

set(TEST_SOURCES
    test_main.cpp
    test_lexer.cpp
    test_ast.cpp
    test_parser.cpp
    test_semantics.cpp
    test_runtime.cpp
)

if (NOT WIN32)
    list(APPEND TEST_SOURCES test_integration.cpp)
endif()

add_executable(bloch_tests ${TEST_SOURCES})

target_link_libraries(bloch_tests
    bloch_lib
)

add_test(NAME bloch_tests COMMAND bloch_tests)
