#!/bin/sh

# Copyright 2013-2018 Brian Sidebotham <brian.sidebotham@gmail.com>
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

scriptdir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
compilerdir=${scriptdir}/../cmake
basedir=${scriptdir}/../
firmwaredir=${scriptdir}/../firmware

# Include the common shell functions
. ${scriptdir}/common_functions.sh

if [ ! -d ${firmwaredir} ] ; then
  mkdir ${firmwaredir}
  cd ${basedir} && git clone --depth 1 https://github.com/raspberrypi/firmware.git firmware
fi

toolchain_latest_version="9.2-2019.12"
compiler=gcc-arm-${toolchain_latest_version}-x86_64-arm-none-eabi
archive=${compiler}.tar.xz
source=https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-a/9.2-2019.12/binrel/${archive}

if [ -d ${compilerdir}/${compiler} ]; then
  exit
fi

must_have curl
must_have md5sum
must_have tar
must_have ruby

if [ ! -f ${compilerdir}/${archive} ]; then
    must_run curl --fail -o ${compilerdir}/${archive} ${source}
fi

# Generate an MD5 file on the fly - we know the MD5 of the compiler package we're after, and the filename needs to
# include the full path to the archive
cat << EOF > ${compilerdir}/.compiler.md5
f7cc38b807c9b9815e5b0fb8440e3657  ${compilerdir}/${archive}
EOF

must_run md5sum -c ${compilerdir}/.compiler.md5
must_run tar -C ${compilerdir} -xf ${compilerdir}/${archive}

# Remove the archive after we're finished with it
rm -rf ${compilerdir}/${archive} > /dev/null 2>&1

# Test the compiler before quitting
must_run ${compilerdir}/${compiler}/bin/arm-none-eabi-gcc --version

# Get a compiler variable that we can ma
toolchain_version=$(${compilerdir}/${compiler}/bin/arm-none-eabi-gcc --version | grep -o "[0-9]\+\.[0-9]\+-[0-9]\+\.[0-9]\+")

# Create a compiler configuration file
cat << EOF > "${compilerdir}/.compiler_config"
#!/bin/sh
tcpath=${compilerdir}/${compiler}/bin
toolchain=${compilerdir}/${compiler}/bin/arm-none-eabi-
toolchain_version=${toolchain_version}
toolchain_latest_version=${toolchain_latest_version}
EOF

exit 0
