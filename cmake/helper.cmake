include(GNUInstallDirs)

function(copy_dlls_if_needed name)
    if (WIN32 AND BUILD_SHARED_LIBS)
        add_custom_command(TARGET ${name} POST_BUILD COMMAND ${CMAKE_COMMAND} -E
                copy_if_different $<TARGET_RUNTIME_DLLS:${name}> $<TARGET_FILE:${name}> $<TARGET_FILE_DIR:${name}> COMMAND_EXPAND_LISTS)
    endif ()
endfunction()

function(install_target name)
    install(TARGETS ${name}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endfunction()

find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE TLCT_GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else()
    set(TLCT_GIT_TAG "unknown")
endif()

set(TLCT_EPILOG "[${TLCT_GIT_TAG} by ${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}]")
set(TLCT_TESTDATA_DIR "${tlct-test-data_SOURCE_DIR}")

set(TLCT_CONFIGURE_DIR "${PROJECT_SOURCE_DIR}/src/include/tlct/common")
configure_file("${TLCT_CONFIGURE_DIR}/cmake.h.in" "${TLCT_CONFIGURE_DIR}/cmake.h" @ONLY)
