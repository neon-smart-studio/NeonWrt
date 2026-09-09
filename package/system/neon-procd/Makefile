include $(TOPDIR)/rules.mk

PKG_NAME:=neon-procd
PKG_VERSION:=0.2.2
PKG_RELEASE:=1

PKG_LICENSE:=GPL-2.0-only
PKG_MAINTAINER:=NeonWrt

include $(INCLUDE_DIR)/package.mk

 define Package/neon-procd
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=OpenWrt procd compatibility layer for systemd
  DEPENDS:=+systemd +libjson-c +libubox +ubus
 endef

 define Package/neon-procd/description
  Compatibility layer that lets OpenWrt USE_PROCD init scripts launch and
  control services through systemd transient units.
 endef

TARGET_CFLAGS += -Wall -Wextra -Werror
TARGET_LDFLAGS += -ljson-c
UBUS_LDFLAGS := -lubus -lubox -lblobmsg_json

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_CPPFLAGS) \
		-o $(PKG_BUILD_DIR)/neon-procd $(PKG_BUILD_DIR)/neon-procd.c \
		$(TARGET_LDFLAGS)
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_CPPFLAGS) \
		-o $(PKG_BUILD_DIR)/neon-procd-ubus $(PKG_BUILD_DIR)/neon-procd-ubus.c \
		$(UBUS_LDFLAGS)
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) ./src/neon-procd.c $(PKG_BUILD_DIR)/
	$(CP) ./src/neon-procd-ubus.c $(PKG_BUILD_DIR)/
endef

define Package/neon-procd/install
	$(INSTALL_DIR) $(1)/lib/functions
	$(INSTALL_DATA) ./files/lib/functions/procd.sh \
		$(1)/lib/functions/procd.sh

	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/neon-procd $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/neon-procd-ubus $(1)/usr/sbin/

	$(INSTALL_DIR) $(1)/usr/libexec
	$(INSTALL_BIN) ./files/usr/libexec/neon-rc $(1)/usr/libexec/
	$(INSTALL_BIN) ./files/usr/libexec/neon-network-ready \
		$(1)/usr/libexec/neon-network-ready

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system

	$(INSTALL_DATA) ./files/usr/lib/systemd/system/neon-runtime.service \
		$(1)/usr/lib/systemd/system/neon-runtime.service

	$(INSTALL_DATA) ./files/usr/lib/systemd/system/ubusd.service \
		$(1)/usr/lib/systemd/system/ubusd.service

	$(INSTALL_DATA) ./files/usr/lib/systemd/system/neon-procd-ubus.service \
		$(1)/usr/lib/systemd/system/neon-procd-ubus.service

	$(INSTALL_DATA) ./files/usr/lib/systemd/system/neon-openwrt-rc.service \
		$(1)/usr/lib/systemd/system/neon-openwrt-rc.service

	$(INSTALL_DATA) ./files/usr/lib/systemd/system/neon-network-ready.service \
		$(1)/usr/lib/systemd/system/neon-network-ready.service

	$(INSTALL_DIR) $(1)/etc/systemd/system/multi-user.target.wants

	$(LN) /usr/lib/systemd/system/neon-runtime.service \
		$(1)/etc/systemd/system/multi-user.target.wants/neon-runtime.service

	$(LN) /usr/lib/systemd/system/ubusd.service \
		$(1)/etc/systemd/system/multi-user.target.wants/ubusd.service

	$(LN) /usr/lib/systemd/system/neon-procd-ubus.service \
		$(1)/etc/systemd/system/multi-user.target.wants/neon-procd-ubus.service

	$(LN) /usr/lib/systemd/system/neon-openwrt-rc.service \
		$(1)/etc/systemd/system/multi-user.target.wants/neon-openwrt-rc.service

	$(LN) /usr/lib/systemd/system/neon-network-ready.service \
		$(1)/etc/systemd/system/multi-user.target.wants/neon-network-ready.service
endef

$(eval $(call BuildPackage,neon-procd))
