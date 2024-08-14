#!/bin/sh

# Copyright 2013-2018 Brian Sidebotham <brian.sidebotham@gmail.com>
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# Use an include guard
if [ "${__valvers_arm_rpi_common_build}X" != "X" ]; then
    return
fi

__valvers_arm_rpi_common_build="included"

# Which compiler are we going to use
compiler_config_file=${basedir}/compiler/.compiler_config
if [ -f ${compiler_config_file} ]; then
    . ${compiler_config_file}
    if [ "${toolchain_version}X" != "X" ]; then
        if [ "${toolchain_version}" != "${toolchain_latest_version}" ]; then
            echo "WARNING: The toolchain version your using (${toolchain_version}) is not the latest version" >&2
            echo "         for the tutorial. Please run the compiler/get_compiler.sh script again to update" >&2
            echo "         your compiler verison" >&2
        fi
    fi
else
    # Drop back to hoping the compiler is in the PATH
    toolchain=arm-none-eabi-
fi

# Setup the common cmake toolchain directory (the toolchains should always be common)
cmake_toolchain_dir=${basedir}/compiler/cmake-toolchains
