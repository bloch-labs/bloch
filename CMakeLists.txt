cmake_minimum_required(VERSION 3.16)
project(bloch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


option(BLOCH_ENABLE_COVERAGE "Enable coverage instrumentation" OFF)

if(BLOCH_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Coverage instrumentation enabled")
        add_compile_options(-O0 -g --coverage)
        add_link_options(--coverage)
    else()
        message(WARNING "Coverage instrumentation not supported for compiler ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()


# Determine version from CI tag or local git
set(BLOCH_VERSION "unknown")
if(DEFINED ENV{BLOCH_VERSION} AND NOT "$ENV{BLOCH_VERSION}" STREQUAL "")
    # Explicit override (recommended in CI)
    set(BLOCH_VERSION "$ENV{BLOCH_VERSION}")
elseif(DEFINED ENV{GITHUB_REF_NAME} AND NOT "$ENV{GITHUB_REF_NAME}" STREQUAL "")
    # GitHub Actions tag name when building a tagged release
    set(BLOCH_VERSION "$ENV{GITHUB_REF_NAME}")
elseif(DEFINED ENV{CI_COMMIT_TAG} AND NOT "$ENV{CI_COMMIT_TAG}" STREQUAL "")
    # GitLab CI tag compatibility
    set(BLOCH_VERSION "$ENV{CI_COMMIT_TAG}")
else()
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty=-modified
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_DESCRIBE
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE GIT_DESCRIBE_RESULT
        )
        if(GIT_DESCRIBE_RESULT EQUAL 0 AND NOT "${GIT_DESCRIBE}" STREQUAL "")
            set(BLOCH_VERSION "${GIT_DESCRIBE}")
        endif()
    endif()
endif()
message(STATUS "Bloch version: ${BLOCH_VERSION}")


# Treat warnings as errors
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-Wall -Wextra -Werror -pedantic)
endif()

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)

# CPack packaging configuration (used in release builds)
set(CPACK_PACKAGE_NAME "bloch")
string(REGEX REPLACE "^v" "" CPACK_PACKAGE_VERSION "${BLOCH_VERSION}")
set(CPACK_PACKAGE_VENDOR "Bloch Labs")
set(CPACK_PACKAGE_CONTACT "hello@bloch-labs.com")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Bloch Labs <hello@bloch-labs.com>")
include(CPack)
