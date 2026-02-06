include ../config.mk

.PHONY: all clean
.SUFFIXES: .c $(O)

OBJS = server$(O) stb_ds$(O)

all: $(LIB)fr$(SO)

.c$(O):
	$(CC) $(CFLAGS) $(PIC) $(INCDIR) ../fpr $(INCDIR) ../lib $(INCDIR) ../external/stb -c -o $@ $<

$(LIB)fr$(SO): $(OBJS) ../fpr/$(LIB)fpr$(SO)
	$(CC) $(LDFLAGS) $(SHARED) $(LIBDIR) ../fpr  -o $@ $(OBJS) $(LIBS) $(LINK) fpr

clean:
	rm -f *$(SO) *$(O)
