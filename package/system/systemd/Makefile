include $(TOPDIR)/rules.mk

PKG_NAME:=systemd
PKG_VERSION:=256.7
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/systemd/systemd/archive/v$(PKG_VERSION)
PKG_HASH:=skip

PKG_LICENSE:=LGPL-2.1+ GPL-2.0+
PKG_LICENSE_FILES:=LICENSE.GPL2 LICENSE.LGPL2.1

PKG_BUILD_DEPENDS:=intltool/host gperf/host python-jinja2/host python-pyelftools/host python-pefile/host
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/meson.mk

TARGET_CFLAGS += --std=gnu99

SYSTEMD_DISABLE_PKG = $(if $(CONFIG_PACKAGE_systemd-$(1)),,--disable-$(2))

CONFIGURE_ARGS_FAST_BUILD = \
	--enable-gtk-doc \
	--enable-gtk-doc-html \
	--enable-manpages

CONFIGURE_ARGS_EMBEDDED = \
	--enable-backlight \
	--enable-binfmt \
	--enable-firstboot \
	--enable-hibernate \
	--enable-ldconfig \
	--enable-microhttpd \
	--enable-nls \
	--disable-qrencode \
	--enable-quotacheck \
	--enable-vconsole \
	--enable-xkbcommon

CONFIGURE_ARGS += \
	--prefix=/usr \
	--sysconfdir=/etc \
	-Dc_link_args="-Wl,--disable-new-dtags -Wl,-rpath=/usr/lib" \
	--enable-dbus \
	--enable-udevd \
	--enable-getty \
	--enable-shutdown \
	--enable-pam \
	--disable-logind \
	--enable-seccomp \
	$(CONFIGURE_ARGS_FAST_BUILD) \
	$(CONFIGURE_ARGS_EMBEDDED) \
	--enable-apparmor \
	--enable-introspection \
	--enable-gudev \
	--enable-chkconfig \
	--enable-ima \
	--enable-selinux \
	--enable-audit \
	--enable-smack \
	--enable-xz \
	--enable-acl \
	--enable-gcrypt \
	--enable-myhostname \
	--enable-libcryptsetup \
	--enable-polkit \
	--enable-elfutils \
	--enable-gnutls \
	--enable-libcurl \
	--enable-switch-root \
	--enable-sulogin \
	--enable-networkd

MESON_ARGS += \
	-Dlibdir=/usr/lib \
	-Dutmp=true \
	-Dlogind=false \
	-Dpam=enabled \
	-Dseccomp=enabled \
	-Dstatic-libsystemd=false \
	-Dlink-executor-shared=true \
	-Ddefault-dnssec=allow-downgrade \
	-Ddefault-user-shell=/bin/sh \
	-Dinstall-tests=false

define Package/libsystemd
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=systemd client library (libsystemd)
  DEPENDS:=+libc +libcap +libacl +libapparmor +libaudit +libblkid +libmount +libpam +libseccomp +libselinux +openssl-util 
endef

define Package/libsystemd/description
  This package provides the client-side systemd library (libsystemd),
  which applications use to communicate with systemd services.
endef

define Package/libsystemd/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsystemd.so* $(1)/usr/lib/
	ln -sf libsystemd.so.0 $(1)/usr/lib/libsystemd.so

	$(INSTALL_DIR) $(1)/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsystemd.so* $(1)/lib

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/systemd/libsystemd-core-*.so $(1)/usr/lib/
	ln -s libsystemd-core-256.so $(1)/usr/lib/libsystemd-core.so
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/systemd/libsystemd-shared-*.so $(1)/usr/lib/
	ln -s libsystemd-shared-256.so $(1)/usr/lib/libsystemd-shared.so
endef

define Package/systemd
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=Systemd init system
  DEPENDS:=+libsystemd +libapparmor +libc +libcap +libselinux +libkmod +libgcrypt +libseccomp +libarchive +liblz4 \
	+libacl +libpam +libxkbcommon +libbpf +libmicrohttpd +libcurl +libidn +libidn2 +liblzma +libzstd +libxkbcommon \
  	+valgrind +auditd +p11-kit +bzip2 +block-mount +fstools +mount-utils +fdisk +lsblk +blkid +logger +agetty +cryptsetup +gnutls-utils +openssl-util
endef

define Package/systemd/description
  systemd is a modern init system and system manager for Linux.
  (Only Support for GLIBC, Musl not Support!)
endef

define Package/systemd/conffiles
/etc/hostname
/etc/machine-id
/etc/systemd/system.conf
/etc/systemd/user.conf
endef

define Package/systemd-executor
$(call Package/systemd/Default)
	TITLE:=systemd executor component
	DEPENDS+= systemd +libapparmor +libpam +libselinux +libseccomp
endef

define Package/systemd-tmpfiles
$(call Package/systemd/Default)
	TITLE:=systemd tmpfiles component
	DEPENDS+= systemd +libacl
endef


define Package/systemd/install
	$(INSTALL_DIR) $(1)/etc/binfmt.d
	$(INSTALL_DIR) $(1)/etc/credstore
	$(INSTALL_DIR) $(1)/etc/credstore.encrypted
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/kernel/install.d
	$(INSTALL_DIR) $(1)/etc/modules-load.d

	$(INSTALL_DIR) $(1)/etc/ssh/ssh_config.d
	$(CP) $(PKG_INSTALL_DIR)/etc/ssh/ssh_config.d/20-systemd-ssh-proxy.conf $(1)/etc/ssh/ssh_config.d

	$(INSTALL_DIR) $(1)/etc/ssh/sshd_config.d
	$(CP) $(PKG_INSTALL_DIR)/etc/ssh/sshd_config.d/20-systemd-userdb.conf $(1)/etc/ssh/sshd_config.d

	$(INSTALL_DIR) $(1)/etc/sysctl.d

	$(INSTALL_DIR) $(1)/etc/systemd
	$(CP) $(PKG_INSTALL_DIR)/etc/systemd/*.conf $(1)/etc/systemd
	
	echo "LogLevel=debug" > $(1)/etc/systemd/system.conf
	echo "ForwardToConsole=yes" >> $(1)/etc/systemd/journald.conf

	$(INSTALL_DIR) $(1)/etc/systemd/network

	$(INSTALL_DIR) $(1)/etc/systemd/system/multi-user.target.wants
	ln -sf \
		/usr/lib/systemd/system/remote-fs.target \
		$(1)/etc/systemd/system/multi-user.target.wants/.

	$(INSTALL_DIR) $(1)/etc/systemd/user

	$(INSTALL_DIR) $(1)/etc/tmpfiles.d

	$(INSTALL_DIR) $(1)/etc/udev
	$(CP) $(PKG_INSTALL_DIR)/etc/udev/iocost.conf $(1)/etc/udev
	$(CP) $(PKG_INSTALL_DIR)/etc/udev/udev.conf $(1)/etc/udev

	$(INSTALL_DIR) $(1)/usr/lib/udev
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/udev/ata_id $(1)/usr/lib/udev
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/udev/cdrom_id $(1)/usr/lib/udev
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/udev/dmi_memory_id $(1)/usr/lib/udev
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/udev/fido_id $(1)/usr/lib/udev
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/udev/iocost $(1)/usr/lib/udev
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/udev/mtd_probe $(1)/usr/lib/udev
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/udev/scsi_id $(1)/usr/lib/udev
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/udev/v4l_id $(1)/usr/lib/udev

	$(INSTALL_DIR) $(1)/usr/lib/udev/rules.d
	$(INSTALL_DIR) $(1)/usr/lib/udev/hwdb.d

	$(INSTALL_DIR) $(1)/etc/X11/xinit/xinitrc.d
	$(CP) $(PKG_INSTALL_DIR)/etc/X11/xinit/xinitrc.d/50-systemd-user.sh $(1)/etc/X11/xinit/xinitrc.d

	$(INSTALL_DIR) $(1)/etc/xdg/systemd
	$(CP) $(PKG_INSTALL_DIR)/etc/xdg/systemd/user $(1)/etc/xdg/systemd

	$(INSTALL_DIR) $(1)/etc/network

	# 確保 dbus-1 目錄存在
	$(INSTALL_DIR) $(1)/etc/dbus-1/system.d
	# 複製 org.freedesktop.systemd1.conf 到 dbus-1/system.d
	$(CP) \
		$(PKG_BUILD_DIR)/src/core/org.freedesktop.systemd1.conf \
		$(1)/etc/dbus-1/system.d/org.freedesktop.systemd1.conf
		
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/include/systemd
	$(CP) $(PKG_INSTALL_DIR)/usr/include/systemd/*.h $(1)/usr/include/systemd

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnss_myhostname.so* $(1)/usr/lib
	ln -sf libnss_myhostname.so.2 $(1)/usr/lib/libnss_myhostname.so
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnss_mymachines.so* $(1)/usr/lib
	ln -sf libnss_mymachines.so.2 $(1)/usr/lib/libnss_mymachines.so
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnss_resolve.so* $(1)/usr/lib
	ln -sf libnss_resolve.so.2 $(1)/usr/lib/libnss_resolve.so
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnss_systemd.so* $(1)/usr/libacl

	ln -sf libnss_systemd.so.2 $(1)/usr/lib/libnss_systemd.so

	$(INSTALL_DIR) $(1)/usr/lib/binfmt.d
	$(INSTALL_DIR) $(1)/usr/lib/credstore
	$(INSTALL_DIR) $(1)/usr/lib/environment.d
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/environment.d/99-environment.conf $(1)/usr/lib/environment.d
	$(INSTALL_DIR) $(1)/usr/lib/kernel
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/kernel/install.conf $(1)/usr/lib/kernel
	$(INSTALL_DIR) $(1)/usr/lib/kernel/install.d
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/kernel/install.d/*.install $(1)/usr/lib/kernel/install.d
	$(INSTALL_DIR) $(1)/usr/lib/modprobe.d
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/modprobe.d/systemd.conf $(1)/usr/lib/modprobe.d
	$(INSTALL_DIR) $(1)/usr/lib/modules-load.d
	$(INSTALL_DIR) $(1)/usr/lib/pam.d
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pam.d/systemd-run0 $(1)/usr/lib/pam.d
	$(INSTALL_DIR) $(1)/usr/lib/rpm/macros.d
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/rpm/macros.d/macros.systemd $(1)/usr/lib/rpm/macros.d

	$(INSTALL_DIR) $(1)/usr/lib/sysctl.d
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/sysctl.d/*.conf \
		$(1)/usr/lib/sysctl.d

	$(INSTALL_DIR) $(1)/usr/lib/systemd
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/systemd/systemd $(1)/usr/lib/systemd
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/systemd/systemd-* $(1)/usr/lib/systemd
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/systemd/import-pubring.gpg  $(1)/usr/lib/systemd
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/systemd/resolv.conf  $(1)/usr/lib/systemd

	$(INSTALL_DIR) $(1)/usr/lib/systemd/catalog
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/catalog/*.catalog \
		$(1)/usr/lib/systemd/catalog

	$(INSTALL_DIR) $(1)/usr/lib/systemd/network
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/network/* \
		$(1)/usr/lib/systemd/network

	$(INSTALL_DIR) $(1)/usr/lib/systemd/ntp-units.d
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/ntp-units.d/*.list \
		$(1)/usr/lib/systemd/ntp-units.d

	$(INSTALL_DIR) $(1)/usr/lib/systemd/portable/profile/default
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/systemd/portable/profile/default/*.conf $(1)/usr/lib/systemd/portable/profile/default
	$(INSTALL_DIR) $(1)/usr/lib/systemd/portable/profile/nonetwork
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/systemd/portable/profile/nonetwork/*.conf $(1)/usr/lib/systemd/portable/profile/nonetwork
	$(INSTALL_DIR) $(1)/usr/lib/systemd/portable/profile/strict
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/systemd/portable/profile/strict/*.conf $(1)/usr/lib/systemd/portable/profile/strict
	$(INSTALL_DIR) $(1)/usr/lib/systemd/portable/profile/trusted
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/systemd/portable/profile/trusted/*.conf $(1)/usr/lib/systemd/portable/profile/trusted

	$(INSTALL_DIR) $(1)/usr/lib/systemd/repart/definitions/confext.repart.d
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/systemd/repart/definitions/confext.repart.d/*.conf $(1)/usr/lib/systemd/repart/definitions/confext.repart.d
	$(INSTALL_DIR) $(1)/usr/lib/systemd/repart/definitions/portable.repart.d
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/systemd/repart/definitions/portable.repart.d/*.conf $(1)/usr/lib/systemd/repart/definitions/portable.repart.d
	$(INSTALL_DIR) $(1)/usr/lib/systemd/repart/definitions/sysext.repart.d
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/systemd/repart/definitions/sysext.repart.d/*.conf $(1)/usr/lib/systemd/repart/definitions/sysext.repart.d

	$(INSTALL_DIR) $(1)/usr/lib/systemd/ssh_config.d
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/ssh_config.d/*.conf \
		$(1)/usr/lib/systemd/ssh_config.d

	$(INSTALL_DIR) $(1)/usr/lib/systemd/sshd_config.d
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/sshd_config.d/*.conf \
		$(1)/usr/lib/systemd/sshd_config.d

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/systemd/system/*.automount $(1)/usr/lib/systemd/system
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/systemd/system/*.mount $(1)/usr/lib/systemd/system
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/systemd/system/*.path $(1)/usr/lib/systemd/system
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/systemd/system/*.service $(1)/usr/lib/systemd/system
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/systemd/system/*.slice $(1)/usr/lib/systemd/system
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/systemd/system/*.socket $(1)/usr/lib/systemd/system
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/systemd/system/*.target $(1)/usr/lib/systemd/system
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/systemd/system/*.timer $(1)/usr/lib/systemd/system

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/graphical.target.wants
	$(CP) -a $(PKG_INSTALL_DIR)/usr/lib/systemd/system/graphical.target.wants/* $(1)/usr/lib/systemd/system/graphical.target.wants
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/initrd-root-fs.target.wants
	$(CP) -a $(PKG_INSTALL_DIR)/usr/lib/systemd/system/initrd-root-fs.target.wants/* $(1)/usr/lib/systemd/system/initrd-root-fs.target.wants
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/initrd.target.wants
	$(CP) -a $(PKG_INSTALL_DIR)/usr/lib/systemd/system/initrd.target.wants/* $(1)/usr/lib/systemd/system/initrd.target.wants
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/local-fs.target.wants
	$(CP) -a $(PKG_INSTALL_DIR)/usr/lib/systemd/system/local-fs.target.wants/* $(1)/usr/lib/systemd/system/local-fs.target.wants
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/machines.target.wants
	$(CP) -a $(PKG_INSTALL_DIR)/usr/lib/systemd/system/machines.target.wants/* $(1)/usr/lib/systemd/system/machines.target.wants
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/multi-user.target.wants
	$(CP) -a $(PKG_INSTALL_DIR)/usr/lib/systemd/system/multi-user.target.wants/* $(1)/usr/lib/systemd/system/multi-user.target.wants
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/remote-fs.target.wants
	$(CP) -a $(PKG_INSTALL_DIR)/usr/lib/systemd/system/remote-fs.target.wants/* $(1)/usr/lib/systemd/system/remote-fs.target.wants
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/rescue.target.wants
	$(CP) -a $(PKG_INSTALL_DIR)/usr/lib/systemd/system/rescue.target.wants/* $(1)/usr/lib/systemd/system/rescue.target.wants
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/runlevel1.target.wants
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/runlevel2.target.wants
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/runlevel3.target.wants
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/runlevel4.target.wants
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/runlevel5.target.wants
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/sockets.target.wants
	$(CP) -a $(PKG_INSTALL_DIR)/usr/lib/systemd/system/sockets.target.wants/* $(1)/usr/lib/systemd/system/sockets.target.wants
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/sysinit.target.wants
	$(CP) -a $(PKG_INSTALL_DIR)/usr/lib/systemd/system/sysinit.target.wants/* $(1)/usr/lib/systemd/system/sysinit.target.wants
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/timers.target.wants
	$(CP) -a $(PKG_INSTALL_DIR)/usr/lib/systemd/system/timers.target.wants/* $(1)/usr/lib/systemd/system/timers.target.wants
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/user-.slice.d
	$(CP) -a $(PKG_INSTALL_DIR)/usr/lib/systemd/system/user-.slice.d/* $(1)/usr/lib/systemd/system/user-.slice.d
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/user@.service.d
	$(CP) -a $(PKG_INSTALL_DIR)/usr/lib/systemd/system/user@.service.d/* $(1)/usr/lib/systemd/system/user@.service.d
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/user@0.service.d
	$(CP) -a $(PKG_INSTALL_DIR)/usr/lib/systemd/system/user@0.service.d/* $(1)/usr/lib/systemd/system/user@0.service.d

	$(INSTALL_DIR) $(1)/etc/systemd/system/getty.target.wants
	ln -sf /usr/lib/systemd/system/serial-getty@.service \
		$(1)/etc/systemd/system/getty.target.wants/serial-getty@ttyAMA0.service
	ln -sf /usr/lib/systemd/system/getty@.service \
	    $(1)/etc/systemd/system/getty.target.wants/getty@tty1.service
	ln -sf /etc/systemd/system/serial-shell.service $(1)/usr/lib/systemd/system/multi-user.target.wants/serial-shell.service

	ln -sf /etc/systemd/system/load-kmods.service $(1)/usr/lib/systemd/system/sysinit.target.wants/load-kmods.service

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/network-online.target.wants
	ln -sf /usr/lib/systemd/system/network-online.target.wants/systemd-networkd-wait-online.service \
		$(1)/usr/lib/systemd/system/network-online.target.wants/systemd-networkd-wait-online.service

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system-generators
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/system-generators/systemd-* \
		$(1)/usr/lib/systemd/system-generators

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system-preset
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/system-preset/*.preset \
		$(1)/usr/lib/systemd/system-preset

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system-shutdown
	$(INSTALL_DIR) $(1)/usr/lib/systemd/system-sleep

	$(INSTALL_DIR) $(1)/usr/lib/systemd/user
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/user/* \
		$(1)/usr/lib/systemd/user

	$(INSTALL_DIR) $(1)/usr/lib/systemd/user-environment-generators
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/user-environment-generators/30-systemd-environment-d-generator \
		$(1)/usr/lib/systemd/user-environment-generators

	$(INSTALL_DIR) $(1)/usr/lib/systemd/user-generators
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/user-generators/systemd-* \
		$(1)/usr/lib/systemd/user-generators

	$(INSTALL_DIR) $(1)/usr/lib/systemd/user-preset
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/user-preset/*.preset \
		$(1)/usr/lib/systemd/user-preset

	$(INSTALL_DIR) $(1)/usr/lib/sysusers.d
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/sysusers.d/*.conf \
		$(1)/usr/lib/sysusers.d

	$(INSTALL_DIR) $(1)/usr/lib/tmpfiles.d
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/tmpfiles.d/*.conf \
		$(1)/usr/lib/tmpfiles.d

	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/sbin/mount.ddi $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/halt $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/init $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/poweroff $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/reboot $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/resolvconf $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/runlevel $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/shutdown $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/telinit $(1)/usr/sbin

	$(INSTALL_DIR) $(1)/usr/share/dbus-1/services
	$(INSTALL_DIR) $(1)/usr/share/dbus-1/system-services
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/share/dbus-1/system-services/org.freedesktop.systemd1.service \
		$(1)/usr/share/dbus-1/system-services
	ln -sf \
		/usr/share/dbus-1/system-services/org.freedesktop.systemd1.service \
		$(1)/usr/share/dbus-1/services/.

	$(INSTALL_DIR) $(1)/var/log/journal

	$(INSTALL_DIR) $(1)/sbin
	ln -sf \
		/usr/lib/systemd/systemd \
		$(1)/sbin/init

	$(CP) ./files/* $(1)/
	
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/etc/os-release $(1)/etc/os-release
	$(INSTALL_DATA) ./files/etc/machine-id $(1)/etc/machine-id
	$(INSTALL_DATA) ./files/etc/hostname $(1)/etc/hostname

	$(INSTALL_DIR) $(1)/run
	ln -sf /run $(1)/var/run

	$(INSTALL_DIR) $(1)/sys/fs/cgroup

	ln -sf \
		/usr/lib/systemd/system/multi-user.target \
		$(1)/etc/systemd/system/default.target

	# 安裝自定義 init 腳本
	$(INSTALL_BIN) ./files/init $(1)/init

	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/systemd-* \
		$(1)/usr/lib/systemd

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/systemd-tmpfiles $(1)/usr/bin
	
endef

define Package/systemd-executor/install
	$(INSTALL_DIR) $(1)/usr/lib/systemd
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/systemd-executor \
		$(1)/usr/lib/systemd
endef

define Package/systemd-tmpfiles/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/systemd-tmpfiles \
		$(1)/usr/bin
endef

define Package/libudev
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libudev from systemd
  DEPENDS:=+libc +libcap
endef

define Package/libudev/description
  libudev is a device management library extracted from systemd.
endef

define Package/libudev/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libudev.so* $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/include/libudev
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libudev.h $(1)/usr/include/libudev/
endef

define Package/systemd-nspawn
$(call Package/systemd/Default)
	TITLE:=systemd namespace container support
	DEPENDS+= systemd +iptables
endef

define Package/systemd-nspawn/description
$(call Package/systemd/description/Default)
This package contains the systemd namespace container support.
endef

define Package/systemd-nspawn/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/systemd-nspawn \
		$(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/system/systemd-nspawn@.service \
		$(1)/usr/lib/systemd/system
endef


define Package/systemd-tmpfiles
$(call Package/systemd/Default)
	TITLE:=systemd temporary file management support
	DEPENDS+= systemd
endef

define Package/systemd-tmpfiles/description
$(call Package/systemd/description/Default)
This package contains the systemd temporary file management support.
endef

define Package/systemd-tmpfiles/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/systemd-tmpfiles \
		$(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system
	$(INSTALL_DATA) \
		$(wildcard $(PKG_INSTALL_DIR)/usr/lib/systemd/system/systemd-tmpfiles-*) \
		$(1)/usr/lib/systemd/system

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/sysinit.target.wants
	$(CP) \
		$(addprefix $(PKG_INSTALL_DIR)/usr/lib/systemd/system/sysinit.target.wants/,
			systemd-tmpfiles-setup.service \
			systemd-tmpfiles-setup-dev.service ) \
		$(1)/usr/lib/systemd/system/sysinit.target.wants

	$(INSTALL_DIR) $(1)/usr/lib/systemd/system/timers.target.wants
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/systemd/system/timers.target.wants/systemd-tmpfiles-clean.timer \
		$(1)/usr/lib/systemd/system/timers.target.wants

	$(INSTALL_DIR) $(1)/etc/tmpfiles.d	
	$(INSTALL_DIR) $(1)/usr/lib/tmpfiles.d
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/tmpfiles.d/*.conf \
		$(1)/usr/lib/tmpfiles.d
endef


define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libudev.so* $(1)/usr/lib
#	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgudev-1.0.so* $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsystemd.so* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/share/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/share/pkgconfig/*.pc $(1)/usr/share/pkgconfig
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc $(1)/usr/lib/pkgconfig
endef

define Package/systemd-timesyncd/conffiles
/etc/systemd/timesyncd.conf
endef

define Package/systemd-udevd/conffiles
/etc/udev/udev.conf
endef

$(eval $(call BuildPackage,libsystemd))
$(eval $(call BuildPackage,systemd))
$(eval $(call BuildPackage,systemd-nspawn))
$(eval $(call BuildPackage,libudev))
#$(eval $(call BuildPackage,systemd-tmpfiles))