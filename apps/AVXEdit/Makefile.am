#                                       -*- Automake -*-
# Process this file with automake to produce a Makefile.

# Extra silent rules for Qt build tools
rcc_verbose = $(rcc_verbose_@AM_V@)
rcc_verbose_ = $(rcc_verbose_@AM_DEFAULT_V@)
rcc_verbose_0 = @echo RCC $@;
moc_verbose = $(moc_verbose_@AM_V@)
moc_verbose_ = $(moc_verbose_@AM_DEFAULT_V@)
moc_verbose_0 = @echo MOC $@;

bin_PROGRAMS = AVXEdit

AVXEdit_SOURCES = \
	src/main.cpp       \
	src/mainwindow.cpp \
	src/mainwindow.h   \
	application.qrc    \
	images/copy.png    \
	images/cut.png     \
	images/mplayer.png \
	images/new.png     \
	images/open.png    \
	images/paste.png   \
	images/save.png

nodist_AVXEdit_SOURCES = \
	src/moc_mainwindow.cpp \
	src/qrc_application.cpp

BUILT_SOURCES = \
	src/moc_mainwindow.cpp \
	src/qrc_application.cpp

CLEANFILES = \
	src/moc_mainwindow.cpp \
	src/qrc_application.cpp

AVXEdit_LDADD = \
	$(top_builddir)/apps/avxframeserver/libavxframeserver.la \
	$(top_builddir)/avxutils/libavxutils.la                  \
	$(top_builddir)/avxsynth/libavxsynth.la                  \
	$(Qt_LIBS)

AVXEdit_CPPFLAGS = \
	-I$(top_srcdir)/apps/avxframeserver/frameserverlib/src \
	-I$(top_srcdir)/include \
	$(AM_CPPFLAGS)

AVXEdit_CXXFLAGS = \
	$(Qt_CFLAGS) \
	$(AM_CXXFLAGS)

# Custom rules to handle moc and rcc
src/qrc_application.cpp: application.qrc \
		images/copy.png images/cut.png images/mplayer.png images/new.png \
		images/open.png images/paste.png images/save.png
	$(rcc_verbose)$(RCC) -name application $(srcdir)/application.qrc -o src/qrc_application.cpp

src/moc_mainwindow.cpp: $(srcdir)/src/mainwindow.h
	$(moc_verbose)$(MOC) $(AVXEdit_CPPFLAGS) $(CPPFLAGS) $< -o $@
