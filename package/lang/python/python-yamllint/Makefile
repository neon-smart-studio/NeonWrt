# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

PKG_NAME:=python-yamllint
PKG_VERSION:=1.37.1
PKG_RELEASE:=1

PYPI_NAME:=yamllint
PKG_HASH:=81f7c0c5559becc8049470d86046b36e96113637bcbe4753ecef06977c00245d

PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=LICENSE

HOST_BUILD_DEPENDS:= \
	python3/host \
	python-build/host \
	python-installer/host \
	python-wheel/host \
	python-pathspec/host \
	python-yaml/host

PKG_BUILD_DEPENDS:= \
	python3/host \
	python-build/host \
	python-installer/host \
	python-wheel/host

include ../pypi.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include ../python3-package.mk
include ../python3-host-build.mk


define Package/python3-yamllint
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=YAML linter
  URL:=https://github.com/adrienverge/yamllint
  DEPENDS:= \
	+python3-light \
	+python3-yaml \
	+python3-pathspec
endef


define Package/python3-yamllint/description
 yamllint checks YAML files for syntax validity and style problems.
endef


$(eval $(call Py3Package,python3-yamllint))
$(eval $(call BuildPackage,python3-yamllint))
$(eval $(call BuildPackage,python3-yamllint-src))

$(eval $(call HostBuild))