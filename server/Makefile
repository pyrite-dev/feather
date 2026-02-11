include ../config.mk

.PHONY: all clean install
.SUFFIXES: .c $(O)

OBJS = main$(O) config$(O) path$(O) arg$(O) log$(O) server$(O) ssl$(O) http$(O) mime$(O)
OBJS += stb_ds$(O)

all: fhttpd$(E)

.c$(O):
	$(CC) $(CFLAGS) $(DEFINE)_FHTTPD $(INCDIR)../fpr $(INCDIR)../lib $(INCDIR). $(INCDIR)../external/stb -c -o $@ $<

fhttpd$(E): $(OBJS) ../fpr/$(LIB)fpr$(SO)
	$(CC) $(LDFLAGS) $(RPATH) $(LIBDIR)../fpr $(LIBDIR)../lib -o $@ $(OBJS) $(LIBS) $(SSL) $(LINK) fpr `for i in $(EMBED); do echo "../module/$$i/mod_$$i$(A)" ; done`

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/etc
	cp fhttpd$(E) $(DESTDIR)$(PREFIX)/bin/
	if [ ! -f "$(DESTDIR)$(PREFIX)/etc/fhttpd.conf" ]; then \
		cp ../config/fhttpd.conf $(DESTDIR)$(PREFIX)/etc/ ; \
	fi
	if [ ! -f "$(DESTDIR)$(PREFIX)/etc/mime.types" ]; then \
		cp ../config/mime.types $(DESTDIR)$(PREFIX)/etc/ ; \
	fi
	if [ ! -d "$(DESTDIR)$(PREFIX)/www" ]; then \
		cp -r ../www $(DESTDIR)$(PREFIX)/www ; \
	fi

clean:
	rm -f fhttpd$(E) *$(O)
