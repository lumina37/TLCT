enable_testing()

function(tlct_add_test name srcs)
    add_executable(${name} ${srcs})
    target_link_libraries(${name} PRIVATE Catch2::Catch2WithMain tlct::lib)
    target_compile_definitions(${name} PRIVATE TLCT_TESTDATA_DIR="${TLCT_TESTDATA_DIR}")

    copy_dlls_if_needed(${name})
    catch_discover_tests(${name})
endfunction()

tlct_add_test(test_constexpr_math "test_constexpr_math.cpp")
tlct_add_test(test_corners_arrange "test_corners_arrange.cpp")
tlct_add_test(test_offset_arrange "test_offset_arrange.cpp")
