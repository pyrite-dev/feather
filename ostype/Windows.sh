#!/bin/sh
PREFIX="C:/Feather"

RPATH=''

RCFLAGS="-O coff"

PPR="$PPR \$(LINK)ws2_32"

SO='.dll'

E='.exe'

RESFILE='fhttpd.res'
