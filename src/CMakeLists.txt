set(BLOCH_SOURCES
    bloch/lexer/lexer.cpp
    bloch/parser/parser.cpp
    bloch/semantics/semantic_analyser.cpp
    bloch/semantics/type_system.cpp
    bloch/semantics/built_ins.cpp
    bloch/runtime/runtime_evaluator.cpp
    bloch/runtime/qasm_simulator.cpp
)

add_library(bloch_lib ${BLOCH_SOURCES})

target_include_directories(bloch_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(bloch
    main.cpp
)

target_link_libraries(bloch
    bloch_lib
)

# Embed the version string into the binary
target_compile_definitions(bloch PRIVATE BLOCH_VERSION="${BLOCH_VERSION}")

# Install target for local development installs
install(TARGETS bloch RUNTIME DESTINATION bin)
