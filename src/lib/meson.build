opencv_dep = dependency(
    'opencv',
    version : '>=4.9',
    modules : ['opencv_imgproc'],
    required : true
)

argparse_dep = dependency(
    'argparse',
    version : '>=3.0',
    fallback : 'argparse',
    required : true,
)

tlct_lib = static_library(
    'tlct-' + meson.project_version(), 'lib.cpp',
    include_directories : '../include',
    dependencies : [opencv_dep, argparse_dep],
    # cpp_shared_args : ['-D_TLCT_BUILD_LIBS'],
)

tlct_dep = declare_dependency(
    link_with : feat_header_only.enabled() ? [] : tlct_lib,
    include_directories : '../include',
    dependencies : [opencv_dep, argparse_dep],
)
