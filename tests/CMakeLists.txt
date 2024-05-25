function(tlct_add_test name srcs)
    add_executable(${name} ${srcs})
    target_link_libraries(${name} PRIVATE tlct GTest::gtest_main)
    copy_dlls_if_needed(${name})
    gtest_discover_tests(${name})
endfunction()

tlct_add_test(test_common_cfg "test_common_cfg.cpp")
tlct_add_test(test_tspc_cfg "test_tspc_cfg.cpp")
