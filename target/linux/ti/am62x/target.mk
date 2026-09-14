ARCH:=aarch64
SUBTARGET:=am62x
BOARDNAME:=TI AM62x
CPU_TYPE:=cortex-a53
CPU_SUBTYPE:=

FEATURES+=rtc

DEFAULT_PACKAGES += \
	kmod-usb-net \
	kmod-usb-net-cdc-ether