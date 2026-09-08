#!/bin/sh
set -eu

NEON_UCI_BIN="${NEON_UCI_BIN:-/usr/sbin/neon-uci}"

log() {
    printf '%s\n' "neon-uci-handler[$(basename "$0")]: $*" >&2
}

uci_get() {
    key="$1"
    def="${2-}"
    if val="$($NEON_UCI_BIN get "$key" 2>/dev/null)"; then
        printf '%s' "$val"
    else
        printf '%s' "$def"
    fi
}

uci_list() {
    key="$1"
    sep="${2- }"
    if val="$($NEON_UCI_BIN list "$key" "$sep" 2>/dev/null)"; then
        printf '%s' "$val"
    else
        printf '%s' ""
    fi
}

uci_bool() {
    case "${1:-}" in
        1|yes|true|on|enabled) printf '%s' yes ;;
        0|no|false|off|disabled) printf '%s' no ;;
        *) printf '%s' "${2:-no}" ;;
    esac
}

safe_simple_value() {
    case "$1" in
        *[!A-Za-z0-9._:/@+-]*) return 1 ;;
        *) return 0 ;;
    esac
}

atomic_write() {
    target="$1"
    mode="${2:-0644}"
    dir=$(dirname "$target")
    base=$(basename "$target")
    mkdir -p "$dir"
    tmp="$dir/.${base}.tmp.$$"
    trap 'rm -f "$tmp"' EXIT INT TERM HUP
    cat > "$tmp"
    chmod "$mode" "$tmp"
    mv -f "$tmp" "$target"
    trap - EXIT INT TERM HUP
}
