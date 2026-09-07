#!/bin/sh
# NeonWrt procd API compatibility layer backed by systemd.
# Intentionally keeps the OpenWrt shell API surface so USE_PROCD=1 scripts work.

. "$IPKG_INSTROOT/usr/share/libubox/jshn.sh"

PROCD_RELOAD_DELAY=${PROCD_RELOAD_DELAY:-1000}
_PROCD_SERVICE=
_PROCD_INSTANCE_SEQ=0
_PROCD_TRIGGER_OPEN=0

_procd_call() {
	local old_cb
	json_set_namespace procd old_cb
	"$@"
	json_set_namespace "$old_cb"
}

_procd_wrapper() {
	while [ -n "$1" ]; do
		eval "$1() { _procd_call _$1 \"\$@\"; }"
		shift
	done
}

_procd_open_service() {
	local name="$1" script="$2"
	_PROCD_SERVICE="$name"
	_PROCD_INSTANCE_SEQ=0
	json_init
	json_add_string name "$name"
	[ -n "$script" ] && json_add_string script "$script"
	json_add_object instances
}

_procd_close_service() {
	json_close_object

	# Keep init scripts that declare triggers working. v0.1 records trigger data
	# but does not yet install an ubus->systemd trigger bridge.
	if type service_triggers >/dev/null 2>&1; then
		_procd_open_trigger
		service_triggers
		_procd_close_trigger
	fi

	if ubus -S list service >/dev/null 2>&1; then
		ubus call service set "$(json_dump)"
	else
		json_dump | /usr/sbin/neon-procd set
	fi
	local rc=$?
	json_cleanup
	return $rc
}

_procd_add_array_data() {
	while [ "$#" -gt 0 ]; do
		json_add_string "" "$1"
		shift
	done
}

_procd_add_array() {
	json_add_array "$1"; shift
	_procd_add_array_data "$@"
	json_close_array
}

_procd_add_table_data() {
	while [ -n "$1" ]; do
		local var="${1%%=*}" val="${1#*=}"
		[ "$1" = "$val" ] && val=
		json_add_string "$var" "$val"
		shift
	done
}

_procd_add_table() {
	json_add_object "$1"; shift
	_procd_add_table_data "$@"
	json_close_object
}

_procd_open_instance() {
	local name="$1"
	_PROCD_INSTANCE_SEQ=$((_PROCD_INSTANCE_SEQ + 1))
	name="${name:-instance$_PROCD_INSTANCE_SEQ}"
	json_add_object "$name"
}

_procd_close_instance() {
	local vals threshold timeout retry
	_json_no_warning=1
	if json_select respawn; then
		json_get_values vals
		if [ -z "$vals" ]; then
			threshold="$(uci_get system.@service[0].respawn_threshold 2>/dev/null)"
			timeout="$(uci_get system.@service[0].respawn_timeout 2>/dev/null)"
			retry="$(uci_get system.@service[0].respawn_retry 2>/dev/null)"
			_procd_add_array_data "${threshold:-3600}" "${timeout:-5}" "${retry:-5}"
		fi
		json_select ..
	fi
	json_close_object
}

_procd_set_param() {
	local type="$1"; shift
	case "$type" in
		env|data|limits) _procd_add_table "$type" "$@" ;;
		command|netdev|file|respawn|watch|watchdog) _procd_add_array "$type" "$@" ;;
		error) json_add_array "$type"; json_add_string "" "$*"; json_close_array ;;
		nice) json_add_int "$type" "$1" ;;
		pidfile|user|group|seccomp|capabilities) json_add_string "$type" "$1" ;;
		stdout|stderr|no_new_privs) json_add_boolean "$type" "$1" ;;
	esac
}

_procd_append_param() {
	local type="$1"; shift
	local _json_no_warning=1
	json_select "$type" 2>/dev/null || { _procd_set_param "$type" "$@"; return; }
	case "$type" in
		env|data|limits) _procd_add_table_data "$@" ;;
		command|netdev|file|respawn|watch|watchdog) _procd_add_array_data "$@" ;;
		error) json_add_string "" "$*" ;;
	esac
	json_select ..
}

_procd_add_instance() {
	_procd_open_instance
	_procd_set_param command "$@"
	_procd_close_instance
}

_procd_kill() {
	if ubus -S list service >/dev/null 2>&1; then
		json_init; [ -n "$1" ] && json_add_string name "$1"; [ -n "$2" ] && json_add_string instance "$2"
		ubus call service delete "$(json_dump)"; local rc=$?; json_cleanup; return $rc
	fi
	/usr/sbin/neon-procd delete "$1" "$2"
}

procd_running() {
	/usr/sbin/neon-procd running "$1" "$2"
}

_procd_status() {
	/usr/sbin/neon-procd status "$1" "$2"
}

# Trigger JSON is accepted to preserve API compatibility. A later neon-ubus-trigger
# daemon can consume these records and call systemctl reload/restart.
_procd_open_trigger() {
	[ "$_PROCD_TRIGGER_OPEN" -gt 0 ] && { _PROCD_TRIGGER_OPEN=$((_PROCD_TRIGGER_OPEN + 1)); return; }
	_PROCD_TRIGGER_OPEN=1
	json_add_array triggers
}

_procd_close_trigger() {
	[ "$_PROCD_TRIGGER_OPEN" -le 0 ] && return
	_PROCD_TRIGGER_OPEN=$((_PROCD_TRIGGER_OPEN - 1))
	[ "$_PROCD_TRIGGER_OPEN" -eq 0 ] && json_close_array
}

_procd_add_interface_trigger() {
	json_add_array ""; _procd_add_array_data "$1"; shift
	json_add_array ""; _procd_add_array_data "if"
	json_add_array ""; _procd_add_array_data "eq" "interface" "$1"; shift; json_close_array
	json_add_array ""; _procd_add_array_data "run_script" "$@"; json_close_array
	json_close_array; json_close_array
}

_procd_add_config_trigger() {
	json_add_array ""; _procd_add_array_data "$1"; shift
	json_add_array ""; _procd_add_array_data "if"
	json_add_array ""; _procd_add_array_data "eq" "package" "$1"; shift; json_close_array
	json_add_array ""; _procd_add_array_data "run_script" "$@"; json_close_array
	json_close_array; json_close_array
}

_procd_add_raw_trigger() {
	json_add_array ""; _procd_add_array_data "$1"; shift
	local timeout="$1"; shift
	json_add_array ""; json_add_array ""; _procd_add_array_data "run_script" "$@"; json_close_array; json_close_array
	json_add_int "" "${timeout:-0}"; json_close_array
}

_procd_add_reload_trigger() {
	local script="$(readlink "$initscript" 2>/dev/null)" name file
	name="$(basename "${script:-$initscript}")"
	_procd_open_trigger
	for file in "$@"; do _procd_add_config_trigger "config.change" "$file" /etc/init.d/"$name" reload; done
	_procd_close_trigger
}

_procd_add_reload_interface_trigger() {
	local script="$(readlink "$initscript" 2>/dev/null)" name
	name="$(basename "${script:-$initscript}")"
	_procd_open_trigger
	_procd_add_interface_trigger "interface.*" "$1" /etc/init.d/"$name" reload
	_procd_close_trigger
}

_procd_open_validate() { json_add_array validate; }
_procd_close_validate() { json_close_array; }
_procd_add_validation() { :; }

# Jail API stubs: preserve init script execution; helper warns when unsupported fields matter.
_procd_add_jail() { json_add_object jail; json_add_string name "$1"; json_close_object; }
_procd_add_jail_mount() { :; }
_procd_add_jail_mount_rw() { :; }

_procd_set_config_changed() {
	json_init; json_add_string type config.change; json_add_object data; json_add_string package "$1"; json_close_object
	if ubus -S list service >/dev/null 2>&1; then ubus call service event "$(json_dump)"; else json_dump | /usr/sbin/neon-procd event; fi
	local rc=$?; json_cleanup; return $rc
}

_procd_wrapper \
	procd_open_service procd_close_service procd_add_instance \
	procd_open_instance procd_close_instance procd_set_param procd_append_param \
	procd_kill procd_open_trigger procd_close_trigger procd_add_reload_trigger \
	procd_add_reload_interface_trigger procd_add_interface_trigger \
	procd_add_config_trigger procd_add_raw_trigger procd_open_validate \
	procd_close_validate procd_add_validation procd_add_jail \
	procd_add_jail_mount procd_add_jail_mount_rw procd_set_config_changed
