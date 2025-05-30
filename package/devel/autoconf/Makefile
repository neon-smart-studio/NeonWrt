#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=autoconf
PKG_VERSION:=2.72
PKG_RELEASE:=1

PKG_SOURCE_URL:=@GNU/autoconf
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_HASH:=ba885c1319578d6c94d46e9b0dceb4014caafe2490e437a0dbca3f270a223f5a

PKG_MAINTAINER:=Heinrich Schuchardt <xypron.glpk@gmx.de>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYINGv3 COPYING.EXCEPTION

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/autoconf
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=autoconf
  URL:=https://www.gnu.org/software/autoconf/
  DEPENDS:=+m4 +perl +perlbase-data +perlbase-file +perlbase-getopt \
  +perlbase-dynaloader +perlbase-text
endef

define Package/autoconf/description
  Autoconf is an extensible package of M4 macros that produce shell scripts to
  automatically configure software source code packages.
endef

CONFIGURE_VARS += M4=m4 EMACS=no

FIX_PATHS = $(SED) '1c \#!/usr/bin/perl' -e 's| /[^ ]*/bin/perl| /usr/bin/perl|g'

define Package/autoconf/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	grep -rEl "#\!.*perl" $(1)/usr/bin/ | xargs $(FIX_PATHS)
	$(INSTALL_DIR) $(1)/usr/share/autoconf
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/autoconf/INSTALL \
	$(1)/usr/share/autoconf/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/autoconf/autom4te.cfg \
	$(1)/usr/share/autoconf/
	$(INSTALL_DIR) $(1)/usr/share/autoconf/Autom4te
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/autoconf/Autom4te/* \
	$(1)/usr/share/autoconf/Autom4te/
	$(INSTALL_DIR) $(1)/usr/share/autoconf/autoconf
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/autoconf/autoconf/* \
	$(1)/usr/share/autoconf/autoconf/
	$(INSTALL_DIR) $(1)/usr/share/autoconf/autoscan
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/autoconf/autoscan/* \
	$(1)/usr/share/autoconf/autoscan/
	$(INSTALL_DIR) $(1)/usr/share/autoconf/autotest
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/autoconf/autotest/* \
	$(1)/usr/share/autoconf/autotest/
	$(INSTALL_DIR) $(1)/usr/share/autoconf/m4sugar
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/autoconf/m4sugar/* \
	$(1)/usr/share/autoconf/m4sugar/
endef

$(eval $(call BuildPackage,autoconf))
