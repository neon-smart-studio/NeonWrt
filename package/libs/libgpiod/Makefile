#
# Copyright (C) 2018-2019 Michael Heimpold <mhei@heimpold.de>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libgpiod
PKG_VERSION:=2.1.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/software/libs/libgpiod/
PKG_HASH:=2be4c0b03e995d236c0e476e14aeb475d7b431dd1439609b6d65c540f91eaf58

PKG_MAINTAINER:=Michael Heimpold <mhei@heimpold.de>
PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PYTHON3_PKG_BUILD:=0

include $(INCLUDE_DIR)/package.mk
include ../../lang/python/python3-package.mk

ifneq ($(CONFIG_PACKAGE_libgpiodcxx),)
CONFIGURE_ARGS += --enable-bindings-cxx
else
CONFIGURE_ARGS += --disable-bindings-cxx
endif

ifneq ($(CONFIG_PACKAGE_gpiod-tools),)
CONFIGURE_ARGS += --enable-tools
endif

PYTHON3_PKG_WHEEL_NAME:=gpiod
PYTHON3_PKG_WHEEL_VERSION:=2.0.1

PYTHON3_PKG_SETUP_DIR:=bindings/python
PYTHON3_PKG_SETUP_VARS += LINK_SYSTEM_LIBGPIOD=1

TARGET_CPPFLAGS += -I$(PKG_BUILD_DIR)/include
TARGET_LDFLAGS += -L$(PKG_BUILD_DIR)/lib/.libs

define Package/libgpiod
  SECTION:=libs
  CATEGORY:=Libraries
  URL:=https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git
  TITLE:=Library for interacting with Linux's GPIO character device
  KCONFIG:= \
    CONFIG_GPIO_CDEV=y
  DEPENDS:=@GPIO_SUPPORT
endef

define Package/libgpiod/description
  C library for interacting with the linux GPIO character device
  (gpiod stands for GPIO device).
endef

define Package/gpiod-tools
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Tools for interacting with GPIO pins
  DEPENDS:=+libgpiod
endef

define Package/gpiod-tools/description
  Tools for interacting with the linux GPIO character device
  (gpiod stands for GPIO device).
endef

define Package/libgpiodcxx
  SECTION:=libs
  CATEGORY:=Libraries
  URL:=https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git
  TITLE:=C++ binding for libgpiod
  DEPENDS:=+libstdcpp +libgpiod
endef

define Package/libgpiodcxx/description
  This package contains the C++ binding for libgpiod.
endef

define Package/python3-gpiod
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Python bindings for libgpiod
  URL:=https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git
  DEPENDS:=+python3-light +libgpiod
endef

define Package/python3-gpiod/description
  This package contains the Python bindings for libgpiod.
endef

define Build/Configure
	$(call Build/Configure/Default)
    ifneq ($(CONFIG_PACKAGE_python3-gpiod),)
	$(call Py3Build/Configure)
    endif
endef

define Build/Compile
	$(call Build/Compile/Default)
    ifneq ($(CONFIG_PACKAGE_python3-gpiod),)
	$(call Py3Build/Compile)
    endif
endef

define Build/Install
	$(call Build/Install/Default)
    ifneq ($(CONFIG_PACKAGE_python3-gpiod),)
	$(call Py3Build/Install/Default)
    endif
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/gpiod.h $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgpiod.{so*,a} $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libgpiod.pc $(1)/usr/lib/pkgconfig/

    ifneq ($(CONFIG_PACKAGE_libgpiodcxx),)
	$(CP) $(PKG_INSTALL_DIR)/usr/include/gpiodcxx $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/gpiod.hpp $(1)/usr/include/

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgpiodcxx.{so*,a} $(1)/usr/lib/

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libgpiodcxx.pc $(1)/usr/lib/pkgconfig/
    endif

    ifneq ($(CONFIG_PACKAGE_python3-gpiod),)
	$(INSTALL_DIR) $(1)$(PYTHON3_PKG_DIR)
	$(CP) $(PKG_INSTALL_DIR)$(PYTHON3_PKG_DIR)/* $(1)$(PYTHON3_PKG_DIR)
    endif
endef

define Package/libgpiod/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgpiod.so.* $(1)/usr/lib/
endef

define Package/libgpiodcxx/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgpiodcxx.so.* $(1)/usr/lib/
endef

define Package/gpiod-tools/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

define Py3Package/python3-gpiod/install
	# this empty define prevent installing tools from /usr/bin
endef

$(eval $(call BuildPackage,libgpiod))
$(eval $(call BuildPackage,libgpiodcxx))
$(eval $(call BuildPackage,gpiod-tools))
$(eval $(call Py3Package,python3-gpiod))
$(eval $(call BuildPackage,python3-gpiod))
$(eval $(call BuildPackage,python3-gpiod-src))
