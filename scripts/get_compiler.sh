#!/bin/sh

scriptdir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
compilerdir=${scriptdir}/../compiler

# Include the common shell functions
. ${scriptdir}/common_functions.sh

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
