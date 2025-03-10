enable_testing()

set(_TLCT_TEST_TARGETS)

function(tlct_add_test name srcs)
    add_executable(${name} ${srcs})
    target_link_libraries(${name} PRIVATE Catch2::Catch2WithMain tlct::lib::static)
    target_compile_definitions(${name} PRIVATE TLCT_TESTDATA_DIR="${TLCT_TESTDATA_DIR}")

    catch_discover_tests(${name})
    list(APPEND _TLCT_TEST_TARGETS ${name})
    set(_TLCT_TEST_TARGETS ${_TLCT_TEST_TARGETS} PARENT_SCOPE)

    tlct_copy_dlls_if_needed(${name})
endfunction()

tlct_add_test(test_constexpr_math "test_constexpr_math.cpp")
tlct_add_test(test_corners_arrange "test_corners_arrange.cpp")
tlct_add_test(test_offset_arrange "test_offset_arrange.cpp")

add_custom_target(tlct-tests DEPENDS ${_TLCT_TEST_TARGETS})
