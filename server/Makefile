TOP = ..
MODE = server
include ../config.mk

.PHONY: all clean install
.SUFFIXES: .c $(O)

OBJS = main$(O) config$(O) path$(O) arg$(O) log$(O) server$(O) ssl$(O) http$(O) mime$(O) module$(O) context$(O) util$(O) machdep_psp$(O)
OBJS += stb_ds$(O)

all: fhttpd$(E)

.c$(O):
	$(CC) $(CFLAGS) $(DEFINE)_FHTTPD $(INCDIR)../fpr $(INCDIR). $(INCDIR)../external/stb -c -o $@ $<

.relink:
	touch $@

fhttpd$(E): $(OBJS) ../fpr/$(LIB)fpr$(A) $(RESFILE) .relink
	$(CC) $(LDFLAGS) $(RPATH) $(LIBDIR)../fpr -o $@ $(OBJS) $(SSL) $(THREAD) `for i in $(EMBED); do echo "../module/$$i/mod_$$i$(A)" ; done` $(FPR) $(LIBS)
	$(AFTER)

fhttpd.res: fhttpd.rc
	$(RC) $(RCFLAGS) fhttpd.rc $@

install.exe: fhttpd$(E) install.nsi
	rm -rf tmp
	cd .. ; $(MAKE) install DESTDIR=`pwd`/server/tmp/
	mv tmp/C:/Feather/etc tmp/etc
	mv tmp/C:/Feather/www tmp/www
	sed -i 's%/usr/fhttpd%$(PREFIX)%g' tmp/etc/fhttpd/fhttpd.conf
	makensis install.nsi
	rm -rf tmp

fhttpd-psp.zip: fhttpd.pbp
	rm -rf tmp
	make install DESTDIR=`pwd`/tmp/
	rm -rf tmp/ms0:/PSP/GAME/fhttpd/bin
	cp fhttpd.pbp tmp/ms0:/PSP/GAME/fhttpd/EBOOT.PBP
	sed -i 's%/usr/fhttpd%$(PREFIX)%g' tmp/ms0:/PSP/GAME/fhttpd/etc/fhttpd/fhttpd.conf
	echo 'Feather HTTPd for PSP' > tmp/ms0:/PSP/GAME/fhttpd/README
	echo '========================' >> tmp/ms0:/PSP/GAME/fhttpd/README
	echo 'To install, just copy this `fhttpd'"'"' folder into your /PSP/GAME of the PSP memorystick.' >> tmp/ms0:/PSP/GAME/fhttpd/README
	echo 'lib/include folders are not required to run the HTTPd.' >> tmp/ms0:/PSP/GAME/fhttpd/README
	cd tmp/ms0:/PSP/GAME/ && zip -rv ../../../../fhttpd-psp.zip .
	rm -rf tmp

fhttpd_strip$(E): fhttpd$(E)
	$(STRIP) fhttpd$(E) -o $@

fhttpd.pbp: fhttpd_strip$(E) param.sfo
	pack-pbp $@ param.sfo ../logo/opaque.png NULL NULL NULL NULL fhttpd_strip$(E) NULL

param.sfo:
	mksfoex -d MEMSIZE=1 'Feather HTTPd' $@

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
	rm -f fhttpd$(E) fhttpd_strip$(E) *$(O) *.res *.pbp *.sfo .relink install.exe
