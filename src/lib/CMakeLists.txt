if (TLCT_HEADER_ONLY)
    set(TLCT_LIBRARY_TYPE "INTERFACE")
    set(TLCT_LIBRARY_DEP_SCOPE "INTERFACE")
else ()
    if (TLCT_BUILD_SHARED_LIBS)
        set(TLCT_LIBRARY_TYPE "SHARED")
        set(TLCT_LIBRARY_COMPILE_DEFS "_TLCT_BUILD_LIBS")
    else ()
        set(TLCT_LIBRARY_TYPE "STATIC")
    endif ()
    set(TLCT_LIBRARY_DEP_SCOPE "PUBLIC")
    set(TLCT_LIBRARY_SRCS "lib.cpp")
endif ()

add_library(tlct ${TLCT_LIBRARY_TYPE} ${TLCT_LIBRARY_SRCS})
target_compile_definitions(tlct PRIVATE ${TLCT_LIBRARY_COMPILE_DEFS})
target_include_directories(tlct ${TLCT_LIBRARY_DEP_SCOPE}
        ${OpenCV_INCLUDE_DIR}
        "${pugixml_SOURCE_DIR}/src"
        "${CMAKE_SOURCE_DIR}/src/include")
target_link_libraries(tlct ${TLCT_LIBRARY_DEP_SCOPE}
        ${OpenCV_LIBS}
        pugixml)
