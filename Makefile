.PHONY: all pre distclean clean fpr server

all: pre fpr server

pre:
	@ST=0 ; \
	if [ ! -f "config.h" ]; then \
		cp config.h.in config.h ; \
		echo "Copied config.h.in to config.h" ; \
		ST=1 ; \
	fi ; \
	if [ ! -f "config.mk" ]; then \
		cp config.mk.in config.mk ; \
		echo "Copied config.mk.in to config.mk" ; \
		ST=1 ; \
	fi ; \
	if [ "$$ST" = "1" ]; then \
		echo "Please remember to review them!" ; \
	fi ; \
	exit $$ST

fpr: pre
	cd fpr ; $(MAKE)

server: pre fpr
	cd server ; $(MAKE)

clean:
	-cd fpr ; $(MAKE) clean
	-cd server ; $(MAKE) clean

distclean: clean
	rm -f config.h config.mk
