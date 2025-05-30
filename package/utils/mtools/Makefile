include $(TOPDIR)/rules.mk

PKG_NAME:=mtools
PKG_VERSION:=4.0.45
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@GNU/mtools
PKG_HASH:=0b008a96bd0efe0e542fa4383d469af66bc4a93394990b103730a8160a67d618

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/mtools
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Disc
  TITLE:=Collection of utilities to access MS-DOS disks
  URL:=https://www.gnu.org/software/mtools
endef

define Package/mtools/description
  Mtools is a collection of utilities to access MS-DOS disks from GNU and Unix without mounting them.
endef

CONFIGURE_VARS += \
	ac_cv_have_x="have_x=no" \
	ac_cv_lib_bsd_main=no

define Package/mtools/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/amuFormat.sh $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mcheck $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mcomp $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mkmanifest $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mtools $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mxtar $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/tgz $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/uz $(1)/usr/bin/
	$(LN) uz $(1)/usr/bin/lz
	$(LN) mtools $(1)/usr/bin/mattrib
	$(LN) mtools $(1)/usr/bin/mbadblock
	$(LN) mtools $(1)/usr/bin/mcat
	$(LN) mtools $(1)/usr/bin/mcd
	$(LN) mtools $(1)/usr/bin/mclasserase
	$(LN) mtools $(1)/usr/bin/mcopy
	$(LN) mtools $(1)/usr/bin/mdel
	$(LN) mtools $(1)/usr/bin/mdeltree
	$(LN) mtools $(1)/usr/bin/mdir
	$(LN) mtools $(1)/usr/bin/mdu
	$(LN) mtools $(1)/usr/bin/mformat
	$(LN) mtools $(1)/usr/bin/minfo
	$(LN) mtools $(1)/usr/bin/mlabel
	$(LN) mtools $(1)/usr/bin/mmd
	$(LN) mtools $(1)/usr/bin/mmount
	$(LN) mtools $(1)/usr/bin/mmove
	$(LN) mtools $(1)/usr/bin/mpartition
	$(LN) mtools $(1)/usr/bin/mrd
	$(LN) mtools $(1)/usr/bin/mren
	$(LN) mtools $(1)/usr/bin/mshortname
	$(LN) mtools $(1)/usr/bin/mshowfat
	$(LN) mtools $(1)/usr/bin/mtoolstest
	$(LN) mtools $(1)/usr/bin/mtype
	$(LN) mtools $(1)/usr/bin/mzip
endef

$(eval $(call BuildPackage,mtools))
