#!/bin/sh
#
# SPDX-License-Identifier: GPL-2.0-only
#
# Generate TI AM62 SD card image
#

set -eu

if [ "$#" -ne 8 ]; then
	echo "Usage: $0 <output.img> <rootfs.img> <rootfs_size_mb> <linux_dir> <ti_k3_stage> <extlinux.conf> <gen_image_generic.sh> <fat_blocks>"
	exit 1
fi

OUTPUT="$1"
ROOTFS="$2"
ROOTFS_SIZE="$3"
LINUX_DIR="$4"
TI_K3_STAGE="$5"
EXTLINUX_CONF="$6"
GEN_IMAGE_GENERIC="$7"
FAT32_BLOCKS="$8"

BOOT_IMG="${OUTPUT}.boot"
EMPTY_DIR="${OUTPUT}.empty"

BOOT_SIZE_MB=128
ALIGN_KB=1024

cleanup()
{
	rm -rf "${EMPTY_DIR}"
	rm -f "${BOOT_IMG}"
}

trap cleanup EXIT INT TERM

echo "==> Generating TI AM62 SD card image"
echo "    Output: ${OUTPUT}"
echo "    RootFS: ${ROOTFS}"
echo "    Linux:  ${LINUX_DIR}"
echo "    K3:     ${TI_K3_STAGE}"

#
# Verify input files
#
for file in \
	"${TI_K3_STAGE}/tiboot3.bin" \
	"${TI_K3_STAGE}/tispl.bin" \
	"${TI_K3_STAGE}/u-boot.img" \
	"${LINUX_DIR}/arch/arm64/boot/Image" \
	"${LINUX_DIR}/arch/arm64/boot/dts/ti/k3-am625-sk.dtb" \
	"${EXTLINUX_CONF}" \
	"${ROOTFS}" \
	"${GEN_IMAGE_GENERIC}"
do
	if [ ! -f "${file}" ]; then
		echo "ERROR: Required file not found:"
		echo "       ${file}"
		exit 1
	fi
done

rm -f "${BOOT_IMG}"
rm -rf "${EMPTY_DIR}"

#
# Create 128 MiB FAT32 boot filesystem
#
echo "==> Creating FAT32 boot filesystem"

mkfs.fat \
	-F 32 \
	-n BOOT \
	-C "${BOOT_IMG}" \
	"${FAT32_BLOCKS}"

#
# TI K3 boot chain
#
echo "==> Installing TI K3 bootloader"

mcopy -i "${BOOT_IMG}" \
	"${TI_K3_STAGE}/tiboot3.bin" \
	::tiboot3.bin

mcopy -i "${BOOT_IMG}" \
	"${TI_K3_STAGE}/tispl.bin" \
	::tispl.bin

mcopy -i "${BOOT_IMG}" \
	"${TI_K3_STAGE}/u-boot.img" \
	::u-boot.img

#
# Linux kernel
#
echo "==> Installing Linux kernel"

mcopy -i "${BOOT_IMG}" \
	"${LINUX_DIR}/arch/arm64/boot/Image" \
	::Image

#
# Device Tree
#
echo "==> Installing Device Tree"

mcopy -i "${BOOT_IMG}" \
	"${LINUX_DIR}/arch/arm64/boot/dts/ti/k3-am625-sk.dtb" \
	::k3-am625-sk.dtb

#
# extlinux
#
echo "==> Installing extlinux.conf"

mmd -i "${BOOT_IMG}" ::extlinux

mcopy -i "${BOOT_IMG}" \
	"${EXTLINUX_CONF}" \
	::extlinux/extlinux.conf

#
# Create partitioned SD image.
#
# gen_image_generic.sh creates:
#
#   partition 1 = temporary kernel filesystem
#   partition 2 = rootfs
#
# We then overwrite partition 1 with our FAT32 boot filesystem.
#
mkdir -p "${EMPTY_DIR}"

echo "==> Creating partition table and root filesystem"

KERNELPARTTYPE=c \
PADDING=1 \
"${GEN_IMAGE_GENERIC}" \
	"${OUTPUT}" \
	"${BOOT_SIZE_MB}" \
	"${EMPTY_DIR}" \
	"${ROOTFS_SIZE}" \
	"${ROOTFS}" \
	"${ALIGN_KB}"

#
# ALIGN=1024 means partition 1 starts at:
#
#   1024 KiB / 512 = sector 2048
#
echo "==> Writing FAT32 boot partition"

dd \
	if="${BOOT_IMG}" \
	of="${OUTPUT}" \
	bs=512 \
	seek=2048 \
	conv=notrunc

sync

echo "==> TI AM62 SD image completed:"
echo "    ${OUTPUT}"
