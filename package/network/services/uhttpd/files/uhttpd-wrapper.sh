#!/bin/sh

. /lib/functions.sh
. /etc/diag.sh

UHTTPD_BIN=/usr/sbin/uhttpd
PX5G_BIN=/usr/sbin/px5g
OPENSSL_BIN=/usr/bin/openssl
CERT=/etc/uhttpd.crt
KEY=/etc/uhttpd.key

# 自動產生憑證
if [ ! -s "$CERT" ] || [ ! -s "$KEY" ]; then
	[ -x "$PX5G_BIN" ] && $PX5G_BIN selfsigned -der -days 397 -newkey rsa:2048 \
		-keyout "$KEY" -out "$CERT" \
		-subj "/CN=OpenWrt" \
		-addext extendedKeyUsage=serverAuth -addext subjectAltName=DNS:OpenWrt
fi

# 執行 uhttpd，支援基本參數，你可加入 uci 解析進一步擴展
exec $UHTTPD_BIN -f -h /www -r OpenWrt -x /cgi-bin -p 0.0.0.0:80 -s 0.0.0.0:443 -C "$CERT" -K "$KEY"
