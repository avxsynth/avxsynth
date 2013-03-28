#                                       -*- Automake -*-
# Process this file with automake to produce a Makefile.

pkglib_LTLIBRARIES = libavxframecapture.la

libavxframecapture_la_SOURCES = \
	src/AvxFrameCapture.cpp     \
	src/FrameCaptureProfile.cpp \
	src/FrameRenderer.cpp

libavxframecapture_la_LIBADD = \
	$(top_builddir)/avxutils/libavxutils.la \
	$(jpeg_LIBS)

libavxframecapture_la_CPPFLAGS = \
	-I$(top_srcdir)/include       \
	$(jpeg_CFLAGS)                \
	$(AM_CPPFLAGS)

libavxframecapture_la_CXXFLAGS = $(AM_CXXFLAGS) -shared

libavxframecapture_la_LDFLAGS = $(AM_LDFLAGS) -module -shared -avoid-version -export-dynamic

# We don't want .la files for plugins
install-exec-hook:
	rm -f $(DESTDIR)$(pkglibdir)/libavxframecapture.la

uninstall-local:
	rm -f $(DESTDIR)$(pkglibdir)/libavxframecapture.so
