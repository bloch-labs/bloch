set(BLOCH_AVAILABLE_FEATURE_FLAGS
    "BLOCH_CLASS_SYSTEM"
)

set(_BLOCH_ENABLED_FEATURE_DEFINITIONS "")
set(_BLOCH_ENABLED_FEATURE_LABELS "")

foreach(_bloch_feature ${BLOCH_AVAILABLE_FEATURE_FLAGS})
    if(NOT _bloch_feature MATCHES "^BLOCH_[A-Z_]+$")
        message(FATAL_ERROR "Invalid feature flag '${_bloch_feature}'. Flags must match BLOCH_[A-Z_]+")
    endif()
    set(_bloch_option "BLOCH_FEATURE_${_bloch_feature}")
    option(${_bloch_option} "Enable Bloch feature flag ${_bloch_feature}" OFF)
    if(${_bloch_option})
        list(APPEND _BLOCH_ENABLED_FEATURE_DEFINITIONS ${_bloch_option})
        list(APPEND _BLOCH_ENABLED_FEATURE_LABELS ${_bloch_feature})
    endif()
endforeach()

# Prefer static OpenSSL to avoid a runtime dependency; fall back to shared if
# static libraries are unavailable. Release artifacts can force static by
# ensuring libssl.a/libcrypto.a are present.
set(OpenSSL_USE_STATIC_LIBS ON)
find_package(OpenSSL QUIET)
if(NOT OpenSSL_FOUND)
    set(OpenSSL_USE_STATIC_LIBS OFF)
    find_package(OpenSSL REQUIRED)
endif()

# ---------------------------------------------------------------------------
# Source layout by responsibility (compiler / runtime / cli / updater)
# ---------------------------------------------------------------------------
set(BLOCH_COMPILER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/bloch/compiler/import/module_loader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bloch/compiler/lexer/lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bloch/compiler/parser/parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bloch/compiler/semantics/built_ins.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bloch/compiler/semantics/semantic_analyser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bloch/compiler/semantics/type_system.cpp
)

set(BLOCH_RUNTIME_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/bloch/runtime/qasm_simulator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bloch/runtime/runtime_evaluator.cpp
)

set(BLOCH_CLI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/bloch/cli/cli.cpp
)

set(BLOCH_UPDATE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/bloch/update/update_manager.cpp
)

set(BLOCH_HTTP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/bloch/http/http_client.cpp
)

# Shared headers and compile flags that are safe across all components.
add_library(bloch_support INTERFACE)
target_include_directories(bloch_support INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(bloch_compiler ${BLOCH_COMPILER_SOURCES})
target_link_libraries(bloch_compiler
    PUBLIC bloch_support
)

add_library(bloch_runtime ${BLOCH_RUNTIME_SOURCES})
target_link_libraries(bloch_runtime
    PUBLIC bloch_compiler bloch_support
)

add_library(bloch_update ${BLOCH_UPDATE_SOURCES})
target_link_libraries(bloch_update
    PUBLIC bloch_support OpenSSL::SSL OpenSSL::Crypto
)
target_compile_definitions(bloch_update PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

add_library(bloch_http ${BLOCH_HTTP_SOURCES})
target_link_libraries(bloch_http
    PUBLIC bloch_support
)

add_library(bloch_cli ${BLOCH_CLI_SOURCES})
target_link_libraries(bloch_cli
    PUBLIC bloch_runtime bloch_update bloch_support
)

target_compile_definitions(bloch_cli PUBLIC
    BLOCH_VERSION="${BLOCH_VERSION}"
    BLOCH_COMMIT_HASH="${BLOCH_COMMIT_HASH}"
)

# Apply feature flags to public libraries (propagates to dependents).
set(_BLOCH_PUBLIC_TARGETS bloch_compiler bloch_runtime bloch_cli bloch_http)
foreach(_bloch_target IN LISTS _BLOCH_PUBLIC_TARGETS)
    foreach(_bloch_definition ${_BLOCH_ENABLED_FEATURE_DEFINITIONS})
        target_compile_definitions(${_bloch_target} PUBLIC ${_bloch_definition}=1)
    endforeach()
endforeach()

add_executable(bloch
    main.cpp
)

target_link_libraries(bloch
    PRIVATE bloch_cli
)

if(_BLOCH_ENABLED_FEATURE_LABELS)
    list(JOIN _BLOCH_ENABLED_FEATURE_LABELS ", " _bloch_enabled_str)
    message(STATUS "Bloch feature flags enabled: ${_bloch_enabled_str}")
endif()

# Install targets for local development installs
install(TARGETS bloch bloch_cli bloch_compiler bloch_runtime bloch_update bloch_http
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bloch
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/stdlib/
    DESTINATION share/bloch/stdlib
    OPTIONAL
)
