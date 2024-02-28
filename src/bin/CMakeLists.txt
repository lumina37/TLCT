function(tlct_add_app name srcs)
    add_executable(${name} ${srcs})
    target_include_directories(${name} PRIVATE ${TLCT_INCLUDE_DIR})
    target_link_libraries(${name} PRIVATE ${TLCT_LINK_LIBS})
    target_compile_features(${name} PRIVATE cxx_std_20)
endfunction()

tlct_add_app(hello_world "main.cpp")
