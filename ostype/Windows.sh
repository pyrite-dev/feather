#!/bin/sh
RPATH=''

RCFLAGS="-O coff"

FPR="$FPR \$(LINK)ws2_32"

SO='.dll'

E='.exe'

RESFILE='fhttpd.res'
