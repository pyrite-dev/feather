TOP = ..
MODE = hsregex
include ../../config.mk

.PHONY: all clean
.SUFFIXES: .c $(O)

OBJS = regcomp$(O) regerror$(O) regexec$(O) regfree$(O)

all: $(LIB)hsregex$(A)

.c$(O):
	$(CC) $(CFLAGS) $(INCDIR). $(PIC) -c -o $@ $<

$(LIB)hsregex$(A): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	rm -f *$(A) *$(O)
