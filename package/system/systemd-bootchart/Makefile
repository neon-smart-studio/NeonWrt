################################################################################
#
# systemd-bootchart
#
################################################################################

include $(TOPDIR)/rules.mk

PKG_NAME:=systemd-bootchart
PKG_VERSION:=233
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/systemd/systemd-bootchart/releases/download/v$(PKG_VERSION)
PKG_HASH:=skip

PKG_LICENSE:=LGPL-2.1+
PKG_LICENSE_FILES:=LICENSE.LGPL2.1
PKG_BUILD_FLAGS:=gc-sections lto

include $(INCLUDE_DIR)/package.mk

define Package/systemd-bootchart
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=Systemd Bootchart - boot performance graphing tool
  URL:=https://github.com/systemd/systemd-bootchart
  DEPENDS:=+systemd
endef

define Package/systemd-bootchart/description
  Systemd Bootchart is a boot performance graphing tool that collects data
  during system startup and produces graphs showing CPU, I/O and other metrics.
endef

TARGET_LDFLAGS += -Wl,--as-needed

CONFIGURE_ARGS += --disable-man

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/src/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib/*.so* $(1)/usr/lib/
endef

define Package/systemd-bootchart/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/bootchart $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/systemd/bootchart.service $(1)/usr/lib/systemd/system/
endef

$(eval $(call BuildPackage,systemd-bootchart))
