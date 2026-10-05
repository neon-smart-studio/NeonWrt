include $(TOPDIR)/rules.mk

PKG_NAME:=seeed-odyssey-ap6236
PKG_RELEASE:=3

include $(INCLUDE_DIR)/package.mk

define Package/seeed-odyssey-ap6236
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=Seeed Odyssey STM32MP157C AP6236 firmware
endef

define Package/seeed-odyssey-ap6236/description
 Firmware package for the AP6236 Wi-Fi/Bluetooth module used on the
 Seeed Studio Odyssey STM32MP157C.

 Provides:
  - Broadcom/Cypress BCM43430B0 Wi-Fi firmware
  - Odyssey AP6236 board-specific NVRAM
  - BCM43430B0 Bluetooth HCD firmware
endef

define Build/Compile
endef

define Package/seeed-odyssey-ap6236/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm

	# AP6236 Wi-Fi firmware
	$(INSTALL_DATA) \
		./files/fw_bcm43436b0.bin \
		$(1)/lib/firmware/brcm/brcmfmac43430b0-sdio.bin

	# Odyssey/AP6236 board-specific NVRAM
	$(INSTALL_DATA) \
		./files/nvram_ap6236.txt \
		$(1)/lib/firmware/brcm/brcmfmac43430b0-sdio.seeed,stm32mp157c-odyssey.txt

	# AP6236 Bluetooth firmware
	$(INSTALL_DATA) \
		./files/BCM43430B0.hcd \
		$(1)/lib/firmware/brcm/BCM43430B0.hcd
endef

$(eval $(call BuildPackage,seeed-odyssey-ap6236))