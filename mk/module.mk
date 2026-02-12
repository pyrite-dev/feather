include ../../config.mk

.PHONY: install clean
.SUFFIXES: .c $(O)

OBJS += mod_$(MODULE)$(O)

mod_$(MODULE)$(A): $(OBJS)
	$(AR) rcs $@ $(OBJS)
	touch ../../server/.relink

mod_$(MODULE)$(SO): $(OBJS) ../fpr/$(LIB)fpr$(A)
	$(CC) $(LDFLAGS) $(RPATH) $(SHARED) $(LIBDIR)../../fpr -o $@ $(OBJS)../../fpr/ $(LINK) fpr

.c$(O):
	$(CC) $(CFLAGS) $(PIC) $(INCDIR)../../fpr $(INCDIR)../../server $(INCDIR)../../external/stb -c -o $@ $<

install:
	mkdir -p $(PREFIX)/lib/fhttpd
	-cp *.so *.dll $(PREFIX)/lib/fhttpd/

clean:
	rm -f *.o mod_$(MODULE)$(A) mod_$(MODULE)$(SO)
