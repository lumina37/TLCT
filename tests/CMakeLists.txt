enable_testing()

set(_TLCT_TEST_TARGETS)

function(tlct_add_test name lib srcs)
    add_executable(${name} ${srcs})
    target_link_libraries(${name} PRIVATE Catch2::Catch2WithMain tlct::lib::static)
    target_compile_definitions(${name} PRIVATE TLCT_TESTDATA_DIR="${TLCT_TESTDATA_DIR}")

    catch_discover_tests(${name})
    list(APPEND _TLCT_TEST_TARGETS ${name})
    set(_TLCT_TEST_TARGETS ${_TLCT_TEST_TARGETS} PARENT_SCOPE)

    tlct_copy_dlls_if_needed(${name})
endfunction()

tlct_add_test(test_constexpr_math tlct::lib::static "test_constexpr_math.cpp")

tlct_add_test(test_corners_arrange tlct::lib::static "test_corners_arrange.cpp")
tlct_add_test(test_offset_arrange tlct::lib::static "test_offset_arrange.cpp")

tlct_add_test(test_corners_arrange_header_only_lib tlct::lib::header-only "test_corners_arrange.cpp")
tlct_add_test(test_offset_arrange_header_only_lib tlct::lib::header-only "test_offset_arrange.cpp")

tlct_add_test(test_corners_arrange_dynamic_lib tlct::lib::dynamic "test_corners_arrange.cpp")
tlct_add_test(test_offset_arrange_dynamic_lib tlct::lib::dynamic "test_offset_arrange.cpp")

add_custom_target(tlct-tests DEPENDS ${_TLCT_TEST_TARGETS})
