#!/bin/sh

SCRIPT_DIR=`dirname "$(readlink -f "$0")"`

CUR_DIR=$(pwd)
cd $SCRIPT_DIR
./802.15.4_setup.sh ncp-uart-rts-cts-use-with-serial-uart-btl-5.7.4.ebl 5.7.4 &
cd $CUR_DIR
