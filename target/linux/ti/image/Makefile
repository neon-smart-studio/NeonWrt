TI_IMAGE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

TI_K3_STAGE := $(STAGING_DIR_IMAGE)/ti-k3

FAT32_BLOCKS := 131072


define Build/prepare-arm64-image
	cp $(LINUX_DIR)/arch/arm64/boot/Image $(KDIR)/Image
endef


define Build/ti-am62-sdcard
	$(TI_IMAGE_DIR)gen_ti_am62x_sdcard_img.sh \
		$@ \
		$(IMAGE_ROOTFS) \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE) \
		$(LINUX_DIR) \
		$(TI_K3_STAGE) \
		$(TI_IMAGE_DIR)extlinux.conf \
		$(TOPDIR)/scripts/gen_image_generic.sh \
		$(FAT32_BLOCKS)
endef


define Device/Default
  PROFILES := Default

  KERNEL_NAME := Image
  KERNEL := prepare-arm64-image | kernel-bin
  KERNEL_INSTALL := 1

  IMAGES := sdcard.img.gz

  IMAGE/sdcard.img.gz := \
	ti-am62-sdcard | \
	gzip
endef


define Device/ti_sk-am62
  DEVICE_VENDOR := Texas Instruments
  DEVICE_MODEL := SK-AM62
  DEVICE_VARIANT := AM625

  DEVICE_DTS := k3-am625-sk
  DEVICE_DTS_DIR := $(DTS_DIR)/ti

  DEVICE_PACKAGES := \
	kmod-usb2 \
	kmod-usb3
endef

TARGET_DEVICES += ti_sk-am62


$(eval $(call BuildImage))