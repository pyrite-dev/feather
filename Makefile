.PHONY: all format pre distclean clean fpr server module install

all: pre fpr server module

format:
	clang-format --verbose -i `find fpr server -name "*.c" -or -name "*.h"`

pre:
	@ST=0 ; \
	if [ ! -f "config.h" ]; then \
		cp config.h.in config.h ; \
		echo "Copied config.h.in to config.h" ; \
		ST=1 ; \
	fi ; \
	if [ ! -f "config.mk" ]; then \
		if [ -f "mk/ostype/`uname -s`.mk" ]; then \
			cp mk/config.mk.in config.mk ; \
			echo >> config.mk ; \
			echo "# Automatically added platform-dependent flags" >> config.mk ; \
			cat mk/ostype/`uname -s`.mk >> config.mk ; \
		else \
			cp mk/config.mk.in config.mk ; \
		fi ; \
		echo "Copied config.mk.in to config.mk" ; \
		ST=1 ; \
	fi ; \
	if [ "$$ST" = "1" ]; then \
		echo "Please remember to review them!" ; \
	fi ; \
	exit $$ST

fpr: pre
	cd fpr ; $(MAKE)

server: pre fpr module
	cd server ; $(MAKE)

module: pre fpr
	cd module ; $(MAKE)

install: server module
	cd server ; $(MAKE) install

clean:
	-cd fpr ; $(MAKE) clean
	-cd server ; $(MAKE) clean
	-cd module ; $(MAKE) clean

distclean: clean
	rm -f config.h config.mk
