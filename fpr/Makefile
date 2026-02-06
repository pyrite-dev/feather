include ../config.mk

.PHONY: all clean
.SUFFIXES: .c $(O)

OBJS = poll$(O) file$(O) socket$(O) arpa$(O) string$(O) dlfcn$(O)
OBJS += wildcard$(O)

all: $(LIB)fpr$(A)

.c$(O):
	$(CC) $(CFLAGS) $(PIC) $(INCDIR) ../fpr -c -o $@ $<

$(LIB)fpr$(A): $(OBJS)
	$(AR) rcs $@ $(OBJS)

clean:
	rm -f *$(A) *$(O)
