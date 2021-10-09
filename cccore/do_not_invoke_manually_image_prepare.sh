#! /usr/bin/env bash
set -ex
BUILD_DIR=$1
dd conv=sync if=/dev/zero of=${BUILD_DIR}/cccore_limine.img bs=512 count=20480
echo 'type=83' | sfdisk ${BUILD_DIR}/cccore_limine.img
limine-install ${BUILD_DIR}/cccore_limine.img
mkfs.ext2 -F -E offset=1048576 ${BUILD_DIR}/cccore_limine.img
dd if=${BUILD_DIR}/cccore_limine.img bs=4096 skip=256 of=${BUILD_DIR}/cccore_limine.ext2.img
e2mkdir ${BUILD_DIR}/cccore_limine.ext2.img:/boot
e2cp limine/limine.cfg ${BUILD_DIR}/cccore_limine.ext2.img:/boot/
e2cp limine/limine.sys ${BUILD_DIR}/cccore_limine.ext2.img:/boot/
e2cp ${BUILD_DIR}/cccore.elf ${BUILD_DIR}/cccore_limine.ext2.img:/boot/
e2ls ${BUILD_DIR}/cccore_limine.ext2.img:/
dd if=${BUILD_DIR}/cccore_limine.ext2.img bs=4096 seek=256 of=${BUILD_DIR}/cccore_limine.img
