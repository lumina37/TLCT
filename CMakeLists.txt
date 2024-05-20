cmake_minimum_required(VERSION 3.21)

include(cmake/policies.cmake)

project(TLCT VERSION 0.4.1 LANGUAGES CXX)

include(cmake/options.cmake)
include(cmake/dependencies.cmake)
include(cmake/helper.cmake)

add_subdirectory(src)
if (TLCT_BUILD_TESTS)
    add_subdirectory(tests)
endif ()
