#!/bin/sh
CC='owcc -bnt`if [ "$(MODE)" = "server" -o "$(MODE)" = "fpr" ]; then echo ; elif ( echo "$(MODULES)" | grep "$(MODULE)" ); then echo "_dll" ; fi`'
AR='wlib'

ARFLAGS='-q -b -n -fo'

RPATH=''

LIBS="$LIBS ws2_32.lib"

SO='.dll'

E='.exe'

RESFILE='fhttpd.rc'

AFTER='wrc -q -bt=nt -fe=fhttpd.exe fhttpd.rc'
