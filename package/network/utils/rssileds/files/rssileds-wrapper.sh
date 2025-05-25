#!/bin/sh

RSSILEDS_BIN="/usr/sbin/rssileds"
UCI_GET="/sbin/uci get"
UCI_SHOW="/sbin/uci show"
PID_FILE="/var/run/rssileds-wrapper.pid"

start() {
	[ -x "$RSSILEDS_BIN" ] || exit 1
	config_load system || exit 1
	# 多介面支援
	config_foreach start_rssid rssid
	echo $$ > "$PID_FILE"
}

stop() {
	config_load system
	config_foreach stop_rssid rssid
	config_foreach off_led led
	rm -f "$PID_FILE"
}

start_rssid() {
	local dev refresh threshold leds
	config_get dev "$1" dev
	config_get refresh "$1" refresh
	config_get threshold "$1" threshold
	leds="$( cur_iface=$1; config_foreach get_led led )"
	[ -n "$dev" ] && $RSSILEDS_BIN $dev $refresh $threshold $leds &
}

stop_rssid() {
	local dev
	config_get dev "$1" dev
	pkill -f "$RSSILEDS_BIN $dev"
}

get_led() {
	local sysfs trigger iface minq maxq offset factor
	config_get sysfs $1 sysfs
	config_get trigger $1 trigger
	config_get iface $1 iface
	config_get minq $1 minq
	config_get maxq $1 maxq
	config_get offset $1 offset
	config_get factor $1 factor
	[ "$trigger" = "rssi" ] || return
	[ "$iface" = "$cur_iface" ] || return
	[ -n "$minq" ] && [ -n "$maxq" ] && [ -n "$offset" ] && [ -n "$factor" ] || return
	echo "none" > /sys/class/leds/$sysfs/trigger
	echo "$sysfs $minq $maxq $offset $factor"
}

off_led() {
	local sysfs trigger
	config_get sysfs $1 sysfs
	config_get trigger $1 trigger
	[ "$trigger" = "rssi" ] || return
	echo "0" > /sys/class/leds/$sysfs/brightness
}

. /lib/functions.sh

case "$1" in
	start) start ;;
	stop)  stop ;;
	restart)
		stop
		start
		;;
	*) echo "Usage: $0 {start|stop|restart}" ;;
esac
