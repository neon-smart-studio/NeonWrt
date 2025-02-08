################################################################################
#
# python-pyelftools
#
################################################################################

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pyelftools
PKG_VERSION:=0.31
PKG_RELEASE:=1

PYPI_NAME:=pyelftools
PKG_SOURCE:=$(PYPI_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://files.pythonhosted.org/packages/88/56/0f2d69ed9a0060da009f672ddec8a71c041d098a66f6b1d80264bf6bbdc0
PKG_HASH:=skip

PKG_BUILD_DEPENDS:=python3/host
PKG_LICENSE:=Public domain
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Your Name <your.email@example.com>
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

HOST_BUILD_DEPENDS:=python-markupsafe/host

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include ../python3-package.mk
include ../python3-host-build.mk

define Package/python3-pyelftools
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Python library for parsing and analyzing ELF files
  URL:=https://github.com/eliben/pyelftools
  DEPENDS:=+python3
endef

define Package/python3-pyelftools/description
  pyelftools is a pure-Python library for parsing and analyzing ELF (Executable
  and Linkable Format) files and DWARF debugging information embedded in ELF files.
endef

define Build/Compile
	$(call Build/Compile/Py3Mod,,install --prefix=/usr --root=$(PKG_INSTALL_DIR))
endef

define Package/python3-pyelftools/install
	$(INSTALL_DIR) $(1)$(PYTHON3_PKG_DIR)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/python*/site-packages/* $(1)$(PYTHON3_PKG_DIR)/
endef

define Host/Compile
	$(call HostPython3/Run, \
		$(HOST_BUILD_DIR), \
		-m pip install --no-cache-dir --prefix="$(STAGING_DIR_HOSTPKG)" pyelftools==$(PKG_VERSION))
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOSTPKG)/lib/python$(PYTHON3_VERSION)/site-packages
endef

define Host/Package/python3-pyelftools
  SECTION:=host
  CATEGORY:=Languages
  TITLE:=Python library for parsing and analyzing ELF files
  URL:=https://github.com/eliben/pyelftools
endef

$(eval $(call BuildPackage,python3-pyelftools))
$(eval $(call HostBuild))
