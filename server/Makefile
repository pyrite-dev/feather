include ../config.mk

.PHONY: all clean install
.SUFFIXES: .c $(O)

OBJS = main$(O) config$(O) path$(O) arg$(O) log$(O) server$(O)
OBJS += stb_ds$(O)

all: fhttpd$(E)

.c$(O):
	$(CC) $(CFLAGS) $(INCDIR) ../fpr $(INCDIR) ../lib $(INCDIR) . $(INCDIR) ../external/stb -c -o $@ $<

fhttpd$(E): $(OBJS) ../fpr/$(LIB)fpr$(SO) ../lib/$(LIB)fr$(SO)
	$(CC) $(LDFLAGS) $(LIBDIR) ../fpr $(LIBDIR) ../lib -o $@ $(OBJS) $(LIBS) $(SSL) $(LINK) fpr $(LINK) fr `echo '$(EMBED)' | perl -pe 's/([^ \n]+)/..\/module\/\1\/mod_\1$(A)/'`

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/etc
	cp fhttpd$(E) $(DESTDIR)$(PREFIX)/bin/
	if [ ! -f "$(DESTDIR)$(PREFIX)/etc/fhttpd.conf" ]; then \
		cp fhttpd.conf $(DESTDIR)$(PREFIX)/etc/ ; \
	fi
	if [ ! -d "$(DESTDIR)$(PREFIX)/www" ]; then \
		cp -r ../www $(DESTDIR)$(PREFIX)/www ; \
	fi

clean:
	rm -f fhttpd$(E) *$(O)
