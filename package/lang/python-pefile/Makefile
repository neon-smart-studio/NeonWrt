################################################################################
#
# python-pefile
#
################################################################################

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pefile
PKG_VERSION:=2024.8.26
PKG_RELEASE:=1

PYPI_NAME:=pefile
PKG_SOURCE:=$(PYPI_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://files.pythonhosted.org/packages/03/4f/2750f7f6f025a1507cd3b7218691671eecfd0bbebebe8b39aa0fe1d360b8
PKG_HASH:=skip

PKG_BUILD_DEPENDS:=python3/host
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Your Name <your.email@example.com>
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

HOST_BUILD_DEPENDS:=python-markupsafe/host

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include ../python3-package.mk
include ../python3-host-build.mk

define Package/python3-pefile
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Python module to parse and work with PE (Portable Executable) files
  URL:=https://github.com/erocarrera/pefile
  DEPENDS:=+python3 +python3-setuptools
endef

define Package/python3-pefile/description
  pefile is a Python module to read and work with Portable Executable (PE) files. 
  Most of the information contained in the PE headers is accessible, as well as all
  sections' details and their data.
endef

define Build/Compile
	$(call Build/Compile/Py3Mod,,install --prefix=/usr --root=$(PKG_INSTALL_DIR))
endef

define Package/python3-pefile/install
	$(INSTALL_DIR) $(1)$(PYTHON3_PKG_DIR)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/python*/site-packages/* $(1)$(PYTHON3_PKG_DIR)/
endef

define Host/Compile
	$(call HostPython3/Run, \
		$(HOST_BUILD_DIR), \
		-m pip install --no-cache-dir --prefix="$(STAGING_DIR_HOSTPKG)" pefile==$(PKG_VERSION))
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOSTPKG)/lib/python$(PYTHON3_VERSION)/site-packages
endef

define Host/Package/python3-pefile
  SECTION:=host
  CATEGORY:=Languages
  TITLE:=Python module to parse and work with PE (Portable Executable) files
  URL:=https://github.com/erocarrera/pefile
endef

$(eval $(call BuildPackage,python3-pefile))
$(eval $(call HostBuild))
