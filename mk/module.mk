TOP = ../..
MODE = module
include ../../config.mk

.PHONY: install clean
.SUFFIXES: .c $(O)

OBJS += mod_$(MODULE)$(O) $(DEPS)

mod_$(MODULE)$(A): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)
	touch ../../server/.relink

mod_$(MODULE)$(SO): $(OBJS) ../../ppr/$(LIB)ppr$(A)
	$(CC) $(LDFLAGS) $(RPATH) $(SHARED) $(LIBDIR)../../ppr -o $@ $(OBJS) $(PPR)

.c$(O):
	$(CC) $(CFLAGS) $(PIC) $(INCDIR)../../ppr/git/include $(INCDIR)../../server $(INCDIR)../../external/stb -c -o $@ $<

install:
	mkdir -p $(DESTDIR)$(PREFIX)/lib/fhttpd
	-cp *.so *.dll *.nlm $(DESTDIR)$(PREFIX)/lib/fhttpd/

clean:
	rm -f *.o mod_$(MODULE)$(A) mod_$(MODULE)$(SO)
