include ../config.mk

.PHONY: all clean install
.SUFFIXES: .c $(O)

OBJS = main$(O) config$(O) util$(O)

all: fhttpd$(E)

.c$(O):
	$(CC) $(CFLAGS) $(INCDIR) ../fpr $(INCDIR) . -c -o $@ $<

fhttpd$(E): $(OBJS) ../fpr/$(LIB)fpr$(A)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) $(LINK) ../fpr/$(LIB)fpr$(A)

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/etc
	cp fhttpd$(E) $(DESTDIR)$(PREFIX)/bin/
	if [ ! -f "$(DESTDIR)$(PREFIX)/etc/fhttpd.conf" ]; then \
		cp fhttpd.conf $(DESTDIR)$(PREFIX)/etc/ ; \
	fi

clean:
	rm -f fhttpd$(E) *$(O)
