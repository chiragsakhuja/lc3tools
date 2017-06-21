#!/bin/bash

export npm_config_target=1.6.11
export npm_config_arg=x64
export npm_config_target_arch=x64
export npm_config_disturl=https://atom.io/download/electron
export npm_config_runtime=electron
export npm_config_build_from_source=true
HOME=./.electron-gyp npm install
