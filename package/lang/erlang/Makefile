#
# Copyright (C) 2009-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=erlang
PKG_VERSION:=27.0.1
PKG_RELEASE:=1

PKG_SOURCE:=otp_src_$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/erlang/otp/releases/download/OTP-$(PKG_VERSION)
PKG_HASH:=26d894e2f0dda9d13560af08ea589afc01569df6b5486e565beb5accb99c9cf4

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE.txt
PKG_MAINTAINER:=Arnaud Sautaux <arnaud.sautaux@infoteam.ch>
PKG_CPE_ID:=cpe:/a:erlang:erlang

PKG_BUILD_DEPENDS:=erlang/host openssl
PKG_ASLR_PIE:=0

PKG_CONFIG_DEPENDS:= \
	CONFIG_KERNEL_TRANSPARENT_HUGEPAGE

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/otp_src_$(PKG_VERSION)
PKG_BUILD_DIR:=$(BUILD_DIR)/otp_src_$(PKG_VERSION)

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

define Package/erlang/Default
  SUBMENU:=Erlang
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Erlang/OTP programming language
  URL:=https://www.erlang.org/
endef

define Package/erlang/Default/description
 Erlang/OTP is a general-purpose programming language and runtime
 environment. Erlang has built-in support for concurrency, distribution
 and fault tolerance.
endef

define Package/erlang
$(call Package/erlang/Default)
  DEPENDS+= +libncurses +librt +zlib +libstdcpp
  PROVIDES:= erlang-erts=15.0.1 erlang-kernel=10.0.1 erlang-sasl=4.2.2 erlang-stdlib=6.0.1
endef

define Package/erlang/description
$(call Package/erlang/Default/description)
 .
 This package contains the runtime implementation and a minimal set of
 modules (erts, kernel, sasl & stdlib).
endef


define Package/erlang-asn1
$(call Package/erlang/Default)
  TITLE:=Abstract Syntax Notation One (ASN.1) support
  VERSION:=5.3
  DEPENDS+= +erlang +erlang-syntax-tools
endef

define Package/erlang-asn1/description
$(call Package/erlang/Default/description)
 .
 This Erlang/OTP package provides Abstract Syntax Notation One (ASN.1)
 support.
endef


define Package/erlang-compiler
$(call Package/erlang/Default)
  TITLE:=Byte code compiler
  VERSION:=8.5.1
  DEPENDS+= +erlang
endef

define Package/erlang-compiler/description
$(call Package/erlang/Default/description)
 .
 This Erlang/OTP package provides a byte code compiler for Erlang which
 produces highly compact code.
endef


define Package/erlang-crypto
$(call Package/erlang/Default)
  TITLE:=Cryptography support
  VERSION:=5.5
  DEPENDS+= +erlang +libopenssl
endef

define Package/erlang-crypto/description
$(call Package/erlang/Default/description)
 .
 This Erlang/OTP package provides functions for computation of message
 digests, and encryption and decryption functions.
endef


define Package/erlang-inets
$(call Package/erlang/Default)
  TITLE:=Internet clients and servers
  VERSION:=9.2
  DEPENDS+= +erlang
endef

define Package/erlang-inets/description
$(call Package/erlang/Default/description)
 .
 This Erlang/OTP package provides a container for Internet clients and
 servers. Currently a FTP client, a HTTP client and server, and a tftp
 client and server have been incorporated in Inets.
endef


define Package/erlang-mnesia
$(call Package/erlang/Default)
  TITLE:=Distributed database
  VERSION:=4.23.2
  DEPENDS+= +erlang
endef

define Package/erlang-mnesia/description
$(call Package/erlang/Default/description)
 .
 This Erlang/OTP package provides a distributed DataBase Management
 System (DBMS), appropriate for telecommunications applications and
 other Erlang applications which require continuous operation and
 exhibit soft real-time properties.
endef


define Package/erlang-runtime-tools
$(call Package/erlang/Default)
  TITLE:=Low-profile debugging/tracing tools
  VERSION:=2.1
  DEPENDS+= +erlang
endef

define Package/erlang-runtime-tools/description
$(call Package/erlang/Default/description)
 .
 This Erlang/OTP package provides low footprint tracing/debugging tools
 suitable for inclusion in a production system.
endef


define Package/erlang-snmp
$(call Package/erlang/Default)
  TITLE:=Simple Network Management Protocol (SNMP) support
  VERSION:=5.16
  DEPENDS+= +erlang +erlang-asn1
endef

define Package/erlang-snmp/description
$(call Package/erlang/Default/description)
 .
 This Erlang/OTP package provides Simple Network Management Protocol
 (SNMP) support including a MIB compiler and tools for creating SNMP
 agents.
endef


define Package/erlang-public-key
$(call Package/erlang/Default)
  TITLE:=Public Key support
  VERSION:=1.16.1
  DEPENDS+= +erlang +erlang-crypto +erlang-asn1
endef

define Package/erlang-public-key/description
$(call Package/erlang/Default/description)
 .
 This Erlang/OTP package provides functions to handle public-key infrastructure.
endef


define Package/erlang-ssh
$(call Package/erlang/Default)
  TITLE:=Secure Shell (SSH) support
  VERSION:=5.2.1
  DEPENDS+= +erlang +erlang-crypto
endef

define Package/erlang-ssh/description
$(call Package/erlang/Default/description)
 .
 This Erlang/OTP package provides an implementation of the Secure Shell
 protocol, with SSH & SFTP support.
endef


define Package/erlang-ssl
$(call Package/erlang/Default)
  TITLE:=Secure Sockets Layer (SSL) support
  VERSION:=11.2.1
  DEPENDS+= +erlang +erlang-crypto
endef

define Package/erlang-ssl/description
$(call Package/erlang/Default/description)
 .
 This Erlang/OTP package provides support for secure communication over
 sockets.
endef


define Package/erlang-syntax-tools
$(call Package/erlang/Default)
  TITLE:=Abstract Erlang syntax trees handling support
  VERSION:=3.2
  DEPENDS+= +erlang
endef

define Package/erlang-syntax-tools/description
$(call Package/erlang/Default/description)
 .
 This Erlang/OTP package provides support for handling abstract Erlang
 syntax trees.
endef


define Package/erlang-tools
$(call Package/erlang/Default)
  TITLE:=Erlang tools support
  VERSION:=4.0
  DEPENDS+= +erlang
endef

define Package/erlang-tools/description
$(call Package/erlang/Default/description)
 .
 This Erlang/OTP package provides support for misc tools.
endef


define Package/erlang-reltool
$(call Package/erlang/Default)
  TITLE:=Erlang reltool support
  VERSION:=1.0.1
  DEPENDS+= +erlang
endef

define Package/erlang-reltool/description
$(call Package/erlang/Default/description)
 .
 This Erlang/OTP package provides support for release management.
endef


define Package/erlang-erl-interface
$(call Package/erlang/Default)
  TITLE:=Erlang erl_interface support
  VERSION:=5.5.2
  DEPENDS+= +erlang
endef

define Package/erlang-erl-interface/description
$(call Package/erlang/Default/description)
 .
 This Erlang/OTP package provides support for erlang interoperability with other languages.
endef

define Package/erlang-os_mon
$(call Package/erlang/Default)
  TITLE:=Erlang OS Monitoring Application
  VERSION:=2.10
  DEPENDS+= +erlang
endef

define Package/erlang-os_mon/description
$(call Package/erlang/Default/description)
 .
 This Erlang/OTP package provides the following services:
  cpu_sup CPU load and utilization supervision
  disksup Disk supervision
  memsup Memory supervision
endef

define Package/erlang-xmerl
$(call Package/erlang/Default)
  TITLE:=Erlang XML export
  VERSION:=2.0
  DEPENDS+= +erlang
endef

define Package/erlang-xmerl/description
$(call Package/erlang/Default/description)
 .
 This Erlang/OTP package provides functions for exporting XML data to an external format
endef

# Host

HOST_CONFIGURE_ARGS += \
	--without-wx \
	--with-ssl="$(STAGING_DIR_HOST)" \
	--without-javac

# Target

CONFIGURE_ARGS += \
	--without-javac \
	--enable-dynamic-ssl-lib

## Override incorrect THP detection by configure script
ERTS_THP=no
ifeq ($(CONFIG_KERNEL_TRANSPARENT_HUGEPAGE),y)
	ERTS_THP=yes
endif

CONFIGURE_VARS += \
	SHLIB_LD="$(TARGET_CC)" \
	TARGET_ARCH="$(TARGET_ARCH)" \
	erts_cv_linux_thp=$(ERTS_THP) \
	ac_cv_func_mmap_fixed_mapped=yes \
	ac_cv_path_WX_CONFIG_PATH=no \
	erl_xcomp_getaddrinfo=no \
	erl_xcomp_sysroot="$(STAGING_DIR)"

EXTRA_CFLAGS+=-D_GNU_SOURCE
EXTRA_LDFLAGS+=-lz

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		noboot
	$(MAKE) -C $(PKG_BUILD_DIR) \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		install
endef

define Package/erlang/install
	$(INSTALL_DIR) $(1)/usr/bin
	for f in epmd erl erlc escript run_erl; do \
		$(CP) $(PKG_INSTALL_DIR)/usr/bin/$$$$f $(1)/usr/bin/ ; \
	done
	$(INSTALL_DIR) $(1)/usr/lib/erlang/bin
	for f in epmd erl erlc escript run_erl start start.boot start.script start_clean.boot start_erl start_sasl.boot no_dot_erlang.boot to_erl; do \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/erlang/bin/$$$$f $(1)/usr/lib/erlang/bin/ ; \
	done
	$(INSTALL_DIR) $(1)/usr/lib/erlang/lib
	for m in erts kernel sasl stdlib; do \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/erlang/lib/$$$$m-* $(1)/usr/lib/erlang/lib/ ; \
		rm -rf $(1)/usr/lib/erlang/lib/$$$$m-*/examples ; \
		rm -rf $(1)/usr/lib/erlang/lib/$$$$m-*/src ; \
	done
	$(INSTALL_DIR) $(1)/usr/lib/erlang
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/erlang/erts-* $(1)/usr/lib/erlang/
	rm -rf $(1)/usr/lib/erlang/erts-*/{doc,include,lib,man,src}
	rm -rf $(1)/usr/lib/erlang/erts-*/bin/*.src
	$(INSTALL_DIR) $(1)/usr/lib/erlang/releases
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/erlang/releases/* $(1)/usr/lib/erlang/releases/
	$(SED) 's,%ERL_ROOT%,/usr/lib/erlang,g' \
		$(1)/usr/lib/erlang/releases/RELEASES.src
	mv -f $(1)/usr/lib/erlang/releases/RELEASES.src \
		$(1)/usr/lib/erlang/releases/RELEASES
	for f in bin/erl bin/start erts-*/bin/erl erts-*/bin/start; do \
		$(SED) 's,^\(ROOTDIR\)=.*,\1=/usr/lib/erlang,g' \
			$(1)/usr/lib/erlang/$$$$f ; \
	done
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib/erl_interface/obj/*/*.a $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/lib/erl_interface/include/*.h $(1)/usr/include/
endef

define BuildModule

  define Package/erlang-$(1)/install
	$(INSTALL_DIR) $$(1)/usr/lib/erlang/lib
	for m in $(2); do \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/erlang/lib/$$$$$$$$m-* $$(1)/usr/lib/erlang/lib/ ; \
		rm -rf $$(1)/usr/lib/erlang/lib/$$$$$$$$m-*/{examples,priv/obj,src} ; \
	done
  endef

  $$(eval $$(call BuildPackage,erlang-$(1)))

endef


$(eval $(call HostBuild))
$(eval $(call BuildPackage,erlang))
$(eval $(call BuildModule,asn1,asn1))
$(eval $(call BuildModule,compiler,compiler))
$(eval $(call BuildModule,crypto,crypto))
$(eval $(call BuildModule,inets,inets))
$(eval $(call BuildModule,mnesia,mnesia))
$(eval $(call BuildModule,runtime-tools,runtime_tools))
$(eval $(call BuildModule,snmp,snmp))
$(eval $(call BuildModule,public-key,public_key))
$(eval $(call BuildModule,ssh,ssh))
$(eval $(call BuildModule,ssl,ssl))
$(eval $(call BuildModule,syntax-tools,syntax_tools))
$(eval $(call BuildModule,tools,tools))
$(eval $(call BuildModule,reltool,reltool))
$(eval $(call BuildModule,erl-interface,erl_interface))
$(eval $(call BuildModule,os_mon,os_mon))
$(eval $(call BuildModule,xmerl,xmerl))
