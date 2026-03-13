#!/bin/sh
CC="psp-gcc"
AR="psp-ar"

CFLAGS="$CFLAGS -I$PSPDEV/psp/sdk/include -D_PSP_FW_VERSION=600"
LDFLAGS="$LDFLAGS -Wl,-zmax-page-size=128 -L$PSPDEV/psp/sdk/lib"
LIBS="$LIBS -lpspgum -lpspgu -lpspdebug -lpspdisplay -lpspge -lpspctrl -lpspnet -lpspnet_apctl -lcglue -lpspwlan"

E='.elf'
