cmake_minimum_required(VERSION 3.21)

include(cmake/polices.cmake)

project(TLCT LANGUAGES CXX)

set(CMAKE_PROJECT_VERSION_MAJOR 0)
set(CMAKE_PROJECT_VERSION_MINOR 1)
set(CMAKE_PROJECT_VERSION_PATCH 0)
set(CMAKE_PROJECT_VERSION ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include(cmake/options.cmake)
include(cmake/dependencies.cmake)

add_subdirectory(src)
if (BUILD_TESTS)
    add_subdirectory(tests)
endif ()
