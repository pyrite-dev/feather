TOP = ..
MODE = fpr
include ../config.mk

.PHONY: all install clean
.SUFFIXES: .c $(O)

OBJS = base/poll$(O) base/file$(O) base/socket$(O) base/arpa$(O) base/string$(O) base/dlfcn$(O) base/unistd$(O) base/stat$(O) base/thread$(O) base/process$(O)
OBJS += crypto/md5$(O) crypto/sha256$(O)
OBJS += url$(O)
OBJS += wildcard$(O)

all: $(LIB)fpr$(A)

.c$(O):
	$(CC) $(CFLAGS) $(PIC) $(INCDIR) ../fpr -c -o $@ $<

$(LIB)fpr$(A): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	mkdir -p $(DESTDIR)$(PREFIX)/include
	-cp *.dll $(DESTDIR)$(PREFIX)/bin/
	-cp *.a *.so $(DESTDIR)$(PREFIX)/lib/
	cp *.h $(DESTDIR)$(PREFIX)/include/

clean:
	rm -f *$(A) *$(O) */*$(O) *$(SO)
