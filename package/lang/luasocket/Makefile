#
# Copyright (C) 2009-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luasocket
PKG_VERSION:=3.1.0
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_SOURCE_URL:=https://github.com/lunarmodules/luasocket
PKG_MIRROR_HASH:=1ee81f1f5a63d0d14c8c8571e8940604cbf1443c3b18ee7d3d1bac6791f853fc

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/luasocket/default
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  URL:=http://w3.impa.br/~diego/software/luasocket
endef

define Package/luasocket
  $(Package/luasocket/default)
  TITLE:=LuaSocket
  DEPENDS:=+lua
  VARIANT:=lua-51
  DEFAULT_VARIANT:=1
endef

define Package/luasocket5.3
  $(Package/luasocket/default)
  TITLE:=LuaSocket 5.3
  DEPENDS:=+liblua5.3
  VARIANT:=lua-53
endef

ifeq ($(BUILD_VARIANT),lua-51)
  LUA_VERSION=5.1
endif

ifeq ($(BUILD_VARIANT),lua-53)
  LUA_VERSION=5.3
endif


define Package/luasocket/default/description
  LuaSocket is the most comprehensive networking support
  library for the Lua language. It provides easy access to
  TCP, UDP, DNS, SMTP, FTP, HTTP, MIME and much more.
endef
Package/luasocket/description     = $(Package/luasocket/default/description)
Package/luasocket5.3/description = $(Package/luasocket/default/description)

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		LIBDIR="$(TARGET_LDFLAGS)" \
		CC="$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_CPPFLAGS) $(FPIC)" \
		LD="$(TARGET_CROSS)ld -shared" \
		LUAV=$(LUA_VERSION) LUAINC_linux_base=$(STAGING_DIR)/usr/include \
		all
endef

define Package/luasocket/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/{ltn12,mime,socket}.lua $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/mime-1.0.3.so $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/socket-3.0.0.so $(1)/usr/lib/lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/mime
	ln -sf ../mime-1.0.3.so $(1)/usr/lib/lua/mime/core.so
	$(INSTALL_DIR) $(1)/usr/lib/lua/socket
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/{ftp,http,smtp,tp,url,headers}.lua $(1)/usr/lib/lua/socket
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/unix.so $(1)/usr/lib/lua/socket
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/serial.so $(1)/usr/lib/lua/socket
	ln -sf ../socket-3.0.0.so $(1)/usr/lib/lua/socket/core.so
endef


define Package/luasocket5.3/install
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		DESTDIR="$(1)" \
		LUAV=$(LUA_VERSION) \
		install
endef


$(eval $(call BuildPackage,luasocket))
$(eval $(call BuildPackage,luasocket5.3))
