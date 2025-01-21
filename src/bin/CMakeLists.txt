function(tlct_add_executable name srcs)
    add_executable(${name} ${srcs})
    target_link_libraries(${name} PRIVATE tlct::lib)
    target_compile_definitions(${name} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)

    if (TLCT_ENABLE_LTO)
        set_target_properties(${name} PROPERTIES
                INTERPROCEDURAL_OPTIMIZATION ON
        )
    endif ()

    copy_dlls_if_needed(${name})
endfunction()

tlct_add_executable(tlct-bin "main.cpp")
set_target_properties(tlct-bin PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
)
add_executable(tlct::bin ALIAS tlct-bin)
