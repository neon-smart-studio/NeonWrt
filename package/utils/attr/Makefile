#
# Copyright (C) 2014-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=attr
PKG_VERSION:=2.5.2
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@SAVANNAH/attr
PKG_HASH:=f2e97b0ab7ce293681ab701915766190d607a1dba7fae8a718138150b700a70b

PKG_MAINTAINER:=Maxim Storchak <m.storchak@gmail.com>
PKG_CPE_ID:=cpe:/a:attr_project:attr

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/attr/Default
  TITLE:=Extended attributes (xattr) manipulation
  URL:=http://savannah.nongnu.org/projects/attr
  SUBMENU:=Filesystem
endef

define Package/attr/Default/description
  Extended attributes support
endef

define Package/attr
$(call Package/attr/Default)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE+=utils
  LICENSE:=GPL-2.0-or-later
  LICENSE_FILES:=doc/COPYING
  DEPENDS:=+libattr
endef

define Package/libattr
$(call Package/attr/Default)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+=library
  LICENSE:=LGPL-2.1-or-later
  LICENSE_FILES:=doc/COPYING.LGPL
endef

define Package/libattr/description
$(call Package/attr/Default/description)
 This package provides libattr
endef

define Package/attr/description
$(call Package/attr/Default/description)
 This package provides xattr manipulation utilities
 - attr
 - getfattr
 - setfattr
endef

CONFIGURE_ARGS += --enable-static --enable-shared

define Package/attr/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

define Package/libattr/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc
	$(CP) $(PKG_INSTALL_DIR)/etc $(1)/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

define Package/attr/conffiles
/etc/xattr.conf
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/{include,lib} $(1)/usr/
endef

$(eval $(call BuildPackage,attr))
$(eval $(call BuildPackage,libattr))
