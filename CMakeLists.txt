cmake_minimum_required(VERSION 3.12)
project(TLCT LANGUAGES CXX)

set(CMAKE_PROJECT_VERSION_MAJOR 0)
set(CMAKE_PROJECT_VERSION_MINOR 1)
set(CMAKE_PROJECT_VERSION_PATCH 0)
set(CMAKE_PROJECT_VERSION ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

option(BUILD_TESTS "Build tests" OFF)
#option(BUILD_SHARED_LIBS "Specifies the type of libraries (SHARED or STATIC) to build" ON)

include(cmake/dependencies.cmake)

add_subdirectory(src)
if (BUILD_TESTS)
    add_subdirectory(tests)
endif ()
