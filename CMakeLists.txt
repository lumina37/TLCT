cmake_minimum_required(VERSION 3.12)
project(TLCT LANGUAGES CXX)

set(CMAKE_PROJECT_VERSION_MAJOR 0)
set(CMAKE_PROJECT_VERSION_MINOR 1)
set(CMAKE_PROJECT_VERSION_PATCH 0)
set(CMAKE_PROJECT_VERSION ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

option(BUILD_TESTS "Build tests" OFF)
#option(BUILD_SHARED_LIBS "Specifies the type of libraries (SHARED or STATIC) to build" OFF)
#option(BUILD_WITH_MT "Build libraries as MultiThreaded DLL (Windows Only)" OFF)

include(cmake/dependencies.cmake)

add_subdirectory(src)
if (BUILD_TESTS)
    add_subdirectory(tests)
endif ()