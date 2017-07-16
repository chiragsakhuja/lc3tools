{
    'targets': [
        {
            'include_dirs': [
                "<!(node -e \"require('nan')\")",
                '../..'
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
            'link_settings': {
                'libraries': ['-Wl,-rpath,/Users/chirag/work/lc3v2/build/lib',
                    '-L/Users/chirag/work/lc3v2/build/lib','-llc3core']
            }
        }
    ]
}
