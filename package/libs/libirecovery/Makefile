#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libirecovery
PKG_VERSION:=1.2.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://github.com/libimobiledevice/libirecovery/releases/download/$(PKG_VERSION)
PKG_HASH:=d25f4b85c24df206efbbbd2d6d45d1637229e756c52d535eef047a163799f67c

PKG_MAINTAINER:=Rosen Penev <rosenp@gmail.com>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libirecovery/Default
  URL:=https://github.com/libimobiledevice/libirecovery
  SUBMENU:=libimobiledevice
endef

define Package/libirecovery/Default/description
  libirecovery is a cross-platform library which implements communication
  to iBoot/iBSS found on Apple's iOS devices via USB.
endef

define Package/libirecovery
  $(call Package/libirecovery/Default)
  TITLE:=A library that talks to Apple iBoot/iBSS
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libimobiledevice-glue +libreadline +libusb-1.0
endef

define Package/libirecovery/description
  $(call Package/libirecovery/Default/description)
endef

define Package/irecovery
  $(call Package/libirecovery/Default)
  TITLE:=A utility that talks to Apple iBoot/iBSS
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libirecovery
endef

define Package/irecovery/description
  $(call Package/libirecovery/Default/description)
  This package contains the libirecovery utilities.
endef

CONFIGURE_ARGS += \
	--disable-static \
	--without-udev

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libirecovery.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libirecovery-1.0.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libirecovery-1.0.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libirecovery/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libirecovery-1.0.so.* $(1)/usr/lib/
endef

define Package/irecovery/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/irecovery $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libirecovery))
$(eval $(call BuildPackage,irecovery))
