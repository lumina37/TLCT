function(tlct_add_executable name srcs)
    add_executable(${name} ${srcs})
    target_link_libraries(${name} PRIVATE ${TLCT_LIB_NAME}})
    copy_dlls_if_needed(${name})
endfunction()

tlct_add_executable(dbg_gen_patchsizes "dbg_gen_patchsizes.cpp")
tlct_add_executable(dbg_mv "dbg_mv.cpp")
tlct_add_executable(dbg_draw "dbg_draw.cpp")
