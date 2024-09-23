include(CMakeDependentOption)

option(TLCT_BUILD_TESTS "Build tests" OFF)
option(TLCT_HEADER_ONLY "Enable the header-only mode" OFF)
cmake_dependent_option(TLCT_BUILD_SHARED_LIBS "Specifies the type of TLCT to build" ON
        "NOT TLCT_HEADER_ONLY AND BUILD_SHARED_LIBS" OFF)
cmake_dependent_option(TLCT_ENABLE_LTO "Enable full link-time-optimizations (LTO)" OFF
        "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)
option(TLCT_ENABLE_INSPECT "Enable inspector of rendering" OFF)
option(TLCT_VERBOSE_WARNING "Show more verbose compiler warnings" OFF)

if (MSVC)
    option(TLCT_WITH_STATIC_CRT "Link with STATIC CRT" OFF)
    set(MSVC_RUNTIME_TYPE MultiThreaded$<$<NOT:$<BOOL:${TLCT_WITH_STATIC_CRT}>>:DLL>)
endif ()
