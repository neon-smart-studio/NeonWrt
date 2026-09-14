include $(TOPDIR)/rules.mk

ARCH:=aarch64
BOARD:=ti
BOARDNAME:=Texas Instruments K3
FEATURES:=ext4 squashfs usb usbgadget gpio
SUBTARGETS:=am62x

KERNEL_PATCHVER:=6.12

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += \
	kmod-usb2 kmod-usb3 \
	kmod-usb-storage \
	block-mount

$(eval $(call BuildTarget))