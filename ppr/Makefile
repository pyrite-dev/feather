TOP = ..
MODE = server
include ../config.mk

.PHONY: all clean install
.SUFFIXES: .c $(O)

include objs.mk

all: $(LIB)ppr$(A)

.c$(O):
	$(CC) $(CFLAGS) $(PIC) $(INCDIR) git/include -c -o $@ $<

$(LIB)ppr$(A): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	mkdir -p $(DESTDIR)$(PREFIX)/include
	-cp *.dll $(DESTDIR)$(PREFIX)/bin/
	-cp *.a *.so $(DESTDIR)$(PREFIX)/lib/
	cp git/include/*.h $(DESTDIR)$(PREFIX)/include/

clean:
	rm -f *$(A) *$(O) */*$(O) *$(SO)
