include $(TOPDIR)/rules.mk

PKG_NAME:=btop
PKG_VERSION:=1.4.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL=https://codeload.github.com/aristocratos/btop/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=ac0d2371bf69d5136de7e9470c6fb286cbee2e16b4c7a6d2cd48a14796e86650

PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_FLAGS:=no-lto
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/btop
  SECTION:=admin
  CATEGORY:=Administration
  TITLE:=A monitor of resources
  URL:=https://github.com/aristocratos/btop
  DEPENDS:=+libstdcpp
endef

define Package/btop/description
  Resource monitor that shows usage and stats for processor, memory,
  disks, network and processes.

  C++ version and continuation of bashtop and bpytop.
endef

BTOP_LDFLAGS:=-pthread -DFMT_HEADER_ONLY -D_FILE_OFFSET_BITS=64
MAKE_FLAGS+= \
	PLATFORM=Linux \
	OPTFLAGS="$(TARGET_CXXFLAGS)" \
	LDCXXFLAGS="$(TARGET_LDFLAGS) $(BTOP_LDFLAGS)"

define Package/btop/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/local/bin/btop $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/local/share/btop $(1)/usr/share/

	$(INSTALL_DIR) $(1)/etc/profile.d
	$(CP) $(CURDIR)/files/btop.sh $(1)/etc/profile.d/
endef

$(eval $(call BuildPackage,btop))
