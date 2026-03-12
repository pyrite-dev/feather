TOP = ..
MODE = fpr
include ../config.mk

.PHONY: all install clean
.SUFFIXES: .c $(O)

OBJS = poll$(O) file$(O) socket$(O) arpa$(O) string$(O) dlfcn$(O) unistd$(O) stat$(O)
OBJS += wildcard$(O)

all: $(LIB)fpr$(A)

.c$(O):
	$(CC) $(CFLAGS) $(PIC) $(INCDIR) ../fpr -c -o $@ $<

$(LIB)fpr$(A): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	-cp *.dll $(DESTDIR)$(PREFIX)/bin/
	-cp *.a *.so $(DESTDIR)$(PREFIX)/lib/

clean:
	rm -f *$(A) *$(O) *$(SO)
