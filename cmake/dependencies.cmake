include(FetchContent)

find_package(OpenCV COMPONENTS core imgcodecs imgproc quality)

set(PUGIXML_NO_XPATH ON)
set(PUGIXML_NO_EXCEPTIONS ON)
set(PUGIXML_NO_STL ON)
FetchContent_Declare(
        pugixml
        GIT_REPOSITORY https://github.com/zeux/pugixml.git
        GIT_TAG v1.14
)
FetchContent_MakeAvailable(pugixml)

set(TLCT_INCLUDE_DIR
        ${OpenCV_INCLUDE_DIR}
        "${pugixml_SOURCE_DIR}/src"
        "${CMAKE_SOURCE_DIR}/src/include")

set(TLCT_LINK_LIBS
        ${OpenCV_LIBS}
        pugixml)

if (BUILD_TESTS)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
    )

    set(gtest_force_shared_crt ON)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    include(GoogleTest)
endif ()
