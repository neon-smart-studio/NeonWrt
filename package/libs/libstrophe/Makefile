#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libstrophe
PKG_VERSION:=0.13.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/strophe/libstrophe/releases/download/$(PKG_VERSION)
PKG_HASH:=86c0abd348ae66feb40b6886f2c7f04525f1d5b20a48c10d4db4ed532dc63f22

PKG_MAINTAINER:=Chih-Wei Chen <changeway@gmail.com>
PKG_LICENSE:=MIT OR GPL-3.0-only
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

PKG_CONFIG_DEPENDS:= \
	CONFIG_libstrophe-libxml2 \
	CONFIG_libstrophe-expat

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/libstrophe
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=XMPP client library
	URL:=http://strophe.im/libstrophe
	DEPENDS:= +libopenssl +libstrophe-libxml2:libxml2 +libstrophe-expat:libexpat +zlib
	MENU:=1
endef

define Package/libstrophe/description
	A simple, lightweight C library for writing XMPP clients
endef

define Package/libstrophe/config
	source "$(SOURCE)/Config.in"
endef

CONFIGURE_ARGS += \
	--with$(if $(CONFIG_libstrophe-libxml2),,out)-libxml2

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/ $(1)/usr/

	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libstrophe.{la,a,so*} $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libstrophe.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/libstrophe/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libstrophe.so.* $(1)/usr/lib
endef

$(eval $(call BuildPackage,libstrophe))
