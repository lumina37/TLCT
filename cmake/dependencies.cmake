include(FetchContent)

# OpenCV
find_package(OpenCV REQUIRED COMPONENTS imgproc imgcodecs quality)

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
        OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(pugixml)

if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    FetchContent_Declare(
            argparse
            GIT_REPOSITORY https://github.com/p-ranav/argparse.git
            GIT_TAG v3.1
            OVERRIDE_FIND_PACKAGE
    )
    FetchContent_MakeAvailable(argparse)
endif ()

# google test
if (TLCT_BUILD_TESTS)
    FetchContent_Declare(
            doctest
            GIT_REPOSITORY https://github.com/doctest/doctest.git
            GIT_TAG v2.4.11
            OVERRIDE_FIND_PACKAGE
    )
    FetchContent_MakeAvailable(doctest)

    FetchContent_Declare(
            tlct-test-data
            GIT_REPOSITORY https://github.com/SIGS-TZ/TLCT-test-data.git
            GIT_TAG 55b89b4300201abdbd004d99c1571fdbcbeb854b
    )
    FetchContent_MakeAvailable(tlct-test-data)
endif ()
