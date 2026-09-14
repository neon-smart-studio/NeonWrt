# SPDX-License-Identifier: GPL-2.0-only
#
# OP-TEE OS for TI K3 / AM62x.
# Source set aligned with TI Processor SDK Linux AM62x 11.01.05 / Linux 6.12.

include $(TOPDIR)/rules.mk

PKG_NAME:=optee-os-ti
PKG_VERSION:=4.6.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/OP-TEE/optee_os.git
PKG_SOURCE_VERSION:=71785645fa6ce42db40dbf5a54e0eaedc4f61591
PKG_MIRROR_HASH:=skip

PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_FLAGS:=nonshared
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

OPTEE_OUT:=$(PKG_BUILD_DIR)/out/arm-plat-k3

define Package/optee-os-ti-k3-am62x
  SECTION:=boot
  CATEGORY:=Boot Loaders
  TITLE:=OP-TEE OS for TI AM62x
  URL:=https://www.op-tee.org/
  DEPENDS:=@TARGET_ti_am62x
endef

define Package/optee-os-ti-k3-am62x/description
Builds 64-bit OP-TEE for TI AM62x and stages tee-pager_v2.bin and tee-raw.bin
for the TI U-Boot A53 SPL/binman build.
endef

define Build/Configure
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		O="$(OPTEE_OUT)" \
		PLATFORM=k3-am62x \
		CFG_ARM64_core=y \
		CFG_WITH_SOFTWARE_PRNG=y \
		CFG_USER_TA_TARGETS=ta_arm64 \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		CROSS_COMPILE64="$(TARGET_CROSS)" \
		CROSS_COMPILE_core="$(TARGET_CROSS)" \
		CROSS_COMPILE_ta_arm64="$(TARGET_CROSS)" \
		PYTHON3="$(STAGING_DIR_HOST)/bin/python3"
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)/ti-k3
	$(CP) $(OPTEE_OUT)/core/tee-pager_v2.bin \
		$(STAGING_DIR_IMAGE)/ti-k3/tee-pager_v2.bin
	$(CP) $(OPTEE_OUT)/core/tee-raw.bin \
		$(STAGING_DIR_IMAGE)/ti-k3/tee-raw.bin
endef

define Package/optee-os-ti-k3-am62x/install
	:
endef

$(eval $(call BuildPackage,optee-os-ti-k3-am62x))
