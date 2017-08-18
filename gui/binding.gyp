{
    'targets': [
        {
            'include_dirs': [
                "<!(node -e \"require('nan')\")",
                '../core/src'
            ],
            'target_name': 'addon',
            'sources': ['wrapper.cpp'],
            'cflags!': ['-fno-exceptions'],
            'cflags_cc!': ['-fno-exceptions'],
            'conditions': [['OS=="mac"', {
                'xcode_settings': {
                    'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
                }
            }]],
	    'libraries': ['<(module_root_dir)/liblc3core.a']
        }
    ]
}
