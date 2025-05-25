#!/bin/sh

. /lib/functions.sh
/usr/sbin/nu801 "$(board_name)"

# blink LED after nu801 init
. /etc/diag.sh
set_state preinit_regular