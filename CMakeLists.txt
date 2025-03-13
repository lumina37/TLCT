cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

# The pre-defined value will not be replaced by default value from `option`.
cmake_policy(SET CMP0077 NEW)
# Enable target property setting `MSVC_RUNTIME_LIBRARY`.
cmake_policy(SET CMP0091 NEW)

project(tlct
        VERSION 1.4.3
        DESCRIPTION "The official implementation of the Tsinghua Lenslet Conversion Tool"
        HOMEPAGE_URL "https://github.com/lumina37/TLCT"
        LANGUAGES CXX
)

set(TLCT_VERSION_BETA_SUFFIX "alpha.3")
if (NOT TLCT_VERSION_BETA_SUFFIX)
    set(TLCT_VERSION ${PROJECT_VERSION})
else ()
    set(TLCT_VERSION ${PROJECT_VERSION}-${TLCT_VERSION_BETA_SUFFIX})
endif ()

# Compile Options
include(CMakeDependentOption)

option(TLCT_ENABLE_TESTS "Enable tests" OFF)
cmake_dependent_option(TLCT_ENABLE_LTO "Enable full link-time-optimizations (LTO)" OFF
        "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)
option(TLCT_VERBOSE_WARNING "Show verbose compiler warnings" OFF)

if (MSVC)
    option(TLCT_WITH_STATIC_CRT "Link with STATIC CRT" OFF)
    set(_TLCT_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<NOT:$<BOOL:${TLCT_WITH_STATIC_CRT}>>:DLL>)
endif ()

# Dependencies
include(FetchContent)

find_package(OpenCV
        REQUIRED COMPONENTS imgproc
        OPTIONAL_COMPONENTS imgcodecs
)

set(TLCT_ARGPARSE_PATH "https://github.com/p-ranav/argparse/archive/refs/tags/v3.2.tar.gz" CACHE STRING
        "Specifies the path of argparse")

FetchContent_Declare(
        argparse
        DOWNLOAD_EXTRACT_TIMESTAMP ON
        URL ${TLCT_ARGPARSE_PATH}
)
FetchContent_MakeAvailable(argparse)

if (TLCT_ENABLE_TESTS)
    include(CTest)

    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.8.0
    )
    FetchContent_MakeAvailable(Catch2)
    include(Catch)

    FetchContent_Declare(
            tlct-test-data
            GIT_REPOSITORY https://github.com/lumina37/TLCT-test-data.git
            GIT_TAG 0c5a1e0f8beacecb05123edc12eff8a4a1f3b616
    )
    FetchContent_MakeAvailable(tlct-test-data)
    set(TLCT_TESTDATA_DIR "${tlct-test-data_SOURCE_DIR}")
endif ()

# Helper
function(tlct_copy_dlls_if_needed name)
    if (WIN32)
        add_custom_command(TARGET ${name} POST_BUILD COMMAND ${CMAKE_COMMAND} -E
                copy_if_different $<TARGET_RUNTIME_DLLS:${name}> $<TARGET_FILE:${name}> $<TARGET_FILE_DIR:${name}> COMMAND_EXPAND_LISTS)
    endif ()
endfunction()

set(TLCT_COMPILE_INFO "[TLCT v${TLCT_VERSION}] [OpenCV v${OpenCV_VERSION}] by [${CMAKE_CXX_COMPILER_ID} v${CMAKE_CXX_COMPILER_VERSION} (${CMAKE_SYSTEM_PROCESSOR})]")

set(TLCT_CONFIGURE_DIR "${PROJECT_SOURCE_DIR}/src/include/tlct/common")
configure_file("${TLCT_CONFIGURE_DIR}/config.h.in" "${TLCT_CONFIGURE_DIR}/config.h" @ONLY)

# Add lib/bin/tests
add_subdirectory(src)
if (TLCT_ENABLE_TESTS)
    add_subdirectory(tests)
endif ()

# Install
include(GNUInstallDirs)

if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include(CMakePackageConfigHelpers)

    install(TARGETS ${PROJECT_NAME}-lib-dynamic argparse
            EXPORT ${PROJECT_NAME}Targets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    install(TARGETS ${PROJECT_NAME}-lib-static
            EXPORT ${PROJECT_NAME}Targets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    install(EXPORT ${PROJECT_NAME}Targets
            FILE ${PROJECT_NAME}Targets.cmake
            NAMESPACE tlct::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    configure_package_config_file(
            "${PROJECT_SOURCE_DIR}/cmake/config.cmake.in"
            "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    write_basic_package_version_file(
            "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY ExactVersion
    )

    install(FILES
            "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
            "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )
endif ()
