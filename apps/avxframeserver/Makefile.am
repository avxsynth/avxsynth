#                                       -*- Automake -*-
# Process this file with automake to produce a Makefile.

noinst_LTLIBRARIES = libavxframeserver.la

noinst_HEADERS = \
	frameserverlib/src/frameserverlib.h

bin_PROGRAMS = avxFrameServer

libavxframeserver_la_SOURCES = \
	frameserverlib/src/avxSynthAppInterface.cpp

libavxframeserver_la_CPPFLAGS = \
	-I$(srcdir)/frameserverlib/src          \
	-I$(top_srcdir)/avxsynth/common/include \
	-I$(top_srcdir)/include                 \
	$(AM_CPPFLAGS)

avxFrameServer_SOURCES = frameserverapp/src/main.cpp

# Hack: Add libavxsynth so libtool includes it in the temporary RPATH.
avxFrameServer_LDADD = \
	libavxframeserver.la                    \
	$(top_builddir)/avxutils/libavxutils.la \
	$(top_builddir)/avxsynth/libavxsynth.la
	
avxFrameServer_CPPFLAGS = \
	-I$(srcdir)/frameserverlib/src \
	-I$(top_srcdir)/include        \
	$(AM_CPPFLAGS)
