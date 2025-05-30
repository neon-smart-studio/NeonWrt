#
# Copyright (C) 2023 Michele Primavera
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libopen62541
PKG_VERSION:=1.3.6
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/open62541/open62541.git
PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_MIRROR_HASH:=4e1513dfea70c2be52cc9c096386063e9e20ef757912a197f78e669a7e9dfb3d

PKG_LICENSE:=MPL-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:open62541:open62541
PKG_MAINTAINER:=Michele Primavera <primavera@elmod.it>

CMAKE_INSTALL:=1

define Package/libopen62541/config
        source "$(SOURCE)/Config.in"
endef

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

ifeq ($(CONFIG_HAS_MIPS16),y)
  TARGET_CFLAGS += -fstack-check=generic
endif

define Package/libopen62541
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=An OPC UA library
  URL:=https://open62541.org/
  DEPENDS:=+librt +zlib +libstdcpp +libpthread +libatomic \
		+LIBOPEN62541_MBDEDTLS:libmbedtls +LIBOPEN62541_OPENSSL:libopenssl
endef

define Package/libopen62541/description
 open62541 (http://open62541.org) is an open source and free implementation of
 OPC UA (OPC Unified Architecture) written in the common subset of the C99 and
 C++98 languages. The library is usable with all major compilers and provides
 the necessary tools to implement dedicated OPC UA clients and servers, or to
 integrate OPC UA-based communication into existing applications. open62541
 library is platform independent.
endef

CMAKE_OPTIONS += \
	-DBUILD_SHARED_LIBS=ON \
	-DUA_LOGLEVEL=500 \
	-DUA_ENABLE_AMALGAMATION=OFF \
	-DCMAKE_BUILD_TYPE=MinSizeRel \
	-DUA_ARCHITECTURE=posix \
	-DUA_BUILD_EXAMPLES=OFF \
	-DUA_BUILD_TOOLS=OFF \
	-DUA_BUILD_UNIT_TESTS=OFF \
	-DOPEN62541_VERSION=v$(PKG_VERSION) \
	-DUA_ENABLE_SUBSCRIPTIONS=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_SUBSCRIPTIONS),ON,OFF) \
	-DUA_ENABLE_SUBSCRIPTIONS_EVENTS=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_SUBSCRIPTIONS_EVENTS),ON,OFF) \
	-DUA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS),ON,OFF) \
	-DUA_ENABLE_METHODCALLS=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_METHODCALLS),ON,OFF) \
	-DUA_ENABLE_PARSING=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_PARSING),ON,OFF) \
	-DUA_ENABLE_NODEMANAGEMENT=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_NODEMANAGEMENT),ON,OFF) \
	-DUA_ENABLE_IMMUTABLE_NODES=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_IMMUTABLE_NODES),ON,OFF) \
	-DUA_ENABLE_DISCOVERY=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_DISCOVERY),ON,OFF) \
	-DUA_ENABLE_DISCOVERY_MULTICAST=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_DISCOVERY_MULTICAST),ON,OFF) \
	-DUA_ENABLE_DISCOVERY_SEMAPHORE=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_DISCOVERY_SEMAPHORE),ON,OFF) \
	-DUA_ENABLE_ENCRYPTION_TPM2=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_ENCRYPTION_TPM2),ON,OFF) \
	-DUA_ENABLE_TYPEDESCRIPTION=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_TYPEDESCRIPTION),ON,OFF) \
	-DUA_ENABLE_STATUSCODE_DESCRIPTIONS=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_STATUSCODE_DESCRIPTIONS),ON,OFF) \
	-DUA_ENABLE_PUBSUB=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_PUBSUB),ON,OFF) \
	-DUA_ENABLE_PUBSUB_DELTAFRAMES=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_PUBSUB_DELTAFRAMES),ON,OFF) \
	-DUA_ENABLE_PUBSUB_FILE_CONFIG=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_PUBSUB_FILE_CONFIG),ON,OFF) \
	-DUA_ENABLE_PUBSUB_INFORMATIONMODEL=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_PUBSUB_INFORMATIONMODEL),ON,OFF) \
	-DUA_ENABLE_PUBSUB_MONITORING=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_PUBSUB_MONITORING),ON,OFF) \
	-DUA_ENABLE_PUBSUB_ETH_UADP=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_PUBSUB_ETH_UADP),ON,OFF) \
	-DUA_ENABLE_PUBSUB_ENCRYPTION=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_PUBSUB_ENCRYPTION),ON,OFF) \
	-DUA_ENABLE_HISTORIZING=$(if $(CONFIG_LIBOPEN62541_UA_ENABLE_HISTORIZING),ON,OFF)

ifeq ($(CONFIG_LIBOPEN62541_MBDEDTLS),y)
	CMAKE_OPTIONS += -DUA_ENABLE_ENCRYPTION=MBEDTLS
else ifeq ($(CONFIG_LIBOPEN62541_OPENSSL),y)
	CMAKE_OPTIONS += -DUA_ENABLE_ENCRYPTION=OPENSSL
else
	CMAKE_OPTIONS += -DUA_ENABLE_ENCRYPTION=OFF
endif

ifeq ($(CONFIG_LIBOPEN62541_NAMESPACEZERO_REDUCED),y)
	CMAKE_OPTIONS += -DUA_NAMESPACE_ZERO=REDUCED
else ifeq ($(CONFIG_LIBOPEN62541_NAMESPACEZERO_MINIMAL),y)
	CMAKE_OPTIONS += -DUA_NAMESPACE_ZERO=MINIMAL
else
	CMAKE_OPTIONS += -DUA_NAMESPACE_ZERO=FULL
endif

define Package/libopen62541/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libopen62541.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libopen62541))
