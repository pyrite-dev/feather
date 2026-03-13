TOP = ..
include ../config.mk

.PHONY: all clean install $(EMBED) $(MODULES)

all: $(EMBED) $(MODULES)

$(EMBED):
	cd $@ ; $(MAKE) mod_$@$(A)

$(MODULES):
	cd $@ ; $(MAKE) mod_$@$(SO)

clean install:
	for i in $(EMBED) $(MODULES); do \
		cd $$i || exit 1 ; \
		$(MAKE) $@ || exit 1 ; \
		cd .. ; \
	done
