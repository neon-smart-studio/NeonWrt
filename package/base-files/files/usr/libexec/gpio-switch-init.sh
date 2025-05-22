#!/bin/sh

. /lib/functions.sh

load_gpio_switch() {
	local name
	local gpio_pin
	local value

	config_get gpio_pin "$1" gpio_pin
	config_get name "$1" name
	config_get value "$1" value 0

	[ -z "$gpio_pin" ] && {
		echo >&2 "Skipping gpio_switch '$name' due to missing gpio_pin"
		return 1
	}

	local gpio_path
	if echo "$gpio_pin" | grep -qE "^[0-9]+$"; then
		gpio_path="/sys/class/gpio/gpio${gpio_pin}"
		[ -d "$gpio_path" ] || {
			echo "$gpio_pin" > /sys/class/gpio/export
			[ -d "$gpio_path" ] || sleep 1
		}

		if [ -e "$gpio_path/direction" ]; then
			[ "$value" = "0" ] && echo "low" || echo "high" > "$gpio_path/direction"
		else
			[ "$value" = "0" ] && echo "0" || echo "1" > "$gpio_path/value"
		fi
	else
		gpio_path="/sys/class/gpio/${gpio_pin}"
		[ -d "$gpio_path" ] && {
			[ "$value" = "0" ] && echo "0" || echo "1" > "$gpio_path/value"
		}
	fi
}

[ -e /sys/class/gpio/ ] && {
	config_load system
	config_foreach load_gpio_switch gpio_switch
}
