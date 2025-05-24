#!/bin/sh

CONF_FILE="/var/run/hostapd.conf"
WLAN_IFACE="$(iw dev | awk '/Interface/ { print $2; exit }')"

# 若找不到 wlan 介面就退出
if [ -z "$WLAN_IFACE" ]; then
    echo "No wireless interface found."
    exit 1
fi

# 取得 MAC 後 3 bytes，格式例如：334455
MAC=$(cat /sys/class/net/"$WLAN_IFACE"/address | tr -d ':')
MAC_SUFFIX="${MAC:6:6}"  # 例如 334455
SSID="NeonWrt-${MAC_SUFFIX}"

mkdir -p /var/run

cat <<EOF > "$CONF_FILE"
interface=$WLAN_IFACE
driver=nl80211
ssid=$SSID
hw_mode=g
channel=6
ieee80211n=1
wmm_enabled=1
auth_algs=1
ignore_broadcast_ssid=0
EOF

chown network:network "$CONF_FILE"
chmod 600 "$CONF_FILE"
