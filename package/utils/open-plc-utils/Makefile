#
# Copyright (C) 2013-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=open-plc-utils
PKG_RELEASE:=6

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/qca/open-plc-utils.git
PKG_SOURCE_VERSION:=1ba7d5a042e4e8ff6858b08e113eec5dc4e89cf2
PKG_MIRROR_HASH:=8a3c22fc9acf700b9ac5ba9a8ca4c27577173ad9401c65e78ab0076374bb03e4

PKG_MAINTAINER:=Michael Heimpold <mhei@heimpold.de>

PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/open-plc-utils/Default
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Qualcomm Atheros Power Line Communication utilities
  URL:=https://github.com/qca/open-plc-utils/blob/master/README
endef

define Package/open-plc-utils
  $(call Package/open-plc-utils/Default)
  MENU:=1
endef

define GenPlugin
  define Package/$(addprefix open-plc-utils-,$(1))
    $(call Package/open-plc-utils/Default)
    DEPENDS:=open-plc-utils
    TITLE:=Utility $(1) from the Open PLC utilities
  endef

  define Package/$(addprefix open-plc-utils-,$(1))/description
    Utility $(1) from the Open PLC utilities package.
  endef
endef

OPEN_PLC_UTILS_APPS:=efbu efeu efru efsu edru edsu nics \
		     hpavkey hpavkeys rkey mac2pw mac2pwd \
		     mdioblock mdioblock2 mdiodump mdiogen \
		     hpav mme \
		     chknvm chknvm2 nvmsplit nvmmerge \
		     chkpib chkpib2 setpib getpib modpib pib2xml \
		     pibcomp pibdump pibruin xml2pib psin psout pskey \
		     psgraph psnotch pibrump \
		     int6k int6kboot int6keth int6kf int6khost \
		     int64host int6kid int6klist int6klog int6kmdio \
		     int6kmdio2 int6kmod int6kstat int6ktest int6krate \
		     int6krule int6ktone int6kwait CMEncrypt sada \
		     coqos_add coqos_info coqos_man coqos_mod coqos_rel \
		     mdustats ampboot amphost ampID amplist amprate amprule \
		     ampstat amptest amptool amptone ampwait \
		     plcboot plchost plcID plclist plcrate plcrule \
		     plcstat plctest plctool plctone \
		     plcwait plchostd plcget plcset plcotst plcfwd \
		     plcdevs plclog plcmdio16 plcmdio32 \
		     config2cfg sdram \
		     int6kuart int6kbaud ttysig ptsctl weeder ttysend \
		     ttyrecv ttycat int6kdetect \
		     pev evse

$(foreach a,$(OPEN_PLC_UTILS_APPS),$(eval $(call GenPlugin,$(a))))

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		EXTRA_CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_CFLAGS) $(TARGET_LDFLAGS)" \
		CROSS="$(TARGET_CROSS)" \
		ROOTFS="$(PKG_INSTALL_DIR)" \
		OWNER="$(shell id -u $(shell whoami))" \
		GROUP="$(shell id -g $(shell whoami))" \
		all install
endef

define Package/open-plc-utils/install
	true
endef

define BuildPlugin
  define Package/$(1)/install
	$(INSTALL_DIR) $$(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/local/bin/$(subst open-plc-utils-,,$(1)) \
		$$(1)/usr/bin/
  endef

  $$(eval $$(call BuildPackage,$(1)))
endef

$(eval $(call BuildPackage,open-plc-utils))
$(foreach a,$(OPEN_PLC_UTILS_APPS),$(eval $(call BuildPlugin,open-plc-utils-$(a))))
