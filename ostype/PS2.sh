#!/bin/sh
CC="mips64r5900el-ps2-elf-gcc"
AR="mips64r5900el-ps2-elf-ar"
STRIP="mips64r5900el-ps2-elf-strip"

CFLAGS="$CFLAGS -D_EE -I$PS2SDK/ee/include -I$PS2SDK/common/include"
LDFLAGS="-T$PS2SDK/ee/startup/linkfile -L$PS2SDK/ee/lib"
LIBS="$LIBS \$(LINK)debug"

PPR="$PPR \$(LINK)socket"

PIC=""

E='.elf'
