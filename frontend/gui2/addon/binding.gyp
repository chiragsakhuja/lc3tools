{
    'targets': [
        {
            'include_dirs': [
                "<!(node -e \"require('nan')\")",
                '../../../backend/src'
            ],
            'target_name': 'lc3interface',
            'sources': ['wrapper.cpp'],
            'cflags!': ['-fno-exceptions'],
            'cflags_cc!': ['-fno-exceptions'],
            'conditions': [['OS=="mac"', {
                'xcode_settings': {
                    'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
                }
            }]],
            'link_settings': {
                'libraries': ['-L<(module_root_dir)', '-llc3core'],
                'ld_flags': "-Wl,-rpath,'$$ORIGIN'"
            }
        }
    ]
}
