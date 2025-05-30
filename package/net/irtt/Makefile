# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2021 Marcel Vital

include $(TOPDIR)/rules.mk

PKG_NAME:=irtt
PKG_VERSION:=0.9.1
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/heistp/irtt.git
PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_MIRROR_HASH:=c6a3de22b651b4ece1c0e2b2654aa43b2fbb769a91ad15229884dc815b23300e

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Marcel Vital <ralmina@tuta.io>

PKG_BUILD_DIR:=$(BUILD_DIR)/irtt-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_BUILD_FLAGS:=no-mips16

GO_PKG:=github.com/heistp/irtt

include $(INCLUDE_DIR)/package.mk
include ../../lang/golang/golang-package.mk

define Package/irtt
  SECTION:=net
  CATEGORY:=Network
  TITLE:=irtt
  URL:=https://github.com/heistp/irtt
  DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/irtt/description
  Isochronous round trip time tool. 
  Useful for measuring one-way send or recv delay between hosts,
  among other things.
endef

define Package/irtt/install
	$(call GoPackage/Package/Install/Bin,$(PKG_INSTALL_DIR))

	$(INSTALL_DIR) $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/irtt $(1)/usr/bin/
endef

$(eval $(call GoBinPackage,irtt))
$(eval $(call BuildPackage,irtt))
