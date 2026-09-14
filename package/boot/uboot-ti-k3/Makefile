# SPDX-License-Identifier: GPL-2.0-only
#
# TI U-Boot / binman integration for SK-AM62 (AM625).
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/prereq.mk

PKG_NAME:=uboot-ti-k3

# TI Processor SDK Linux AM62x 11.01.05
#
# Do not put "-ti-11.01.05" in PKG_VERSION.
# OpenWrt APK treats '-' specially and rejects that version format.
PKG_VERSION:=2025.01
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://git.ti.com/git/ti-u-boot/ti-u-boot.git
PKG_SOURCE_VERSION:=d2a72467939e169210bcd8e57de4b3e6ce2ef149
PKG_MIRROR_HASH:=skip

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=Licenses/README
PKG_FLAGS:=nonshared
PKG_BUILD_PARALLEL:=1

# Host-only build dependency for TI binman ti-board-config.
# The python-yamllint package must provide a /host build.
PKG_BUILD_DEPENDS:=python-yamllint/host \
		   python-jsonschema/host

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/package/lang/python/python3-host.mk


#
# TI K3 build settings
#

TI_K3_R5_CROSS_COMPILE ?= arm-linux-gnueabihf-
TI_K3_SOC_TYPE := $(subst ",,$(CONFIG_TI_K3_DEVICE_TYPE))

R5_OUT:=$(PKG_BUILD_DIR)/out/r5
A53_OUT:=$(PKG_BUILD_DIR)/out/a53

TI_K3_FW_DIR:=$(STAGING_DIR_IMAGE)/ti-k3-firmware
TI_K3_STAGE:=$(STAGING_DIR_IMAGE)/ti-k3

TI_K3_PYTHON:=$(HOST_PYTHON3_BIN)


#
# OpenWrt U-Boot framework
#
# Device:
#
#   CONFIG_TARGET_ti_am62x_DEVICE_ti_sk-am62=y
#
# causes u-boot.mk to default-enable:
#
#   CONFIG_PACKAGE_u-boot-ti_sk-am62=y
#
# BUILD_DEVICES uses the U-Boot variant name, which deliberately matches
# the OpenWrt device name.
#

define U-Boot/Default
  BUILD_TARGET:=ti
  BUILD_SUBTARGET:=am62x
  BUILD_DEVICES:=$(1)
endef


define U-Boot/ti_sk-am62
  NAME:=Texas Instruments SK-AM62 (AM625)

  DEPENDS:= \
	+trusted-firmware-a-ti-k3-am62x \
	+optee-os-ti-k3-am62x \
	+ti-linux-firmware-am62x
endef


UBOOT_TARGETS := \
	ti_sk-am62


#
# Select output according to AM62 device security type
#
# Default SK-AM62 production/dev boards are normally HS-FS.
#

TI_K3_SOC_TYPE := hs-fs
TI_K3_TIBOOT3  := tiboot3-am62x-hs-fs-evm.bin
TI_K3_TISPL    := tispl.bin
TI_K3_UBOOT    := u-boot.img

ifeq ($(CONFIG_TI_K3_DEVICE_GP),y)
  TI_K3_SOC_TYPE := gp
  TI_K3_TIBOOT3  := tiboot3-am62x-gp-evm.bin
  TI_K3_TISPL    := tispl.bin_unsigned
  TI_K3_UBOOT    := u-boot.img_unsigned
endif

ifeq ($(CONFIG_TI_K3_DEVICE_HS_FS),y)
  TI_K3_SOC_TYPE := hs-fs
  TI_K3_TIBOOT3  := tiboot3-am62x-hs-fs-evm.bin
  TI_K3_TISPL    := tispl.bin
  TI_K3_UBOOT    := u-boot.img
endif

ifeq ($(CONFIG_TI_K3_DEVICE_HS_SE),y)
  TI_K3_SOC_TYPE := hs-se
  TI_K3_TIBOOT3  := tiboot3-am62x-hs-evm.bin
  TI_K3_TISPL    := tispl.bin
  TI_K3_UBOOT    := u-boot.img
endif

#
# Host prerequisites
#

$(eval $(call TestHostCommand,ti-k3-r5-gcc, \
	AM62x tiboot3.bin needs a 32-bit ARM cross compiler. \
	Install gcc-arm-linux-gnueabihf or set TI_K3_R5_CROSS_COMPILE=... , \
	$(TI_K3_R5_CROSS_COMPILE)gcc --version))


$(eval $(call TestHostCommand,ti-k3-python3, \
	TI U-Boot requires the OpenWrt host Python interpreter, \
	$(TI_K3_PYTHON) --version))


$(eval $(call TestHostCommand,python3-pyelftools, \
	TI U-Boot binman needs Python3 pyelftools, \
	$(TI_K3_PYTHON) -c 'import elftools'))


#
# AM62 cannot use the normal single-config Build/Configure/U-Boot path.
#
# We need two independent U-Boot configurations:
#
#   R5:
#       am62x_evm_r5_defconfig
#
#   A53:
#       am62x_evm_a53_defconfig
#

define Build/Configure
endef


#
# Host build environment used by both R5 and A53 builds.
#
# PYTHON3 is especially important:
# binman must use OpenWrt's staging Python, otherwise modules installed in
# staging_dir/host (pyelftools, yamllint, etc.) are invisible.
#

UBOOT_HOST_FLAGS = \
	PATH="$(STAGING_DIR_HOSTPKG)/bin:$(STAGING_DIR_HOST)/bin:$(PATH)" \
	PYTHON3="$(TI_K3_PYTHON)" \
	HOSTCC="$(HOSTCC)" \
	HOSTCFLAGS="$(HOST_CFLAGS) $(HOST_CPPFLAGS) -std=gnu11" \
	HOSTLDFLAGS="$(HOST_LDFLAGS)" \
	STAGING_PREFIX="$(STAGING_DIR_HOST)"


define Build/Compile
	@$(TI_K3_PYTHON) -c 'import yamllint; import yamllint.config; import yamllint.linter' || { \
		echo "ERROR: python-yamllint/host did not install yamllint into $(TI_K3_PYTHON)"; \
		exit 1; \
	}

	$(RM) -r $(R5_OUT) $(A53_OUT)
	$(INSTALL_DIR) $(R5_OUT) $(A53_OUT)

	@echo "=============================================="
	@echo " TI K3 AM62x: building R5 SPL"
	@echo " Python: $(TI_K3_PYTHON)"
	@echo "=============================================="

	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		O="$(R5_OUT)" \
		ARCH=arm \
		CROSS_COMPILE="$(TI_K3_R5_CROSS_COMPILE)" \
		$(UBOOT_HOST_FLAGS) \
		am62x_evm_r5_defconfig

	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		O="$(R5_OUT)" \
		ARCH=arm \
		CROSS_COMPILE="$(TI_K3_R5_CROSS_COMPILE)" \
		BINMAN_INDIRS="$(TI_K3_FW_DIR)" \
		$(UBOOT_HOST_FLAGS)

	@echo "=============================================="
	@echo " TI K3 AM62x: building A53 SPL / U-Boot"
	@echo "=============================================="

	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		O="$(A53_OUT)" \
		ARCH=arm \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		$(UBOOT_HOST_FLAGS) \
		am62x_evm_a53_defconfig

	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		O="$(A53_OUT)" \
		ARCH=arm \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		BL31="$(TI_K3_STAGE)/bl31.bin" \
		TEE="$(TI_K3_STAGE)/tee-pager_v2.bin" \
		BINMAN_INDIRS="$(TI_K3_FW_DIR)" \
		$(UBOOT_HOST_FLAGS)
endef


#
# Stage canonical AM62 boot binaries.
#
# These files are consumed by the final disk image builder and are not
# installed into the target rootfs.
#

define Build/InstallDev
	$(INSTALL_DIR) $(TI_K3_STAGE)

	@test -f "$(R5_OUT)/$(TI_K3_TIBOOT3)" || { \
		echo "ERROR: missing $(R5_OUT)/$(TI_K3_TIBOOT3)"; \
		exit 1; \
	}

	@test -f "$(A53_OUT)/$(TI_K3_TISPL)" || { \
		echo "ERROR: missing $(A53_OUT)/$(TI_K3_TISPL)"; \
		exit 1; \
	}

	@test -f "$(A53_OUT)/$(TI_K3_UBOOT)" || { \
		echo "ERROR: missing $(A53_OUT)/$(TI_K3_UBOOT)"; \
		exit 1; \
	}

	$(CP) \
		$(R5_OUT)/$(TI_K3_TIBOOT3) \
		$(TI_K3_STAGE)/tiboot3.bin

	$(CP) \
		$(A53_OUT)/$(TI_K3_TISPL) \
		$(TI_K3_STAGE)/tispl.bin

	$(CP) \
		$(A53_OUT)/$(TI_K3_UBOOT) \
		$(TI_K3_STAGE)/u-boot.img

	#
	# Keep security-type-specific filenames as well.
	#

	@for f in \
		tiboot3-am62x-gp-evm.bin \
		tiboot3-am62x-hs-fs-evm.bin \
		tiboot3-am62x-hs-evm.bin; \
	do \
		[ ! -f "$(R5_OUT)/$$f" ] || \
			cp -fp "$(R5_OUT)/$$f" "$(TI_K3_STAGE)/$$f"; \
	done

	@for f in \
		tispl.bin \
		tispl.bin_unsigned \
		u-boot.img \
		u-boot.img_unsigned; \
	do \
		[ ! -f "$(A53_OUT)/$$f" ] || \
			cp -fp "$(A53_OUT)/$$f" "$(TI_K3_STAGE)/$$f"; \
	done
endef


#
# Nothing goes into the target rootfs.
#

define Package/u-boot/install/default
endef


$(eval $(call BuildPackage/U-Boot))