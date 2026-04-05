#!/bin/sh
RPATH=''

RCFLAGS="-O coff"

LIBS="$LIBS -lws2_32"

SO='.dll'

E='.exe'

RESFILE='fhttpd.res'
