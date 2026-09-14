include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

TI_K3_STAGE := $(STAGING_DIR_IMAGE)/ti-k3


define Build/prepare-arm64-image
	cp $(LINUX_DIR)/arch/arm64/boot/Image $(KDIR)/Image
endef


define Build/ti-am62-sdcard
	rm -rf $@.boot
	mkdir -p $@.boot/extlinux

	test -f "$(TI_K3_STAGE)/tiboot3.bin"
	test -f "$(TI_K3_STAGE)/tispl.bin"
	test -f "$(TI_K3_STAGE)/u-boot.img"

	$(CP) "$(TI_K3_STAGE)/tiboot3.bin" \
		$@.boot/tiboot3.bin

	$(CP) "$(TI_K3_STAGE)/tispl.bin" \
		$@.boot/tispl.bin

	$(CP) "$(TI_K3_STAGE)/u-boot.img" \
		$@.boot/u-boot.img

	$(CP) "$(LINUX_DIR)/arch/arm64/boot/Image" \
		$@.boot/Image

	$(CP) "$(LINUX_DIR)/arch/arm64/boot/dts/ti/k3-am625-sk.dtb" \
		$@.boot/k3-am625-sk.dtb

	$(CP) ./extlinux.conf \
		$@.boot/extlinux/extlinux.conf

	KERNELPARTTYPE=c \
	PADDING=1 \
	$(TOPDIR)/scripts/gen_image_generic.sh \
		$@ \
		128 \
		$@.boot \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE) \
		$(IMAGE_ROOTFS) \
		1024

	rm -rf $@.boot
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