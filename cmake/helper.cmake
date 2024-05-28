function(copy_dlls_if_needed name)
    if (WIN32)
        add_custom_command(TARGET ${name} POST_BUILD COMMAND ${CMAKE_COMMAND} -E
                copy_if_different $<TARGET_RUNTIME_DLLS:${name}> $<TARGET_FILE_DIR:${name}> COMMAND_EXPAND_LISTS)
    endif ()
endfunction()

set(TLCT_CONFIGURE_DIR "${PROJECT_SOURCE_DIR}/src/include/tlct/common")
set(TLCT_TESTDATA_DIR "${tlct-test-data_SOURCE_DIR}")
configure_file("${TLCT_CONFIGURE_DIR}/cmake.h.in" "${TLCT_CONFIGURE_DIR}/cmake.h" @ONLY)
