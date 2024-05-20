include(CMakeDependentOption)

option(TLCT_BUILD_TESTS "Build tests" OFF)
option(TLCT_HEADER_ONLY "Enable the header-only mode" OFF)
option(BUILD_SHARED_LIBS "Specifies the type of libraries (SHARED or STATIC) to build" ON)
cmake_dependent_option(TLCT_BUILD_SHARED_LIBS "Specifies the type of TLCT to build" ON
        "BUILD_SHARED_LIBS;NOT TLCT_HEADER_ONLY" OFF)
option(TLCT_LOCAL_DEPS "Dependencies via local git repo for offline build" OFF)

if (MSVC)
    option(TLCT_WITH_STATIC_CRT "Link with STATIC CRT (this wont work if OpenCV is already linked with SHARED CRT)" OFF)
    set(MSVC_RUNTIME_TYPE MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<NOT:$<BOOL:${TLCT_WITH_STATIC_CRT}>>:DLL>)
endif ()
