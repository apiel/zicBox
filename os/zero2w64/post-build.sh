#!/bin/sh
set -e

./board/raspberrypizero2w-64/post-build.sh

# Configure fstab to mount /boot partition
mkdir -p ${TARGET_DIR}/boot
echo "/dev/mmcblk0p1 /boot vfat defaults 0 2" >> ${TARGET_DIR}/etc/fstab

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
ZICBOX_DIR=${TARGET_DIR}/../..

cd ${ZICBOX_DIR}
npm run build:pixel
make pixelLibs cc=pixel_arm64
make buildPixel cc=pixel_arm64
make splash cc=pixel_arm64

rm -rf ${TARGET_DIR}/opt/zicBox || true
mkdir -p ${TARGET_DIR}/opt/zicBox
cp -r ${ZICBOX_DIR}/build/arm/* ${TARGET_DIR}/opt/zicBox/.
cp -r ${ZICBOX_DIR}/data ${TARGET_DIR}/opt/zicBox/.

mv ${TARGET_DIR}/etc/init.d/S10mdev ${TARGET_DIR}/etc/init.d/S000mdev || true
mv ${TARGET_DIR}/etc/init.d/S05avahi-setup.sh ${TARGET_DIR}/etc/init.d/S35avahi-setup.sh || true
