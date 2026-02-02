include ../config.mk

.PHONY: all clean
.SUFFIXES: .c $(O)

OBJS = main$(O)

all: fhttpd$(E)

.c$(O):
	$(CC) $(CFLAGS) $(INCDIR) ../fpr -c -o $@ $<

fhttpd$(E): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) $(LINK) ../fpr/$(LIB)fpr$(A)

clean:
	rm -f fhttpd$(E) *$(O)
