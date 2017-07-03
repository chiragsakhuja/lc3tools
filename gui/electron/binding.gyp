{
    "targets": [
        {
            "include_dirs": [
                "<!(node -e \"require('nan')\")",
                "../.."
            ],
            "target_name": "addon",
            "sources": ["wrapper.cpp"],
            "link_settings": {
                "libraries": ["-Wl,-rpath,/Users/chirag/work/lc3v2/build/lib","-L/Users/chirag/work/lc3v2/build/lib","-llc3core"]
            }
        }
    ]
}
