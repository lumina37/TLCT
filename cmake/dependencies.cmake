include(FetchContent)

find_package(OpenCV COMPONENTS core imgcodecs imgproc quality)

set(PUGIXML_NO_XPATH ON CACHE BOOL "" FORCE)
set(PUGIXML_NO_EXCEPTIONS ON CACHE BOOL "" FORCE)
set(PUGIXML_NO_STL ON CACHE BOOL "" FORCE)
set(PUGIXML_USE_POSTFIX ON CACHE BOOL "" FORCE)
FetchContent_Declare(
        pugixml
        GIT_REPOSITORY https://github.com/zeux/pugixml.git
        GIT_TAG v1.14
)
FetchContent_MakeAvailable(pugixml)

if (BUILD_TESTS)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
    )

    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(GTEST_LINKED_AS_SHARED_LIBRARY 1 CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    include(GoogleTest)
endif ()
