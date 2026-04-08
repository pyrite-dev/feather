#!/bin/sh
PREFIX="SYS:/Feather"

CC='owcc -bnetware_clib_lite'
CFLAGS="-I$NOVELLNDK/include -I$NOVELLNDK/include/nlm/obsolete -I$NOVELLNDK/include/nlm -DN_PLAT_NLM"
AR='wlib'
ARFLAGS='-q -b -n -fo'
LIBS="$LIBS"

RPATH=''

SO='.nlm'

E='.nlm'
