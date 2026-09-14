# SPDX-License-Identifier: GPL-2.0-only
#
# TI K3 boot firmware staging package for NeonWrt/OpenWrt.
# Source set aligned with TI Processor SDK Linux AM62x 11.01.05 / Linux 6.12.

include $(TOPDIR)/rules.mk

PKG_NAME:=ti-linux-firmware
PKG_VERSION:=11.01.05
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/TexasInstruments/ti-linux-firmware.git
PKG_SOURCE_VERSION:=33bec0e2ea9a25362c0c8243b2f1ed392dbc5b02
PKG_MIRROR_HASH:=skip

PKG_LICENSE:=Various
PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/ti-linux-firmware-am62x
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=TI K3 boot firmware for AM62x
  URL:=https://github.com/TexasInstruments/ti-linux-firmware
  DEPENDS:=@TARGET_ti_am62x
endef

define Package/ti-linux-firmware-am62x/description
Stages TI Foundational Security/System Firmware and Device Manager firmware
used by U-Boot binman to create AM62x tiboot3.bin and tispl.bin.
endef

define Build/Configure
endef

define Build/Compile
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)/ti-k3-firmware
	$(RM) -r $(STAGING_DIR_IMAGE)/ti-k3-firmware/ti-sysfw
	$(RM) -r $(STAGING_DIR_IMAGE)/ti-k3-firmware/ti-dm
	$(CP) $(PKG_BUILD_DIR)/ti-sysfw $(STAGING_DIR_IMAGE)/ti-k3-firmware/
	$(CP) $(PKG_BUILD_DIR)/ti-dm $(STAGING_DIR_IMAGE)/ti-k3-firmware/
endef

define Package/ti-linux-firmware-am62x/install
	:
endef

$(eval $(call BuildPackage,ti-linux-firmware-am62x))
