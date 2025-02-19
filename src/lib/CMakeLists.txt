if (TLCT_HEADER_ONLY)
    set(TLCT_LIB_TYPE "INTERFACE")
    set(TLCT_LIB_DEP_SCOPE "INTERFACE")
else ()
    if (TLCT_BUILD_SHARED_LIBS)
        set(TLCT_LIB_TYPE "SHARED")
    else ()
        set(TLCT_LIB_TYPE "STATIC")
    endif ()
    set(TLCT_LIB_DEP_SCOPE "PUBLIC")

    set(TLCT_LIB_SRCS "lib.cpp")
endif ()

add_library(tlct-lib ${TLCT_LIB_TYPE} ${TLCT_LIB_SRCS})
add_library(tlct::lib ALIAS tlct-lib)
set_target_properties(tlct-lib PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
target_compile_features(tlct-lib ${TLCT_LIB_DEP_SCOPE} cxx_std_20)
if (MSVC)
    target_compile_options(tlct-lib ${TLCT_LIB_DEP_SCOPE} /arch:AVX2)
else ()
    target_compile_options(tlct-lib ${TLCT_LIB_DEP_SCOPE} -mavx2)
endif ()
target_include_directories(tlct-lib ${TLCT_LIB_DEP_SCOPE}
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${OpenCV_INCLUDE_DIRS}>
)
target_link_libraries(tlct-lib ${TLCT_LIB_DEP_SCOPE}
        ${OpenCV_LIBS}
        argparse
)

if (NOT TLCT_HEADER_ONLY AND TLCT_BUILD_SHARED_LIBS)
    target_compile_definitions(tlct-lib PRIVATE "_TLCT_BUILD_LIBS")
endif ()

if (TLCT_VERBOSE_WARNING)
    if (MSVC)
        target_compile_options(tlct-lib ${TLCT_LIB_DEP_SCOPE} /W4)
    else ()
        target_compile_options(tlct-lib ${TLCT_LIB_DEP_SCOPE} -Wall -Wextra)
    endif ()
endif ()
