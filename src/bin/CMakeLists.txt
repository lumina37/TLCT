function(tlct_add_executable name srcs)
    add_executable(${name} ${srcs})
    target_link_libraries(${name} PRIVATE argparse tlct::lib)
    target_include_directories(${name} PRIVATE ${argparse_SOURCE_DIR}/include)
    target_compile_definitions(${name} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)

    if (TLCT_ENABLE_LTO)
        set_target_properties(${name} PROPERTIES
                INTERPROCEDURAL_OPTIMIZATION ON
        )
    endif ()

    install_target(${name})
    copy_dlls_if_needed(${name})
endfunction()

tlct_add_executable(tlct "main.cpp")
add_executable(tlct::bin ALIAS tlct)

# tlct_add_executable(dbg_draw "dbg_draw.cpp")
