include(CMakeDependentOption)

option(TLCT_BUILD_TESTS "Build tests" OFF)
option(TLCT_HEADER_ONLY "Enable the header-only mode" OFF)
cmake_dependent_option(TLCT_BUILD_SHARED_LIBS "Specifies the type of TLCT to build" OFF
        "NOT TLCT_HEADER_ONLY" OFF)
option(TLCT_ENABLE_INSPECT "Enable inspector of rendering" OFF)

if (MSVC)
    option(TLCT_WITH_STATIC_CRT "Link with STATIC CRT (this wont work if OpenCV is already linked with SHARED CRT)" OFF)
    set(MSVC_RUNTIME_TYPE MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<NOT:$<BOOL:${TLCT_WITH_STATIC_CRT}>>:DLL>)
endif ()
