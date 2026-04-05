#!/bin/sh
CC='owcc -bnt`if [ "$(MODE)" = "server" -o "$(MODE)" = "fpr" -o "$(MODE)" = "hsregex" ]; then echo ; elif ( echo " $(MODULES) " | grep " $(MODULE) " >/dev/null ); then echo "_dll" ; fi`'
AR='wlib'
ARFLAGS='-q -b -n -fo'
LIBS="$LIBS"

FPR="$FPR ws2_32.lib"

RPATH=''

SO='.dll'

E='.exe'

RESFILE='fhttpd.rc'

AFTER='wrc -q -bt=nt fhttpd.rc fhttpd.exe'
