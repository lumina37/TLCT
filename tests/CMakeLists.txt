function(tlct_add_test name srcs)
    add_executable(${name} ${srcs})
    target_link_libraries(${name} PRIVATE tlct GTest::gtest_main)
    if (TLCT_BUILD_SHARED_LIBS AND WIN32)
        add_custom_command(TARGET ${name} POST_BUILD COMMAND ${CMAKE_COMMAND} -E
                copy_if_different $<TARGET_RUNTIME_DLLS:${name}> $<TARGET_FILE_DIR:${name}> COMMAND_EXPAND_LISTS)
    endif ()
    gtest_discover_tests(${name})
endfunction()

tlct_add_test(test_tspc_config "test_tspc_config.cpp")
tlct_add_test(test_raytrix_config "test_raytrix_config.cpp")
tlct_add_test(test_tspc_patchsizes "test_tspc_patchsizes.cpp")
