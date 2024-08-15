include(FetchContent)

# OpenCV
find_package(OpenCV REQUIRED COMPONENTS imgproc imgcodecs quality)

# pugixml
set(TLCT_PUGIXML_VERSION 1.14 CACHE STRING "Specifies the version of pugixml")

set(TLCT_PUGIXML_PATH "https://github.com/zeux/pugixml/archive/refs/tags/v${TLCT_PUGIXML_VERSION}.tar.gz" CACHE STRING
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

# google test
if (TLCT_BUILD_TESTS)
    set(BUILD_GMOCK OFF CACHE BOOL "")
    set(GTEST_LINKED_AS_SHARED_LIBRARY 1 CACHE BOOL "")
    set(gtest_force_shared_crt ON CACHE INTERNAL "" FORCE)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
            FIND_PACKAGE_ARGS
    )
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    include(GoogleTest)

    FetchContent_Declare(
            tlct-test-data
            GIT_REPOSITORY https://github.com/SIGS-TZ/TLCT-test-data.git
            GIT_TAG 55b89b4300201abdbd004d99c1571fdbcbeb854b
    )
    FetchContent_MakeAvailable(tlct-test-data)
endif ()
