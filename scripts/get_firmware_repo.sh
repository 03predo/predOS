#!/bin/sh

scriptdir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
basedir=${scriptdir}/../
firmwaredir=${scriptdir}/../firmware
if [ ! -d ${firmwaredir} ] ; then
  mkdir ${firmwaredir}
  cd ${basedir} && git clone --depth 1 https://github.com/raspberrypi/firmware.git firmware
fi
