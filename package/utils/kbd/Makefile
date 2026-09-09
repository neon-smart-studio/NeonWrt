include $(TOPDIR)/rules.mk

PKG_NAME:=kbd
PKG_VERSION:=2.8.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/utils/kbd/
PKG_HASH:=01f5806da7d1d34f594b7b2a6ae1ab23215344cf1064e8edcd3a90fef9776a11

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/kbd-loadkeys
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Linux console loadkeys utility
  URL:=https://kbd-project.org/
endef

define Package/kbd-loadkeys/description
  Linux virtual console keyboard map loader.
endef

CONFIGURE_ARGS += \
	--disable-vlock

define Package/kbd-loadkeys/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/loadkeys \
		$(1)/usr/bin/loadkeys

	$(INSTALL_DIR) $(1)/usr/share/keymaps
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/share/keymaps/. \
		$(1)/usr/share/keymaps/
endef

$(eval $(call BuildPackage,kbd-loadkeys))