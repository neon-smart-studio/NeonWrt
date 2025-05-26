include $(TOPDIR)/rules.mk

PKG_NAME:=urandom-seed
PKG_RELEASE:=3
PKG_LICENSE:=GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

define Package/urandom-seed
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+getrandom
  TITLE:=/etc/urandom.seed handling for OpenWrt
  URL:=https://openwrt.org/
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Quilt
endef

define Build/Compile/Default
endef
Build/Compile = $(Build/Compile/Default)

define Package/urandom-seed/install
	$(INSTALL_DIR) $(1)/lib/preinit
	$(INSTALL_BIN) ./files/lib/preinit/81_urandom_seed $(1)/lib/preinit
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) ./files/sbin/urandom_seed $(1)/sbin
	$(INSTALL_BIN) ./files/sbin/urandom_seed_save $(1)/sbin

	$(INSTALL_DIR) $(1)/etc/systemd/system
	$(INSTALL_DATA) ./files/urandom-seed.service $(1)/etc/systemd/system/urandom-seed.service
	$(INSTALL_DATA) ./files/urandom-seed-save.service $(1)/etc/systemd/system/urandom-seed-save.service

	$(INSTALL_DIR) $(1)/etc/systemd/system/sysinit.target.wants
	ln -sf /etc/systemd/system/urandom-seed.service $(1)/etc/systemd/system/sysinit.target.wants/urandom-seed.service

	$(INSTALL_DIR) $(1)/etc/systemd/system/shutdown.target.wants
	ln -sf /etc/systemd/system/urandom-seed-save.service $(1)/etc/systemd/system/shutdown.target.wants/urandom-seed-save.service
endef

$(eval $(call BuildPackage,urandom-seed))
