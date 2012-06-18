#                                       -*- Automake -*-
# Process this file with automake to produce a Makefile.

pkglib_LTLIBRARIES = libautocrop.la

libautocrop_la_SOURCES = src/autocrop.cpp

libautocrop_la_CPPFLAGS = -I$(top_srcdir)/include $(AM_CPPFLAGS)

libautocrop_la_CXXFLAGS = $(AM_CXXFLAGS) -shared

libautocrop_la_LDFLAGS = $(AM_LDFLAGS) -module -shared -avoid-version -export-dynamic

dist_doc_DATA = AutoCrop.txt

# We don't want .la files for plugins
install-exec-hook:
	rm -f $(DESTDIR)$(pkglibdir)/libautocrop.la

uninstall-local:
	rm -f $(DESTDIR)$(pkglibdir)/libautocrop.so
