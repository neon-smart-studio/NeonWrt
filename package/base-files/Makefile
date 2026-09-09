#
# Copyright (C) 2007-2021 OpenWrt.org
# Copyright (C) 2010 Vertical Communications
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/version.mk
include $(INCLUDE_DIR)/feeds.mk

PKG_NAME:=base-files
PKG_FLAGS:=nonshared
PKG_RELEASE:=$(COMMITCOUNT)

PKG_FILE_DEPENDS:=$(PLATFORM_DIR)/ $(GENERIC_PLATFORM_DIR)/base-files/
PKG_BUILD_DEPENDS:=usign/host ucert/host
PKG_LICENSE:=GPL-2.0

# Extend depends from version.mk
PKG_CONFIG_DEPENDS += \
	CONFIG_SIGNED_PACKAGES CONFIG_TARGET_INIT_PATH CONFIG_TARGET_PREINIT_DISABLE_FAILSAFE \
	CONFIG_NAND_SUPPORT \
	CONFIG_LEGACY_SDCARD_SUPPORT \
	CONFIG_EMMC_SUPPORT \
	CONFIG_CLEAN_IPKG \
	CONFIG_PER_FEED_REPO \
	CONFIG_TARGET_ROOTFS_PERSIST_VAR \
	$(foreach feed,$(FEEDS_AVAILABLE),CONFIG_FEED_$(feed))

include $(INCLUDE_DIR)/package.mk

ifneq ($(DUMP),1)
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_$(shell echo $(CONFIG_TARGET_INIT_PATH) | $(MKHASH) md5)
  TARGET:=-$(BOARD)
endif

define Package/base-files
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:= \
	+systemd +uci +neon-procd +neon-uci \
	+netifd +libc +jsonfilter +SIGNED_PACKAGES:usign +SIGNED_PACKAGES:openwrt-keyring \
	+NAND_SUPPORT:ubi-utils +fstools +fwtool \
	+SELINUX:busybox-selinux +!SELINUX:busybox
  TITLE:=Base filesystem for NeonWrt
  URL:=https://openwrt.org/
  VERSION:=$(PKG_RELEASE)~$(lastword $(subst -, ,$(REVISION)))
endef

define Package/base-files/conffiles
/etc/config/
/etc/config/network
/etc/config/system
/etc/dropbear/
/etc/ethers
/etc/group
/etc/hosts
/etc/inittab
/etc/iproute2/rt_protos
/etc/iproute2/rt_tables
/etc/passwd
/etc/profile
/etc/profile.d
/etc/protocols
/etc/rc.local
/etc/services
/etc/shadow
/etc/shells
/etc/shinit
/etc/sysctl.conf
/etc/sysupgrade.conf
$(call $(TARGET)/conffiles)
endef

define Package/base-files/description
 This package contains a base filesystem and system scripts for NeonWrt.
endef

define ImageConfigOptions
	mkdir -p $(1)/lib/preinit
	echo 'pi_suppress_stderr="$(CONFIG_TARGET_PREINIT_SUPPRESS_STDERR)"' >$(1)/lib/preinit/00_preinit.conf
	echo 'fs_failsafe_wait_timeout=$(if $(CONFIG_TARGET_PREINIT_TIMEOUT),$(CONFIG_TARGET_PREINIT_TIMEOUT),4)' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_init_path="$(TARGET_INIT_PATH)"' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_init_env=$(if $(CONFIG_TARGET_INIT_ENV),$(CONFIG_TARGET_INIT_ENV),"")' >>$(1)/lib/preinit/00_preinit.conf

	# systemd package owns /sbin/init.
	# Keep preinit generic and hand off through /sbin/init rather than
	# hard-coding /usr/lib/systemd/systemd in base-files.
	echo 'pi_init_cmd="/sbin/init"' >>$(1)/lib/preinit/00_preinit.conf

	echo 'pi_init_suppress_stderr="$(CONFIG_TARGET_INIT_SUPPRESS_STDERR)"' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_ifname=$(if $(CONFIG_TARGET_PREINIT_IFNAME),$(CONFIG_TARGET_PREINIT_IFNAME),"")' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_ip=$(if $(CONFIG_TARGET_PREINIT_IP),$(CONFIG_TARGET_PREINIT_IP),"192.168.1.1")' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_netmask=$(if $(CONFIG_TARGET_PREINIT_NETMASK),$(CONFIG_TARGET_PREINIT_NETMASK),"255.255.255.0")' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_broadcast=$(if $(CONFIG_TARGET_PREINIT_BROADCAST),$(CONFIG_TARGET_PREINIT_BROADCAST),"192.168.1.255")' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_preinit_net_messages="$(CONFIG_TARGET_PREINIT_SHOW_NETMSG)"' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_preinit_no_failsafe_netmsg="$(CONFIG_TARGET_PREINIT_SUPPRESS_FAILSAFE_NETMSG)"' >>$(1)/lib/preinit/00_preinit.conf
	echo 'pi_preinit_no_failsafe="$(CONFIG_TARGET_PREINIT_DISABLE_FAILSAFE)"' >>$(1)/lib/preinit/00_preinit.conf
ifeq ($(CONFIG_TARGET_DEFAULT_LAN_IP_FROM_PREINIT),y)
	mkdir -p $(1)/etc/board.d
	echo '. /lib/functions/uci-defaults.sh' >$(1)/etc/board.d/99-lan-ip
	echo 'logger -t 99-lan-ip "setting custom default LAN IP"' >>$(1)/etc/board.d/99-lan-ip
	echo 'board_config_update' >>$(1)/etc/board.d/99-lan-ip
	echo 'json_select network' >>$(1)/etc/board.d/99-lan-ip
	echo 'json_select lan' >>$(1)/etc/board.d/99-lan-ip
	echo 'json_add_string ipaddr $(if $(CONFIG_TARGET_PREINIT_IP),$(CONFIG_TARGET_PREINIT_IP),"192.168.1.1")' >>$(1)/etc/board.d/99-lan-ip
	echo 'json_add_string netmask $(if $(CONFIG_TARGET_PREINIT_NETMASK),$(CONFIG_TARGET_PREINIT_NETMASK),"255.255.255.0")' >>$(1)/etc/board.d/99-lan-ip
	echo 'json_select ..' >>$(1)/etc/board.d/99-lan-ip
	echo 'json_select ..' >>$(1)/etc/board.d/99-lan-ip
	echo 'board_config_flush' >>$(1)/etc/board.d/99-lan-ip
endif
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Quilt
endef

define Build/Compile/Default

endef
Build/Compile = $(Build/Compile/Default)

ifneq ($(CONFIG_USE_APK),)
ifndef CONFIG_BUILDBOT
  define Package/base-files/install-key
	mkdir -p $(1)/etc/apk/keys
	$(CP) $(BUILD_KEY_APK_PUB) $(1)/etc/apk/keys/
  endef
endif
else
ifdef CONFIG_SIGNED_PACKAGES
  define Build/Configure
	[ -s $(BUILD_KEY) -a -s $(BUILD_KEY).pub ] || \
		$(STAGING_DIR_HOST)/bin/usign -G -s $(BUILD_KEY) -p $(BUILD_KEY).pub -c "Local build key"

	[ -s $(BUILD_KEY).ucert ] || \
		$(STAGING_DIR_HOST)/bin/ucert -I -c $(BUILD_KEY).ucert -p $(BUILD_KEY).pub -s $(BUILD_KEY)

  endef

ifndef CONFIG_BUILDBOT
  define Package/base-files/install-key
	mkdir -p $(1)/etc/opkg/keys
	$(CP) $(BUILD_KEY).pub $(1)/etc/opkg/keys/`$(STAGING_DIR_HOST)/bin/usign -F -p $(BUILD_KEY).pub`
  endef
endif
endif
endif

ifeq ($(CONFIG_NAND_SUPPORT),)
  define Package/base-files/nand-support
	rm -f $(1)/lib/upgrade/nand.sh
  endef
endif

ifeq ($(CONFIG_EMMC_SUPPORT),)
  define Package/base-files/emmc-support
	rm -f $(1)/lib/upgrade/emmc.sh
  endef
endif

ifeq ($(CONFIG_LEGACY_SDCARD_SUPPORT),)
  define Package/base-files/legacy-sdcard-support
	rm -f $(1)/lib/upgrade/legacy-sdcard.sh
  endef
endif


define Package/base-files/install
	$(CP) ./files/* $(1)/
	$(Package/base-files/install-key)
	$(Package/base-files/nand-support)
	$(Package/base-files/legacy-sdcard-support)
	$(Package/base-files/emmc-support)

	if [ -d $(GENERIC_PLATFORM_DIR)/base-files/. ]; then \
		$(CP) $(GENERIC_PLATFORM_DIR)/base-files/* $(1)/; \
	fi

	if [ -d $(PLATFORM_DIR)/base-files/. ]; then \
		$(CP) $(PLATFORM_DIR)/base-files/* $(1)/; \
	fi

	$(if $(filter-out $(PLATFORM_DIR),$(PLATFORM_SUBDIR)), \
		if [ -d $(PLATFORM_SUBDIR)/base-files/. ]; then \
			$(CP) $(PLATFORM_SUBDIR)/base-files/* $(1)/; \
		fi; \
	)

	$(VERSION_SED_SCRIPT) \
		$(1)/etc/banner \
		$(1)/etc/device_info \
		$(1)/etc/openwrt_release \
		$(1)/etc/openwrt_version

	$(SED) "s#%PATH%#$(TARGET_INIT_PATH)#g" \
		$(1)/sbin/hotplug-call \
		$(1)/etc/preinit \
		$(1)/etc/profile

	mkdir -p \
		$(1)/CONTROL \
		$(1)/dev \
		$(1)/etc/config \
		$(1)/etc/crontabs \
		$(1)/etc/rc.d \
		$(1)/overlay \
		$(1)/lib/firmware \
		$(1)/mnt \
		$(1)/proc \
		$(1)/tmp \
		$(1)/usr/lib \
		$(1)/usr/bin \
		$(1)/sys \
		$(1)/www

	mkdir -p -m 750 \
		$(1)/root

	$(LN) /proc/mounts $(1)/etc/mtab
	$(if $(LIB_SUFFIX),-$(LN) lib $(1)/lib$(LIB_SUFFIX))
	$(if $(LIB_SUFFIX),-$(LN) lib $(1)/usr/lib$(LIB_SUFFIX))

	#
	# NeonWrt/systemd filesystem layout
	#
	# /run must exist in the on-disk rootfs before PID 1 starts.
	# systemd mounts a tmpfs over it during very early initialization.
	# Keep /run/lock in the image as well so /var/lock can safely point to it.
	#
	mkdir -p \
		$(1)/run \
		$(1)/run/lock

	# OpenWrt runtime state is volatile by default.  Keeping /var as a real
	# directory on the rootfs breaks when the root filesystem is mounted ro
	# (dnsmasq, dropbear, random-seed, auditd, etc. all need writable state).
	# Restore the standard OpenWrt /var -> /tmp layout unless the target
	# explicitly requests a persistent /var.
ifneq ($(CONFIG_TARGET_ROOTFS_PERSIST_VAR),y)
	rm -rf $(1)/var
	$(LN) tmp $(1)/var
else
	rm -rf $(1)/var
	mkdir -p \
		$(1)/var \
		$(1)/var/lib \
		$(1)/var/log \
		$(1)/var/cache \
		$(1)/var/tmp
	chmod 1777 $(1)/var/tmp
	$(LN) /run $(1)/var/run
	$(LN) /run/lock $(1)/var/lock
endif

	# Runtime directories expected by OpenWrt init scripts.  systemd-tmpfiles
	# creates these after /run and /tmp are mounted, before neon-openwrt-rc.
	mkdir -p $(1)/usr/lib/tmpfiles.d
	printf '%s\n' \
		'd /run/lock 0755 root root -' \
		'd /tmp/.uci 0700 root root -' \
		'd /tmp/resolv.conf.d 0755 root root -' \
		'f /tmp/resolv.conf.d/resolv.conf.auto 0644 root root -' \
		'L+ /tmp/resolv.conf - - - - /tmp/resolv.conf.d/resolv.conf.auto' \
		'd /var/etc 0755 root root -' \
		'd /var/etc/ssl 0755 root root -' \
		'd /var/lib 0755 root root -' \
		'd /var/lib/systemd 0755 root root -' \
		'd /var/cache 0755 root root -' \
		'd /var/log 0755 root root -' \
		'd /var/log/audit 0700 root root -' \
		'd /var/state 0755 root root -' \
		'd /var/tmp 1777 root root -' \
		'L+ /var/run - - - - /run' \
		'L+ /var/lock - - - - /run/lock' \
		> $(1)/usr/lib/tmpfiles.d/neonwrt.conf

	# Minimal local identities required by systemd tmpfiles/network services.
	# Add only when missing so platform overlays may provide their own IDs.
	grep -q '^disk:' $(1)/etc/group || echo 'disk:x:6:' >> $(1)/etc/group
	grep -q '^utmp:' $(1)/etc/group || echo 'utmp:x:22:' >> $(1)/etc/group
	grep -q '^wheel:' $(1)/etc/group || echo 'wheel:x:10:root' >> $(1)/etc/group
	grep -q '^kvm:' $(1)/etc/group || echo 'kvm:x:36:' >> $(1)/etc/group
	grep -q '^systemd-journal:' $(1)/etc/group || echo 'systemd-journal:x:190:' >> $(1)/etc/group
	grep -q '^systemd-network:' $(1)/etc/group || echo 'systemd-network:x:192:' >> $(1)/etc/group
	grep -q '^systemd-resolve:' $(1)/etc/group || echo 'systemd-resolve:x:193:' >> $(1)/etc/group
	grep -q '^systemd-timesync:' $(1)/etc/group || echo 'systemd-timesync:x:194:' >> $(1)/etc/group

	grep -q '^systemd-network:' $(1)/etc/passwd || \
		echo 'systemd-network:x:192:192:systemd Network Management:/:/bin/false' >> $(1)/etc/passwd
	grep -q '^systemd-resolve:' $(1)/etc/passwd || \
		echo 'systemd-resolve:x:193:193:systemd Resolver:/:/bin/false' >> $(1)/etc/passwd
	grep -q '^systemd-timesync:' $(1)/etc/passwd || \
		echo 'systemd-timesync:x:194:194:systemd Time Synchronization:/:/bin/false' >> $(1)/etc/passwd

	grep -q '^systemd-network:' $(1)/etc/shadow || \
		echo 'systemd-network:!:0:0:99999:7:::' >> $(1)/etc/shadow
	grep -q '^systemd-resolve:' $(1)/etc/shadow || \
		echo 'systemd-resolve:!:0:0:99999:7:::' >> $(1)/etc/shadow
	grep -q '^systemd-timesync:' $(1)/etc/shadow || \
		echo 'systemd-timesync:!:0:0:99999:7:::' >> $(1)/etc/shadow

	# systemd-remount-fs has no OpenWrt-style /etc/fstab entry to tell it to
	# remount an ext4 root rw.  Add a tiny early helper/unit: writable
	# filesystems are remounted rw, while squashfs is intentionally left ro.
	mkdir -p \
		$(1)/usr/libexec \
		$(1)/usr/lib/systemd/system \
		$(1)/usr/lib/systemd/system/sysinit.target.wants
	printf '%s\n' \
		'#!/bin/sh' \
		'fstype=' \
		'while read dev mp type opts rest; do' \
		'    [ "$$mp" = "/" ] && { fstype="$$type"; break; }' \
		'done < /proc/mounts' \
		'[ "$$fstype" = "squashfs" ] && exit 0' \
		'/bin/mount -o remount,rw /' \
		> $(1)/usr/libexec/neon-root-rw
	chmod 0755 $(1)/usr/libexec/neon-root-rw

	printf '%s\n' \
		'[Unit]' \
		'Description=Prepare writable NeonWrt root filesystem' \
		'DefaultDependencies=no' \
		'After=systemd-remount-fs.service' \
		'Before=systemd-random-seed.service systemd-tmpfiles-setup.service systemd-tmpfiles-setup-dev.service' \
		'' \
		'[Service]' \
		'Type=oneshot' \
		'ExecStart=/usr/libexec/neon-root-rw' \
		'RemainAfterExit=yes' \
		> $(1)/usr/lib/systemd/system/neon-root-rw.service
	ln -sf /usr/lib/systemd/system/neon-root-rw.service \
		$(1)/usr/lib/systemd/system/sysinit.target.wants/neon-root-rw.service


	$(LN) /tmp/resolv.conf /tmp/TZ /tmp/localtime $(1)/etc/

	chmod 0600 $(1)/etc/shadow
	chmod 1777 $(1)/tmp

	$(call ImageConfigOptions,$(1))
	$(call Package/base-files/install-target,$(1))

	# Force the UCI bootstrap wiring into the final package image after all
	# generic, platform and install-target overlays have been applied.
	mkdir -p \
		$(1)/usr/lib/systemd/system/sysinit.target.wants \
		$(1)/usr/lib/systemd/system/neon-uci.service.d \
		$(1)/usr/lib/systemd/system/neon-openwrt-rc.service.d
	chmod 0755 $(1)/usr/libexec/neon-config-generate
	ln -sfn ../neon-config-generate.service \
		$(1)/usr/lib/systemd/system/sysinit.target.wants/neon-config-generate.service

	# Before=/After= only orders units already in the transaction.  These
	# drop-ins also pull the bootstrap in whenever either consumer starts.
	printf '%s\n' \
		'[Unit]' \
		'Requires=neon-config-generate.service' \
		'After=neon-config-generate.service' \
		> $(1)/usr/lib/systemd/system/neon-uci.service.d/10-neon-config-generate.conf
	printf '%s\n' \
		'[Unit]' \
		'Requires=neon-config-generate.service' \
		'After=neon-config-generate.service' \
		> $(1)/usr/lib/systemd/system/neon-openwrt-rc.service.d/10-neon-config-generate.conf

	for conffile in $(1)/etc/config/*; do \
		if [ -f "$$$$conffile" ]; then \
			grep "$$$${conffile##$(1)}" $(1)/CONTROL/conffiles || \
				echo "$$$${conffile##$(1)}" >> $(1)/CONTROL/conffiles; \
		fi \
	done

	$(if $(CONFIG_INCLUDE_CONFIG), \
		echo -e "# Build configuration for board $(BOARD)/$(SUBTARGET)/$(PROFILE)\n" >$(1)/etc/build.config; \
		cat $(BIN_DIR)/config.buildinfo >>$(1)/etc/build.config; \
		cat $(BIN_DIR)/feeds.buildinfo >>$(1)/etc/build.feeds; \
		cat $(BIN_DIR)/version.buildinfo >>$(1)/etc/build.version)

	$(if $(CONFIG_TARGET_PREINIT_DISABLE_FAILSAFE), \
		rm -f $(1)/etc/banner.failsafe,)

ifneq ($(CONFIG_USE_APK),)
	mkdir -p $(1)/etc/apk/repositories.d
	$(call FeedSourcesAppendAPK,$(1)/etc/apk/repositories.d/distfeeds.list)
	$(VERSION_SED_SCRIPT) $(1)/etc/apk/repositories.d/distfeeds.list

	rm -f $(1)/etc/uci-defaults/13_fix-group-user
	rm -f $(1)/sbin/pkg_check
	echo $(PKG_RELEASE)~$(lastword $(subst -, ,$(REVISION))) >$(STAGING_DIR)/base-files.version
else
	$(if $(CONFIG_CLEAN_IPKG),, \
		mkdir -p $(1)/etc/opkg; \
		$(call FeedSourcesAppendOPKG,$(1)/etc/opkg/distfeeds.conf); \
		$(VERSION_SED_SCRIPT) $(1)/etc/opkg/distfeeds.conf)

	$(if $(CONFIG_IPK_FILES_CHECKSUMS),, \
		rm -f $(1)/sbin/pkg_check)
endif

	#
	# IMPORTANT:
	# /sbin/init is owned exclusively by the systemd package.
	#
	# base-files source/platform overlays may still contain an old OpenWrt
	# /sbin/init. Remove it from this package image so apk/opkg does not
	# report a file ownership collision with systemd.
	#
	rm -f $(1)/sbin/init
	
	#
	# NeonWrt boot-critical scripts
	#
	chmod 0755 $(1)/bin/board_detect
	chmod 0755 $(1)/bin/config_generate
	chmod 0755 $(1)/bin/ipcalc.sh
	chmod 0755 $(1)/etc/init.d/boot
	chmod 0755 $(1)/usr/libexec/neon-config-generate
	chmod 0755 $(1)/usr/libexec/neon-wifi-config
endef

ifneq ($(DUMP),1)
  -include $(PLATFORM_DIR)/base-files.mk
  -include $(PLATFORM_SUBDIR)/base-files.mk
endif

$(eval $(call BuildPackage,base-files))
