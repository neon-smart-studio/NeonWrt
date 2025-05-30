#
# Copyright (C) 2017 Steven Hessing
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libyaml-cpp
PKG_VERSION:=0.7.0
PKG_RELEASE:=1

PKG_SOURCE:=yaml-cpp-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/jbeder/yaml-cpp/tar.gz/yaml-cpp-$(PKG_VERSION)?
PKG_HASH:=43e6a9fcb146ad871515f0d0873947e5d497a1c9c60c58cb102a97b47208b7c3
PKG_BUILD_DIR:=$(BUILD_DIR)/yaml-cpp-yaml-cpp-$(PKG_VERSION)

PKG_MAINTAINER:= Steven Hessing <steven.hessing@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:yaml-cpp_project:yaml-cpp

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libyaml-cpp
  SECTION:=development
  CATEGORY:=Libraries
  TITLE:=libyaml-cpp
  URL:=https://github.com/jbeder/yaml-cpp
  DEPENDS:=+libstdcpp
  ABI_VERSION:=0.7
endef

define Package/libyaml-cpp/description
  yaml-cpp is a YAML parser and emitter in C++ matching the YAML 1.2 spec.
endef

CMAKE_OPTIONS += \
	-DYAML_BUILD_SHARED_LIBS=ON \
	-DYAML_CPP_BUILD_TESTS=OFF \
	-DYAML_CPP_BUILD_TOOLS=OFF

define Package/libyaml-cpp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libyaml-cpp.so.$(ABI_VERSION) $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libyaml-cpp))
