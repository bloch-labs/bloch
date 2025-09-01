cmake_minimum_required(VERSION 3.16)
project(bloch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# Determine version from CI tag or local git
set(BLOCH_VERSION "unknown")
if(DEFINED ENV{CI_COMMIT_TAG} AND NOT "$ENV{CI_COMMIT_TAG}" STREQUAL "")
    # GitLab CI provides the release tag in CI_COMMIT_TAG when building a tagged release
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
