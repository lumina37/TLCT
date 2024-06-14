include(FetchContent)

find_package(OpenCV QUIET COMPONENTS core imgcodecs imgproc quality)

set(TLCT_PUGIXML_PATH "https://github.com/zeux/pugixml.git" CACHE STRING
        "Specifies the path of pugixml (git repo or local dir)")
set(PUGIXML_NO_XPATH ON CACHE BOOL "")
set(PUGIXML_NO_EXCEPTIONS ON CACHE BOOL "")
set(PUGIXML_NO_STL ON CACHE BOOL "")

if (TLCT_PUGIXML_PATH MATCHES "\.git$")
    FetchContent_Declare(
            pugixml
            GIT_REPOSITORY ${TLCT_PUGIXML_PATH}
            GIT_TAG v1.14
    )
else ()
    FetchContent_Declare(
            pugixml
            URL ${TLCT_PUGIXML_PATH}
    )
endif ()
FetchContent_MakeAvailable(pugixml)

if (TLCT_BUILD_TESTS)
    set(BUILD_GMOCK OFF CACHE BOOL "")
    set(GTEST_LINKED_AS_SHARED_LIBRARY 1 CACHE BOOL "")
    set(gtest_force_shared_crt ON CACHE INTERNAL "" FORCE)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
    )
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    include(GoogleTest)

    FetchContent_Declare(
            tlct-test-data
            GIT_REPOSITORY https://github.com/SIGS-TZ/TLCT-test-data.git
            GIT_TAG 0992e594e7f849b352dbd03aeb68ca1ae6c657fb
    )
    FetchContent_MakeAvailable(tlct-test-data)
endif ()
