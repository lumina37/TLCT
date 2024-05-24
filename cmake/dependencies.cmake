include(FetchContent)

find_package(OpenCV COMPONENTS core imgcodecs imgproc quality)

cmake_dependent_option(TLCT_PUGIXML_GITREPO "Specifies the git repo of pugixml"
        "" "TLCT_LOCAL_DEPS" "https://github.com/zeux/pugixml.git")
set(PUGIXML_NO_XPATH ON CACHE BOOL "" FORCE)
set(PUGIXML_NO_EXCEPTIONS ON CACHE BOOL "" FORCE)
set(PUGIXML_NO_STL ON CACHE BOOL "" FORCE)
FetchContent_Declare(
        pugixml
        GIT_REPOSITORY ${TLCT_PUGIXML_GITREPO}
        GIT_TAG v1.14
)
FetchContent_MakeAvailable(pugixml)

if (TLCT_BUILD_TESTS)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(GTEST_LINKED_AS_SHARED_LIBRARY 1 CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
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
            GIT_TAG 947410b1e5e762dc312b30631c01df0eb13b9761
    )
    FetchContent_MakeAvailable(tlct-test-data)
endif ()
