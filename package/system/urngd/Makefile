include $(TOPDIR)/rules.mk

PKG_NAME:=urngd
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/urngd.git
PKG_SOURCE_DATE:=2023-11-01
PKG_SOURCE_VERSION:=44365eb1e1165f2a44cb31f404b04cf85031718e
PKG_MIRROR_HASH:=b153530c4e66d1b0fe7d98b0ec24df580964f24a756e754323e3fa297eed1229

PKG_LICENSE:=GPL-2.0 BSD-3-Clause
PKG_LICENSE_FILES:=

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/urngd
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=OpenWrt non-physical true random number generator based on timing jitter
  DEPENDS:=+libubox
endef

define Package/urngd/description
  urngd is OpenWrt's micro non-physical true random number generator based on
  timing jitter.

  Using the Jitter RNG core, the rngd provides an entropy source that feeds into
  the Linux /dev/random device if its entropy runs low. It updates the
  /dev/random entropy estimator such that the newly provided entropy unblocks
  /dev/random.

  The seeding of /dev/random also ensures that /dev/urandom benefits from
  entropy. Especially during boot time, when the entropy of Linux is low, the
  Jitter RNGd provides a source of sufficient entropy.
endef

define Package/urngd/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/sbin/$(PKG_NAME)

	$(INSTALL_DIR) $(1)/etc/systemd/system
	$(INSTALL_DATA) ./files/urngd.service $(1)/etc/systemd/system/urngd.service

	$(INSTALL_DIR) $(1)/etc/systemd/system/sysinit.target.wants
	ln -sf /etc/systemd/system/urngd.service $(1)/etc/systemd/system/sysinit.target.wants/urngd.service
endef

$(eval $(call BuildPackage,urngd))
