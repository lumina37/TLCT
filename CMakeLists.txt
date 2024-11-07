cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

include(cmake/policies.cmake)

project(TLCT
        VERSION 0.16.0.4
        LANGUAGES CXX
)

include(cmake/options.cmake)
include(cmake/dependencies.cmake)
include(cmake/helper.cmake)

add_subdirectory(src)
if (TLCT_BUILD_TESTS)
    add_subdirectory(tests)
endif ()
