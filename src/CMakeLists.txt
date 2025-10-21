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

add_library(bloch_lib ${BLOCH_SOURCES})

target_include_directories(bloch_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
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
target_compile_definitions(bloch PRIVATE BLOCH_VERSION="${BLOCH_VERSION}")

# Install target for local development installs
install(TARGETS bloch RUNTIME DESTINATION bin)
