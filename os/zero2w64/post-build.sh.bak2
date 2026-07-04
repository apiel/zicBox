#!/bin/sh
set -e

./board/raspberrypizero2w-64/post-build.sh

# Configure fstab to mount /boot partition
mkdir -p ${TARGET_DIR}/boot
echo "/dev/mmcblk0p1 /boot vfat defaults 0 2" >> ${TARGET_DIR}/etc/fstab

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
ZICBOX_DIR=${SCRIPT_DIR}/../..
ZIC_PROJECT=${ZIC_PROJECT:-zicXYv2}
ZIC_PROJECT_DIR=${ZICBOX_DIR}/${ZIC_PROJECT}
ZIC_PROJECT_BUILD_DIR=${ZIC_PROJECT_BUILD_DIR:-${ZIC_PROJECT_DIR}/build/arm64}
ZIC_PROJECT_BUILD_TARGET=${ZIC_PROJECT_BUILD_TARGET:-buildPi}

if [ ! -d "${ZIC_PROJECT_DIR}" ]; then
	echo "Error: ZIC_PROJECT_DIR does not exist: ${ZIC_PROJECT_DIR}"
	exit 1
fi

echo "Building ZicBox ${ZICBOX_DIR}"
echo "Building firmware project ${ZIC_PROJECT_DIR} (target: ${ZIC_PROJECT_BUILD_TARGET})"
cd ${ZICBOX_DIR}
pwd
make -C ${ZIC_PROJECT_DIR} ${ZIC_PROJECT_BUILD_TARGET}
make buildSplash cc=arm64

rm -rf ${TARGET_DIR}/opt/zicBox || true
mkdir -p ${TARGET_DIR}/opt/zicBox
cp -r ${ZIC_PROJECT_BUILD_DIR}/* ${TARGET_DIR}/opt/zicBox/.
cp ${ZICBOX_DIR}/build/arm64/splash ${TARGET_DIR}/opt/zicBox/.
cp -r ${ZICBOX_DIR}/data ${TARGET_DIR}/opt/zicBox/.
cp ${ZICBOX_DIR}/config.json ${TARGET_DIR}/opt/zicBox/.
rm -rf ${TARGET_DIR}/opt/zicBox/data/.git || true

mv ${TARGET_DIR}/etc/init.d/S10mdev ${TARGET_DIR}/etc/init.d/S000mdev || true
mv ${TARGET_DIR}/etc/init.d/S05avahi-setup.sh ${TARGET_DIR}/etc/init.d/S35avahi-setup.sh || true
