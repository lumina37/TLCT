cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

# The pre-defined value will not be replaced by default value from `option`.
cmake_policy(SET CMP0077 NEW)
# Enable target property setting `MSVC_RUNTIME_LIBRARY`.
cmake_policy(SET CMP0091 NEW)

project(tlct
        VERSION 0.16.0.6
        LANGUAGES CXX
)

# Compile Options
include(CMakeDependentOption)

option(TLCT_BUILD_TESTS "Build tests" OFF)
option(TLCT_HEADER_ONLY "Enable the header-only mode" OFF)
cmake_dependent_option(TLCT_BUILD_SHARED_LIBS "Specifies the type of TLCT to build" ON
        "NOT TLCT_HEADER_ONLY AND BUILD_SHARED_LIBS" OFF)
cmake_dependent_option(TLCT_ENABLE_LTO "Enable full link-time-optimizations (LTO)" OFF
        "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)
option(TLCT_VERBOSE_WARNING "Show more verbose compiler warnings" OFF)

if (MSVC)
    option(TLCT_WITH_STATIC_CRT "Link with STATIC CRT" OFF)
    set(MSVC_RUNTIME_TYPE MultiThreaded$<$<NOT:$<BOOL:${TLCT_WITH_STATIC_CRT}>>:DLL>)
endif ()

# Dependencies
include(FetchContent)

find_package(OpenCV REQUIRED COMPONENTS imgproc OPTIONAL_COMPONENTS imgcodecs)

set(TLCT_PUGIXML_PATH "https://github.com/zeux/pugixml/archive/refs/tags/v1.14.tar.gz" CACHE STRING
        "Specifies the path of pugixml")
set(PUGIXML_NO_XPATH ON CACHE BOOL "")
set(PUGIXML_NO_STL ON CACHE BOOL "")

FetchContent_Declare(
        pugixml
        DOWNLOAD_EXTRACT_TIMESTAMP ON
        URL ${TLCT_PUGIXML_PATH}
)
FetchContent_MakeAvailable(pugixml)

if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(TLCT_ARGPARSE_PATH "https://github.com/p-ranav/argparse/archive/refs/tags/v3.1.tar.gz" CACHE STRING
            "Specifies the path of argparse")
    FetchContent_Declare(
            argparse
            DOWNLOAD_EXTRACT_TIMESTAMP ON
            URL ${TLCT_ARGPARSE_PATH}
    )
    FetchContent_MakeAvailable(argparse)
endif ()

if (TLCT_BUILD_TESTS)
    include(CTest)

    FetchContent_Declare(
            doctest
            GIT_REPOSITORY https://github.com/doctest/doctest.git
            GIT_TAG v2.4.11
    )
    FetchContent_MakeAvailable(doctest)
    include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)

    FetchContent_Declare(
            tlct-test-data
            GIT_REPOSITORY https://github.com/lumina37/TLCT-test-data.git
    )
    FetchContent_MakeAvailable(tlct-test-data)
endif ()

# Helper
include(GNUInstallDirs)

function(copy_dlls_if_needed name)
    if (WIN32 AND BUILD_SHARED_LIBS)
        add_custom_command(TARGET ${name} POST_BUILD COMMAND ${CMAKE_COMMAND} -E
                copy_if_different $<TARGET_RUNTIME_DLLS:${name}> $<TARGET_FILE:${name}> $<TARGET_FILE_DIR:${name}> COMMAND_EXPAND_LISTS)
    endif ()
endfunction()

function(install_target name)
    install(TARGETS ${name}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endfunction()

set(TLCT_COMPILE_INFO "[TLCT v${PROJECT_VERSION}] [OpenCV v${OpenCV_VERSION}] by [${CMAKE_CXX_COMPILER_ID} v${CMAKE_CXX_COMPILER_VERSION} (${CMAKE_SYSTEM_PROCESSOR})]")
set(TLCT_TESTDATA_DIR "${tlct-test-data_SOURCE_DIR}")

set(TLCT_CONFIGURE_DIR "${PROJECT_SOURCE_DIR}/src/include/tlct/common")
configure_file("${TLCT_CONFIGURE_DIR}/cmake.h.in" "${TLCT_CONFIGURE_DIR}/cmake.h" @ONLY)


add_subdirectory(src)
if (TLCT_BUILD_TESTS)
    add_subdirectory(tests)
endif ()

# Install
if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include(CMakePackageConfigHelpers)

    write_basic_package_version_file(
            "${PROJECT_NAME}-config-version.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY ExactVersion
    )

    configure_package_config_file(
            "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in"
            "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
            INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
    )

    install(TARGETS ${PROJECT_NAME}-lib ${PROJECT_NAME}-bin
            EXPORT ${PROJECT_NAME}Targets
    )
    install(EXPORT ${PROJECT_NAME}Targets
            FILE ${PROJECT_NAME}Targets.cmake
            NAMESPACE tlct::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )
    install(FILES
            "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
            "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )
endif ()
