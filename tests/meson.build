fs = import('fs')

doctest_dep = dependency(
    'doctest',
    version : '>=2.4',
    fallback : 'doctest',
    required : true,
)

data_dir = meson.project_build_root() / 'TLCT-test-data'
if not fs.is_dir(data_dir)
    git = find_program('git')
    data_tgt = custom_target(
        'tlct_test_data',
        output : 'TLCT-test-data',
        command : [
            git,
            'clone',
            'https://github.com/lumina37/TLCT-test-data.git',
            '--depth', '1'
        ],
    )
endif

tlct_test_dep = declare_dependency(
    dependencies : [tlct_dep, doctest_dep],
    sources : fs.is_dir(data_dir) ? [] : data_tgt,
)

foreach case : [
    'test_constexpr_math',
    'test_corners_arrange',
    'test_offset_arrange',
]
    test_bin = executable(
        case, case + '.cpp',
        dependencies : tlct_test_dep,
    )
    test(case, test_bin)
endforeach
