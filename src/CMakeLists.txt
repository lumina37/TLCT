add_subdirectory(lib)
if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    add_subdirectory(bin)
endif ()
