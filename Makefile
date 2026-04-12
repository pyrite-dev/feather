.PHONY: all format pre distclean clean install
.PHONY: ppr external/hsregex package/install.exe package/fhttpd.pbp package/fhttpd-psp.zip package/fhttpd-netware.zip package module

all: pre ppr server module

format:
	clang-format --verbose -i `find server module -name "*.c" -or -name "*.h"`

pre:
	@if [ ! -f "config.h" -o ! -f "config.mk" ]; then \
		echo "Please run ./configure" ; \
		exit 1 ; \
	fi

ppr: pre
	cd $@ ; $(MAKE)

external/hsregex: pre
	cd $@ ; $(MAKE)

package/install.exe package/fhttpd.pbp package/fhttpd-netware.zip: module server
	cd package ; $(MAKE) `echo $@ | cut -d/ -f2-`

package/fhttpd-psp.zip: module server
	cd server ; $(MAKE) fhttpd_strip.elf
	cd package ; $(MAKE) `echo $@ | cut -d/ -f2-`

server: pre ppr module external/hsregex
	cd $@ ; $(MAKE)

module: pre ppr
	cd $@ ; $(MAKE)

install: server module
	cd ppr ; $(MAKE) install DESTDIR=$(DESTDIR)
	cd server ; $(MAKE) install DESTDIR=$(DESTDIR)
	cd module ; $(MAKE) install DESTDIR=$(DESTDIR)

clean:
	-cd ppr ; $(MAKE) clean
	-cd server ; $(MAKE) clean
	-cd module ; $(MAKE) clean
	-cd package ; $(MAKE) clean
	-cd external/hsregex ; $(MAKE) clean

distclean: clean
	rm -f config.h config.mk local.mk
