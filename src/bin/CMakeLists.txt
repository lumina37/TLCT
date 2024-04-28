function(tlct_add_executable name srcs)
    add_executable(${name} ${srcs})
    target_include_directories(${name} PRIVATE ${TLCT_INCLUDE_DIR})
    target_link_libraries(${name} PRIVATE ${TLCT_LINK_LIBS})
endfunction()

tlct_add_executable(dbg_gen_patchsizes "dbg_gen_patchsizes.cpp")
tlct_add_executable(dbg_mv "dbg_mv.cpp")
tlct_add_executable(dbg_draw "dbg_draw.cpp")
