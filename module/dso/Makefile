include ../../config.mk

.PHONY: clean
.SUFFIXES: .c $(O)

OBJS = mod_dso.o

mod_dso$(A): $(OBJS)
	$(AR) rcs $@ $(OBJS)

mod_dso$(SO): $(OBJS) ../fpr/$(LIB)fpr$(A)
	$(CC) $(LDFLAGS) $(SHARED) $(LIBDIR) ../../fpr $(LIBDIR) ../../lib -o $@ $(OBJS) ../../fpr/ $(LINK) fpr $(LINK) fr

.c$(O):
	$(CC) $(CFLAGS) $(PIC) $(INCDIR) ../../fpr $(INCDIR) ../../lib -c -o $@ $<

clean:
	rm -f *.o mod_dso$(A) mod_dso$(SO)
