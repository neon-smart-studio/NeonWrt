#!/bin/sh

CONF_FILE="/var/run/hostapd.conf"
WLAN_IFACE="$(iw dev | awk '/Interface/ { print $2; exit }')"

# 若找不到 wlan 介面就退出
if [ -z "$WLAN_IFACE" ]; then
    echo "No wireless interface found."
    exit 1
fi

mkdir -p /var/run
cat <<EOF > "$CONF_FILE"
interface=$WLAN_IFACE
driver=nl80211
ssid=OpenWrt-FreeWiFi
channel=6
hw_mode=g
auth_algs=1
EOF

chown network:network "$CONF_FILE"
chmod 600 "$CONF_FILE"
