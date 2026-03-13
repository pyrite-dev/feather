TOP = ..
MODE = server
include ../config.mk

.PHONY: clean

install.exe: ../server/fhttpd$(E) install.nsi
	rm -rf tmp
	cd .. ; $(MAKE) install DESTDIR=`pwd`/package/tmp/
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

fhttpd.pbp: ../server/fhttpd_strip$(E) param.sfo
	pack-pbp $@ param.sfo ../logo/opaque.png NULL NULL NULL NULL fhttpd_strip$(E) NULL

param.sfo:
	mksfoex -d MEMSIZE=1 'Feather HTTPd' $@

../server/fhttpd_strip$(E): ../server/fhttpd$(E)
	cd ../server && $(MAKE) fhttpd_strip$(E)

clean:
	rm -f *.pbp *.sfo *.zip *.exe
