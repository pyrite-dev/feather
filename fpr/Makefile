include ../config.mk

.PHONY: all clean
.SUFFIXES: .c $(O)

OBJS = poll$(O)

all: $(LIB)fpr$(A)

.c$(O):
	$(CC) $(CFLAGS) $(INCDIR) ../fpr -c -o $@ $<

$(LIB)fpr$(A): $(OBJS)
	$(AR) rcs $@ $(OBJS)

clean:
	rm -f *$(A) *$(O)
