TOP = ..
MODE = server
include ../config.mk

.PHONY: all clean install
.SUFFIXES: .c $(O)

OBJS = config$(O) path$(O) arg$(O) log$(O) server$(O) ssl$(O) http$(O) mime$(O) module$(O) context$(O) util$(O)
OBJS += machdep_psp$(O)
OBJS += stb_ds$(O)

all: fhttpd$(E)

.c$(O):
	$(CC) $(CFLAGS) $(DEFINE)_FHTTPD $(INCDIR)../fpr $(INCDIR). $(INCDIR)../external/stb $(INCDIR)../external/hsregex -c -o $@ $<

.relink:
	touch $@

fhttpd$(E): main$(O) ../fpr/$(LIB)fpr$(A) ../external/hsregex/$(LIB)hsregex$(A) $(LIB)fhttpd$(A) $(RESFILE) .relink
	$(CC) $(LDFLAGS) $(RPATH) $(LIBDIR)../fpr $(LIBDIR)../external/hsregex $(LIBDIR). -o $@ main$(O) $(LIB)fhttpd$(A) $(SSL) $(THREAD) `for i in $(EMBED); do echo "../module/$$i/mod_$$i$(A)" ; done` $(FPR) $(REGEX) $(LIBS)
	$(AFTER)

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
	rm -f fhttpd$(E) fhttpd_strip$(E) *$(O) *$(A) pcre/*$(O) *.res .relink install.exe
