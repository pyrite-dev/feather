include ../config.mk

.PHONY: all install clean
.SUFFIXES: .c $(O)

OBJS = poll$(O) file$(O) socket$(O) arpa$(O) string$(O) dlfcn$(O) unistd$(O)
OBJS += wildcard$(O)

all: $(LIB)fpr$(SO)

.c$(O):
	$(CC) $(CFLAGS) $(PIC) $(INCDIR) ../fpr -c -o $@ $<

$(LIB)fpr$(SO): $(OBJS)
	$(CC) $(LDFLAGS) $(SHARED) -o $@ $(OBJS) $(LIBS)

install:
	mkdir -p $(PREFIX)/bin
	mkdir -p $(PREFIX)/lib
	-cp *.dll $(PREFIX)/bin/
	-cp *.a *.so $(PREFIX)/lib/

clean:
	rm -f *$(SO) *$(O)
