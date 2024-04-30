function(tlct_add_executable name srcs)
    add_executable(${name} ${srcs})
    target_link_libraries(${name} PRIVATE tlct)
    if (TLCT_BUILD_SHARED_LIBS AND WIN32)
        add_custom_command(TARGET ${name} POST_BUILD COMMAND ${CMAKE_COMMAND} -E
                copy_if_different $<TARGET_RUNTIME_DLLS:${name}> $<TARGET_FILE_DIR:${name}> COMMAND_EXPAND_LISTS)
    endif ()
endfunction()

tlct_add_executable(dbg_gen_patchsizes "dbg_gen_patchsizes.cpp")
tlct_add_executable(dbg_mv "dbg_mv.cpp")
tlct_add_executable(dbg_draw "dbg_draw.cpp")
