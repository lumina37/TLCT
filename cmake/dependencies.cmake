include(FetchContent)

# OpenCV
set(TLCT_OPENCV_PATH "https://github.com/OpenCV/OpenCV.git" CACHE STRING
        "Specifies the path of OpenCV (git repo or local dir)")
set(TLCT_OPENCV_CONTRIB_PATH "https://github.com/OpenCV/OpenCV_contrib.git" CACHE STRING
        "Specifies the path of OpenCV-contrib (git repo or local dir)")

set(OpenCV_VERSION 4.10.0)
set(OpenCV_REQUIRED_MODULES imgcodecs imgproc quality)
string(REPLACE ";" "," OpenCV_BUILD_LIST "${OpenCV_REQUIRED_MODULES}")
string(REPLACE ";" " " OpenCV_FIND_PACKAGE_ARGS "${OpenCV_REQUIRED_MODULES}")

set(BUILD_LIST ${OpenCV_BUILD_LIST} CACHE STRING "")
set(BUILD_WITH_STATIC_CRT OFF CACHE BOOL "")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(CV_TRACE OFF CACHE BOOL "")
set(OPENCV_GENERATE_SETUPVARS OFF CACHE BOOL "")
set(OPENCV_PYTHON3_VERSION "" CACHE STRING "")
set(ENABLE_PRECOMPILED_HEADERS OFF CACHE BOOL "")

set(CPU_BASELINE AVX2 CACHE STRING "")
set(CPU_DISPATCH AVX2 CACHE STRING "")

set(BUILD_OpenCV_apps OFF CACHE BOOL "")
set(WITH_ADE OFF CACHE BOOL "")
set(WITH_DSHOW OFF CACHE BOOL "")
set(WITH_FFMPEG OFF CACHE BOOL "")
set(WITH_FLATBUFFERS OFF CACHE BOOL "")
set(WITH_GSTREAMER OFF CACHE BOOL "")
set(WITH_IMGCODEC_HDR OFF CACHE BOOL "")
set(WITH_IMGCODEC_PFM OFF CACHE BOOL "")
set(WITH_IMGCODEC_PXM OFF CACHE BOOL "")
set(WITH_IMGCODEC_SUNRASTER OFF CACHE BOOL "")
set(WITH_IPP OFF CACHE BOOL "")
set(WITH_JASPER OFF CACHE BOOL "")
set(WITH_JPEG OFF CACHE BOOL "")
set(WITH_LAPACK OFF CACHE BOOL "")
set(WITH_MSMF OFF CACHE BOOL "")
set(WITH_MSMF_DXVA OFF CACHE BOOL "")
set(WITH_OPENCL OFF CACHE BOOL "")
set(WITH_OPENEXR OFF CACHE BOOL "")
set(WITH_OPENJPEG OFF CACHE BOOL "")
set(WITH_PROTOBUF OFF CACHE BOOL "")
set(WITH_TIFF OFF CACHE BOOL "")
set(WITH_VTK OFF CACHE BOOL "")
set(WITH_WEBP OFF CACHE BOOL "")

if (TLCT_OPENCV_CONTRIB_PATH MATCHES "\.git$")
    FetchContent_Declare(
            OpenCV_contrib
            GIT_REPOSITORY ${TLCT_OPENCV_CONTRIB_PATH}
            GIT_TAG ${OpenCV_VERSION}
            OVERRIDE_FIND_PACKAGE
    )
else ()
    FetchContent_Declare(
            OpenCV_contrib
            DOWNLOAD_EXTRACT_TIMESTAMP ON
            URL ${TLCT_OPENCV_CONTRIB_PATH}
            OVERRIDE_FIND_PACKAGE
    )
endif ()
FetchContent_MakeAvailable(OpenCV_contrib)

set(OPENCV_EXTRA_MODULES_PATH ${opencv_contrib_SOURCE_DIR}/modules CACHE STRING "")

if (TLCT_OPENCV_PATH MATCHES "\.git$")
    FetchContent_Declare(
            OpenCV
            GIT_REPOSITORY ${TLCT_OPENCV_PATH}
            GIT_TAG ${OpenCV_VERSION}
            OVERRIDE_FIND_PACKAGE
    )
else ()
    FetchContent_Declare(
            OpenCV
            DOWNLOAD_EXTRACT_TIMESTAMP ON
            URL ${TLCT_OPENCV_PATH}
            OVERRIDE_FIND_PACKAGE
    )
endif ()
FetchContent_MakeAvailable(OpenCV)

set(OpenCV_INCLUDE_DIR ${OPENCV_CONFIG_FILE_INCLUDE_DIR})
set(OpenCV_LIBS ${OPENCV_MODULES_BUILD})

foreach (mod ${OpenCV_LIBS})
    list(APPEND OpenCV_INCLUDE_DIR ${OPENCV_MODULE_${mod}_LOCATION}/include)
endforeach ()

# pugixml
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
            OVERRIDE_FIND_PACKAGE
    )
else ()
    FetchContent_Declare(
            pugixml
            DOWNLOAD_EXTRACT_TIMESTAMP ON
            URL ${TLCT_PUGIXML_PATH}
            OVERRIDE_FIND_PACKAGE
    )
endif ()
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
            GIT_TAG 7b501f584c510d79d979bb14ca41034c8b0e5bc6
    )
    FetchContent_MakeAvailable(tlct-test-data)
endif ()
