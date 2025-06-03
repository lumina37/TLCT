function(tlct_add_executable name srcs)
    add_executable(${name} ${srcs})
    target_link_libraries(${name} PRIVATE
            tlct::lib::static
            argparse::argparse
    )

    if (TLCT_ENABLE_LTO)
        set_target_properties(${name} PROPERTIES
                INTERPROCEDURAL_OPTIMIZATION ON
        )
    endif ()

    if (MSVC)
        set_target_properties(${name} PROPERTIES
                MSVC_RUNTIME_LIBRARY ${_TLCT_MSVC_RUNTIME_LIBRARY}
        )
    endif ()

    tlct_copy_dlls_if_needed(${name})
endfunction()

tlct_add_executable(tlct-bin "convertor.cpp")
set_target_properties(tlct-bin PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
)
add_executable(tlct::bin ALIAS tlct-bin)

# Debug Only
tlct_add_executable(tlct-painter "painter.cpp")
