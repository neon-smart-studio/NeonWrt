include $(TOPDIR)/rules.mk

PKG_NAME:=iftop
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://code.blinkace.com/pdw/iftop.git
PKG_SOURCE_DATE:=2018-10-03
PKG_SOURCE_VERSION:=77901c8c53e01359d83b8090aacfe62214658183
PKG_MIRROR_HASH:=8ea3a23f9ac8416d259962c51adbda52f98ed811bd882abcd753eb77cf9cfe9f
PKG_MAINTAINER:=Paul Spooren <mail@aparcar.org>
PKG_LICENSE:=GPL-2.0

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/iftop
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libpcap +libncurses +libpthread
  TITLE:=display bandwith usage on an interface
  URL:=http://www.ex-parrot.com/~pdw/iftop/
endef

define Package/iftop/description
	iftop does for network usage what top(1) does for CPU usage. It
	listens to network traffic on a named interface and displays a
	table of current bandwidth usage by pairs of hosts. Handy for
	answering the question 'why is our ADSL link so slow?'.
endef

define Package/iftop/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/iftop $(1)/usr/bin/
endef

$(eval $(call BuildPackage,iftop))

