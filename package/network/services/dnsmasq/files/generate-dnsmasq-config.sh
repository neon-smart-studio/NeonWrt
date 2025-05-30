#!/bin/sh
# generate-dnsmasq-config.sh
# 根據 /etc/config/dhcp 與 OpenWrt uci 設定，產生 /var/etc/dnsmasq.default.conf

set -e

CFG_NAME="default"
CONFIGFILE="/var/etc/dnsmasq.${CFG_NAME}.conf"
HOSTFILE="/tmp/hosts/dhcp.${CFG_NAME}"
HOSTFILE_DIR="$(dirname "$HOSTFILE")"
BASECONFIGFILE="/var/etc/dnsmasq.conf"

mkdir -p /var/run/dnsmasq
mkdir -p /var/etc
mkdir -p "$HOSTFILE_DIR"

# 引入必要函式庫
. /lib/functions.sh

# 清除暫存
CONFIGFILE_TMP="$CONFIGFILE.$$"
HOSTFILE_TMP="$HOSTFILE.$$"
echo "# auto-generated by generate-dnsmasq-config.sh" > "$CONFIGFILE_TMP"
echo "# auto-generated by generate-dnsmasq-config.sh" > "$HOSTFILE_TMP"

# 建立 xappend 功能
xappend() {
	echo "$1" >> "$CONFIGFILE_TMP"
}

# 匯入 DHCP 設定並轉換
config_load dhcp

# 處理 dnsmasq 主設定
append_interface() {
	local ifname
	network_get_device ifname "$1" || ifname="$1"
	xappend "interface=$ifname"
}
append_server() {
	xappend "server=$1"
}
append_address() {
	xappend "address=$1"
}
config_list_foreach "$CFG_NAME" "interface" append_interface
config_list_foreach "$CFG_NAME" "server" append_server
config_list_foreach "$CFG_NAME" "address" append_address

# 常用選項
xappend "addn-hosts=$HOSTFILE"
xappend "dhcp-authoritative"
xappend "dhcp-broadcast=tag:needs-broadcast"
xappend "dhcp-leasefile=/tmp/dhcp.leases"

# 加入附加設定
[ -f /etc/dnsmasq.${CFG_NAME}.conf ] && xappend "conf-file=/etc/dnsmasq.${CFG_NAME}.conf"
[ -f /usr/share/dnsmasq/trust-anchors.conf ] && xappend "conf-file=/usr/share/dnsmasq/trust-anchors.conf"

# 加入每個 DHCP 區段的設定（包含 wlan0）
append_dhcp() {
	local cfg="$1"
	local iface start limit leasetime ignore ipaddr base start_ip end_ip

	config_get iface "$cfg" interface
	config_get start "$cfg" start
	config_get limit "$cfg" limit
	config_get leasetime "$cfg" leasetime
	config_get_bool ignore "$cfg" ignore 0

	[ "$ignore" = "1" ] && return
	[ -z "$iface" ] && return

	dev="$iface"  # 在 systemd-networkd 下 iface 通常就是 device name

	# 嘗試取得 IP（不依賴 ubus）
	ipaddr=$(ip -4 addr show dev "$dev" | awk '/inet / {print $2}' | cut -d/ -f1)

	[ -z "$ipaddr" ] && return

	base="${ipaddr%.*}"

	[ -z "$start" ] && start=100
	[ -z "$limit" ] && limit=50
	[ -z "$leasetime" ] && leasetime="12h"

	end=$((start + limit - 1))
	[ "$end" -gt 254 ] && end=254

	start_ip="${base}.${start}"
	end_ip="${base}.${end}"

	xappend "interface=$dev"
	xappend "dhcp-range=${start_ip},${end_ip},${leasetime}"
}
config_foreach append_dhcp dhcp

# 完成設定檔產生
mv "$CONFIGFILE_TMP" "$CONFIGFILE"
mv "$HOSTFILE_TMP" "$HOSTFILE"
