#!/bin/sh
#
# Copyright (C) 2007 OpenWrt.org
#
#

. /lib/functions.sh
. /lib/functions/lantiq.sh

do_fixboss() {
	mtd fixboss firmware 1>/tmp/fixboss.log 2>&1
}

board=$(board_name)
case "$board" in
bintec,rs353)
        do_fixboss
	return 0
        ;;
esac
