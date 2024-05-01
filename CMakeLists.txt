cmake_minimum_required(VERSION 3.21)

include(cmake/polices.cmake)

project(TLCT VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include(cmake/options.cmake)
include(cmake/dependencies.cmake)
include(cmake/helper.cmake)

add_subdirectory(src)
if (BUILD_TESTS)
    add_subdirectory(tests)
endif ()
