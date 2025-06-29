add_library(bloch_lib
    bloch/version/version.cpp
)

target_include_directories(bloch_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(bloch
    main.cpp
)

target_link_libraries(bloch
    bloch_lib
)