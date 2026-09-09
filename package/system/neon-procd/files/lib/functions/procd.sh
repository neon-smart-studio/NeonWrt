#!/bin/sh
# NeonWrt procd API compatibility layer backed by systemd.
# Intentionally keeps the OpenWrt shell API surface so USE_PROCD=1 scripts work.

. "$IPKG_INSTROOT/usr/share/libubox/jshn.sh"

PROCD_RELOAD_DELAY=${PROCD_RELOAD_DELAY:-1000}
_PROCD_SERVICE=
_PROCD_INSTANCE_SEQ=0
_PROCD_TRIGGER_OPEN=0

procd_lock() {
	local basescript service_name lockfile
	basescript="$(readlink "$initscript" 2>/dev/null)"
	service_name="$(basename "${basescript:-$initscript}")"
	lockfile="$IPKG_INSTROOT/var/lock/procd_${service_name}.lock"
	mkdir -p "$(dirname "$lockfile")" 2>/dev/null
	if command -v flock >/dev/null 2>&1; then
		exec 1000>"$lockfile"
		flock 1000
	fi
}

_procd_ubus_call() {
	local cmd="$1" rc
	[ -n "$PROCD_DEBUG" ] && json_dump >&2
	if ubus -S list service >/dev/null 2>&1; then
		ubus call service "$cmd" "$(json_dump)"
		rc=$?
	else
		case "$cmd" in
			set|add) json_dump | /usr/sbin/neon-procd set ;;
			delete) return 1 ;;
			list) /usr/sbin/neon-procd status ;;
			*) return 1 ;;
		esac
		rc=$?
	fi
	return $rc
}

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

	# Preserve OpenWrt trigger declarations in the service registry.
	# neon-procd-ubus subscribes to ubus events and forwards matching events to
	# neon-procd, which evaluates these records and executes the original
	# run_script action (normally /etc/init.d/<service> reload).
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

# Trigger JSON is stored with the service definition. neon-procd-ubus bridges
# real ubus events to neon-procd's trigger evaluator, preserving the original
# OpenWrt run_script reload/restart semantics.
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

_procd_add_data_trigger() {
	json_add_array ""
	_procd_add_array_data "service.data.update"
	json_add_array ""
	_procd_add_array_data "if"
	json_add_array ""
	_procd_add_array_data "eq" "name" "$1"
	shift
	json_close_array
	json_add_array ""
	_procd_add_array_data "run_script" "$@"
	json_close_array
	json_close_array
	[ "$PROCD_RELOAD_DELAY" -gt 0 ] 2>/dev/null && json_add_int "" "$PROCD_RELOAD_DELAY"
	json_close_array
}

_procd_add_reload_data_trigger() {
	local script name t
	script="$(readlink "$initscript" 2>/dev/null)"
	name="$(basename "${script:-$initscript}")"
	_procd_open_trigger
	for t in "$@"; do
		_procd_add_data_trigger "$t" /etc/init.d/"$name" reload
	done
	_procd_close_trigger
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
_procd_add_validation() {
	_procd_open_validate
	"$@"
	_procd_close_validate
}

uci_validate_section() {
	local _package="$1" _type="$2" _name="$3" _result _error
	shift 3

	if [ ! -x /sbin/validate_data ]; then
		echo "uci_validate_section: /sbin/validate_data is missing" >&2
		return 127
	fi

	_result="$(/sbin/validate_data "$_package" "$_type" "$_name" "$@" 2>/dev/null)"
	_error=$?
	eval "$_result"
	[ "$_error" = 0 ] || /sbin/validate_data "$_package" "$_type" "$_name" "$@" >/dev/null
	return "$_error"
}

uci_load_validate() {
	local _package="$1" _type="$2" _name="$3" _function="$4" _option _result
	shift 4

	for _option in "$@"; do
		eval "local ${_option%%:*}"
	done

	uci_validate_section "$_package" "$_type" "$_name" "$@"
	_result=$?
	[ -n "$_function" ] || return "$_result"
	eval "$_function \"\$_name\" \"\$_result\""
}

# Jail API stubs: preserve init script execution; helper warns when unsupported fields matter.
_procd_add_jail() { json_add_object jail; json_add_string name "$1"; json_close_object; }
_procd_add_jail_mount() { :; }
_procd_add_jail_mount_rw() { :; }

# OpenWrt procd mDNS/umdns compatibility.
_procd_add_mdns() {
	local service="$1" port="$2"
	shift 2

	json_add_object mdns
	json_add_string service "$service"
	[ -n "$port" ] && json_add_int port "$port"
	if [ "$#" -gt 0 ]; then
		json_add_array txt
		_procd_add_array_data "$@"
		json_close_array
	fi
	json_close_object
}

_procd_set_config_changed() {
	json_init; json_add_string type config.change; json_add_object data; json_add_string package "$1"; json_close_object
	if ubus -S list service >/dev/null 2>&1; then ubus call service event "$(json_dump)"; else json_dump | /usr/sbin/neon-procd event; fi
	local rc=$?; json_cleanup; return $rc
}

_procd_wrapper \
	procd_open_service procd_close_service procd_add_instance \
	procd_open_instance procd_close_instance procd_set_param procd_append_param \
	procd_kill procd_open_trigger procd_close_trigger procd_add_reload_trigger \
	procd_add_reload_interface_trigger procd_add_reload_data_trigger procd_add_interface_trigger \
	procd_add_config_trigger procd_add_raw_trigger procd_open_validate \
	procd_close_validate procd_add_validation procd_add_jail \
	procd_add_jail_mount procd_add_jail_mount_rw procd_add_mdns procd_set_config_changed
