TOP = ..
MODE = server
include ../config.mk

.PHONY: all clean install
.SUFFIXES: .c $(O)

OBJS = core$(O) config$(O) path$(O) arg$(O) log$(O) server$(O) ssl$(O) http$(O) mime$(O) module$(O) context$(O) util$(O)
OBJS += machdep_psp$(O) machdep_ps2$(O) machdep_netware$(O)
OBJS += stb_ds$(O)

LDARGS1 = $(LDFLAGS) $(RPATH) $(LIBDIR)../ppr $(LIBDIR)../external/hsregex $(LIBDIR). -o $@
LDARGS2 = $(LIB)fhttpd$(A) $(SSL) `for i in $(EMBED); do echo "../module/$$i/mod_$$i$(A)" ; done` $(PPR) $(REGEX) $(LIBS)

DEPS = ../ppr/$(LIB)ppr$(A) ../external/hsregex/$(LIB)hsregex$(A) $(LIB)fhttpd$(A) $(RESFILE) .relink

all: fhttpd$(E)

.c$(O):
	$(CC) $(CFLAGS) $(DEFINE)_FHTTPD $(INCDIR)../ppr/git/include $(INCDIR). $(INCDIR)../external/stb $(INCDIR)../external/hsregex -c -o $@ $<

.relink:
	touch $@

fhttpd$(E): main$(O) $(DEPS)
	$(CC) $(LDARGS1) main$(O) $(LDARGS2)
	$(AFTER)

fhttpdmon$(E): monitor$(O) $(DEPS)
	$(CC) $(LDARGS1) monitor$(O) $(LDARGS2) $(LINK)curses

$(LIB)fhttpd$(A): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

fhttpd.res: fhttpd.rc
	$(RC) $(RCFLAGS) fhttpd.rc $@

fhttpd_strip$(E): fhttpd$(E)
	$(STRIP) fhttpd$(E) -o $@

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/etc/fhttpd
	mkdir -p $(DESTDIR)$(PREFIX)/include
	cp fhttpd$(E) $(DESTDIR)$(PREFIX)/bin/
	-cp fhttpdmon$(E) $(DESTDIR)$(PREFIX)/bin/
	if [ ! -f "$(DESTDIR)$(PREFIX)/etc/fhttpd/fhttpd.conf" ]; then \
		cp ../config/fhttpd.conf $(DESTDIR)$(PREFIX)/etc/fhttpd/ ; \
	fi
	if [ ! -f "$(DESTDIR)$(PREFIX)/etc/fhttpd/mime.types" ]; then \
		cp ../config/mime.types $(DESTDIR)$(PREFIX)/etc/fhttpd/ ; \
	fi
	if [ ! -d "$(DESTDIR)$(PREFIX)/www" ]; then \
		cp -r ../www $(DESTDIR)$(PREFIX)/www ; \
	fi
	cp *.h $(DESTDIR)$(PREFIX)/include/

clean:
	rm -f fhttpd$(E) fhttpdmon$(E) fhttpd_strip$(E) *$(O) *$(A) pcre/*$(O) *.res .relink install.exe
