#
# Copyright (C) 2011-2012 Daniel Golle <dgolle@allnet.de>
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rssileds
PKG_RELEASE:=5
PKG_LICENSE:=GPL-2.0+

include $(INCLUDE_DIR)/package.mk

define Package/rssileds
  SECTION:=net
  CATEGORY:=Network
  TITLE:=RSSI real-time LED indicator
  DEPENDS:=+libiwinfo +libnl-tiny +libubox +libuci +libuci-lua
  MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
endef

define Package/rssileds/description
  A small process written in C to update the signal-strength indicator LEDs
endef

define Build/Configure
endef

TARGET_LDFLAGS += -liwinfo -luci -lubox -lnl-tiny

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_CPPFLAGS) -Wall \
		-o $(PKG_BUILD_DIR)/rssileds $(PKG_BUILD_DIR)/rssileds.c $(TARGET_LDFLAGS)
endef

define Package/rssileds/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rssileds $(1)/usr/sbin/

	$(INSTALL_DIR) $(1)/usr/libexec
	$(INSTALL_BIN) ./files/rssileds-wrapper.sh $(1)/usr/libexec/

	$(INSTALL_DIR) $(1)/etc/systemd/system
	$(INSTALL_CONF) ./files/rssileds.service $(1)/etc/systemd/system/
endef

$(eval $(call BuildPackage,rssileds))
