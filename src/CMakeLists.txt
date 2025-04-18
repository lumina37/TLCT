function(tlct_common_lib_setting name)
    get_target_property(__TYPE ${name} TYPE)
    if (${__TYPE} STREQUAL "INTERFACE_LIBRARY")
        set(__PUB_DEP_SCOPE "INTERFACE")
    else ()
        set(__PUB_DEP_SCOPE "PUBLIC")
    endif ()

    target_compile_features(${name} ${__PUB_DEP_SCOPE} cxx_std_23)

    target_include_directories(${name} ${__PUB_DEP_SCOPE}
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
            $<BUILD_INTERFACE:${OpenCV_INCLUDE_DIRS}>
    )
    target_link_libraries(${name} ${__PUB_DEP_SCOPE}
            ${OpenCV_LIBS}
            OpenMP::OpenMP_CXX
    )

    if (MSVC)
        target_compile_options(${name} ${__PUB_DEP_SCOPE} /arch:AVX2)
    else ()
        target_compile_options(${name} ${__PUB_DEP_SCOPE} -mavx2)
    endif ()

    if (TLCT_VERBOSE_WARNING)
        if (MSVC)
            target_compile_options(${name} ${__PUB_DEP_SCOPE} /W4)
        else ()
            target_compile_options(${name} ${__PUB_DEP_SCOPE} -Wall -Wextra)
        endif ()
    endif ()
endfunction()

file(GLOB_RECURSE _TLCT_SRCS "include/*.cpp")

add_library(tlct-lib-static STATIC ${_TLCT_SRCS})
tlct_common_lib_setting(tlct-lib-static)
if (MSVC)
    set_target_properties(tlct-lib-static PROPERTIES
            MSVC_RUNTIME_LIBRARY ${_TLCT_MSVC_RUNTIME_LIBRARY}
    )
endif ()
add_library(tlct::lib::static ALIAS tlct-lib-static)

add_library(tlct-lib-dynamic SHARED ${_TLCT_SRCS})
tlct_common_lib_setting(tlct-lib-dynamic)
set_target_properties(tlct-lib-dynamic PROPERTIES
        OUTPUT_NAME tlct
        DEBUG_POSTFIX "-${TLCT_VERSION}d"
        RELEASE_POSTFIX "-${TLCT_VERSION}"
)
if (MSVC)
    set_target_properties(tlct-lib-dynamic PROPERTIES
            MSVC_RUNTIME_LIBRARY ${_TLCT_MSVC_RUNTIME_LIBRARY}
    )
endif ()
target_compile_definitions(tlct-lib-dynamic PUBLIC "_TLCT_LIB_DYNAMIC")
add_library(tlct::lib::dynamic ALIAS tlct-lib-dynamic)

add_library(tlct-lib-header-only INTERFACE)
tlct_common_lib_setting(tlct-lib-header-only)
target_compile_definitions(tlct-lib-header-only INTERFACE "_TLCT_LIB_HEADER_ONLY")
add_library(tlct::lib::header-only ALIAS tlct-lib-header-only)
