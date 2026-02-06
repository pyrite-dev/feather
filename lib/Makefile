include ../config.mk

.PHONY: all install clean
.SUFFIXES: .c $(O)

OBJS = server$(O) stb_ds$(O)

all: $(LIB)fr$(SO)

.c$(O):
	$(CC) $(CFLAGS) $(PIC) $(INCDIR) ../fpr $(INCDIR) ../lib $(INCDIR) ../external/stb -c -o $@ $<

$(LIB)fr$(SO): $(OBJS) ../fpr/$(LIB)fpr$(SO)
	$(CC) $(LDFLAGS) $(SHARED) $(LIBDIR) ../fpr  -o $@ $(OBJS) $(LIBS) $(LINK) fpr

install:
	mkdir -p $(PREFIX)/bin
	mkdir -p $(PREFIX)/lib
	-cp *.dll $(PREFIX)/bin/
	-cp *.a *.so $(PREFIX)/lib/

clean:
	rm -f *$(SO) *$(O)
