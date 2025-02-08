################################################################################
#
# libxkbcommon
#
################################################################################

include $(TOPDIR)/rules.mk

PKG_NAME:=libxkbcommon
PKG_VERSION:=1.7.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://xkbcommon.org/download
PKG_HASH:=skip

PKG_LICENSE:=MIT/X11
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID_VENDOR:=xkbcommon

PKG_BUILD_DEPENDS:=host-bison host-flex

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/meson.mk

define Package/libxkbcommon
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Keyboard handling library
  URL:=https://xkbcommon.org
  DEPENDS:=$(if $(CONFIG_PACKAGE_libxkbcommon_tools),+wayland +wayland-protocols) $(if $(CONFIG_PACKAGE_xorg),+libxcb)
endef

define Package/libxkbcommon/description
  libxkbcommon is a library to handle keyboard descriptions, including
  loading them from disk, parsing them and handling their state.
endef

define Package/libxkbcommon/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libxkbcommon*.so* $(1)/usr/lib/
endef

ifeq ($(CONFIG_PACKAGE_libxkbcommon_tools),y)
define Package/libxkbcommon-tools
  SECTION:=libs
  CATEGORY:=Utilities
  TITLE:=Tools for libxkbcommon
  DEPENDS:=+libxkbcommon +wayland +wayland-protocols
endef

define Package/libxkbcommon-tools/description
  Tools for libxkbcommon, including utilities for inspecting
  and debugging keymaps.
endef

define Package/libxkbcommon-tools/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libxkbcommon-tools))
endif

MESON_ARGS += \
	-Denable-docs=false \
	-Denable-xkbregistry=false \
	-Denable-x11=$(if $(CONFIG_PACKAGE_xorg),true,false) \
	-Denable-tools=$(if $(CONFIG_PACKAGE_libxkbcommon_tools),true,false) \
	-Denable-wayland=$(if $(CONFIG_PACKAGE_wayland),true,false)

$(eval $(call BuildPackage,libxkbcommon))
