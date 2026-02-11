include ../../config.mk

.PHONY: install clean
.SUFFIXES: .c $(O)

OBJS = mod_$(MODULE).o

mod_$(MODULE)$(A): $(OBJS)
	$(AR) rcs $@ $(OBJS)

mod_$(MODULE)$(SO): $(OBJS) ../fpr/$(LIB)fpr$(A)
	$(CC) $(LDFLAGS) $(RPATH) $(SHARED) $(LIBDIR) ../../fpr $(LIBDIR) ../../lib -o $@ $(OBJS) ../../fpr/ $(LINK) fpr $(LINK) fr

.c$(O):
	$(CC) $(CFLAGS) $(PIC) $(INCDIR) ../../fpr $(INCDIR) ../../lib -c -o $@ $<

install:
	mkdir -p $(PREFIX)/lib/fhttpd
	-cp *.so *.dll $(PREFIX)/lib/fhttpd/

clean:
	rm -f *.o mod_$(MODULE)$(A) mod_$(MODULE)$(SO)
