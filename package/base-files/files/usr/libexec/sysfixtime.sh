#!/bin/sh

RTC_DEV=/dev/rtc0
HWCLOCK=/sbin/hwclock

hwclock_load() {
	[ -e "$RTC_DEV" ] && [ -x "$HWCLOCK" ] && "$HWCLOCK" -s -u -f "$RTC_DEV"
}

hwclock_save() {
	[ -e "$RTC_DEV" ] && [ -x "$HWCLOCK" ] && "$HWCLOCK" -w -u -f "$RTC_DEV" && \
		logger -t sysfixtime "saved '$(date)' to $RTC_DEV"
}

find_max_time() {
	local file newest

	for file in $(find /etc -type f); do
		[ -z "$newest" -o "$newest" -ot "$file" ] && newest="$file"
	done
	[ "$newest" ] && date -r "$newest" +%s
}

# Boot logic
boot_time_sync() {
	hwclock_load
	local maxtime="$(find_max_time)"
	local curtime="$(date +%s)"
	if [ "$curtime" -lt "$maxtime" ]; then
		date -s "@$maxtime"
		hwclock_save
	fi
}

case "$1" in
  start)
    boot_time_sync
    ;;
  stop)
    hwclock_save
    ;;
esac
