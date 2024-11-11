include(FetchContent)

# OpenCV
find_package(OpenCV REQUIRED COMPONENTS imgproc OPTIONAL_COMPONENTS imgcodecs)

# pugixml
set(TLCT_PUGIXML_PATH "https://github.com/zeux/pugixml/archive/refs/tags/v1.14.tar.gz" CACHE STRING
        "Specifies the path of pugixml")
set(PUGIXML_NO_XPATH ON CACHE BOOL "")
set(PUGIXML_NO_EXCEPTIONS ON CACHE BOOL "")
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

# doctest
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
            GIT_TAG 055a30feb33d62e01bd9b82ff7982cb99c2596d1
    )
    FetchContent_MakeAvailable(tlct-test-data)
endif ()
