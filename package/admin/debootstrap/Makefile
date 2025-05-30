#
# Copyright (C) 2010 Gianluigi Tiesi <sherpya@netfarm.it>
# Copyright (C) 2011-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=debootstrap
PKG_REAL_VERSION:=1.0.128+nmu2+deb12u1
PKG_VERSION:=1.0.128.2~deb121
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-udeb_$(PKG_REAL_VERSION)_all.udeb
PKG_SOURCE_URL:=@DEBIAN/pool/main/d/debootstrap
PKG_HASH:=4fa4ec7c144ed047c47d0d8eb9b91b56eaa9b2db2b52510777abbabf5965d268

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>
PKG_LICENSE:=Unique
PKG_LICENSE_FILES:=debian/copyright

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_REAL_VERSION)

UNPACK_CMD=ar -p "$(DL_DIR)/$(PKG_SOURCE)" data.tar.xz | xzcat | tar -C $(1) -xf -

include $(INCLUDE_DIR)/package.mk

define Package/debootstrap
  SECTION:=admin
  CATEGORY:=Administration
  TITLE:=Bootstrap a basic Debian system
  URL:=https://wiki.debian.org/Debootstrap
  DEPENDS:= +coreutils +coreutils-chroot +coreutils-sha1sum +ar +xz-utils +xz
endef

define Package/debootstrap/description
 debootstrap is used to create a Debian base system from scratch, without
 requiring the availability of dpkg or apt. It does this by downloading .deb
 files from a mirror site, and carefully unpacking them into a directory which
 can eventually be chrooted into.
endef

define Build/Compile
# file pkgdetails.c was imported from debian package base-installer version 1.130
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) ./files/pkgdetails.c -o $(PKG_BUILD_DIR)/usr/share/debootstrap/pkgdetails
endef

define Package/debootstrap/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/usr/sbin/$(PKG_NAME) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/share/debootstrap
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/usr/share/debootstrap/pkgdetails $(1)/usr/share/debootstrap
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/usr/share/debootstrap/functions $(1)/usr/share/debootstrap
	$(INSTALL_DIR) $(1)/usr/share/debootstrap/scripts
	$(CP) $(PKG_BUILD_DIR)/usr/share/debootstrap/scripts/* $(1)/usr/share/debootstrap/scripts
endef

$(eval $(call BuildPackage,debootstrap))
