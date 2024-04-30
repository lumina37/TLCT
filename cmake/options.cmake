include(CMakeDependentOption)

option(TLCT_BUILD_TESTS "Build tests" OFF)
option(TLCT_HEADER_ONLY "Enable the header-only mode" OFF)
option(BUILD_SHARED_LIBS "Specifies the type of libraries (SHARED or STATIC) to build" ON)
cmake_dependent_option(TLCT_BUILD_SHARED_LIBS "Specifies the type of TLCT to build" ON
        "BUILD_SHARED_LIBS;NOT TLCT_HEADER_ONLY" OFF)

set(TLCT_CONFIGURE_DIR "${CMAKE_SOURCE_DIR}/src/include/tlct/common")
configure_file("${TLCT_CONFIGURE_DIR}/cmake.h.in" "${TLCT_CONFIGURE_DIR}/cmake.h")
