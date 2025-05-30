# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pyelftools
PKG_VERSION:=0.31
PKG_RELEASE:=1

PYPI_NAME:=pyelftools
PKG_HASH:=c774416b10310156879443b81187d182d8d9ee499660380e645918b50bc88f99

PKG_MAINTAINER:=Austin Lane <vidplace7@gmail.com>
PKG_LICENSE:=Unlicense
PKG_LICENSE_FILES:=LICENSE

HOST_BUILD_DEPENDS:= \
	python3/host \
	python-build/host \
	python-installer/host \
	python-wheel/host

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include ../python3-package.mk
include ../python3-host-build.mk

define Package/python3-pyelftools
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Library for analyzing ELF files and DWARF debugging information
  URL:=https://pypi.org/project/pyelftools
  DEPENDS:=+python3-light +python3-logging
endef

define Package/python3-pyelftools/description
Library for analyzing ELF files and DWARF debugging information
endef

define Host/Compile
	$(call HostPython3/Run, \
		$(HOST_BUILD_DIR), \
		-m pip install --no-cache-dir --prefix="$(STAGING_DIR_HOSTPKG)" pyelftools==$(PKG_VERSION))
endef

define Host/Install
endef

define Host/Package/python3-pyelftools
  SECTION:=host
  CATEGORY:=Languages
endef

$(eval $(call Py3Package,python3-pyelftools))
$(eval $(call BuildPackage,python3-pyelftools))
$(eval $(call BuildPackage,python3-pyelftools-src))
$(eval $(call HostBuild))
