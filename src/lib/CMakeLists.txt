set(TLCT_LIB_NAME tlc)

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

add_library(${TLCT_LIB_NAME} ${TLCT_LIB_TYPE} ${TLCT_LIB_SRCS})
add_library(tlct::lib ALIAS ${TLCT_LIB_NAME})
set_target_properties(${TLCT_LIB_NAME} PROPERTIES
        VERSION ${TLCT_VERSION}
        SOVERSION ${TLCT_VERSION_MAJOR}.${TLCT_VERSION_MINOR}
        DEBUG_POSTFIX "-${TLCT_VERSION}d"
        RELEASE_POSTFIX "-${TLCT_VERSION}"
)
target_compile_features(${TLCT_LIB_NAME} ${TLCT_LIB_DEP_SCOPE} cxx_std_20)
target_include_directories(${TLCT_LIB_NAME} ${TLCT_LIB_DEP_SCOPE}
        "${PROJECT_SOURCE_DIR}/src/include"
        ${OpenCV_INCLUDE_DIRS}
        "${pugixml_SOURCE_DIR}/src"
)
target_link_libraries(${TLCT_LIB_NAME} ${TLCT_LIB_DEP_SCOPE}
        ${OpenCV_LIBS}
        $<$<NOT:$<BOOL:${PUGIXML_HEADER_ONLY}>>:pugixml>
)

if (NOT TLCT_HEADER_ONLY AND TLCT_BUILD_SHARED_LIBS)
    target_compile_definitions(${TLCT_LIB_NAME} PRIVATE
            "_TLCT_BUILD_LIBS"
            $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
    )
endif ()

if (TLCT_VERBOSE_WARNING)
    if (MSVC)
        target_compile_options(${TLCT_LIB_NAME} ${TLCT_LIB_DEP_SCOPE} /W4)
    else ()
        target_compile_options(${TLCT_LIB_NAME} ${TLCT_LIB_DEP_SCOPE} -Wall -Wextra -Wno-reorder)
    endif ()
endif ()

if (NOT TLCT_HEADER_ONLY)
    install_target(${TLCT_LIB_NAME})
endif ()
