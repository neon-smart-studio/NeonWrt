# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pathspec
PKG_VERSION:=0.12.1
PKG_RELEASE:=1

PYPI_NAME:=pathspec

PKG_LICENSE:=MPL-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:= \
	python3/host \
	python-build/host \
	python-installer/host \
	python-wheel/host

include $(TOPDIR)/feeds/packages/lang/python/pypi.mk
include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/python/python3-package.mk


define Package/python3-pathspec
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Python
  TITLE:=Utility library for gitignore-style path patterns
  URL:=https://github.com/cpburnz/python-pathspec
  DEPENDS:=+python3
endef


define Package/python3-pathspec/description
 pathspec provides gitignore-style path pattern matching for Python.
endef


$(eval $(call Py3Package,python3-pathspec))
$(eval $(call BuildPackage,python3-pathspec))
$(eval $(call BuildPackage,python3-pathspec-src))