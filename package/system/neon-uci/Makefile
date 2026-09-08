include $(TOPDIR)/rules.mk

PKG_NAME:=neon-uci
PKG_VERSION:=0.1.1
PKG_RELEASE:=1

PKG_LICENSE:=MIT
PKG_MAINTAINER:=NeonWrt Project

include $(INCLUDE_DIR)/package.mk

define Package/neon-uci
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=NeonWrt UCI to systemd translation layer
  DEPENDS:=+libuci +systemd
endef

define Package/neon-uci/description
  Watches committed UCI configuration changes and dispatches package-specific
  translators which generate systemd-native runtime configuration. It is
  designed to coexist with OpenWrt-native components such as netifd.
endef

define Package/neon-uci/conffiles
/etc/config/neon_systemd
/etc/neon-uci.d/10-hostname.conf
/etc/neon-uci.d/20-timesyncd.conf
/etc/neon-uci.d/30-journald.conf
endef

define Build/Prepare
	rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CPPFLAGS="$(TARGET_CPPFLAGS)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall -Wextra -Wformat=2" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		LDLIBS="-luci"
endef

define Package/neon-uci/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/neon-uci $(1)/usr/sbin/neon-uci

	$(INSTALL_DIR) $(1)/usr/libexec/neon-uci
	$(INSTALL_BIN) ./files/usr/libexec/neon-uci/common.sh $(1)/usr/libexec/neon-uci/common.sh
	$(INSTALL_BIN) ./files/usr/libexec/neon-uci/translate-hostname $(1)/usr/libexec/neon-uci/translate-hostname
	$(INSTALL_BIN) ./files/usr/libexec/neon-uci/translate-timesyncd $(1)/usr/libexec/neon-uci/translate-timesyncd
	$(INSTALL_BIN) ./files/usr/libexec/neon-uci/translate-journald $(1)/usr/libexec/neon-uci/translate-journald

	$(INSTALL_DIR) $(1)/etc/neon-uci.d
	$(INSTALL_CONF) ./files/etc/neon-uci.d/10-hostname.conf $(1)/etc/neon-uci.d/10-hostname.conf
	$(INSTALL_CONF) ./files/etc/neon-uci.d/20-timesyncd.conf $(1)/etc/neon-uci.d/20-timesyncd.conf
	$(INSTALL_CONF) ./files/etc/neon-uci.d/30-journald.conf $(1)/etc/neon-uci.d/30-journald.conf

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/etc/config/neon_systemd $(1)/etc/config/neon_systemd

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system
	$(INSTALL_DATA) ./files/usr/lib/systemd/system/neon-uci.service $(1)/usr/lib/systemd/system/neon-uci.service
	$(INSTALL_DATA) ./files/usr/lib/systemd/system/neon-uci-apply@.service $(1)/usr/lib/systemd/system/neon-uci-apply@.service

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/multi-user.target.wants
	$(LN) ../neon-uci.service $(1)/usr/lib/systemd/system/multi-user.target.wants/neon-uci.service

	$(INSTALL_DIR) $(1)/usr/share/doc/neon-uci
	$(INSTALL_DATA) ./README.md $(1)/usr/share/doc/neon-uci/README.md
endef

$(eval $(call BuildPackage,neon-uci))
