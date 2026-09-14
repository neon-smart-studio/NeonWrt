NeonWrt – 基於 Systemd 的 OpenWrt 發行版
-------------------------------------------------------------
* Systemd core + full boot flow support
* Targeted at Rockchip, Orange Pi, x86 SBCs, and IoT edge routers
* Full modular system with custom hostapd/dnsmasq/netifd support

**Open Networks. Brighter Possibilities.**

NeonWrt 是一個以 **OpenWrt 為基礎、以 systemd 為核心重新整合的 Linux 網路作業系統／衍生發行版**。

NeonWrt 並非單純替 OpenWrt 更換 init system，而是希望在保留 OpenWrt 成熟的 **UCI、ubus、netifd、LuCI、fw4 與套件生態**的同時，引入 systemd 的服務管理與現代 Linux userspace 架構。

為了銜接兩種不同的服務模型，NeonWrt 開發自己的 **neon-procd compatibility layer**，讓既有 OpenWrt `USE_PROCD` 服務能夠在 systemd 環境中運作，降低既有 OpenWrt 軟體移植至 NeonWrt 的成本。

### 核心方向

* **systemd 作為 PID 1**
* **neon-procd** — OpenWrt procd → systemd 相容層
* 保留 **UCI / ubus / netifd**
* 支援 **LuCI Web Interface**
* 保留 **fw4 / nftables** 網路防火牆架構
* 延續 OpenWrt 套件與硬體支援生態
* 持續追蹤 OpenWrt upstream 更新
* 建立兼具 OpenWrt 彈性與現代 Linux userspace 的網路系統

NeonWrt 的目標不是取代 OpenWrt，而是在 OpenWrt 成熟的嵌入式網路平台之上，探索另一條系統架構路線。

**Freedom · Flexibility · Beyond**

主要新增與修改

package/base-files

package/boot/arm-trusted-firmware-stm32

package/boot/optee-os-stm32

package/boot/uboot-stm32

package/firmware/seeed-odyssey-ap6236

package/system/neon-procd

package/system/neon-uci

package/system/systemd

target/linux/stm32 (image + patches-6.12)
