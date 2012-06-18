#                                       -*- Automake -*-
# Process this file with automake to produce a Makefile.

pkglib_LTLIBRARIES = libavxsubtitle.la

libavxsubtitle_la_SOURCES = \
	src/AvxSubtitle.cpp    \
	src/CaptionEntry.cpp   \
	src/CaptionList.cpp    \
	src/SrtParser.cpp      \
	src/SubtitleParser.cpp \
	src/Utils.cpp

libavxsubtitle_la_LIBADD = $(top_builddir)/avxutils/libavxutils.la

libavxsubtitle_la_CPPFLAGS = \
	-I$(top_srcdir)/include/utils \
	-I$(top_srcdir)/include       \
	$(AM_CPPFLAGS)

libavxsubtitle_la_CXXFLAGS = $(AM_CXXFLAGS) -shared

libavxsubtitle_la_LDFLAGS = $(AM_LDFLAGS) -module -shared -avoid-version -export-dynamic

# We don't want .la files for plugins.
install-exec-hook:
	rm -f $(DESTDIR)$(pkglibdir)/libavxsubtitle.la

uninstall-local:
	rm -f $(DESTDIR)$(pkglibdir)/libavxsubtitle.so
