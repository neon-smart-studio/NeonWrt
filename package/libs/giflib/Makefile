#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=giflib
PKG_VERSION:=5.2.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/giflib
PKG_HASH:=be7ffbd057cadebe2aa144542fd90c6838c6a083b5e8a9048b8ee3b66b29d5fb

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:giflib_project:giflib

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_BUILD_FLAGS:=gc-sections

include $(INCLUDE_DIR)/package.mk

define Package/giflib
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=GIF libraries
  URL:=https://sourceforge.net/projects/giflib
endef

define Package/giflib/description
  giflib is a library for reading and writing gif images.
  It is API and ABI compatible with libungif which was in wide use while
  the LZW compression algorithm was patented.
endef

define Package/giflib-utils
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=GIF utilities
  URL:=https://sourceforge.net/projects/giflib
endef

define Package/giflib-utils/description
  These are the utilities that come with giflib.
endef

TARGET_CFLAGS += $(FPIC)
TARGET_LDFLAGS += -Wl,--as-needed

MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	PREFIX=$(CONFIGURE_PREFIX)

## Avoid 'convert' invocation during the build
define Build/Prepare
	$(call Build/Prepare/Default)
	touch $(PKG_BUILD_DIR)/doc/giflib-logo.gif
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/{lib,include}
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgif.so* $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libgif.a $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
endef

define Package/giflib/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgif.so* $(1)/usr/lib
endef

define Package/giflib-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

$(eval $(call BuildPackage,giflib))
$(eval $(call BuildPackage,giflib-utils))
