#!/bin/sh
PREFIX="SYS:/Feather"

CC='owcc -bnetware_clib_lite`if [ "$(MODE)" = "server" -o "$(MODE)" = "fpr" -o "$(MODE)" = "hsregex" ]; then echo ; elif ( echo " $(MODULES) " | grep " $(MODULE) " >/dev/null ); then echo "_dll" ; fi`'
CFLAGS="-I$NOVELLNDK/include -I$NOVELLNDK/include/nlm -DN_PLAT_NLM"
AR='wlib'
ARFLAGS='-q -b -n -fo'
LIBS="$LIBS"

RPATH=''

SO='.nlm'

E='.nlm'
