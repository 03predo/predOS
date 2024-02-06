#!/bin/sh

scriptdir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
basedir=${scriptdir}/../
firmwaredir=${scriptdir}/../firmware

echo "hello"
if [ ! -d ${firmwaredir} ] ; then
  echo "hello1"
  mkdir ${firmwaredir}
  cd ${basedir} && git clone --depth 1 https://github.com/raspberrypi/firmware.git firmware
fi
