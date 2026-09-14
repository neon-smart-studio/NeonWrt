# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

PKG_VERSION:=2.13
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/TrustedFirmware-A/trusted-firmware-a.git
PKG_SOURCE_VERSION:=e0c4d3903b382bf34f552af53e6d955fae5283ab
PKG_MIRROR_HASH:=skip

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=docs/license.rst

include $(INCLUDE_DIR)/trusted-firmware-a.mk
include $(INCLUDE_DIR)/package.mk

define Trusted-Firmware-A/Default
  BUILD_TARGET:=ti
  BUILD_SUBTARGET:=am62x
  BUILD_DEVICES:=ti_sk-am62
endef

define Trusted-Firmware-A/ti-k3-am62x
  NAME:=TI K3 AM62x
  PLAT:=k3
  TFA_IMAGE:=bl31.bin

  TFA_MAKE_FLAGS += \
	ARCH=aarch64 \
	TARGET_BOARD=lite \
	SPD=opteed
endef

TFA_TARGETS := ti-k3-am62x

define Package/trusted-firmware-a-ti-k3-am62x/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)/ti-k3
	$(CP) \
		$(PKG_BUILD_DIR)/build/k3/lite/release/bl31.bin \
		$(STAGING_DIR_IMAGE)/ti-k3/bl31.bin
endef

$(eval $(call BuildPackage/Trusted-Firmware-A))