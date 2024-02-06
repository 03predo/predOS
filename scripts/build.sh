#!/bin/sh


scriptdir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
basedir=${scriptdir}/../

${scriptdir}/get_compiler.sh

if [ ! -d ${basedir}/build ]; then
  cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=compiler/toolchain-arm-none-eabi-rpi1.cmake
fi

cmake --build build

${scriptdir}/get_firmware_repo.sh
${scriptdir}/make_card.sh
