#!/bin/sh

. /lib/functions.sh

CONF_FILE="/var/run/hostapd.conf"
mkdir -p /var/run

# 抓第一張 WLAN interface
WLAN_IFACE="$(iw dev | awk '/Interface/ { print $2; exit }')"

# 若找不到 wlan interface 就退出
if [ -z "$WLAN_IFACE" ]; then
    echo "No wireless interface found."
    exit 1
fi

# 取得 MAC 後 3 bytes 當 SSID 後綴
MAC=$(cat /sys/class/net/"$WLAN_IFACE"/address | tr -d ':')
MAC_SUFFIX="${MAC:6:6}"

# 先設定預設值
DEFAULT_SSID="NeonWrt-${MAC_SUFFIX}"
DEFAULT_CHANNEL="6"
DEFAULT_HWMODE="11g"
DEFAULT_HWMODE_CONV="g"
DEFAULT_KEY=""
DEFAULT_ENC="none"
DEFAULT_HIDDEN="0"

# 載入 UCI 設定，抓第一個 wifi-iface
config_load wireless
config_get device  wifi_iface device
config_get ssid    wifi_iface ssid
config_get key     wifi_iface key
config_get enc     wifi_iface encryption
config_get mode    wifi_iface mode
config_get hidden  wifi_iface hidden

# 嘗試從 wifi-device 取得 channel/hwmode/ifname
config_get channel "$device" channel
config_get hwmode  "$device" hwmode
config_get ifname  "$device" ifname

# 使用 UCI 設定或預設
SSID="${ssid:-$DEFAULT_SSID}"
CHANNEL="${channel:-$DEFAULT_CHANNEL}"
HWMODE="${hwmode:-$DEFAULT_HWMODE}"
MODE="${mode:-ap}"
KEY="${key:-$DEFAULT_KEY}"
ENC="${enc:-$DEFAULT_ENC}"
HIDDEN="${hidden:-$DEFAULT_HIDDEN}"
WLAN_IFACE="${ifname:-$WLAN_IFACE}"

# 自動轉換為 hostapd 的 hw_mode 格式（g/a/b）
case "$HWMODE" in
  *g) hw_mode="g" ;;
  *a) hw_mode="a" ;;
  *b) hw_mode="b" ;;
  *)  hw_mode="$DEFAULT_HWMODE_CONV" ;;
esac

# 輸出 hostapd.conf
cat <<EOF > "$CONF_FILE"
interface=$WLAN_IFACE
driver=nl80211
ssid=$SSID
hw_mode=$hw_mode
channel=$CHANNEL
ieee80211n=1
wmm_enabled=1
auth_algs=1
ignore_broadcast_ssid=$HIDDEN
EOF

# WPA2 加密設定（僅支援 psk2/psk）
if [ "$ENC" = "psk2" ] || [ "$ENC" = "psk" ]; then
	cat <<EOF >> "$CONF_FILE"
wpa=2
wpa_passphrase=$KEY
wpa_key_mgmt=WPA-PSK
rsn_pairwise=CCMP
EOF
fi

chmod 600 "$CONF_FILE"
