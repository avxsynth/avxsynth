#                                       -*- Automake -*-
# Process this file with automake to produce a Makefile.

lib_LTLIBRARIES = libavxutils.la

libavxutils_la_SOURCES = \
	src/avxlog.cpp        \
	src/AvxTextRender.cpp \
	src/Path.cpp

libavxutils_la_LIBADD = $(log4cpp_LIBS) $(pangocairo_LIBS)

libavxutils_la_CPPFLAGS = \
	-I$(top_srcdir)/include/utils \
	-I$(top_srcdir)/include       \
        $(AM_CPPFLAGS)

libavxutils_la_CXXFLAGS = \
	$(log4cpp_CFLAGS)             \
	$(pangocairo_CFLAGS)          \
	$(AM_CXXFLAGS)

libavxutils_la_LDFLAGS = -version-info 0:0:0 $(AM_LDFLAGS)

# For some reason libtool refuses to run ldconfig
install-exec-hook:
	-ldconfig
