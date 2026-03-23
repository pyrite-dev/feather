TOP = ..
MODE = pcre
include ../../config.mk

.PHONY: all clean
.SUFFIXES: .c $(O)

OBJS = pcre_byte_order$(O) pcre_chartables$(O) pcre_compile$(O) pcre_config$(O) pcre_dfa_exec$(O) pcre_exec$(O) pcre_fullinfo$(O) pcre_get$(O) pcre_globals$(O) pcre_maketables$(O) pcre_newline$(O) pcre_ord2utf8$(O) pcre_refcount$(O) pcre_string_utils$(O) pcre_study$(O) pcre_tables$(O) pcre_ucd$(O) pcre_valid_utf8$(O) pcre_version$(O) pcre_xclass$(O)

all: $(LIB)pcre$(A)

.c$(O):
	$(CC) $(CFLAGS) $(PIC) $(DEFINE)HAVE_CONFIG_H -c -o $@ $<

$(LIB)pcre$(A): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	rm -f *$(A) *$(O)
