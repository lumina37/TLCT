project(
    'TLCT', 'cpp',
    version : '1.3.3a0',
    meson_version : '>=1.3.0',
    license : 'LGPL-3.0-or-later',
    default_options : { 'cpp_std' : 'c++latest' }
)

feat_header_only = get_option('TLCT_HEADER_ONLY')
feat_build_shared_libs = get_option('TLCT_BUILD_SHARED_LIBS')
feat_build_shared_libs.require(feat_header_only.disabled())
feat_build_tests = get_option('TLCT_BUILD_TESTS')

subdir('src')
if feat_build_tests.enabled()
    subdir('tests')
endif
