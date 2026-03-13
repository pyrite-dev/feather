.PHONY: all format pre distclean clean install
.PHONY: fpr server/install.exe server/fhttpd.pbp server/fhttpd-psp.zip server module

all: pre fpr server module

format:
	clang-format --verbose -i `find fpr server module -name "*.c" -or -name "*.h"`

pre:
	@if [ ! -f "config.h" -o ! -f "config.mk" ]; then \
		echo "Please run ./configure" ; \
		exit 1 ; \
	fi

fpr: pre
	cd $@ ; $(MAKE)

server/install.exe: pre fpr module server
	cd server ; $(MAKE) install.exe

server/fhttpd.pbp: pre fpr module server
	cd server ; $(MAKE) fhttpd.pbp

server/fhttpd-psp.zip: pre fpr module server
	cd server ; $(MAKE) fhttpd-psp.zip

server: pre fpr module
	cd $@ ; $(MAKE)

module: pre fpr
	cd $@ ; $(MAKE)

install: server module
	cd fpr ; $(MAKE) install DESTDIR=$(DESTDIR)
	cd server ; $(MAKE) install DESTDIR=$(DESTDIR)
	cd module ; $(MAKE) install DESTDIR=$(DESTDIR)

clean:
	-cd fpr ; $(MAKE) clean
	-cd server ; $(MAKE) clean
	-cd module ; $(MAKE) clean

distclean: clean
	rm -f config.h config.mk local.mk
