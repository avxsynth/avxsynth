#                                       -*- Automake -*-
# Process this file with automake to produce a Makefile.

pkglib_LTLIBRARIES = libavxffms2.la

libavxffms2_la_SOURCES = \
	src/avsutils_avx.cpp   \
	src/ffswscale_avx.cpp  \
	src/avssources_avx.cpp \
	src/avxffms2.cpp

libavxffms2_la_LIBADD = \
	$(ffms2_LIBS) \
	$(libav_LIBS)

# Warning: These defines are not portable.
libavxffms2_la_CPPFLAGS = \
	-D__STDC_CONSTANT_MACROS      \
	-D_FILE_OFFSET_BITS=64        \
	-I$(top_srcdir)/include/utils \
	-I$(top_srcdir)/include       \
	$(AM_CPPFLAGS)

libavxffms2_la_CXXFLAGS = \
        $(ffms2_CFLAGS) \
        $(libav_CFLAGS) \
	$(AM_CXXFLAGS)  \
	-shared

libavxffms2_la_LDFLAGS = $(libav_BSYMBOLIC) $(AM_LDFLAGS) -module -shared -avoid-version -export-dynamic

# We don't want .la files for plugins
install-exec-hook:
	rm -f $(DESTDIR)$(pkglibdir)/libavxffms2.la

uninstall-local:
	rm -f $(DESTDIR)$(pkglibdir)/libavxffms2.so
