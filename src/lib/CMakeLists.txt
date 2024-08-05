if (TLCT_HEADER_ONLY)
    set(TLCT_LIB_TYPE "INTERFACE")
    set(TLCT_LIB_DEP_SCOPE "INTERFACE")
else ()
    if (TLCT_BUILD_SHARED_LIBS)
        set(TLCT_LIB_TYPE "SHARED")
        set(TLCT_LIB_COMPILE_DEFS "_TLCT_BUILD_LIBS")
    else ()
        set(TLCT_LIB_TYPE "STATIC")
    endif ()
    set(TLCT_LIB_DEP_SCOPE "PUBLIC")

    set(TLCT_LIB_SRCS "lib.cpp")
    if (PUGIXML_HEADER_ONLY)
        list(APPEND TLCT_LIB_SRCS "${pugixml_SOURCE_DIR}/src/pugixml.cpp")
    endif ()
endif ()

set(TLCT_LIB_NAME tlct)
add_library(${TLCT_LIB_NAME} ${TLCT_LIB_TYPE} ${TLCT_LIB_SRCS})
set_target_properties(${TLCT_LIB_NAME} PROPERTIES
        VERSION ${TLCT_VERSION}
        SOVERSION ${TLCT_VERSION}
        DEBUG_POSTFIX "-${TLCT_VERSION}d"
        RELEASE_POSTFIX "-${TLCT_VERSION}"
)
target_compile_features(${TLCT_LIB_NAME} ${TLCT_LIB_DEP_SCOPE} cxx_std_20)
target_compile_definitions(${TLCT_LIB_NAME} PRIVATE ${TLCT_LIB_COMPILE_DEFS})
target_include_directories(${TLCT_LIB_NAME} ${TLCT_LIB_DEP_SCOPE}
        "${PROJECT_SOURCE_DIR}/src/include"
        ${OpenCV_INCLUDE_DIRS}
        "${pugixml_SOURCE_DIR}/src"
)
target_link_libraries(${TLCT_LIB_NAME} ${TLCT_LIB_DEP_SCOPE}
        ${OpenCV_LIBS}
        $<$<NOT:$<BOOL:${PUGIXML_HEADER_ONLY}>>:pugixml>
)
