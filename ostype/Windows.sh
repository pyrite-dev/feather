#!/bin/sh
RPATH=''

RCFLAGS="-O coff"
LIBS="$LIBS \$(LINK)ws2_32"

SO='.dll'

E='.exe'

RESFILE='fhttpd.res'
