function(common_target_setting name)
    get_target_property(__TYPE ${name} TYPE)
    if (${__TYPE} STREQUAL "INTERFACE_LIBRARY")
        set(__PUB_DEP_SCOPE "INTERFACE")
    else ()
        set(__PUB_DEP_SCOPE "PUBLIC")
    endif ()

    target_compile_features(${name} ${__PUB_DEP_SCOPE} cxx_std_20)

    target_include_directories(${name} ${__PUB_DEP_SCOPE}
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${OpenCV_INCLUDE_DIRS}>
    )
    target_link_libraries(${name} ${__PUB_DEP_SCOPE}
            ${OpenCV_LIBS}
            argparse
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

add_library(tlct-lib-static STATIC "lib.cpp")
common_target_setting(tlct-lib-static)
add_library(tlct::lib::static ALIAS tlct-lib-static)

add_library(tlct-lib-dynamic SHARED "lib.cpp")
common_target_setting(tlct-lib-dynamic)
set_target_properties(tlct-lib-dynamic PROPERTIES OUTPUT_NAME tlct)
target_compile_definitions(tlct-lib-dynamic PUBLIC "_TLCT_LIB_DYNAMIC")
add_library(tlct::lib::dynamic ALIAS tlct-lib-dynamic)

add_library(tlct-lib-header-only INTERFACE)
common_target_setting(tlct-lib-header-only)
target_compile_definitions(tlct-lib-header-only INTERFACE "_TLCT_LIB_HEADER_ONLY")
add_library(tlct::lib::header-only ALIAS tlct-lib-header-only)
