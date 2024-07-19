function(tlct_add_executable name srcs)
    add_executable(${name} ${srcs})
    target_link_libraries(${name} PRIVATE tlct)

    if (TLCT_ENABLE_LTO)
        set_target_properties(${name} PROPERTIES
                INTERPROCEDURAL_OPTIMIZATION ON
        )
    endif ()

    copy_dlls_if_needed(${name})
endfunction()

tlct_add_executable(dbg_mv "dbg_mv.cpp")
tlct_add_executable(dbg_draw "dbg_draw.cpp")
tlct_add_executable(dbg_psize "dbg_psize.cpp")
tlct_add_executable(dbg_mv_raytrix "dbg_mv_raytrix.cpp")
tlct_add_executable(dbg_draw_raytrix "dbg_draw_raytrix.cpp")
tlct_add_executable(dbg_psize_raytrix "dbg_psize_raytrix.cpp")

tlct_add_executable(get_center "get_center.cpp")
