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

file(GLOB_RECURSE BLOCH_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/bloch/*.cpp
)

# Prefer static OpenSSL to avoid a runtime dependency; fall back to shared if
# static libraries are unavailable. Release artifacts can force static by
# ensuring libssl.a/libcrypto.a are present.
set(OpenSSL_USE_STATIC_LIBS ON)
find_package(OpenSSL QUIET)
if(NOT OpenSSL_FOUND)
    set(OpenSSL_USE_STATIC_LIBS OFF)
    find_package(OpenSSL REQUIRED)
endif()

add_library(bloch_lib ${BLOCH_SOURCES})

target_include_directories(bloch_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(bloch_lib PUBLIC
    CPPHTTPLIB_OPENSSL_SUPPORT
)

target_link_libraries(bloch_lib
    PUBLIC OpenSSL::SSL OpenSSL::Crypto
)

foreach(_bloch_definition ${_BLOCH_ENABLED_FEATURE_DEFINITIONS})
    target_compile_definitions(bloch_lib PUBLIC ${_bloch_definition}=1)
endforeach()

add_executable(bloch
    main.cpp
)

target_link_libraries(bloch
    bloch_lib
)

foreach(_bloch_definition ${_BLOCH_ENABLED_FEATURE_DEFINITIONS})
    target_compile_definitions(bloch PRIVATE ${_bloch_definition}=1)
endforeach()

if(_BLOCH_ENABLED_FEATURE_LABELS)
    list(JOIN _BLOCH_ENABLED_FEATURE_LABELS ", " _bloch_enabled_str)
    message(STATUS "Bloch feature flags enabled: ${_bloch_enabled_str}")
endif()

# Embed the version string into the binary
target_compile_definitions(bloch PRIVATE
    BLOCH_VERSION="${BLOCH_VERSION}"
    BLOCH_COMMIT_HASH="${BLOCH_COMMIT_HASH}"
)

# Install target for local development installs
install(TARGETS bloch RUNTIME DESTINATION bin)
