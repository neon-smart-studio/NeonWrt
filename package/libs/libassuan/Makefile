include $(TOPDIR)/rules.mk

PKG_NAME:=libassuan
PKG_VERSION:=2.5.6
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://gnupg.org/ftp/gcrypt/$(PKG_NAME)
PKG_HASH:=e9fd27218d5394904e4e39788f9b1742711c3e6b41689a31aa3380bd5aa4f426

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING COPYING.LIB

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libassuan
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=GnuPG IPC library
  URL:=https://gnupg.org/software/libassuan/index.html
  DEPENDS:=+libgpg-error
endef

define Package/libassuan/description
Libassuan is a small library implementing the so-called Assuan
protocol. This protocol is used for IPC between most newer GnuPG
components. Both, server and client side functions are provided. 
endef

define Build/InstallDev
	$(INSTALL_DIR) $(2)/bin $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/libassuan-config \
		$(2)/bin/
	$(SED) \
		's,^\(prefix\|exec_prefix\)=.*,\1=$(STAGING_DIR)/usr,g' \
		$(2)/bin/libassuan-config
	ln -sf $(STAGING_DIR)/host/bin/libassuan-config $(1)/usr/bin/libassuan-config

	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/include/assuan.h \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libassuan.{la,so*} \
		$(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/share/aclocal
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/share/aclocal/libassuan.m4 \
		$(1)/usr/share/aclocal/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libassuan.pc \
		$(1)/usr/lib/pkgconfig
endef

define Package/libassuan/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libassuan.so.* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libassuan))
