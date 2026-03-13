.PHONY: all format pre distclean clean install
.PHONY: fpr package/install.exe package/fhttpd-psp.zip package module

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

package/install.exe: pre fpr module server
	cd package ; $(MAKE) `echo $@ | cut -d/ -f2-`

package/fhttpd-psp.zip: pre fpr module server
	cd package ; $(MAKE) `echo $@ | cut -d/ -f2-`

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
	-cd package ; $(MAKE) clean

distclean: clean
	rm -f config.h config.mk local.mk
