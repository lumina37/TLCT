function(tlct_add_test name srcs)
    add_executable(${name} ${srcs})
    target_include_directories(${name} PRIVATE ${TLCT_INCLUDE_DIR})
    target_link_libraries(${name} PRIVATE ${TLCT_LINK_LIBS} GTest::gtest_main)
    target_compile_features(${name} PRIVATE cxx_std_20)
    gtest_discover_tests(${name})
endfunction()

tlct_add_test(test_config "test_config.cpp")
