#!/bin/sh

CERT="/etc/uhttpd.crt"
KEY="/etc/uhttpd.key"
UHTTPD_BIN="/usr/sbin/uhttpd"

# 自動產生 TLS 憑證（僅在首次執行）
if [ ! -s "$CERT" ] || [ ! -s "$KEY" ]; then
	echo "[uhttpd] TLS cert/key not found, generating self-signed..."
	openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
		-keyout "$KEY" -out "$CERT" \
		-subj "/C=ZZ/ST=Somewhere/L=Unknown/O=OpenWrt/CN=OpenWrt"
fi

# 確保 /www 存在
[ -d /www ] || mkdir -p /www

# 啟動 uhttpd
exec $UHTTPD_BIN -f -h /www -r OpenWrt -p 0.0.0.0:80 -s 0.0.0.0:443 -C "$CERT" -K "$KEY"
