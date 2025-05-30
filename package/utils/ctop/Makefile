include $(TOPDIR)/rules.mk

PKG_NAME:=ctop
PKG_VERSION:=0.7.7
PKG_RELEASE:=1

PKG_SOURCE:=v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/bcicen/ctop/archive
PKG_HASH:=0db439f2030af73ad5345884b08a33a762c3b41b30604223dd0ebddde72d2741

PKG_MAINTAINER:=Javier Marcet <javier@marcet.info>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_BUILD_FLAGS:=no-mips16

GO_PKG:=github.com/bcicen/ctop

include $(INCLUDE_DIR)/package.mk
include ../../lang/golang/golang-package.mk

define Package/ctop
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Top-like interface for container metrics
  URL:=https://ctop.sh/
  DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/ctop/description
  Top-like interface for container metrics
endef

MAKE_PATH:=$(GO_PKG_WORK_DIR_NAME)/build/src/$(GO_PKG)
MAKE_VARS += \
	GOPATH=$(GO_PKG_BUILD_DIR) \
	GOCACHE=$(GO_BUILD_CACHE_DIR) \
	CC="$(TARGET_CC)" \
	CXX="$(TARGET_CXX)" \
	$(GO_PKG_VARS)
MAKE_FLAGS += \
	COMMIT=$(PKG_SOURCE_VERSION)

define Package/ctop/install
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/ctop $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,ctop))
