include ../config.mk

.PHONY: all clean
.SUFFIXES: .c $(O)

OBJS = poll$(O) file$(O) socket$(O) arpa$(O) string$(O) dlfcn$(O)
OBJS += wildcard$(O)

all: $(LIB)fpr$(SO)

.c$(O):
	$(CC) $(CFLAGS) $(PIC) $(INCDIR) ../fpr -c -o $@ $<

$(LIB)fpr$(SO): $(OBJS)
	$(CC) $(LDFLAGS) $(SHARED) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f *$(SO) *$(O)
