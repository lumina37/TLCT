compiler_id = meson.get_compiler('cpp').get_id()
compiler_version = meson.get_compiler('cpp').version()
cpu_family = target_machine.cpu_family()
opencv_version = opencv_dep.version()
compile_info = '[TLCT v@0@] [OpenCV v@1@] by [@2@ v@3@ (@4@)]'
compile_info = compile_info.format(
    meson.project_version(),
    opencv_version,
    compiler_id,
    compiler_version,
    cpu_family
)

configure_file(
    input : 'config.h.in',
    output : 'config.h',
    configuration : {
        'TLCT_HEADER_ONLY' : feat_header_only.enabled(),
        'TLCT_BUILD_SHARED_LIBS' : feat_build_shared_libs.enabled(),
        'TLCT_VERSION' : meson.project_version(),
        'TLCT_COMPILE_INFO' : compile_info,
        'TLCT_TEST_DATA_DIR' : meson.project_build_root() / 'TLCT-test-data',
    },
)
