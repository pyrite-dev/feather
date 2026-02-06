include ../../config.mk

.PHONY: clean
.SUFFIXES: .c $(O)

mod_dso$(A): $(OBJS)
	$(AR) rcs $@ $(OBJS)

mod_dso$(SO): $(OBJS) ../fpr/$(LIB)fpr$(A)
	$(CC) $(LDFLAGS) $(SHARED) -o $@ $(OBJS) ../../fpr/ $(LINK) ../../fpr/$(LIB)fpr$(A)

.c$(O):
	$(CC) $(CFLAGS) $(PIC) $(INCDIR) ../../fpr -c -o $@ $<

clean:
	rm -f *.o mod_dso$(A) mod_dso$(SO)
