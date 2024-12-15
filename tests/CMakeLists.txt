enable_testing()

function(tlct_add_test name srcs)
    add_executable(${name} ${srcs})
    target_link_libraries(${name} PRIVATE doctest tlct::lib)
    target_compile_definitions(${name} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)

    copy_dlls_if_needed(${name})
    doctest_discover_tests(${name})
endfunction()

tlct_add_test(test_constexpr_math "test_constexpr_math.cpp")
tlct_add_test(test_raytrix_config "test_raytrix_config.cpp")
tlct_add_test(test_tspc_config "test_tspc_config.cpp")
