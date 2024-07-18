#!/bin/sh

if [ $# -lt 4 ]; then
    echo "usage: img_add_file.sh <mbr> <fat> <file> <disk_image>" >&2
    exit 1
fi

mbr=${1}
fat=${2}
file=${3}
disk_image=${4}
filename="${file##*/}"

mcopy -v -i ${fat} ${file} ::${filename}
dd bs=512 if="${mbr}" of="${disk_image}" count=2048
cat ${fat} >> "${disk_image}"
